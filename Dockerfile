FROM alpine:latest
RUN mkdir /app
COPY . /app
RUN apk update && \
    apk add build-base cmake gdb valgrind cmocka-dev openssl-dev
RUN mkdir /app/build
WORKDIR /app/build
RUN cmake .. && \
    cmake --build .
WORKDIR /app
CMD ["/app/build/miner"]
