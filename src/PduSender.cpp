/*
 * File:   PduSender.cpp
 * Author: dev
 *
 * Created on 16. September 2015, 20:31
 */

#include "PduSender.h"
#include "easylogging++.h"

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

            bool isIp = false;
            Tins::IPv4Address dstIp = zeroIp;
            std::stringstream msgStream;
            const Tins::IP * ip = pdu->find_pdu<Tins::IP>();
            if (ip != nullptr) {
                dstIp = ip->dst_addr();
                msgStream << "sending-ip: dst = " << dstIp << ", src= " << ip->src_addr() << ", checksum = " << ip->checksum();
                isIp = true;
            }

            if (!isIp) {
                const Tins::ARP * arp = pdu->find_pdu<Tins::ARP>();
                if (arp != nullptr) {
                    dstIp = arp->target_ip_addr();
                    msgStream << "sending-arp: dst = " << dstIp << ", src= " << arp->sender_ip_addr() << ", opcode = " << arp->opcode();
                }else{
                    msgStream << "sending-unkown: ";
                }
            }

            for (NatRange & range : this->map->ranges) {
                if (range.calcIpRange(true).contains(dstIp)) {
                    //LOG(INFO) << msgStream.str() << " (size = " << pdu->size() << ")";
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
