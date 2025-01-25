# Build stage
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git \
    libsnmp-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    rapidjson-dev \
    python3-pip \
    ninja-build \
    pkg-config \
    nlohmann-json3-dev \
    && pip3 install meson \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Clean any existing build artifacts
RUN rm -rf build pistache

# Build and install Pistache optimized for production
RUN git clone https://github.com/pistacheio/pistache.git && \
    cd pistache && \
    meson setup build \
    -DPISTACHE_USE_SSL=true \
    --buildtype=release \
    -Doptimization=3 && \
    meson compile -C build && \
    DESTDIR=/app/install meson install -C build

# Build the application optimized for production
RUN mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_TESTS=OFF \
          -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
          -DCMAKE_INSTALL_PREFIX=/app/install .. && \
    make && \
    make install

# Runtime stage
FROM ubuntu:22.04

# Install only the required runtime libraries
RUN apt-get update && apt-get install -y \
    libsnmp40 \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy built artifacts from builder
COPY --from=builder /app/install/usr/local/lib/x86_64-linux-gnu/libpistache.so.* /usr/local/lib/x86_64-linux-gnu/
COPY --from=builder /app/install/usr/local/bin/edge_agent /usr/local/bin/
COPY --from=builder /app/config /config

# Update library cache
RUN ldconfig

# Set working directory
WORKDIR /

# Expose the port (default Pistache port 9081)
EXPOSE 9081

# Run the application
CMD ["edge_agent"]
