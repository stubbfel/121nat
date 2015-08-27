#include "natmap.h"

namespace otonat {

    NatMap::NatMap(NatRangeList rangeList) : ranges(rangeList) {
    }

    NatMap::~NatMap() {
        //dtor
    }

    NatMap::NatMap(const NatMap& other) : ranges(other.ranges), arpMap(other.arpMap), transMap(other.transMap), reqIpMap(other.reqIpMap), incommingPduQueue(other.incommingPduQueue), outgoingPduQueue(other.outgoingPduQueue), zeroIp(other.zeroIp) {
    }

    NatMap& NatMap::operator=(const NatMap& rhs) {
        if (this == &rhs) return *this; // handle self assignment

        ranges = rhs.ranges;
        arpMap = rhs.arpMap;
        transMap = rhs.transMap;
        incommingPduQueue = rhs.incommingPduQueue;
        outgoingPduQueue = rhs.outgoingPduQueue;
        return *this;
    }

    void NatMap::handlePdu(const Tins::PDU * pdu) {
        if (pdu == nullptr) {
            return;
        }

        Tins::PDU * pduCopy = pdu->clone();
        delete pdu;
        Tins::ARP * arp = pduCopy->find_pdu<Tins::ARP>();
        if (arp != nullptr) {
            if (handleArp(arp)) {
                outgoingPduQueue.push(pduCopy);
            }
            return;
        }

        Tins::IP * ip = pduCopy->find_pdu<Tins::IP>();
        if (ip != nullptr) {
            if (handleIp(ip, pduCopy)) {
                outgoingPduQueue.push(pduCopy);
            }
        }
    }

    bool NatMap::handleIp(Tins::IP * ip, const Tins::PDU * originPDU) {

        if (isForMeOrFromMeIp(ip)) {
            return false;
        }

        const Tins::IPv4Address & originDstIp = ip->dst_addr();
        if (!isIpInMyRanges(originDstIp)) {
            return false;
        }

        IpAdressMap::iterator transIpIter = transMap.find(originDstIp);
        if (transIpIter != transMap.end()) {
            // handle know traslation ip
            const Tins::IPv4Address transDstIp = transIpIter->second;
            TranslateIpPacket(ip, transDstIp);
            IpAdressMap::iterator transDstIpIter = transMap.find(transDstIp);
            if (transDstIpIter == transMap.end()) {
                transMap.insert(IPv4AddressEntry(transDstIp, originDstIp));
            } else if (transDstIpIter->second != originDstIp) {
                transMap[transDstIp] = originDstIp;
            }

            return true;
        } else {
            const Tins::EthernetII * eth = originPDU->find_pdu<Tins::EthernetII>();
            if (eth != nullptr) {
                Tins::EthernetII fakeArp = Tins::ARP::make_arp_request(ip->dst_addr(), ip->src_addr(), eth->src_addr());
                SendTranslatedArpRequest(fakeArp.find_pdu<Tins::ARP>());
            }

            return false;
        }
    }

    void NatMap::TranslateIpPacket(Tins::IP * ip, const Tins::IPv4Address & transIp) {

        // set translated dst address
        ip->dst_addr(transIp);

        // translate src adress
        const Tins::IPv4Address & originSrc = ip->src_addr();

        IpAdressMap::const_iterator transIpIter = transMap.find(originSrc);
        Tins::IPv4Address transSrcAttr;
        if (transIpIter != transMap.end()) {
            // set translated src address
            transSrcAttr = transIpIter->second;
        } else {
            transSrcAttr = InsertOrUdpateTranslateIpAddress(originSrc, transIp, ranges);
        }

        ip->src_addr(transSrcAttr);
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & transIp, NatRangeList & rangeList) {
        // calc translated ip address for first up and not same interfaces
        for (NatRange & range : rangeList) {
            // insert or update translated ip address
            const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            Tins::IPv4Range ipRange = range.calcIpRange(true);
            if (!ipRange.contains(transIp)) {
                continue;
            }

            return InsertOrUdpateTranslateIpAddress(originIp, range);
        }

        return zeroIp;
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const NatRange & range) {
        // translated ip address
        Tins::IPv4Address transAddr = range.mapIPv4Addres(originIp, false);

        // insert forward translation
        transMap.insert(IPv4AddressEntry(originIp, transAddr));

        // insert or update backward translation
        IpAdressMap::const_iterator transIpIter = transMap.find(transAddr);
        if (transIpIter != transMap.end()) {
            transMap[transAddr] = originIp;
        } else {
            transMap.insert(IPv4AddressEntry(transAddr, originIp));
        }

        return transAddr;
    }

