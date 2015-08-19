/*
 * File:   nattest.cpp
 * Author: dev
 *
 * Created on 19.08.2015, 21:39:53
 */

#include "nattest.h"


CPPUNIT_TEST_SUITE_REGISTRATION(nattest);

nattest::nattest() {
}

nattest::~nattest() {
}

void nattest::setUp() {
}

void nattest::tearDown() {
}

void nattest::testIpCalcEth0() {
    Tins::IPv4Address expetedIp = deviceIpEth0;
    Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("10.168.23.42");
    resultIp = natMap.mapIPv4Address(deviceIpEth1, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("10.27.123.4");
    resultIp = natMap.mapIPv4Address(deviceIpEth2, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testIpCalcEth1() {
    Tins::IPv4Address expetedIp = Tins::IPv4Address("192.168.23.40");
    Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = deviceIpEth1;
    resultIp = natMap.mapIPv4Address(deviceIpEth1, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("192.168.23.4");
    resultIp = natMap.mapIPv4Address(deviceIpEth2, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testIpCalcEth2() {
    Tins::IPv4Address expetedIp = Tins::IPv4Address("172.16.3.40");
    Tins::IPv4Address resultIp = natMap.mapIPv4Address(deviceIpEth0, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("172.24.23.42");
    resultIp = natMap.mapIPv4Address(deviceIpEth1, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = deviceIpEth2;
    resultIp = natMap.mapIPv4Address(deviceIpEth2, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testTranslateIp() {
    Tins::EthernetII eth = Tins::EthernetII() / Tins::IP() / Tins::TCP();
    otonat::NatMap natMap = otonat::NatMap();
    natMap.handlePdu(&eth);
}

void nattest::testNatInterfaces() {
    CPPUNIT_ASSERT(!natMap.interfaces.empty());
}

