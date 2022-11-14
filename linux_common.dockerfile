#
# Docker image to build Softeq linux-common-library
#
FROM cmake_builder:ubuntu20

RUN apt-get -qq update && \
    apt-get -q -y upgrade

# install dependendant packages
RUN apt-get install -y \
    libcurl4-openssl-dev \
    libxml2-dev \
    libmicrohttpd-dev \
    nlohmann-json3-dev \
    libsqlite3-dev \
    uuid-dev \
    libsystemd-dev \
    libmagic-dev

RUN apt-get clean && rm -rf /var/lib/apt/lists/*
