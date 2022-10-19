cmake_minimum_required(VERSION 2.8.3)
project(ffmpeg_encode_sdp)


find_package(catkin REQUIRED COMPONENTS roscpp sensor_msgs std_msgs cv_bridge image_transport)
#find_package(OpenCV REQUIRED)


set(OpenCV_LIBRARY_DIRS /usr/local/lib)
set(OpenCV_INCLUDE_DIRS /usr/local/include/opencv2)

set(ffmpeg_libs_DIR /usr/local/lib)
set(ffmpeg_include_DIR /usr/local/include)

# # # #指定所添加依赖库的导入路径
# add_library( avcodec SHARED IMPORTED )
# set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavcodec.so ) 
# add_library( avfilter SHARED IMPORTED )
# set_target_properties( avfilter PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavfilter.so ) 
# add_library( swresample SHARED IMPORTED )
# set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswresample.so ) 
# add_library( swscale SHARED IMPORTED )
# set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswscale.so ) 
# add_library( avformat SHARED IMPORTED )
# set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavformat.so ) 
# add_library( avutil SHARED IMPORTED )
# set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavutil.so )
# add_library(  avdevice SHARED IMPORTED )
# set_target_properties( avdevice PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavdevice.so )
# add_library( postproc SHARED IMPORTED )
# set_target_properties( postproc PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libpostproc.so )


catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES ffmpeg_encode_sdp
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

link_directories(${OpenCV_LIBRARY_DIRS} ${ffmpeg_libs_DIR} )

add_executable(ffmpeg_encode_sdp_node
  include/ffmpeg_encode_sdp/utils.h
  src/utils.cpp
  include/ffmpeg_encode_sdp/rtpenc.h
  src/rtpenc.cpp
  include/ffmpeg_encode_sdp/network.h
  src/network.cpp
  include/ffmpeg_encode_sdp/avc.h
  src/avc.cpp
  include/ffmpeg_encode_sdp/encoder.h
  src/encoder.cpp
  src/ffmpeg_encode_sdp_node.cpp
)
add_dependencies(ffmpeg_encode_sdp_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ffmpeg_encode_sdp_node
  ${catkin_LIBRARIES} opencv_highgui opencv_core opencv_imgproc opencv_imgcodecs avcodec avformat avutil swresample swscale 
)