    bool NatMap::handleArp(Tins::ARP * arp) {

        if (isForMeOrFromMeArp(arp)) {
            return false;
        }

        switch (arp->opcode()) {
            case Tins::ARP::REQUEST:
                return this->handleArpReq(arp);
            case Tins::ARP::REPLY:
                return this->handleArpReply(arp);
            default:
                return false;
        }
    }

    bool NatMap::isForMeOrFromMeIp(const Tins::IP* ip) {
        return isForMeOrFromMeIp(ip, ranges);
    }

    bool NatMap::isForMeOrFromMeArp(const Tins::ARP * arp) {
        Tins::IP fakeIp(arp->target_ip_addr(), arp->sender_ip_addr());
        return isForMeOrFromMeIp(&fakeIp);
    }

    bool NatMap::isForMeOrFromMeIp(const Tins::IP * ip, const NatRangeList & rangeList) {
        const Tins::IPv4Address & srcAddr = ip->src_addr();
        const Tins::IPv4Address & dstAddr = ip->dst_addr();
        for (NatRange range : rangeList) {
            const Tins::IPv4Address & interfaceAddr = range.interface.info().ip_addr;
            if (srcAddr == interfaceAddr || dstAddr == interfaceAddr) {
                return true;
            }
        }
        return false;
    }

    bool NatMap::isIpInMyRanges(const Tins::IPv4Address & ipAddr) {
        return isIpInMyRanges(ipAddr, ranges);
    }

    bool NatMap::isIpInMyRanges(const Tins::IPv4Address & ipAddr, const NatRangeList & rangeList) {
        for (NatRange range : rangeList) {
            if (range.calcIpRange(false).contains(ipAddr)) {
                return true;
            }
        }

        return false;
    }

    bool NatMap::handleArpReq(Tins::ARP* arp) {
        Tins::IPv4Address targetIp = arp->target_ip_addr();        
        IpAdressMap::const_iterator transTargetIpIter = this->transMap.find(targetIp);
        if (transTargetIpIter == transMap.end()) {
            SendTranslatedArpRequest(arp);
            return false;
        }

        Tins::IPv4Address transTargetIp = transTargetIpIter->second;
        arp->target_ip_addr(transTargetIp);
        IpAdressMap::const_iterator transSenderIpIter = this->transMap.find(arp->sender_ip_addr());
        if (transSenderIpIter != transMap.end()) {
            arp->sender_ip_addr(transSenderIpIter->second);
            return false;
        }
        
        return handleArpAndTranslateSenderIp(arp);
    }

    bool NatMap::handleArpReply(Tins::ARP* arp) {
        return false;
    }
    
    bool NatMap::handleArpAndTranslateSenderIp(Tins::ARP* arp) {
        for (NatRange & range : this->ranges) {
                const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
                if (!interfaceInfo.is_up) {
                    continue;
                }

                Tins::IPv4Range ipRange = range.calcIpRange(true);
                if (!ipRange.contains(arp->target_ip_addr())) {
                    continue;
                }
                
                Tins::IPv4Address senderIp = arp->sender_ip_addr();
                Tins::IPv4Address transSenderIp = range.mapIPv4Addres(senderIp, true);
                arp->sender_ip_addr(transSenderIp);

                IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(transSenderIp);
                if (transSenderIpReqIter == reqIpMap.end()) {
                    this->reqIpMap.insert(IPv4AddressEntry(transSenderIp, senderIp));
                }
                
                return true;
            }
        
        return false;
    }

    void NatMap::SendTranslatedArpRequest(const Tins::ARP * arp) {
        Tins::IPv4Address targetIp = arp->target_ip_addr();
        for (NatRange & range : this->ranges) {
            const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            Tins::IPv4Range ipRange = range.calcIpRange(true);
            if (ipRange.contains(targetIp)) {
                continue;
            }

            Tins::IPv4Address senderIp = arp->sender_ip_addr();
            Tins::IPv4Address transSenderIp = range.mapIPv4Addres(senderIp, true);
            Tins::IPv4Address transTargetIp = range.mapIPv4Addres(targetIp, true);
            IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(transSenderIp);
            if (transSenderIpReqIter == reqIpMap.end()) {
                this->reqIpMap.insert(IPv4AddressEntry(transSenderIp, senderIp));
            }

            IpAdressMap::const_iterator transTargetIpReqIter = this->reqIpMap.find(transTargetIp);
            if (transTargetIpReqIter == reqIpMap.end()) {
                this->reqIpMap.insert(IPv4AddressEntry(transTargetIp, targetIp));
            }

            Tins::EthernetII transArp = Tins::ARP::make_arp_request(transTargetIp, transSenderIp, arp->sender_hw_addr());
            outgoingPduQueue.push(transArp.clone());
        }
    }
}
