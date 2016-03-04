/*
 * File:   PduSender.cpp
 * Author: dev
 *
 * Created on 16. September 2015, 20:31
 */

#include "PduSender.h"
#include <iostream>
#include <algorithm>

namespace otonat {

    PduSender::PduSender(NatMap * map) {
        this->map = map;
    }

    PduSender::PduSender(const PduSender& orig) {
        this->map = orig.map;
    }

    PduSender& PduSender::operator=(const PduSender& rhs) {
        if (this == &rhs) return *this; // handle self assignment

        this->map = rhs.map;
        return *this;
    }

    PduSender::~PduSender() {
    }

    void PduSender::SendPdusFromQueue() {
        while (true) {
            Tins::PDU * pdu = this->map->popPduOutgoingPduQueue();
            if (pdu == nullptr) {
                continue;
            }

            std::cout << "send pdu:" << pdu->size() << std::endl;
            bool isIp = false;
            Tins::IPv4Address dstIp = zeroIp;
            const Tins::IP * ip = pdu->find_pdu<Tins::IP>();
            if (ip != nullptr) {
                dstIp = ip->dst_addr();
                isIp = true;
            }

            if (!isIp) {
                const Tins::ARP * arp = pdu->find_pdu<Tins::ARP>();
                if (arp != nullptr) {
                    dstIp = arp->target_ip_addr();
                }
            }

            for (NatRange & range : this->map->ranges) {
                if (range.calcIpRange(true).contains(dstIp)) {
                    std::cout << "send pdu:" << pdu->size() << std::endl;
                    sender.send(*pdu, range.interface);
                    delete pdu;
                    break;
                }
            }
        }
    }

    std::thread * PduSender::SendPdusFromQueueThread() {
        std::thread * newThread = new std::thread(std::bind(&PduSender::SendPdusFromQueue, this));
        return newThread;
    }
}
