#include <iostream>
#include <tins/tins.h>
#include <thread>
#include "map/natmap.h"
#include "PduSniffer.h"
#include "PduSender.h"
#include <json/json.h>
#include <json/value.h>
#include <fstream>
int main(int argc, char** argv)
{
    if (argc < 2){
        return 0;
    }
    
    otonat::NatMap::NatRangeList list;
    std::ifstream config_doc(argv[1], std::ifstream::binary);
    Json::Value root;
    config_doc >> root;
    const Json::Value netcards = root;
    for (Json::Value netcard : netcards) {
        const std::string name = netcard.getMemberNames()[0].c_str();
        const Json::Value cardMember = netcard[name];
        const std::string ipStr = cardMember["rangeIpAddr"].asString();
        const std::string maskStr = cardMember["rangeNetmask"].asString();

        const Tins::NetworkInterface net(name);
        const otonat::NatRange netRange(net, Tins::IPv4Address(ipStr), Tins::IPv4Address(maskStr));
        list.push_back(netRange);
    }
    
    otonat::NatMap natMap = otonat::NatMap(list);
    otonat::PduSniffer sniffer(&natMap);
    otonat::PduSender sender(&natMap);
    std::thread * mapThread = natMap.translateThread();
    std::thread * senderThread = sender.SendPdusFromQueueThread();
    for (otonat::NatRange & net : list) {
        sniffer.SniffInterfaceInNewThread(net.interface);
    }
    
    mapThread->join();
    senderThread->join();
    return 0;
}

