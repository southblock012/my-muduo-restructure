cmake_minimum_required(VERSION 3.10)
project(my_muduo)

#mymuduo最终编译成so动态库，设置动态库路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息, 并指定c++11标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

#添加头文件路径
include_directories(./include)
#定义参与编译的源文件列表
aux_source_directory(./src SRC_LIST)
#编译动态库
add_library(my_muduo SHARED ${SRC_LIST})
