/*
 * File:   nattest.cpp
 * Author: dev
 *
 * Created on 19.08.2015, 21:39:53
 */

#include "nattest.h"
#include <iostream>

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
    Tins::EthernetII eth = Tins::EthernetII("00:00:00:00:00:01" ,"00:00:00:00:00:02") / Tins::IP("172.27.0.20", "172.16.3.55") / Tins::TCP();
    Tins::EthernetII eth2 = Tins::EthernetII("00:00:00:00:00:01" ,"00:00:00:00:00:03") / Tins::IP("172.27.0.20", "172.17.3.55") / Tins::TCP();
    Tins::NetworkInterface net1("lo:0");
    Tins::NetworkInterface net2("lo:1");
    otonat::NatMap::NetworkInterfaceList list;
    list.push_back(net1);
    list.push_back(net2);
    otonat::NatMap natMap = otonat::NatMap(list);
    natMap.handlePdu(&eth);
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());
    natMap.transMap.insert(otonat::NatMap::IPv4AddressEntry(Tins::IPv4Address("172.27.0.20"), Tins::IPv4Address("10.0.0.20")));
    natMap.handlePdu(&eth);
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size()== 1);
    const Tins::PDU * result = natMap.outgoingPduQueue.front(); 
    std::cout << "outgoingip: " << result->rfind_pdu<Tins::IP>().src_addr() << std::endl;
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());
}

void nattest::testNatInterfaces() {
    CPPUNIT_ASSERT(!natMap.interfaces.empty());
}

