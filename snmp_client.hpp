#pragma once

#include <string>
#include <memory>
#include <stdexcept>

// Wrap Net-SNMP headers in extern "C" to prevent C++ name mangling
extern "C" {
    #include <net-snmp/net-snmp-config.h>
    #include <net-snmp/net-snmp-includes.h>
}

class SNMPClient {
public:
    SNMPClient() {
        init_snmp("edge_agent");
    }

    std::string getSystemInfo(const std::string& host = "localhost", 
                            const std::string& community = "public", 
                            const std::string& oidStr = "1.3.6.1.2.1.1.1.0") {
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
            throw std::runtime_error("Failed to open SNMP session");
        }

        // Create PDU
        std::unique_ptr<netsnmp_pdu, decltype(&snmp_free_pdu)> pdu(
            snmp_pdu_create(SNMP_MSG_GET),
            snmp_free_pdu
        );
        snmp_add_null_var(pdu.get(), objid, objid_len);

        // Send request
        netsnmp_pdu* response = nullptr;
        int status = snmp_synch_response(ss.get(), pdu.release(), &response);

        // Use unique_ptr for response
        std::unique_ptr<netsnmp_pdu, decltype(&snmp_free_pdu)> response_ptr(
            response,
            snmp_free_pdu
        );

        std::string result;
        if (status == STAT_SUCCESS && response_ptr->errstat == SNMP_ERR_NOERROR) {
            for (netsnmp_variable_list* vars = response_ptr->variables; 
                 vars; 
                 vars = vars->next_variable) {
                char buf[1024];
                snprint_value(buf, sizeof(buf), vars->name, vars->name_length, vars);
                result += buf;
            }
        } else {
            throw std::runtime_error("Error in SNMP response");
        }

        return result;
    }
};
