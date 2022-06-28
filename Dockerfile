#
# Minimum Docker image to build Softeq linux-common-library
#
FROM softeq-dev-env:ubuntu20

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
    libsystemd-dev

# copy system dbus service mockup
COPY tests/env/net.Sdbus.Moduletest.conf /usr/share/dbus-1/system.d

# dir for sysv service .pid file
RUN mkdir -p -m=777 /var/run/user/1000
RUN apt-get clean && rm -rf /var/lib/apt/lists/*
