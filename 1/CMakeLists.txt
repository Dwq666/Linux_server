# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)
project ("Test"  VERSION 0.1)


add_subdirectory ("${CMAKE_SOURCE_DIR}/nanomsg")
add_subdirectory ("${CMAKE_SOURCE_DIR}/Linux_server")



