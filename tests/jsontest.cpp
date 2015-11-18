/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   jsontest.cpp
 * Author: dev
 *
 * Created on 18.11.2015, 20:41:37
 */

#include "jsontest.h"
#include <json/json.h>
#include <json/value.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include "../src/map/NatRange.h"
#include <tins/tins.h>
CPPUNIT_TEST_SUITE_REGISTRATION(jsontest);

jsontest::jsontest() {
}

jsontest::~jsontest() {
}

void jsontest::setUp() {
}

void jsontest::tearDown() {
}

void jsontest::testReadConfig() {
    std::ifstream config_doc("src/config.json", std::ifstream::binary);
    Json::Value root;
    config_doc >> root;
    const Json::Value netcards = root;
    for (Json::Value netcard : netcards) {
        const std::string name = netcard.getMemberNames()[0].c_str();
        const Json::Value cardMember = netcard[name];
        const std::string ipStr = cardMember["rangeIpAddr"].asString();
        const std::string maskStr = cardMember["rangeNetmask"].asString();
        CPPUNIT_ASSERT(name == "vboxnet0" || name == "vboxnet1");
        CPPUNIT_ASSERT(ipStr == "10.0.0.0" || ipStr == "172.27.0.0");
        CPPUNIT_ASSERT(maskStr == "255.255.240.0" || maskStr == "255.255.0.0");
        const Tins::NetworkInterface net(name);
        const otonat::NatRange netRange(net, Tins::IPv4Address(ipStr), Tins::IPv4Address(maskStr));
        CPPUNIT_ASSERT(name ==  netRange.interface.name());
        CPPUNIT_ASSERT(ipStr == netRange.rangeIpAddr.to_string());
        CPPUNIT_ASSERT(maskStr == netRange.rangeNetmask.to_string());
    }

}


