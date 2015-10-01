#include <iostream>
#include <tins/tins.h>
#include <thread>
#include "map/natmap.h"
#include "PduSniffer.h"
#include "PduSender.h"

int main()
{
    Tins::NetworkInterface net1("vboxnet0");
    Tins::NetworkInterface net2("vboxnet1");
    otonat::NatRange range1(net1, "10.0.0.0", "255.255.240.0");
    otonat::NatRange range2(net2, "172.27.0.0", "255.255.0.0");
    otonat::NatMap::NatRangeList list;
    list.push_back(range1);
    list.push_back(range2);
    otonat::NatMap natMap = otonat::NatMap(list);
    otonat::PduSniffer sniffer(&natMap);
    otonat::PduSender sender(&natMap);
    std::thread * mapThread = natMap.translateThread();
    std::thread * senderThread = sender.SendPdusFromQueueThread();
    std::thread * snifferThread1 = sniffer.SniffInterfaceInNewThread(net1);
    std::thread * snifferThread2 = sniffer.SniffInterfaceInNewThread(net2);
    mapThread->join();
    senderThread->join();
    snifferThread1->join();
    snifferThread2->join();
    return 0;
}

