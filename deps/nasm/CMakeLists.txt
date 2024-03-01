cmake_minimum_required(VERSION 3.18)

#包含ExternalProject模块，用于管理外部项目
include(ExternalProject)


ExternalProject_Add(deps-nasm
    URL ${CMAKE_CURRENT_SOURCE_DIR}/nasm-2.14.tar.gz
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/nasm
    BINARY_DIR ${CMAKE_BINARY_DIR}/nasm
    CONFIGURE_COMMAND ./configure ${buildhost} --enable-static
    BUILD_COMMAND make -j
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install) # 安装命令
