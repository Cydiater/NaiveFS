cmake_minimum_required(VERSION 3.20)
project(nfs)

find_package(FUSE REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE NFS_SOURCES src/*.cpp src/*.hpp)

add_executable(nfs ${NFS_SOURCES})
target_link_libraries(nfs PRIVATE ${FUSE_LIBRARIES})
target_include_directories(nfs PRIVATE ${FUSE_INCLUDE_DIRS})
