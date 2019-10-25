FROM ubuntu:bionic

RUN apt-get update && \
	apt-get install -y build-essential git cmake autoconf libtool pkg-config libcpprest-dev redis-server libpthread-stubs0-dev

RUN git clone https://github.com/Cylix/cpp_redis.git
RUN cd /cpp_redis && git submodule init && git submodule update
RUN cd /cpp_redis && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
RUN cd /cpp_redis/build && make
RUN cd /cpp_redis/build && make install

