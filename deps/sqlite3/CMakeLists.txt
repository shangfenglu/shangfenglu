cmake_minimum_required(VERSION 3.18)

#包含ExternProject模块，用于管理外部项目
include(ExternalProject)
include(GNUInstallDirs)

#install command
ExternalProject_Add(deps-sqlite3
	URL ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3440000.tar.gz
	DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
	SOURCE_DIR ${CMAKE_BINARY_DIR}/sqlite3
	BINARY_DIR ${CMAKE_BINARY_DIR}/sqlite3
	CONFIGURE_COMMAND ./configure ${buildhost} --enable-static
	BUILD_COMMAND make -j
	INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR} install )

#build the static library
add_library(sqlite3_a INTERFACE)
target_include_directories(sqlite3_a INTERFACE ${CMAKE_BINARY_DIR}/sqlite3)
target_link_libraries(sqlite3_a INTERFACE ${CMAKE_BINARY_DIR}${CMAKE_INSTALL_PREFIX}/lib/libsqlite3.a)

add_dependencies(sqlite3_a deps-sqlite3)