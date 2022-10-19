#ifndef RTPENC_H
#define RTPENC_H

#include <ros/ros.h>
#include<iostream>
#include<iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "ffmpeg_encode_rtsp/rtpenc.h"
#include "ffmpeg_encode_rtsp/rtp.h"
#include "ffmpeg_encode_rtsp/avc.h"
#include "ffmpeg_encode_rtsp/rtsp.h"
#include "ffmpeg_encode_rtsp/network.h"
#include "ffmpeg_encode_rtsp/encoder.h"

#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

void doClient(int clientSockfd, const char* clientIP, int serverRtpSockfd);

int rtpSendH264Frame(int socket, const char* ip, int16_t port,
                            struct RtpPacket* rtpPacket, uint8_t* frame, uint32_t frameSize);

void imageCallback(const sensor_msgs::ImageConstPtr& msg, int &serverSockfd,const char* &clientip, int &clientport,struct RtpPacket* &rtpPacket);


#endif // RTPENC_H
