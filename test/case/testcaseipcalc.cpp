#include "UnitTest++/UnitTest++.h"
#include "../../src/map/natmap.h"
#include <tins/tins.h>
#include <vector>

SUITE(NatTests)
{
    class NatTestFixure
    {
    public:
        otonat::NatMap natMap = otonat::NatMap();

        Tins::IPv4Address deviceIpEth0 = Tins::IPv4Address("10.0.3.40");

        Tins::IPv4Address deviceIpEth1 = Tins::IPv4Address("192.168.23.42");

        Tins::IPv4Address deviceIpEth2 = Tins::IPv4Address("172.27.123.4");

        Tins::NetworkInterface::Info eth0Info =
        {
            .ip_addr = Tins::IPv4Address("10.0.0.2"),
            .netmask = Tins::IPv4Address("255.0.0.0"),
            .bcast_addr = Tins::IPv4Address("10.255.255.255"),
            .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:01"),
            .is_up = true
        };

        Tins::NetworkInterface::Info eth1Info =
        {
            .ip_addr = Tins::IPv4Address("192.168.23.42"),
            .netmask = Tins::IPv4Address("255.255.255.0"),
            .bcast_addr = Tins::IPv4Address("192.168.23.255"),
            .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:02"),
            .is_up = true
        };

        Tins::NetworkInterface::Info eth2Info =
        {
            .ip_addr = Tins::IPv4Address("172.16.47.11"),
            .netmask = Tins::IPv4Address("255.240.0.0"),
            .bcast_addr = Tins::IPv4Address("172.31.255.255"),
            .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:03"),
            .is_up = true
        };
    };

    TEST_FIXTURE(NatTestFixure, NetworkInterfaces)
    {
        CHECK(!natMap.interfaces.empty());
    }

    TEST_FIXTURE(NatTestFixure, IpCalcEth0)
    {
        Tins::IPv4Address expetedIp = deviceIpEth0;
        Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth0Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = Tins::IPv4Address("10.168.23.42");
        resultIp = natMap.mapIPv4Address(deviceIpEth1, eth0Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = Tins::IPv4Address("10.27.123.4");
        resultIp = natMap.mapIPv4Address(deviceIpEth2, eth0Info);
        CHECK_EQUAL(expetedIp,resultIp);
    }

    TEST_FIXTURE(NatTestFixure, IpCalcEth1)
    {
        Tins::IPv4Address expetedIp = Tins::IPv4Address("192.168.23.40");
        Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth1Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = deviceIpEth1;
        resultIp = natMap.mapIPv4Address(deviceIpEth1, eth1Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = Tins::IPv4Address("192.168.23.4");
        resultIp = natMap.mapIPv4Address(deviceIpEth2, eth1Info);
        CHECK_EQUAL(expetedIp,resultIp);
    }

    TEST_FIXTURE(NatTestFixure, IpCalcEth2)
    {
        Tins::IPv4Address expetedIp = Tins::IPv4Address("172.16.3.40");
        Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth2Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = Tins::IPv4Address("172.24.23.42");
        resultIp = natMap.mapIPv4Address(deviceIpEth1, eth2Info);
        CHECK_EQUAL(expetedIp,resultIp);

        expetedIp = deviceIpEth2;
        resultIp = natMap.mapIPv4Address(deviceIpEth2, eth2Info);
        CHECK_EQUAL(expetedIp,resultIp);
    }

    TEST(TestHandlePDU)
    {
      Tins::EthernetII eth = Tins::EthernetII() / Tins::IP() / Tins::TCP();
      otonat::NatMap natMap = otonat::NatMap();
      natMap.handlePdu(&eth);
    }

}
