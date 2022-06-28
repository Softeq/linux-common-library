
# How to build softeq-lab library on ubuntu 20.04 LTS

## Install prerequisites

1.1 Install dependencies with apt
```
sudo apt install cmake libcurl4-openssl-dev libxml2-dev libmicrohttpd-dev \
    libsqlite3-dev libgstreamer-plugins-bad1.0-dev uuid-dev libsystemd-dev
```
1.2 Install nlohmann json library
```
sudo apt-get update
sudo apt-get install nlohmann-json3-dev

Note: minimal required version is 3.7.3
Note: nlohmann-json library can be built from sources (https://github.com/nlohmann/json.git) as alternative of downloading package.
```
1.3 Install google unit tests library (optional)
```
Note: for Ubuntu 20.04 and newer it's also possible to install from official repositories: sudo apt install libgtest-dev

sudo apt install checkinstall

wget https://github.com/google/googletest/archive/release-1.10.0.tar.gz
tar xf release-1.10.0.tar.gz
cd googletest-release-1.10.0
mkdir build && cd build
cmake ..
make
sudo checkinstall --install=no
sudo dpkg -i <package_name>.deb

```

## Building

2.1 Building on the host
```
git clone ssh://git@stash.softeq.com:7999/emblab/linux-common-library.git
cd linux-common-library/
mkdir build && cd build
cmake ..
make
```
2.2 Building in docker

- Clone and build cmake-docker.io project (by README.md inside);
- if you haven't cloned linux-common-library project yet, run `git clone ssh://git@stash.softeq.com:7999/emblab/linux-common-library.git`;
- `cd linux-common-library`;
- `make docker`;
- run `make help` to see the list of possible actions;
- `make <target> <options>`.
