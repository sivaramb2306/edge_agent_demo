#!/bin/bash

# Exit on any error
set -e

echo "Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y \
    g++ \
    make \
    cmake \
    meson \
    ninja-build \
    pkg-config \
    nlohmann-json3-dev \
    libsnmp-dev \
    libcurl4-openssl-dev \
    libssl-dev

echo "Building and installing Pistache..."
if [ -d "pistache" ]; then
    rm -rf pistache
fi

git clone https://github.com/pistacheio/pistache.git
cd pistache

# Setup and build Pistache
meson setup build \
    --buildtype=debug \
    --prefix=/usr/local \
    -DPISTACHE_USE_SSL=true

cd build
meson compile
sudo meson install
sudo ldconfig

cd ../..

echo "Environment setup complete!"
