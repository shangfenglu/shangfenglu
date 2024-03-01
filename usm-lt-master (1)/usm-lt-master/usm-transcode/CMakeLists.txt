# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.18)

set(CMAKE_VERBOSE_MAKEFILE ON)

# 定义项目名称
project(transcode)

set (LIB_OBJS
    ../lib/algjpeg.c
    ../lib/base64.c
    ../lib/common.c
    ../lib/debug_msg.c
    ../lib/algpng.c
    ../lib/cJSON.c
    ../lib/ini.c
)

set (OBJS
    bitmap.c
    encoder.c
    decoder.c
    sqlite.c
    xcfg.c
)


# 创建可执行文件，并将源代码文件链接到可执行文件
add_executable(${PROJECT_NAME} ${LIB_OBJS} ${OBJS} transcoder.c)

# 链接所需的库到可执行文件
target_link_libraries(${PROJECT_NAME}  pthread)
target_link_libraries(${PROJECT_NAME}  dl)
target_link_libraries(${PROJECT_NAME}  m)
target_link_libraries(${PROJECT_NAME}  z)
target_link_libraries(${PROJECT_NAME} jpeg)
target_link_libraries(${PROJECT_NAME} png)
target_link_libraries(${PROJECT_NAME} ssl_a)
target_link_libraries(${PROJECT_NAME} uv_a)
target_link_libraries(${PROJECT_NAME} sqlite3_a)
target_link_libraries(${PROJECT_NAME} ffmpeg_a)




configure_file(${CMAKE_SOURCE_DIR}/conf/cfg.ini cfg.ini COPYONLY)
# 安装可执行文件
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    
install(FILES cfg.ini DESTINATION bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -fPIC -O2")


