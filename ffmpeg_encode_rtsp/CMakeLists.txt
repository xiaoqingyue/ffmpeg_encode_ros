cmake_minimum_required(VERSION 2.8.3)
project(ffmpeg_encode_rtsp)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS roscpp sensor_msgs std_msgs cv_bridge image_transport)

set(OpenCV_LIBRARY_DIRS /usr/local/lib)
set(OpenCV_INCLUDE_DIRS /usr/local/include/opencv2)

set(ffmpeg_libs_DIR /usr/local/lib)
set(ffmpeg_include_DIR /usr/local/include)



catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES ffmpeg_encode_rtsp
#  CATKIN_DEPENDS other_catkin_pkg
#  DEPENDS system_lib
)

###########
## Build ##
###########

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include ${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${ffmpeg_include_DIR} 
# include
# ${catkin_INCLUDE_DIRS}
)

add_executable(ffmpeg_encode_rtsp_node
  include/ffmpeg_encode_rtsp/encoder.h
  src/encoder.cpp
  include/ffmpeg_encode_rtsp/rtpenc.h
  src/rtpenc.cpp
  include/ffmpeg_encode_rtsp/network.h
  src/network.cpp
  include/ffmpeg_encode_rtsp/rtsp.h
  src/rtsp.cpp
  include/ffmpeg_encode_rtsp/avc.h
  src/avc.cpp
  include/ffmpeg_encode_rtsp/rtp.h
  src/rtp.cpp
  src/ffmpeg_encode_rtsp_node.cpp
)
add_dependencies(ffmpeg_encode_rtsp_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ffmpeg_encode_rtsp_node
  ${catkin_LIBRARIES} opencv_highgui opencv_core opencv_imgproc opencv_imgcodecs avcodec avformat avutil swresample swscale 
)