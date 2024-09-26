# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.18)

# 定义项目名称
project(usm-lt)
 
add_compile_options(-std=gnu11 -g)

# 设置可执行文件的输出目录为项目的根目录
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 添加预定义宏定义
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)

# 添加包含目录路径，用于头文件的搜索路径
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/usm-transcode/include)

#加载第三方库

include_directories(${CMAKE_BINARY_DIR}/usr/include)
include_directories(${CMAKE_BINARY_DIR}/usr/local/include)
link_directories(${CMAKE_BINARY_DIR}/usr/lib)
link_directories(${CMAKE_BINARY_DIR}/usr/local/lib)
link_directories(${CMAKE_BINARY_DIR}/usr/lib64)
link_directories(${CMAKE_BINARY_DIR}/usr/local/lib64)

add_subdirectory(deps)
add_subdirectory(usm-transcode)

include(CPack)

