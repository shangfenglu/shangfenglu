add_subdirectory(libuv)
add_subdirectory(openssl)
add_subdirectory(nasm)
add_subdirectory(x264)
add_subdirectory(libfdk-aac)
add_subdirectory(ffmpeg)
add_subdirectory(sqlite3)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "mips64")
else()
    if(WITH_ENGOCR)
        add_subdirectory(opencv)
    endif()
endif()