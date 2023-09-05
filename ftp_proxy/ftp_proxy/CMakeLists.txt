# CMakeList.txt: ftp_proxy 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
#指定项目名称                                                                                                                                                    
project(ftp_proxy)

#生成可gdb                                                                                                                                                         
add_compile_options(-g )

#指定源文件
aux_source_directory(source SOURCES)

#指定头文件
include_directories(include)

#添加源文件                                                                                                                                    
add_executable(${PROJECT_NAME} ${SOURCES})

#添加链接文件                                                                                                                                  
target_link_libraries(${PROJECT_NAME} pthread )

# TODO: 如有需要，请添加测试并安装目标。
