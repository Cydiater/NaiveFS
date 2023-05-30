cmake_minimum_required(VERSION 3.20)
project(nfs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE NFS_SOURCES src/*.cpp src/*.hpp)

add_executable(nfs ${NFS_SOURCES})
target_include_directories(nfs PUBLIC src)
target_link_libraries(nfs fuse3)
target_compile_options(nfs PRIVATE
    -Wall
    -Wextra
)
target_compile_definitions(nfs PRIVATE
    -DFUSE_USE_VERSION=31
)
