# 1 How to build softeq-lab library on ubuntu 20.04 LTS

## 1 Install prerequisites
### 1.1.1 Install dependencies with apt
```
sudo apt install cmake libcurl4-openssl-dev libxml2-dev libmicrohttpd-dev \
    libsqlite3-dev libgstreamer-plugins-bad1.0-dev uuid-dev libsystemd-dev
```
### 1.1.2 Install nlohmann json library
```
sudo apt-get update
sudo apt-get install nlohmann-json3-dev

Note: minimal required version is 3.7.3
Note: nlohmann-json library can be built from sources (https://github.com/nlohmann/json.git) as alternative of downloading package.
```
### 1.1.3 Install google unit tests library (optional)
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
## 1.2 Building

- clone `linux-common-library`
- in `linux-common-library` directory perform:
```
mkdir build && cd build
cmake ..
make all
```

# 2 Build softeq-lab library with cmake-docker 

- Clone and build cmake-docker.io project (by README.md inside);
- in `linux-common-library` directory perform `cmake_build docker` for build docker image;
- to build the library perform `cmake_build dev`.

# 3 debug in VScode inside the container
    To use VScode inside the docker container you need to install `Remote-Containers` and `C/C++ Extension pack` extentions.
To enter the container press F1 and enter `Remote-Containers: Reopen in container` and select `from 'Dockerfile'` option. In Extensions->LOCAL-INSTALLED turn on `C/C++ Extension pack`. 
Now we can use Vscode editing and debugging tools inside the docker container.