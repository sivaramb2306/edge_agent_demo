# Edge Agent

A C++ SNMP edge agent with REST API capabilities using Pistache framework.

## Prerequisites

- Docker
- Make

## Production Build and Run

### Building the Image
```bash
# Build the Docker image
make image

# Build and start containers
make up

# Build image and start containers in one command
make build-up

# Restart containers
make restart-up
```

### Cleaning Up
```bash
# Stop containers, remove images and build artifacts
make clean
```

## Development Build and Run

### Building for Development
```bash
# Build development image and start containers
make dev-build-up

# Start development containers without rebuilding
make dev-up

# Restart development containers
make dev-restart-up
```

### Development Container Usage
```bash
# Enter the development container
docker exec -it edge-agent-dev bash

# Inside container, build the project
cd /app
rm -rf build  # Clean any existing build
meson setup build
cd build
meson compile
```

### Cleaning Up Development Environment
```bash
# Stop containers, remove dev images and build artifacts
make dev-clean
```

## Available Make Commands

| Command | Description |
|---------|-------------|
| `make image` | Build production Docker image |
| `make up` | Start production containers |
| `make build-up` | Build and start production containers |
| `make restart-up` | Restart production containers |
| `make clean` | Clean production environment |
| `make dev-build-up` | Build and start development containers |
| `make dev-up` | Start development containers |
| `make dev-restart-up` | Restart development containers |
| `make dev-clean` | Clean development environment |

## API Endpoints

The server runs on port 9080. Documentation for API endpoints coming soon.