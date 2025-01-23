# Simple REST API Server in C++

This is a simple REST API server implemented in C++ using the Pistache framework.

## Prerequisites

- C++17 compiler
- Pistache library
- Make build system

## Building the Project

To build the project, run:

```bash
make
```

## Running the Server

After building, run the server:

```bash
./rest_server
```

The server will start on port 9080.

## Available Endpoints

1. GET /hello
   - Returns a "Hello, World!" message
   
2. POST /echo
   - Echoes back the request body

## Testing the API

You can test the endpoints using curl:

```bash
# Test GET /hello
curl http://localhost:9080/hello

# Test POST /echo
curl -X POST -d "Hello Server" http://localhost:9080/echo
```
# snmp_edge_agent
