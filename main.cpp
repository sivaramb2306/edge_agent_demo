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
        try {
            auto query = request.query();
            if (!query.has("mib")) {
                throw std::runtime_error("Missing required 'mib' parameter");
            }

            std::string mibName = query.get("mib").value();
            std::string host = query.has("host") ? query.get("host").value() : "snmpd";
            std::string community = query.has("community") ? query.get("community").value() : "public";

            // Handle multiple MIBs separated by comma
            std::vector<std::string> mibNames;
            std::stringstream ss(mibName);
            std::string mib;
            while (std::getline(ss, mib, ',')) {
                mibNames.push_back(mib);
            }

            json responseJson;
            if (mibNames.size() == 1) {
                auto result = snmpClient->getMibValue(mibNames[0], host, community);
                responseJson = {
                    {"status", "success"},
                    {"data", {
                        {mibNames[0], result}
                    }}
                };
            } else {
                auto results = snmpClient->getMibValues(mibNames, host, community);
                responseJson = {
                    {"status", "success"},
                    {"data", results}
                };
            }

            response.send(Http::Code::Ok, responseJson.dump());
        } catch (const std::exception& e) {
            json errorJson = {
                {"status", "error"},
                {"message", e.what()}
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
