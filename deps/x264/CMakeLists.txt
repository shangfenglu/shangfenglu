cmake_minimum_required(VERSION 3.18)

#包含ExternalProject模块，用于管理外部项目
include(ExternalProject)


ExternalProject_Add(deps-x264
    URL ${CMAKE_CURRENT_SOURCE_DIR}/x264.tar.gz
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/x264
    BINARY_DIR ${CMAKE_BINARY_DIR}/x264
    CONFIGURE_COMMAND ./configure ${buildhost} --enable-static 
    BUILD_COMMAND make -j
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install) # 安装命令

#构建x264静态库
add_library(x264_a INTERFACE)
target_link_libraries(x264_a INTERFACE ${CMAKE_BINARY_DIR}/x264/libx264.a)

add_dependencies(x264_a deps-x264)