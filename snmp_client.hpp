#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <map>
#include <vector>
#include "include/snmp_client_interface.hpp"

// Wrap Net-SNMP headers in extern "C" to prevent C++ name mangling
extern "C" {
    #include <net-snmp/net-snmp-config.h>
    #include <net-snmp/net-snmp-includes.h>
    #include <net-snmp/mib_api.h>
}

class SNMPClient : public SNMPClientInterface {
public:
    SNMPClient(const std::string& mibDir = "/usr/share/snmp/mibs") {
        // Initialize SNMP library
        init_snmp("edge_agent");
        
        // Set the MIB directory environment variable
        setenv("MIBDIRS", (mibDir + ":/usr/share/snmp/mibs").c_str(), 1);
        
        // Initialize MIB
        init_mib();
        
        // Add MIB directories
        add_mibdir("/usr/share/snmp/mibs");
        add_mibdir(mibDir.c_str());
        
        std::cout << "MIB search path: " << mibDir << ":/usr/share/snmp/mibs" << std::endl;
        
        // Load required MIB modules in correct order
        const char* mibs_to_load[] = {
            "SNMPv2-SMI",
            "SNMPv2-TC",
            "SNMPv2-CONF",
            "SNMPv2-MIB",
            "PowerNet-MIB",
            NULL
        };
        
        for (const char** mib = mibs_to_load; *mib; ++mib) {
            if (!read_module(*mib)) {
                std::cerr << "Warning: Failed to load MIB module: " << *mib << std::endl;
            }
        }
        
        // Refresh the MIB tree
        shutdown_mib();
        init_mib();
    }

    ~SNMPClient() override {
        shutdown_mib();
    }

    // Convert MIB name to OID using Net-SNMP library functions
    std::string mibToOid(const std::string& mibName) override {
        oid objid[MAX_OID_LEN];
        size_t objidlen = MAX_OID_LEN;
        
        // Initialize MIB
        netsnmp_init_mib();
        
        // Convert MIB name to OID
        if (!snmp_parse_oid(mibName.c_str(), objid, &objidlen)) {
            throw std::runtime_error("Failed to convert MIB to OID: " + mibName);
        }
        
        // Convert OID to string
        char oidStr[MAX_OID_LEN * 4];  // Should be plenty of space
        size_t len = 0;
        
        // Format each number with dots
        for (size_t i = 0; i < objidlen; i++) {
            len += snprintf(oidStr + len, sizeof(oidStr) - len, "%s%lu", 
                          (i == 0 ? "." : "."), (unsigned long)objid[i]);
        }
        
        return std::string(oidStr);
    }

    // Get SNMP value using MIB name
    std::string getMibValue(const std::string& mibName,
                          const std::string& host = "localhost",
                          const std::string& community = "public") override {
        return getSystemInfo(host, community, mibToOid(mibName));
    }

    // Get multiple SNMP values using MIB names
    std::map<std::string, std::string> getMibValues(
        const std::vector<std::string>& mibNames,
        const std::string& host = "localhost",
        const std::string& community = "public") override {
        
        std::map<std::string, std::string> results;
        for (const auto& mibName : mibNames) {
            try {
                results[mibName] = getMibValue(mibName, host, community);
            } catch (const std::exception& e) {
                results[mibName] = "Error: " + std::string(e.what());
            }
        }
        return results;
    }

    std::string getSystemInfo(const std::string& host = "localhost", 
                            const std::string& community = "public", 
                            const std::string& oidStr = "1.3.6.1.2.1.1.1.0") override {
        // Parse OID
        oid objid[MAX_OID_LEN];
        size_t objid_len = MAX_OID_LEN;
        
        if (!snmp_parse_oid(oidStr.c_str(), objid, &objid_len)) {
            throw std::runtime_error("Failed to parse OID");
        }

        // Create SNMP session
        netsnmp_session session;
        snmp_sess_init(&session);
        session.peername = strdup(host.c_str());
        session.version = SNMP_VERSION_2c;
        session.community = (u_char*)strdup(community.c_str());
        session.community_len = community.length();

        // Open session
        std::unique_ptr<netsnmp_session, decltype(&snmp_close)> ss(
            snmp_open(&session),
            snmp_close
        );

        if (!ss) {
            free(session.peername);
            free(session.community);
            throw std::runtime_error("Failed to open SNMP session");
        }

        // Create PDU
        std::unique_ptr<netsnmp_pdu, decltype(&snmp_free_pdu)> pdu(
            snmp_pdu_create(SNMP_MSG_GET),
            snmp_free_pdu
        );

        snmp_add_null_var(pdu.get(), objid, objid_len);

        // Send PDU
        netsnmp_pdu* response = nullptr;
        int status = snmp_synch_response(ss.get(), pdu.get(), &response);

        // Clean up session parameters
        free(session.peername);
        free(session.community);

        if (status != STAT_SUCCESS || !response) {
            throw std::runtime_error("SNMP request failed");
        }

        // Process response
        std::string result;
        for (netsnmp_variable_list* vars = response->variables; 
             vars; 
             vars = vars->next_variable) {
            char buf[1024];
            snprint_value(buf, sizeof(buf), vars->name, vars->name_length, vars);
            result = buf;
            break;  // We only expect one variable
        }

        snmp_free_pdu(response);
        return result;
    }
};
