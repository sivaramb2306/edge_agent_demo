CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -I./pistache/include `net-snmp-config --cflags`
LDFLAGS = -L./pistache/build/src -lpistache -lpthread -lnetsnmp `net-snmp-config --libs`

TARGET = edge-agent
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

# Check if docker compose v2 is available, otherwise use docker-compose
DOCKER_COMPOSE_V2 := $(shell which docker > /dev/null 2>&1 && docker compose version > /dev/null 2>&1 && echo "yes" || echo "no")
ifeq ($(DOCKER_COMPOSE_V2),yes)
    DOCKER_COMPOSE = docker compose
else
    DOCKER_COMPOSE = docker-compose
endif

DOCKER = docker

.PHONY: all clean run image dev-image up build-up restart-up dev-up dev-build-up dev-restart-up

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	LD_LIBRARY_PATH=./pistache/build/src ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

# Docker targets
image:
	$(DOCKER) build -t edge-agent:latest -f Dockerfile .

dev-image:
	$(DOCKER) build -t edge-agent:dev -f Dockerfile.dev .

up:
	$(DOCKER_COMPOSE) up -d

build-up:
	$(DOCKER_COMPOSE) build . -t edge-agent:latest
	$(DOCKER_COMPOSE) up -d

restart-up:
	$(DOCKER_COMPOSE) down
	$(DOCKER_COMPOSE) up -d

dev-up:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml up -d

dev-build-up:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml build
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml up -d

dev-restart-up:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml down
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml up -d

# Production targets
.PHONY: build
build:
	$(DOCKER) build -t edge-agent .

.PHONY: up
up: build
	$(DOCKER_COMPOSE) up -d

.PHONY: down
down:
	$(DOCKER_COMPOSE) down

# Development targets
.PHONY: dev-build
dev-build:
	$(DOCKER) build -t edge-agent-dev -f Dockerfile.dev .

.PHONY: dev-up
dev-up: dev-build
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml up -d

.PHONY: dev-down
dev-down:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml down

# Clean targets
.PHONY: clean
clean: down dev-down
	$(DOCKER) rmi edge-agent edge-agent-dev || true
	$(DOCKER) system prune -f

# Utility targets
.PHONY: logs
logs:
	$(DOCKER_COMPOSE) logs -f

.PHONY: dev-logs
dev-logs:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml logs -f

.PHONY: shell
shell:
	$(DOCKER_COMPOSE) exec edge-agent /bin/bash

.PHONY: dev-shell
dev-shell:
	$(DOCKER_COMPOSE) -f docker-compose.dev.yml exec edge-agent /bin/bash
