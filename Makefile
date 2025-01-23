CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -I./pistache/include `net-snmp-config --cflags`
LDFLAGS = -L./pistache/build/src -lpistache -lpthread -lnetsnmp `net-snmp-config --libs`

TARGET = rest_server
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	LD_LIBRARY_PATH=./pistache/build/src ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
