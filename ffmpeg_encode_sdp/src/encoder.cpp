#include "ffmpeg_encode_sdp/encoder.h"
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>

encoder::encoder()
{
    w=1920;
    h=1200;
    bufLen=w * h * 3/2;
    udp = {
        .dstIp = "127.0.0.1",   // destination ip
        .dstPort = 1234         // destination port
    };
    // create udp socket
    int res = udpInit(&udp);
    if (res){
        printf("udpInit error.\n");
        //return -1;
    }
    initRTPMuxContext(&ctx);
    av_register_all();//注册FFmpeg所有编解码器。
    //Method1.
    pFormatCtx = avformat_alloc_context(); //初始化输出码流的AVFormatContext。

    video_st = avformat_new_stream(pFormatCtx, 0);//创建输出码流的AVStream。
        // 设置 码率25 帧每秒(fps=25)
    //video_st->time_base.num = 1;
    //video_st->time_base.den = 25;

    if (video_st==NULL){
        return ;
    }
        //为输出文件设置编码的参数和格式
    //Param that must set
    pCodecCtx = video_st->codec; // 从媒体流中获取到编码结构体，一个 AVStream 对应一个  AVCodecContext
    pCodecCtx->codec_id =AV_CODEC_ID_HEVC;// 设置编码器的 id，例如 h265 的编码 id 就是 AV_CODEC_ID_H265
    //pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;//编码器视频编码的类型
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//设置像素格式为 yuv 格式
    pCodecCtx->width =w;  //设置视频的宽高
    pCodecCtx->height = h;
    pCodecCtx->bit_rate = 400000;   //采样的码率；采样码率越大，视频大小越大
    pCodecCtx->gop_size=250;  //每250帧插入1个I帧，I帧越少，视频越小

    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //H264
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    pCodecCtx->qmin = 10; //最大和最小量化系数
    pCodecCtx->qmax = 51;

    //Optional Param
    pCodecCtx->max_b_frames=3; // 设置 B 帧最大的数量，B帧为视频图片空间的前后预测帧， B 帧相对于 I、P 帧来说，压缩率比较大，采用多编码 B 帧提高清晰度

    // Set Option //设置编码速度
    AVDictionary *param = 0;
        //preset的参数调节编码速度和质量的平衡。
    //tune的参数值指定片子的类型，是和视觉优化的参数，
    //zerolatency: 零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
    //H.264
    if(pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
        //av_dict_set(&param, "profile", "main", 0);
    }
    //H.265
    if(pCodecCtx->codec_id == AV_CODEC_ID_H265){
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zero-latency", 0);
    }

    //Show some Information //输出格式的信息，例如时间，比特率，数据流，容器，元数据，辅助数据，编码，时间戳
    //av_dump_format(pFormatCtx, 0, out_file, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);//查找编码器
    if (!pCodec){
        printf("Can not find encoder! \n");
        return ;
    }
       // 打开编码器，并设置参数 param
    if (avcodec_open2(pCodecCtx, pCodec,&param) < 0){
        printf("Failed to open encoder! \n");
        return ;
    }

        //设置原始数据 AVFrame
    pFrame = av_frame_alloc();

    picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
   //    // 将 picture_size 转换成字节数据
    picture_buf = (uint8_t *)av_malloc(picture_size);
   //        // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
   avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    //Write File Header 写封装格式文件头
    //avformat_write_header(pFormatCtx,NULL);

        //创建编码后的数据 AVPacket 结构体来存储 AVFrame 编码后生成的数据  //编码前：AVFrame  //编码后：AVPacket
    av_new_packet(&pkt,picture_size);

        // 设置 yuv 数据中Y亮度图片的宽高，写入数据到 AVFrame 结构体中
    y_size = pCodecCtx->width * pCodecCtx->height;
    
    std::cout<<1235<<std::endl;

}

encoder::~encoder(){

        //Clean
        if (video_st){
                    // 关闭编码器
            avcodec_close(video_st->codec);
                    // 释放 AVFrame
            av_free(pFrame);

                    // 释放图片 buf
            av_free(picture_buf);
        }
        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);

}
void encoder::encode(cv::Mat srcImg)
{
       std::cout<<1234<<std::endl;

        cv::Mat yuvImg;
        cv::cvtColor(srcImg, yuvImg, cv::COLOR_BGR2YUV_I420);
        memcpy(picture_buf, yuvImg.data, bufLen*sizeof(unsigned char));

        pFrame->data[0] = picture_buf;              // Y
        pFrame->data[1] = picture_buf+ y_size;      // U
        pFrame->data[2] = picture_buf+ y_size*5/4;  // V

        std::cout<<1235<<std::endl;

        int i=0;
        //PTS //顺序显示解码后的视频帧
        pFrame->pts=i;
        //设置这一帧的显示时间

        //pFrame->pts=i*(video_st->time_base.den)/((video_st->time_base.num)*25);
        int got_picture=0;        

       

        //Encode //编码一帧视频。即将AVFrame（存储YUV像素数据）编码为AVPacket（存储H.264等格式的码流数据）
        int ret = avcodec_encode_video2(pCodecCtx, &pkt,pFrame, &got_picture);

        std::cout<<1236<<std::endl;

        if(ret < 0){
            printf("Failed to encode! \n");
            return ;
        }
        if (got_picture==1){
            printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
            framecnt++;
            //pkt.stream_index = video_st->index;
           // ret = av_write_frame(pFormatCtx, &pkt);//将编码后的视频码流写入文件（fwrite）

            rtpSendH265HEVC(&ctx, &udp, pkt.data, pkt.size);

            av_free_packet(&pkt);//释放内存
        }
}
