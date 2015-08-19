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

    NatMap::NatMap(const NatMap& other) : interfaces(other.interfaces) {
        operator=(other);
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
            handleArp(arp);
            return;
        }

        Tins::IP * ip = pduCopy->find_pdu<Tins::IP>();
        if (ip != 0) {
            handleIp(ip);
        }
    }

    void NatMap::handleIp(Tins::IP * ip) {
        IpAdressMap::iterator transIpIter = transMap.find(ip->dst_addr());
        if (transIpIter != transMap.end()) {
            // handle know traslation ip
            TranslateIpPacket(ip, transIpIter->second);
        } else {
            // Determine Traslation Ip
        }
    }

    void NatMap::TranslateIpPacket(Tins::IP * ip, const Tins::IPv4Address & transIp) {
        // copy ip packet, for modifaktion
        Tins::IP * modifyIp = ip->clone();

        // set translated dst address
        modifyIp->dst_addr(transIp);

        // translate src adress
        const Tins::IPv4Address & originSrc = ip->src_addr();
        IpAdressMap::const_iterator transIpIter = transMap.find(originSrc);
        Tins::IPv4Address transSrcAttr;
        if (transIpIter != transMap.end()) {
            // set translated src address
            transSrcAttr = transIpIter->second;
        } else {
            transSrcAttr = InsertOrUdpateTranslateIpAddress(originSrc, interfaces);
        }

        modifyIp->src_addr(transSrcAttr);
        outgoingPduQueue.push(modifyIp);
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, NetworkInterfaceList & interfaceList) {
        // calc translated ip address for first up and not same interfaces
        for (const Tins::NetworkInterface & interface : interfaceList) {
            // insert or update translated ip address
            const Tins::NetworkInterface::Info & interfaceInfo = interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            Tins::IPv4Range range = calcIpRange(interfaceInfo); //networkInterfaceIpRangeMap[interfaceInfo.ip_addr];
            if (range.contains(originIp)) {
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

    void NatMap::handleArp(Tins::ARP * /*arp*/) {
        return;
    }
}
