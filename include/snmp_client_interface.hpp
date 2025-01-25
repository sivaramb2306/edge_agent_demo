#pragma once

#include <string>
#include <map>
#include <vector>

class SNMPClientInterface {
public:
    virtual ~SNMPClientInterface() = default;

    virtual std::string mibToOid(const std::string& mibName) = 0;
    virtual std::string getMibValue(const std::string& mibName,
                                  const std::string& host = "localhost",
                                  const std::string& community = "public") = 0;
    virtual std::map<std::string, std::string> getMibValues(
        const std::vector<std::string>& mibNames,
        const std::string& host = "localhost",
        const std::string& community = "public") = 0;
    virtual std::string getSystemInfo(const std::string& host = "localhost",
                                    const std::string& community = "public",
                                    const std::string& oidStr = "1.3.6.1.2.1.1.1.0") = 0;
};
