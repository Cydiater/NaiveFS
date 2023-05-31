FROM ubuntu:22.04

RUN apt update -y 
RUN apt install -y libfuse3-dev cmake g++

COPY . /root/nfs
WORKDIR /root/nfs
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    make -j4
