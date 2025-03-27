FROM docker.io/debian:12

RUN apt-get update
RUN apt-get install -y \
  make \
  gcc-arm-none-eabi \ 
  libnewlib-arm-none-eabi \
  libstdc++-arm-none-eabi-dev \
  kconfig-frontends \
  git \
  g++
WORKDIR /repo
