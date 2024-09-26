cmake_minimum_required(VERSION 3.18)

#包含ExternProject模块，用于管理外部项目
include(ExternalProject)
include(GNUInstallDirs)

#install command
ExternalProject_Add(deps-openssl
	URL ${CMAKE_CURRENT_SOURCE_DIR}/openssl-1.1.1.tar.gz
	DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
	SOURCE_DIR ${CMAKE_BINARY_DIR}/openssl
	BINARY_DIR ${CMAKE_BINARY_DIR}/openssl
	CONFIGURE_COMMAND ./Configure ${buildhost} no-shared linux-x86_64
	BUILD_COMMAND make -j
	INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install )

#build the static library
add_library(ssl_a INTERFACE)
target_include_directories(ssl_a INTERFACE ${CMAKE_BINARY_DIR}/openssl)
target_link_libraries(ssl_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib64/libssl.a )
target_link_libraries(ssl_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib64/libcrypto.a)

add_dependencies(ssl_a deps-openssl)