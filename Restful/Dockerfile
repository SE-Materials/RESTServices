FROM cpp-build-base:0.1.2

WORKDIR /workdir

COPY ./ ./

RUN mkdir build && cd build && cmake .. && make

CMD ["/workdir/build/cppservice"]