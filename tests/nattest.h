/*
 * File:   nattest.h
 * Author: dev
 *
 * Created on 19.08.2015, 21:39:53
 */

#ifndef NATTEST_H
#define	NATTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "../src/map/natmap.h"
#include <tins/tins.h>
#include <vector>

class nattest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(nattest);
    CPPUNIT_TEST(testTranslateIp);
    CPPUNIT_TEST(testNatInterfaces);
    CPPUNIT_TEST(testIpCalcEth0);
    CPPUNIT_TEST(testIpCalcEth1);
    CPPUNIT_TEST(testIpCalcEth2);
    CPPUNIT_TEST(testForMeFromMe);
    CPPUNIT_TEST_SUITE_END();

public:
    nattest();
    virtual ~nattest();
    void setUp();
    void tearDown();

private:
    otonat::NatMap natMap;
    Tins::IPv4Address deviceIpEth0 = Tins::IPv4Address("10.0.3.40");
    Tins::IPv4Address deviceIpEth1 = Tins::IPv4Address("192.168.23.42");
    Tins::IPv4Address deviceIpEth2 = Tins::IPv4Address("172.27.123.4");
    Tins::NetworkInterface::Info eth0Info = {
        .ip_addr = Tins::IPv4Address("10.0.0.2"),
        .netmask = Tins::IPv4Address("255.0.0.0"),
        .bcast_addr = Tins::IPv4Address("10.255.255.255"),
        .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:01"),
        .is_up = true
    };

    Tins::NetworkInterface::Info eth1Info = {
        .ip_addr = Tins::IPv4Address("192.168.23.42"),
        .netmask = Tins::IPv4Address("255.255.255.0"),
        .bcast_addr = Tins::IPv4Address("192.168.23.255"),
        .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:02"),
        .is_up = true
    };

    Tins::NetworkInterface::Info eth2Info = {
        .ip_addr = Tins::IPv4Address("172.16.47.11"),
        .netmask = Tins::IPv4Address("255.240.0.0"),
        .bcast_addr = Tins::IPv4Address("172.31.255.255"),
        .hw_addr = Tins::HWAddress<6>("00:00:00:00:00:03"),
        .is_up = true
    };

    void testNatInterfaces();
    void testIpCalcEth0();
    void testIpCalcEth1();
    void testIpCalcEth2();
    void testTranslateIp();
    void testForMeFromMe();

    void printIp(const Tins::IP & ip);
    void printEth(const Tins::EthernetII & eth);
    void checkAndPrintIp(const Tins::IP & ip, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp);
    void checkAndPrintEth(const Tins::EthernetII & eth, const Tins::HWAddress<6> & targetDstMac, const Tins::HWAddress<6> & targetSrcMac, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp);
    void checkIp(const Tins::IP & ip, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp);
    void checkEth(const Tins::EthernetII & eth, const Tins::HWAddress<6> & targetDstMac, const Tins::HWAddress<6> & targetSrcMac, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp);
};

#endif	/* NATTEST_H */

