cmake_minimum_required(VERSION 3.18)

# 包含ExternalProject模块，用于管理外部项目
include(ExternalProject)
include(GNUInstallDirs)
ExternalProject_Add(deps-libuv
    PREFIX libuv-1.44.2 # 外部项目的安装目录前缀
    URL ${CMAKE_CURRENT_SOURCE_DIR}/libuv-1.44.2.tar.gz # 外部项目的源代码位置
    URL_HASH SHA1=0ea9726936755088e9c01d30caacfca1a384f3bb # 源代码文件的哈希值
    BUILD_BYPRODUCTS
        ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}uv_a${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}uv${CMAKE_SHARED_LIBRARY_SUFFIX}
    CMAKE_ARGS 
        "-DBUILD_TESTING=OFF" # 额外的CMake参数
        "-DBUILD_SHARED_LIBS=OFF" # 禁用构建共享库
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install) # 安装命令

# 设置静态库前缀（仅在Windows平台有效）
if(WIN32)
	set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

# 添加名为uv_a的接口库
add_library(uv_a INTERFACE)
target_include_directories(uv_a INTERFACE ${${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}}/include)
target_link_libraries(uv_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}uv_a${CMAKE_STATIC_LIBRARY_SUFFIX})

# 根据平台添加额外的链接库
if(WIN32)
	target_link_libraries(uv_a INTERFACE "psapi;user32;advapi32;iphlpapi;userenv;ws2_32;dbghelp;ole32;uuid;shell32")
elseif(APPLE)
	target_link_libraries(uv_a INTERFACE "pthread;dl")
else()
	target_link_libraries(uv_a INTERFACE "pthread;dl;rt")
endif()

# 添加uv_a的依赖项为deps-libuv
add_dependencies(uv_a deps-libuv)

# 添加名为uv的接口库
add_library(uv INTERFACE)
target_include_directories(uv INTERFACE ${${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}}/include)
target_compile_definitions(uv INTERFACE "USING_UV_SHARED=1")
target_link_libraries(uv INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}uv${CMAKE_SHARED_LIBRARY_SUFFIX})

# 根据平台添加额外的链接库
if(WIN32)
	target_link_libraries(uv INTERFACE "psapi;user32;advapi32;iphlpapi;userenv;ws2_32;dbghelp;ole32;uuid;shell32")
elseif(APPLE)
	target_link_libraries(uv INTERFACE "pthread;dl")
else()
	target_link_libraries(uv INTERFACE "pthread;dl;rt")
endif()

# 添加uv的依赖项为deps-libuv
add_dependencies(uv deps-libuv)

#这段脚本使用了 CMake 的 ExternalProject 模块来管理外部项目 libuv。它下载了 libuv 的源代码，编译并安装到指定的目录中。然后定义了两个接口库 uv_a 和 uv，它们分别用于静态链接和动态链接 libuv 库。根据不同的平台，添加了对应的链接库依赖。最后，通过 #add_dependencies 添加了 deps-libuv 作为这两个接口库的依赖项。
#
#这段脚本提供了一个方便的方法来管理和使用 libuv 库，您可以根据自己的需求进行进一步的调整和使用。