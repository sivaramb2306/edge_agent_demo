FROM ubuntu:22.04

# Install build dependencies and runtime libraries
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git \
    libsnmp-dev \
    snmp \
    libcurl4-openssl-dev \
    libssl-dev \
    libgtest-dev \
    rapidjson-dev \
    python3-pip \
    ninja-build \
    && pip3 install meson \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build Pistache from source using Meson
RUN git clone https://github.com/pistacheio/pistache.git && \
    cd pistache && \
    meson setup build \
    -DPISTACHE_USE_SSL=true \
    --buildtype=release && \
    meson compile -C build && \
    meson install -C build && \
    ldconfig

# Build the application
RUN make clean && make

# Expose the port (default Pistache port 9081)
EXPOSE 9081

# Run the application
CMD ["./rest_server"]
