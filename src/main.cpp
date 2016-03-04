#include <iostream>
#include <tins/tins.h>
#include <thread>
#include "natmap.h"
#include "PduSniffer.h"
#include "PduSender.h"
#include <json/json.h>
#include <json/value.h>
#include <fstream>

template<typename Container>
void delete_them(Container& c)
{
    while(!c.empty()) {
        delete c.back(), c.pop_back();
    }
    c.clear();
}

int main(int argc, char** argv)
{
    if (argc < 2){
        return 0;
    }

    otonat::NatMap::NatRangeList interfaceList;
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
        interfaceList.push_back(netRange);
    }

    otonat::NatMap natMap = otonat::NatMap(interfaceList);
    otonat::PduSender sender(&natMap);
    std::vector<otonat::PduSniffer* > snifferList;
    std::vector<std::thread * > threadList;
    threadList.push_back(natMap.translateThread());
    threadList.push_back(sender.SendPdusFromQueueThread());
    for (otonat::NatRange & net : interfaceList) {
        otonat::PduSniffer * sniffer = new otonat::PduSniffer(&natMap);
        snifferList.push_back(sniffer);
        threadList.push_back(sniffer->SniffInterfaceInNewThread(net.interface));
    }

    for (std::thread * thread: threadList) {
       thread->join();
    }

    delete_them(threadList);
    delete_them(snifferList);
    threadList.clear();
    snifferList.clear();
    return 0;
}

