cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "Sepecify C++11 Standard")
find_package( OpenCV REQUIRED )

project( yolo_annotation )
add_executable( yolo_annotation yolo_annotation.cpp )
target_link_libraries( yolo_annotation ${OpenCV_LIBS} )
