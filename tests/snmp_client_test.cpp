#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../snmp_client.hpp"
#include "mock_snmp_client.hpp"
#include <cstdlib>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class SNMPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockClient = std::make_unique<NiceMock<MockSNMPClient>>();
    }

    std::unique_ptr<MockSNMPClient> mockClient;
};

TEST_F(SNMPClientTest, MibToOidConversion) {
    // Test conversion of UPS model name OID
    EXPECT_CALL(*mockClient, mibToOid("PowerNet-MIB::upsBasicIdentModel.0"))
        .WillOnce(Return("1.3.6.1.4.1.318.1.1.1.1.1.1.0"));

    std::string oid = mockClient->mibToOid("PowerNet-MIB::upsBasicIdentModel.0");
    EXPECT_EQ(oid, "1.3.6.1.4.1.318.1.1.1.1.1.1.0");
}

TEST_F(SNMPClientTest, GetMibValue) {
    std::string expectedValue = "Smart-UPS 1500";
    EXPECT_CALL(*mockClient, getMibValue("PowerNet-MIB::upsBasicIdentModel.0", _, _))
        .WillOnce(Return(expectedValue));

    std::string value = mockClient->getMibValue("PowerNet-MIB::upsBasicIdentModel.0", "localhost", "public");
    EXPECT_EQ(value, expectedValue);
}

TEST_F(SNMPClientTest, GetMultipleMibValues) {
    std::vector<std::string> mibNames = {
        "PowerNet-MIB::upsBasicIdentModel.0",
        "PowerNet-MIB::upsBasicBatteryStatus.0"
    };
    std::map<std::string, std::string> expectedValues = {
        {"PowerNet-MIB::upsBasicIdentModel.0", "Smart-UPS 1500"},
        {"PowerNet-MIB::upsBasicBatteryStatus.0", "2"} // 2 = batteryNormal
    };

    EXPECT_CALL(*mockClient, getMibValues(mibNames, _, _))
        .WillOnce(Return(expectedValues));

    auto values = mockClient->getMibValues(mibNames, "localhost", "public");
    EXPECT_EQ(values, expectedValues);
}

TEST_F(SNMPClientTest, GetSystemInfo) {
    std::string expectedInfo = "Smart-UPS 1500";
    EXPECT_CALL(*mockClient, getSystemInfo(_, _, "1.3.6.1.4.1.318.1.1.1.1.1.1.0"))
        .WillOnce(Return(expectedInfo));

    std::string info = mockClient->getSystemInfo("localhost", "public", "1.3.6.1.4.1.318.1.1.1.1.1.1.0");
    EXPECT_EQ(info, expectedInfo);
}

class SNMPClientIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Save original MIBDIRS value
        char* original_mibdirs = getenv("MIBDIRS");
        if (original_mibdirs) {
            original_mibdirs_value = original_mibdirs;
        }

        // Set MIBDIRS to include our test MIBs directory
        std::string mibdirs = "./mibs:/usr/share/snmp/mibs";
        setenv("MIBDIRS", mibdirs.c_str(), 1);
    }

    void TearDown() override {
        // Restore original MIBDIRS value
        if (!original_mibdirs_value.empty()) {
            setenv("MIBDIRS", original_mibdirs_value.c_str(), 1);
        } else {
            unsetenv("MIBDIRS");
        }
    }

private:
    std::string original_mibdirs_value;
};

TEST_F(SNMPClientIntegrationTest, RealClientTest) {
    // Enable Net-SNMP debugging
    debug_register_tokens("parse-mibs,snmp_parse_oid,read_config");
    snmp_set_do_debugging(1);
    
    std::cout << "Current MIBDIRS: " << getenv("MIBDIRS") << std::endl;
    std::cout << "Creating SNMPClient with MIB directory './mibs'" << std::endl;
    
    SNMPClient client("./mibs");  // Use our local MIB directory
    
    // Test MIB to OID conversions with PowerNet-MIB
    std::vector<std::pair<std::string, std::string>> testCases = {
        // Basic UPS identification
        {"PowerNet-MIB::upsBasicIdentModel.0", ".1.3.6.1.4.1.318.1.1.1.1.1.1.0"},
        
        // Battery management OIDs
        {"PowerNet-MIB::battManConfigCellsperBattery.0", ".1.3.6.1.4.1.318.1.1.16.6.6.0"},
        {"PowerNet-MIB::battManAlarmManagementController.0", ".1.3.6.1.4.1.318.1.1.16.7.1.0"},
        
        // PDU OIDs
        {"PowerNet-MIB::rPDUIdentModelNumber.0", ".1.3.6.1.4.1.318.1.1.12.1.5.0"},
        {"PowerNet-MIB::xPDUDeviceLoadTieBreakerPresent.0", ".1.3.6.1.4.1.318.1.1.15.2.7.0"}
    };

    for (const auto& testCase : testCases) {
        const auto& [mibName, expectedOid] = testCase;
        std::cout << "\nTesting conversion of " << mibName << std::endl;
        std::cout << "Expected OID: " << expectedOid << std::endl;
        
        EXPECT_NO_THROW({
            std::string oid = client.mibToOid(mibName);
            std::cout << "Converted OID: " << oid << std::endl;
            EXPECT_EQ(oid, expectedOid) << "Failed to convert " << mibName;
        });
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
