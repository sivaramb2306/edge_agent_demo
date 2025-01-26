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

# Create MIB directories and set SNMP environment
RUN mkdir -p /usr/share/snmp/mibs
ENV MIBS=+ALL
ENV MIBDIRS=/usr/share/snmp/mibs:/usr/share/snmp/mibs/ietf:/usr/share/snmp/mibs/iana:/usr/share/snmp/mibs/site
ENV SNMP_PERSISTENT_DIR=/var/lib/snmp

# Copy source files and MIBs
COPY CMakeLists.txt main.cpp snmp_client.hpp ./
COPY include/ include/
COPY snmpd/powernet455.txt /usr/share/snmp/mibs/PowerNet-MIB.txt
COPY snmpd/mibs/ /usr/share/snmp/mibs/

# Clone and build Pistache
RUN git -c advice.detachedHead=false clone -q https://github.com/pistacheio/pistache.git /tmp/pistache \
    && cd /tmp/pistache \
    && git -c advice.detachedHead=false checkout -q v0.4.26 \
    && meson setup build \
        --buildtype=release \
        --prefix=/usr/local \
        -DPISTACHE_USE_SSL=true \
    && cd build \
    && meson compile \
    && meson install \
    && ldconfig \
    && cd /app \
    && rm -rf /tmp/pistache

# Build the application
RUN mkdir -p build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF \
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
        .. \
    && make -j$(nproc) \
    && ls -la edge-agent \
    && cp edge-agent /app/ \
    && chmod +x /app/edge-agent

# Runtime stage
FROM debian:bookworm-slim

# Install runtime dependencies
RUN echo "deb http://deb.debian.org/debian bookworm contrib non-free" >> /etc/apt/sources.list \
    && apt-get update && apt-get install -y \
    libsnmp40 \
    libcurl4 \
    libssl3 \
    snmp \
    snmp-mibs-downloader \
    && download-mibs \
    && rm -rf /var/lib/apt/lists/*

# Copy built artifacts and libraries
COPY --from=builder /usr/local/lib/x86_64-linux-gnu/libpistache.so* /usr/local/lib/x86_64-linux-gnu/
RUN mkdir -p /app /usr/share/snmp/mibs
COPY --from=builder /app/edge-agent /app/
RUN chmod +x /app/edge-agent
COPY --from=builder /usr/share/snmp/mibs/ /usr/share/snmp/mibs/
RUN ldconfig

# Set SNMP environment variables
ENV MIBS=+ALL
ENV MIBDIRS=/usr/share/snmp/mibs:/usr/share/snmp/mibs/ietf:/usr/share/snmp/mibs/iana:/usr/share/snmp/mibs/site
ENV SNMP_PERSISTENT_DIR=/var/lib/snmp

# Set working directory
WORKDIR /app

# Expose the port
EXPOSE 9080

# Set the entrypoint
CMD ["./edge-agent"]
