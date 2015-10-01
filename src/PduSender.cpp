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

    PduSender::PduSender(const PduSender& orig) : checksumList(orig.checksumList) {
        this->map = orig.map;
    }

    PduSender& PduSender::operator=(const PduSender& rhs) {
        if (this == &rhs) return *this; // handle self assignment

        this->map = rhs.map;
        this->checksumList = rhs.checksumList;
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

            Tins::IPv4Address dstIp = zeroIp;
            const Tins::ARP * arp = pdu->find_pdu<Tins::ARP>();
            bool isArp = false;
            if (arp != nullptr) {
                dstIp = arp->target_ip_addr();
                isArp = true;
            }

            if (!isArp) {
                const Tins::IP * ip = pdu->find_pdu<Tins::IP>();
                if (ip != nullptr) {
                    dstIp = ip->dst_addr();

                    unsigned short int checkSum = ip->checksum();
                    ChecksumList::const_iterator endIter = this->checksumList.end();
                    ChecksumList::const_iterator beginIter = this->checksumList.begin();
                    ChecksumList::const_iterator findIter = std::find(beginIter, endIter, checkSum);
                    if (endIter == findIter) {
                        this->checksumList.push_back(checkSum);
                        if (this->checksumList.size() > 10) {
                            this->checksumList.pop_front();
                        }
                    } else {
                        continue;
                    }
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
