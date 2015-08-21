#include "natmap.h"

namespace otonat {

    NatMap::NatMap(NatRangeList rangeList) : ranges(rangeList) {
    }

    NatMap::~NatMap() {
        //dtor
    }

    NatMap::NatMap(const NatMap& other) : ranges(other.ranges), arpMap(other.arpMap), transMap(other.transMap), incommingPduQueue(other.incommingPduQueue), outgoingPduQueue(other.outgoingPduQueue), zeroIp(other.zeroIp) {
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
        Tins::PDU * pduCopy = pdu->clone();
        if (pdu == NULL) {
            return;
        }

        Tins::ARP * arp = pduCopy->find_pdu<Tins::ARP>();
        if (arp != 0) {
            if (handleArp(arp)) {
                outgoingPduQueue.push(pduCopy);
            }
            return;
        }

        Tins::IP * ip = pduCopy->find_pdu<Tins::IP>();
        if (ip != 0) {
            if (handleIp(ip)) {
                outgoingPduQueue.push(pduCopy);
            }
        }
    }

    bool NatMap::handleIp(Tins::IP * ip) {

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

    bool NatMap::handleArp(Tins::ARP * /*arp*/) {
        return false;
    }

    bool NatMap::isForMeOrFromMeIp(const Tins::IP* ip) {
        return isForMeOrFromMeIp(ip, ranges);
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
            if (range.calcIpRange(true).contains(ipAddr)) {
                return true;
            }
        }

        return false;
    }
}
