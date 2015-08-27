/*
 * File:   nattest.cpp
 * Author: dev
 *
 * Created on 19.08.2015, 21:39:53
 */

#include "nattest.h"
#include "../src/map/NatRange.h"
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(nattest);

nattest::nattest() {
    Tins::NetworkInterface net1("vboxnet0");
    Tins::NetworkInterface net2("vboxnet1");
    otonat::NatRange range1(net1, "10.0.0.0", "255.255.240.0");
    otonat::NatRange range2(net2, "172.27.0.0", "255.255.0.0");
    otonat::NatMap::NatRangeList list;
    list.push_back(range1);
    list.push_back(range2);
    natMap = otonat::NatMap(list);
}

nattest::~nattest() {
}

void nattest::setUp() {
}

void nattest::tearDown() {
}

void nattest::testIpCalcEth0() {
    Tins::IPv4Address expetedIp = deviceIpEth0;
    Tins::IPv4Address resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth0, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("10.168.23.42");
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth1, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("10.27.123.4");
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth2, eth0Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testIpCalcEth1() {
    Tins::IPv4Address expetedIp = Tins::IPv4Address("192.168.23.40");
    Tins::IPv4Address resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth0, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = deviceIpEth1;
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth1, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("192.168.23.4");
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth2, eth1Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testIpCalcEth2() {
    Tins::IPv4Address expetedIp = Tins::IPv4Address("172.16.3.40");
    Tins::IPv4Address resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth0, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = Tins::IPv4Address("172.24.23.42");
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth1, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);

    expetedIp = deviceIpEth2;
    resultIp = otonat::NatRange::mapIPv4Address(deviceIpEth2, eth2Info);
    CPPUNIT_ASSERT_EQUAL(expetedIp, resultIp);
}

void nattest::testTranslateIp() {
    Tins::EthernetII ethW = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:02") / Tins::IP("172.17.0.20", "172.16.3.55") / Tins::TCP();
    Tins::EthernetII eth = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:02") / Tins::IP("172.27.0.20", "172.16.3.55") / Tins::TCP();
    Tins::EthernetII ethAck = Tins::EthernetII("00:00:00:00:00:02", "00:00:00:00:00:01") / Tins::IP("10.0.3.55", "10.0.0.20") / Tins::TCP();
    Tins::EthernetII eth2 = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:03") / Tins::IP("172.27.0.20", "172.17.3.55") / Tins::TCP();
    Tins::EthernetII eth2Ack = Tins::EthernetII("00:00:00:00:00:03", "00:00:00:00:00:01") / Tins::IP("10.0.3.55", "10.0.0.20") / Tins::TCP();
    Tins::EthernetII eth3 = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:04") / Tins::IP("172.27.0.20", "172.18.1.40") / Tins::TCP();
    Tins::EthernetII eth3Ack = Tins::EthernetII("00:00:00:00:00:04", "00:00:00:00:00:01") / Tins::IP("10.0.1.40", "10.0.0.20") / Tins::TCP();
    Tins::EthernetII eth4 = Tins::EthernetII("00:00:00:00:00:02", "00:00:00:00:00:05") / Tins::IP("10.0.3.55", "10.0.1.41") / Tins::TCP();
    Tins::EthernetII eth4Ack = Tins::EthernetII("00:00:00:00:00:05", "00:00:00:00:00:02") / Tins::IP("172.27.1.41", "172.17.3.55") / Tins::TCP();

    natMap.handlePdu(eth.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    natMap.outgoingPduQueue.pop();            

    natMap.transMap.insert(otonat::NatMap::IPv4AddressEntry(Tins::IPv4Address("172.27.0.20"), Tins::IPv4Address("10.0.0.20")));

    natMap.handlePdu(ethW.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result = natMap.outgoingPduQueue.front();
    checkEth(result->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:01", "00:00:00:00:00:02", "10.0.0.20", "10.0.3.55");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(ethAck.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * resultAck = natMap.outgoingPduQueue.front();
    checkEth(resultAck->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:02", "00:00:00:00:00:01", "172.16.3.55", "172.27.0.20");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth2.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result2 = natMap.outgoingPduQueue.front();
    checkEth(result2->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:01", "00:00:00:00:00:03", "10.0.0.20", "10.0.3.55");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth2Ack.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result2Ack = natMap.outgoingPduQueue.front();
    checkEth(result2Ack->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:03", "00:00:00:00:00:01", "172.17.3.55", "172.27.0.20");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth3.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result3 = natMap.outgoingPduQueue.front();
    checkEth(result3->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:01", "00:00:00:00:00:04", "10.0.0.20", "10.0.1.40");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth3Ack.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result3Ack = natMap.outgoingPduQueue.front();
    checkEth(result3Ack->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:04", "00:00:00:00:00:01", "172.18.1.40", "172.27.0.20");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth4.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result4 = natMap.outgoingPduQueue.front();
    checkEth(result4->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:02", "00:00:00:00:00:05", "172.17.3.55", "172.27.1.41");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(eth4Ack.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.size() == 1);
    const Tins::PDU * result4Ack = natMap.outgoingPduQueue.front();
    checkEth(result4Ack->rfind_pdu<Tins::EthernetII>(), "00:00:00:00:00:05", "00:00:00:00:00:02", "10.0.1.41", "10.0.3.55");
    natMap.outgoingPduQueue.pop();
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());
}

void nattest::testForMeFromMe() {
    Tins::EthernetII forMe = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:03") / Tins::IP("172.16.0.1", "172.17.3.55") / Tins::TCP();
    Tins::EthernetII FromMe = Tins::EthernetII("00:00:00:00:00:01", "00:00:00:00:00:03") / Tins::IP("172.27.0.20", "172.16.0.1") / Tins::TCP();
    natMap.transMap.insert(otonat::NatMap::IPv4AddressEntry(Tins::IPv4Address("172.27.0.20"), Tins::IPv4Address("10.0.0.20")));

    natMap.handlePdu(forMe.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());

    natMap.handlePdu(FromMe.clone());
    CPPUNIT_ASSERT(natMap.outgoingPduQueue.empty());
}

void nattest::testNatInterfaces() {
    CPPUNIT_ASSERT(!natMap.ranges.empty());
}

void nattest::printIp(const Tins::IP & ip) {
    std::cout << std::endl << "### Ip-Packet ###" << std::endl;
    std::cout << "ip_dst: " << ip.dst_addr() << std::endl;
    std::cout << "ip_src: " << ip.src_addr() << std::endl;
    std::cout << "#################" << std::endl;
}

void nattest::printEth(const Tins::EthernetII & eth) {
    std::cout << std::endl << "+++ Ethernet-Frame +++" << std::endl;
    std::cout << "frame_dst: " << eth.dst_addr() << std::endl;
    std::cout << "frame_src: " << eth.src_addr();
    printIp(eth.rfind_pdu<Tins::IP>());
    std::cout << "++++++++++++++++++++++" << std::endl;
}

void nattest::checkAndPrintIp(const Tins::IP & ip, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp) {
    printIp(ip);
    checkIp(ip, targetDstIp, targetSrcIp);
}

void nattest::checkAndPrintEth(const Tins::EthernetII & eth, const Tins::HWAddress<6> & targetDstMac, const Tins::HWAddress<6> & targetSrcMac, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp) {
    printEth(eth);
    checkEth(eth, targetDstMac, targetSrcMac, targetDstIp, targetSrcIp);
}

void nattest::checkIp(const Tins::IP & ip, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp) {
    const Tins::IPv4Address & packetSrcIp = ip.src_addr();
    const Tins::IPv4Address & packetDstIp = ip.dst_addr();
    CPPUNIT_ASSERT_EQUAL(targetSrcIp, packetSrcIp);
    CPPUNIT_ASSERT_EQUAL(targetDstIp, packetDstIp);
}

void nattest::checkEth(const Tins::EthernetII & eth, const Tins::HWAddress<6> & targetDstMac, const Tins::HWAddress<6> & targetSrcMac, const Tins::IPv4Address & targetDstIp, const Tins::IPv4Address & targetSrcIp) {
    const Tins::HWAddress<6> & frameSrcMac = eth.src_addr();
    const Tins::HWAddress<6> & frameDstMac = eth.dst_addr();
    CPPUNIT_ASSERT_EQUAL(targetSrcMac, frameSrcMac);
    CPPUNIT_ASSERT_EQUAL(targetDstMac, frameDstMac);
    checkIp(eth.rfind_pdu<Tins::IP>(), targetDstIp, targetSrcIp);
}

