#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "snmp_client.hpp"

using namespace Pistache;
using json = nlohmann::json;

class RestServer {
public:
    explicit RestServer(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {
        snmpClient = std::make_unique<SNMPClient>("/usr/share/snmp/mibs");
    }

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
        Routes::Get(router, "/snmp/get", Routes::bind(&RestServer::getSNMPByMib, this));

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
            auto query = request.query();
            std::string host = query.has("host") ? query.get("host").value() : "snmpd";
            std::string community = query.has("community") ? query.get("community").value() : "public";
            std::string oid = query.has("oid") ? query.get("oid").value() : "1.3.6.1.2.1.1.1.0";

            auto result = snmpClient->getSystemInfo(host, community, oid);
            json responseJson = {
                {"status", "success"},
                {"data", result}
            };
            response.send(Http::Code::Ok, responseJson.dump());
        } catch (const std::exception& e) {
            json errorJson = {
                {"status", "error"},
                {"message", e.what()}
            };
            response.send(Http::Code::Internal_Server_Error, errorJson.dump());
        }
    }

    void getSNMPByMib(const Rest::Request& request, Http::ResponseWriter response) {
        auto query = request.query();
        auto mibParam = query.get("mib");
        auto hostParam = query.get("host");
        
        if (!mibParam || !hostParam) {
            json errorJson = {
                {"status", "error"},
                {"message", "Missing required parameters: 'mib' and 'host'"}
            };
            response.send(Http::Code::Bad_Request, errorJson.dump());
            return;
        }

        std::string mibName = *mibParam;
        std::string host = *hostParam;
        std::string community = query.get("community").value_or("public");

        try {
            // Convert MIB name to OID
            std::string oid = snmpClient->mibToOid(mibName);
            if (oid.empty()) {
                json errorJson = {
                    {"status", "error"},
                    {"message", "Invalid MIB name or conversion failed"}
                };
                response.send(Http::Code::Bad_Request, errorJson.dump());
                return;
            }

            // Query the SNMP device
            std::string value = snmpClient->getMibValue(mibName, host, community);
            
            // Create JSON response
            json responseJson = {
                {"status", "success"},
                {"data", {
                    {"mib", mibName},
                    {"oid", oid},
                    {"value", value},
                    {"host", host}
                }}
            };
            
            response.send(Http::Code::Ok, responseJson.dump());
        }
        catch (const std::exception& e) {
            json errorJson = {
                {"status", "error"},
                {"message", std::string("SNMP query failed: ") + e.what()}
            };
            response.send(Http::Code::Internal_Server_Error, errorJson.dump());
        }
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    std::unique_ptr<SNMPClient> snmpClient;
};

int main(int argc, char* argv[]) {
    Port port(9080);
    Address addr(Ipv4::any(), port);
    RestServer server(addr);

    server.init();
    server.start();
}
