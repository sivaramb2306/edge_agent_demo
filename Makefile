CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -I./pistache/include `net-snmp-config --cflags`
LDFLAGS = -L./pistache/build/src -lpistache -lpthread -lnetsnmp `net-snmp-config --libs`

TARGET = rest_server
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

DOCKER_COMPOSE = docker compose
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
	docker build -t edge-agent:latest -f Dockerfile .

dev-image:
	docker build -t edge-agent:dev -f Dockerfile.dev .

up:
	docker-compose up -d

build-up:
	docker-compose build
	docker-compose up -d

restart-up:
	docker-compose down
	docker-compose up -d

dev-up:
	DOCKERFILE=Dockerfile.dev docker-compose up -d

dev-build-up:
	DOCKERFILE=Dockerfile.dev docker-compose build
	DOCKERFILE=Dockerfile.dev docker-compose up -d

dev-restart-up:
	DOCKERFILE=Dockerfile.dev docker-compose down
	DOCKERFILE=Dockerfile.dev docker-compose up -d

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
