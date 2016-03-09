/*
 * File:   PduSniffer.cpp
 * Author: dev
 *
 * Created on 29. September 2015, 21:29
 */

#include "PduSniffer.h"
#include "easylogging++.h"

namespace otonat
{

PduSniffer::PduSniffer(NatMap * map)
{
    this->map = map;
    this->isRunnig = false;
    config.set_promisc_mode(true);
    config.set_immediate_mode(true);
}

PduSniffer::PduSniffer(const PduSniffer& orig) : snifferList(orig.snifferList), config(orig.config)
{
    this->map = orig.map;
    this->isRunnig = orig.isRunnig;
}

PduSniffer::~PduSniffer()
{
    Stop();
    for (Tins::Sniffer * sniffer : snifferList)
    {
        delete sniffer;
    }

    this->snifferList.clear();
}

PduSniffer& PduSniffer::operator=(const PduSniffer& rhs)
{
    if (this == &rhs) return *this; // handle self assignment

    this->map = rhs.map;
    this->isRunnig = rhs.isRunnig;
    this->config = rhs.config;
    this->snifferList = rhs.snifferList;
    return *this;
}

bool PduSniffer::sniffPdu(const Tins::PDU& pdu)
{
    if (map->isOutgoingPdu(pdu, interfaceId))
    {
        LOG(INFO) << "skip-outgoing: interface = " << interfaceName << "( id = " << interfaceId << ") (size = " << pdu.size() << ")";
        return this->isRunnig;
    }

    LOG(INFO) << "sniff-incomming: interface = " << interfaceName << " (id = " << interfaceId << ") (size = " << pdu.size() << ")";
    this->map->pushPduToIncommingPduQueue(pdu.clone());
    return this->isRunnig;
}

void PduSniffer::Start()
{
    this->isRunnig = true;
}

void PduSniffer::Stop()
{
    this->isRunnig = false;
}

void PduSniffer::SniffInterface(const Tins::NetworkInterface & interface)
{
    interfaceId = interface.id();
    interfaceName = interface.name();
    LOG(INFO) << "create-sniffer: interface = " << interfaceName << " (id = " << interfaceId <<  ")";
    Start();
    Tins::Sniffer * sniffer = new Tins::Sniffer(interfaceName, config);
    sniffer->sniff_loop(std::bind(&PduSniffer::sniffPdu, this, std::placeholders::_1));
    this->snifferList.push_back(sniffer);
}

std::thread *  PduSniffer::SniffInterfaceInNewThread(const Tins::NetworkInterface& interface)
{
    std::thread * newThread = new std::thread(std::bind(&PduSniffer::SniffInterface, this,  interface));
    return newThread;
}
}
