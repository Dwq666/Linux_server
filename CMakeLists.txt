# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)
project ("server"  VERSION 0.1)


#获取所有源码
FILE(GLOB_RECURSE SRCCPP "*.cpp" "*.c" "*.h")
# 添加库源文件
SET(LIB_SOURCE ${SRCCPP} )
message("Build Type ====>${CMAKE_BUILD_TYPE}" )

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/win32/bin)
#分别设置输出路径，避免自动增加debug 或者 release 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${PROJECT_SOURCE_DIR}/bin)  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)  


message("Default Resource File ${RESOURCEFILE}") 


# 设置exe     windos后面加win32
add_executable(server  ${LIB_SOURCE})  #WIN32

# 设置添加静态库路径
set(LIBDIR "${CMAKE_SOURCE_DIR}/lib")
target_link_directories(server PUBLIC ${LIBDIR})

message("server LIBDIR ====>${LIBDIR}") 


# 往静态库取需要用到的文件
set(LIBS
    nanomsg 
    anl
    rt
    pthread
    nsl
    c
) 
# 添加文件
target_link_libraries(server ${LIBS})

add_custom_command(TARGET server 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo compile Finish ${PROJECT_SOURCE_DIR}/win32/bin/server.exe
                    VERBATIM
                   ) 






