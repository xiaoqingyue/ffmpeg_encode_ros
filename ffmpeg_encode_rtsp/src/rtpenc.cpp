#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "ffmpeg_encode_rtsp/rtpenc.h"
#include "ffmpeg_encode_rtsp/rtp.h"
#include "ffmpeg_encode_rtsp/avc.h"
#include "ffmpeg_encode_rtsp/rtsp.h"
#include "ffmpeg_encode_rtsp/network.h"
#include "ffmpeg_encode_rtsp/encoder.h"

#define SERVER_PORT      8554
#define SERVER_RTP_PORT  55532
#define BUF_MAX_SIZE     (1024*1024)


encoder *ed = new encoder();

int rtpSendH264Frame(int socket, const char* ip, int16_t port,
                            struct RtpPacket* rtpPacket, uint8_t* frame, uint32_t frameSize)
{
    int naluType; // nalu第一个字节
    int sendBytes = 0;
    int ret;

    naluType = (frame[0]>>1) & 0x3F;

    if (frameSize <= RTP_MAX_PKT_SIZE) // nalu长度小于最大包场：单一NALU单元模式
    {


        /*
         *  0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |F|    Type   |  LayerId  |TID|a single NAL unit ...
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */

        memcpy(rtpPacket->payload, frame, frameSize);
        ret = rtpSendPacket(socket, ip, port, rtpPacket, frameSize);
        if(ret < 0)
            return -1;

        rtpPacket->rtpHeader.seq++;
        sendBytes += ret;

        if (((naluType & 0x7F)>>1) == 32 || ((naluType & 0x7F)>>1) == 33) // 如果是SPS、PPS就不需要加时间戳
            goto out;
    }
    else // nalu长度小于最大包场：分片模式
    {   
        int pktNum = frameSize / RTP_MAX_PKT_SIZE;       // 有几个完整的包
        int remainPktSize = frameSize % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
        int i, pos = 2;

        /* 发送完整的包 */
        for (i = 0; i < pktNum; i++)
        {
            rtpPacket->payload[0] = 49<<1;
            rtpPacket->payload[1] = 1;
            rtpPacket->payload[2] = naluType;

            //rtpPacket->payload[2] |= 1<<7;

            if (i == 0) //第一包数据
                rtpPacket->payload[2] |= 1<<7;// start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                rtpPacket->payload[2] &= ~(1 << 7); // end

            memcpy(rtpPacket->payload+3, frame+pos, RTP_MAX_PKT_SIZE);
            ret = rtpSendPacket(socket, ip, port, rtpPacket, RTP_MAX_PKT_SIZE+3);
            if(ret < 0)
                return -1;

            rtpPacket->rtpHeader.seq++;
            sendBytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }

        /* 发送剩余的数据 */
        if (remainPktSize > 0)
        {
            rtpPacket->payload[0] = 49<<1;
            rtpPacket->payload[1] = 1;
            rtpPacket->payload[2] = naluType;
            rtpPacket->payload[2] |= 1<<6; //end

            memcpy(rtpPacket->payload+3, frame+pos, remainPktSize+3);
            ret = rtpSendPacket(socket, ip, port, rtpPacket, remainPktSize+3);
            if(ret < 0)
                return -1;

            rtpPacket->rtpHeader.seq++;
            sendBytes += ret;
        }
    }

out:

    return sendBytes;
}

void doClient(int clientSockfd, const char* clientIP, int serverRtpSockfd)
{
    char method[40];
    char url[100];
    char version[40];
    int cseq;
    int clientRtpPort, clientRtcpPort;
    char *bufPtr;
    char* rBuf = (char*)malloc(BUF_MAX_SIZE);
    char* sBuf = (char*)malloc(BUF_MAX_SIZE);
    char line[400];

    while(1)
    {
        int recvLen;
        // 接受客户端数据
        recvLen = recv(clientSockfd, rBuf, BUF_MAX_SIZE, 0);
        if(recvLen <= 0)
            goto out;

        rBuf[recvLen] = '\0';
        printf("---------------C->S--------------\n");
        printf("%s", rBuf);

        /* 解析方法 */
        bufPtr = getLineFromBuf(rBuf, line); //从buf中读取一行
        if(sscanf(line, "%s %s %s\r\n", method, url, version) != 3)
        {
            printf("parse err\n");
            goto out;
        }

        /* 如果是SETUP，那么就再解析client_port */
        if(!strcmp(method, "SETUP"))
        {
                bufPtr = getLineFromBuf(bufPtr, line);
                if(!strncmp(line, "Transport:", strlen("Transport:")))
                {
                    sscanf(line, "Transport: RTP/AVP/UDP;unicast;client_port=%d\r\n", &clientRtpPort);
                    printf("clientport:%d\n", clientRtpPort);

                }
       }

        if(!strcmp(method, "OPTIONS"))
        {
            if(handleCmd_OPTIONS(sBuf, cseq))
            {
                printf("failed to handle options\n");
                goto out;
            }
        }
        else if(!strcmp(method, "DESCRIBE"))
        {
            if(handleCmd_DESCRIBE(sBuf, cseq, url))
            {
                printf("failed to handle describe\n");
                goto out;
            }
        }
        else if(!strcmp(method, "SETUP"))
        {
            if(handleCmd_SETUP(sBuf, cseq, clientRtpPort))
            {
                printf("failed to handle setup\n");
                goto out;
            }
        }
        else if(!strcmp(method, "PLAY"))
        {
            if(handleCmd_PLAY(sBuf, cseq))
            {
                printf("failed to handle play\n");
                goto out;
            }
        }
        else
        {
            goto out;
        }

        printf("---------------S->C--------------\n");
        printf("%s", sBuf);
        send(clientSockfd, sBuf, strlen(sBuf), 0);

        /* 开始播放，发送RTP包 */
        if(!strcmp(method, "PLAY"))
         {       

            struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(200000000);
            rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H265, 0,
                            0, 0, 0x88923423);

            ros::NodeHandle nh_; 
            ros::Subscriber image_sub_ = nh_.subscribe<sensor_msgs::Image>("/pylon_camera/image_raw", 10, 
                                                                boost::bind(&imageCallback, _1, serverRtpSockfd,clientIP,clientRtpPort,rtpPacket));   //回调函数的多参数应用
            ros::spin();         
            free(rtpPacket);
            goto out;
        }
    }
out:
    printf("finish\n");
    close(clientSockfd);
    free(rBuf);
    free(sBuf);
}

void imageCallback(const sensor_msgs::ImageConstPtr& msg, int &serverSockfd,const char* &clientip, int &clientport,struct RtpPacket* &rtpPacket)
{   

      cv_bridge::CvImagePtr cv_ptr;

      try   //对错误异常进行捕获，检查数据的有效性
        { 
         cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
         }        
        
      catch(cv_bridge::Exception& e)  //异常处理
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }       
        cv::Mat image;
        cv::resize(cv_ptr->image, image, cv::Size(960, 600)) ;

        ed->encode(image,serverSockfd,clientip,clientport,rtpPacket);
}