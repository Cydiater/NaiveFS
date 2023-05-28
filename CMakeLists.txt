cmake_minimum_required(VERSION 3.20)
project(nfs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE NFS_SOURCES src/*.cpp src/*.hpp)

add_executable(nfs ${NFS_SOURCES})
target_link_libraries(nfs fuse3)
target_compile_definitions(nfs PRIVATE
    -DFUSE_USE_VERSION=31
)
