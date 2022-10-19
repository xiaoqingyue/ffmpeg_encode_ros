#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#include<ros/ros.h> //ros标准库头文件
#include<iostream> //C++标准输入输出库
#include<cv_bridge/cv_bridge.h> 
#include<sensor_msgs/image_encodings.h> 
#include<image_transport/image_transport.h> 
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}
#include "ffmpeg_encode_sdp/rtpenc.h"
#include "ffmpeg_encode_sdp/network.h"
#include "ffmpeg_encode_sdp/utils.h"

class encoder
{
private:
    AVFormatContext* pFormatCtx;
    //AVOutputFormat* fmt;
    AVStream* video_st;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;
    AVPacket pkt;
    uint8_t* picture_buf;
    AVFrame* pFrame;

    int picture_size;
    int y_size;
    int framecnt;
    int w;
    int h;
    int bufLen;    

    RTPMuxContext ctx;
    UDPContext udp;


public:
     encoder();// initencode
     ~encoder();
    void encode(cv::Mat img);  

};

#endif // ENCODER_H