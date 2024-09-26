cmake_minimum_required(VERSION 3.18)

#包含ExternalProject模块，用于管理外部项目
include(ExternalProject)


ExternalProject_Add(deps-ffmpeg
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-4.2.2.tar.bz2
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/ffmpeg
    BINARY_DIR ${CMAKE_BINARY_DIR}/ffmpeg
    CONFIGURE_COMMAND ./configure ${buildhost}
        --enable-static
        --disable-libxcb
        --disable-libxcb-shm
        --disable-libxcb-xfixes
        --disable-libxcb-shape
        --disable-vdpau
        --disable-xlib
        --disable-vaapi
        --enable-gpl
        --enable-version3
        --enable-pthreads
        --disable-libmp3lame
        --enable-libx264
        --enable-pic
        --disable-libfdk-aac
        --disable-openssl
        --enable-nonfree
    BUILD_COMMAND make -j
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install)

#build the ffmpeg目标
add_library(ffmpeg_a INTERFACE)

# 添加 OpenSSL 头文件路径
target_include_directories(ffmpeg_a INTERFACE /root/local/openssl/include)

find_package(X11 REQUIRED)

# 添加 OpenSSL 的库文件路径到 ffmpeg_a
include_directories(${X11_INCLUDE_DIRS})
target_link_libraries(ffmpeg_a INTERFACE ${X11_LIBRARIES})


# 调整链接顺序
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libavformat.a)
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libavcodec.a)
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libswscale.a)
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libswresample.a)
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}/x264/libx264.a)
target_link_libraries(ffmpeg_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libavutil.a)

add_dependencies(ffmpeg_a deps-ffmpeg)