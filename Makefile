CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -I./pistache/include `net-snmp-config --cflags`
LDFLAGS = -L./pistache/build/src -lpistache -lpthread -lnetsnmp `net-snmp-config --libs`

TARGET = rest_server
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean run image dev-image up build-up restart-up

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
