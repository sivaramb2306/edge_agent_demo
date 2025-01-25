#pragma once

#include <gmock/gmock.h>
#include "../include/snmp_client_interface.hpp"

class MockSNMPClient : public SNMPClientInterface {
public:
    MOCK_METHOD(std::string, mibToOid, (const std::string& mibName), (override));
    MOCK_METHOD(std::string, getMibValue, 
        (const std::string& mibName, const std::string& host, const std::string& community), 
        (override));
    MOCK_METHOD((std::map<std::string, std::string>), getMibValues,
        (const std::vector<std::string>& mibNames, const std::string& host, const std::string& community),
        (override));
    MOCK_METHOD(std::string, getSystemInfo,
        (const std::string& host, const std::string& community, const std::string& oidStr),
        (override));
};
