FROM ubuntu:24.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -yy --no-install-recommends \
        ca-certificates gcc clang make git tree bc valgrind ripgrep openssh-client \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
