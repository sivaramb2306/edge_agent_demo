# Build stage
FROM ubuntu:22.04 AS builder

# Install basic build tools
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install library dependencies
RUN apt-get update && apt-get install -y \
    libsnmp-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install and build GTest
RUN apt-get update && apt-get install -y \
    libgtest-dev \
    google-mock \
    && rm -rf /var/lib/apt/lists/*

# Build and install GTest
WORKDIR /usr/src/googletest
RUN cmake . \
    && make -j$(nproc) \
    && cp lib/libgtest*.a /usr/lib/ \
    && cp lib/libgmock*.a /usr/lib/ \
    && mkdir -p /usr/include/gtest \
    && cp -r googletest/include/gtest/* /usr/include/gtest/ \
    && mkdir -p /usr/include/gmock \
    && cp -r googlemock/include/gmock/* /usr/include/gmock/

# Install Meson build system
RUN apt-get update && apt-get install -y \
    python3-pip \
    ninja-build \
    && pip3 install meson \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Clone Pistache
RUN git -c advice.detachedHead=false clone -q https://github.com/pistacheio/pistache.git /tmp/pistache

# Configure Pistache build
WORKDIR /tmp/pistache

RUN git -c advice.detachedHead=false checkout -q v0.4.26 \
    && meson setup build \
        --buildtype=release \
        --prefix=/usr/local \
        -DPISTACHE_USE_SSL=true

# Build and install Pistache
WORKDIR /tmp/pistache/build
RUN meson compile \
    && meson install \
    && ldconfig

# Cleanup and reset working directory
WORKDIR /app
RUN rm -rf /tmp/pistache

# Build the application
RUN rm -rf build \
    && mkdir -p build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF \
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
        .. \
    && make -j$(nproc)

# Runtime stage
FROM debian:bookworm-slim

# Enable non-free repository
RUN echo "deb http://deb.debian.org/debian bookworm contrib non-free" >> /etc/apt/sources.list

# Install only the required runtime libraries
RUN apt-get update && apt-get install -y \
    libsnmp40 \
    libcurl4 \
    libssl3 \
    snmp \
    snmp-mibs-downloader \
    && download-mibs \
    && rm -rf /var/lib/apt/lists/*

# Copy built artifacts and libraries
COPY --from=builder /usr/local/lib/x86_64-linux-gnu/libpistache.so* /usr/local/lib/x86_64-linux-gnu/
RUN mkdir -p /app
COPY --from=builder /app/build/rest_server /app/rest_server
RUN ldconfig

# Create MIB directories
RUN mkdir -p /usr/share/snmp/mibs

# Copy MIBs
COPY snmpd/powernet455.txt /usr/share/snmp/mibs/PowerNet-MIB.txt
COPY snmpd/mibs/ /usr/share/snmp/mibs/

# Set working directory
WORKDIR /app

# Expose the port (default port 9080)
EXPOSE 9080

# Set the entrypoint
ENTRYPOINT ["/app/rest_server"]
