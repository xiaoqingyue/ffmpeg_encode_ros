#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>

#include "ffmpeg_encode_sdp/encoder.h"

using namespace std;
using namespace cv;

encoder *ed = new encoder();  

void imageCallback(const sensor_msgs::ImageConstPtr& msg)
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
        std::cout<<1255<<std::endl; 

       ed->encode(cv_ptr->image);

        
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "ffmpeg_encode_image_sdp");
  ros::NodeHandle nh_; 
  ros::Subscriber image_sub_ = nh_.subscribe("/pylon_camera/image_raw", 1, imageCallback);///middle/pylon_camera_node/image_raw/compressed
  ros::spin();
  return 0;
}
