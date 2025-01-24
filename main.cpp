#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <iostream>
#include "snmp_client.hpp"

using namespace Pistache;

class RestServer {
public:
    explicit RestServer(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t threads = 2) {
        auto opts = Http::Endpoint::options()
            .threads(threads);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->serve();
    }

private:
    void setupRoutes() {
        using namespace Rest;

        Routes::Get(router, "/hello", Routes::bind(&RestServer::getHello, this));
        Routes::Post(router, "/echo", Routes::bind(&RestServer::postEcho, this));
        Routes::Get(router, "/snmp/info", Routes::bind(&RestServer::getSNMPInfo, this));

        httpEndpoint->setHandler(router.handler());
    }

    void getHello(const Rest::Request& request, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Hello, World!");
    }

    void postEcho(const Rest::Request& request, Http::ResponseWriter response) {
        if (request.body().empty()) {
            response.send(Http::Code::Bad_Request, "Empty body");
            return;
        }
        response.send(Http::Code::Ok, request.body());
    }

    void getSNMPInfo(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            // Get query parameters with defaults
            auto query = request.query();
            std::string host = "snmpd";
            std::string community = "public";
            std::string oid = "1.3.6.1.2.1.1.1.0";

            // Override defaults if parameters are provided
            if (query.has("host")) host = query.get("host").value();
            if (query.has("community")) community = query.get("community").value();
            if (query.has("oid")) oid = query.get("oid").value();

            SNMPClient snmp;
            std::string result = snmp.getSystemInfo(host, community, oid);
            response.send(Http::Code::Ok, result);
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, 
                        std::string("SNMP Error: ") + e.what());
        }
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char* argv[]) {
    try {
        Port port(9081);  
        Address addr(Ipv4::any(), port);
        
        std::cout << "Starting REST server on port " << port.toString() << std::endl;
        
        RestServer server(addr);
        server.init();
        server.start();
    } catch (std::runtime_error& e) {
        std::cerr << "Server failed to start: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
