#include "natmap.h"

namespace otonat {

    NatMap::NatMap() : NatMap(Tins::NetworkInterface::all()) {
        //ctor
    }

    NatMap::NatMap(NetworkInterfaceList interfaceList) : interfaces(interfaceList) {
    }

    NatMap::~NatMap() {
        //dtor
    }

    NatMap::NatMap(const NatMap& other) : interfaces(other.interfaces), arpMap(other.arpMap), transMap(other.transMap), incommingPduQueue(other.incommingPduQueue), outgoingPduQueue(other.outgoingPduQueue), zeroIp(other.zeroIp) {
    }

    NatMap& NatMap::operator=(const NatMap& rhs) {
        if (this == &rhs) return *this; // handle self assignment

        interfaces = rhs.interfaces;
        arpMap = rhs.arpMap;
        transMap = rhs.transMap;
        incommingPduQueue = rhs.incommingPduQueue;
        outgoingPduQueue = rhs.outgoingPduQueue;
        return *this;
    }

    const Tins::IPv4Address NatMap::mapIPv4Address(const Tins::IPv4Address & ip, const Tins::NetworkInterface::Info & interfaceInfo) {
        const uint32_t & netmask = interfaceInfo.netmask;
        const uint32_t & interfaceIp = interfaceInfo.ip_addr;
        const uint32_t networkStartIp = interfaceIp & netmask;
        const uint32_t resultIp = (ip & ~netmask) | networkStartIp;
        return Tins::IPv4Address(resultIp);
    }

    const Tins::IPv4Range NatMap::calcIpRange(const Tins::NetworkInterface::Info & interfaceInfo) {
        return Tins::IPv4Range::from_mask(interfaceInfo.ip_addr, interfaceInfo.netmask);
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

        const Tins::IPv4Address originDstIp = ip->dst_addr();
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
            transSrcAttr = InsertOrUdpateTranslateIpAddress(originSrc, transIp, interfaces);
        }

        ip->src_addr(transSrcAttr);
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & transIp, NetworkInterfaceList & interfaceList) {
        // calc translated ip address for first up and not same interfaces
        for (const Tins::NetworkInterface & interface : interfaceList) {
            // insert or update translated ip address
            const Tins::NetworkInterface::Info & interfaceInfo = interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            Tins::IPv4Range range = calcIpRange(interfaceInfo); //networkInterfaceIpRangeMap[interfaceInfo.ip_addr];
            if (!range.contains(transIp)) {
                continue;
            }

            return InsertOrUdpateTranslateIpAddress(originIp, interfaceInfo);
        }

        return zeroIp;
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::NetworkInterface::Info & interfaceInfo) {
        // translated ip address
        Tins::IPv4Address transAddr = mapIPv4Address(originIp, interfaceInfo);

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

    bool NatMap::isForMeOrFromMeIp(const Tins::IP * ip) {
        const Tins::IPv4Address & srcAddr = ip->src_addr();
        const Tins::IPv4Address & dstAddr = ip->dst_addr();
        for (Tins::NetworkInterface interface : interfaces) {
            const Tins::IPv4Address & interfaceAddr = interface.info().ip_addr;
            if (srcAddr == interfaceAddr || dstAddr == interfaceAddr) {
                return true;
            }
        }
        return false;
    }
}
