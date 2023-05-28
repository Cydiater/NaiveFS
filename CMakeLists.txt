cmake_minimum_required(VERSION 3.20)
project(nfs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE NFS_SOURCES src/*.cpp src/*.hpp)

add_executable(nfs ${NFS_SOURCES})
target_link_libraries(nfs fuse3)
