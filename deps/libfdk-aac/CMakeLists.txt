cmake_minimum_required(VERSION 3.18)

#包含ExternalProject模块，用于管理外部项目
include(ExternalProject)


ExternalProject_Add(deps-libfdk-aac
    URL ${CMAKE_CURRENT_SOURCE_DIR}/fdk-aac-2.0.1.tar.gz
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/libfdk-aac
    BINARY_DIR ${CMAKE_BINARY_DIR}/libfdk-aac
    CONFIGURE_COMMAND ./configure ${buildhost} --enable-static
    BUILD_COMMAND make -j
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install) # 安装命令

