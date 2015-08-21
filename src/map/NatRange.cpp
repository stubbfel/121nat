#include "NatRange.h"
namespace otonat {

    NatRange::NatRange(Tins::NetworkInterface interface, Tins::IPv4Address rangeIpAddr, Tins::IPv4Address netmask) : interface(interface), rangeIpAddr(rangeIpAddr), rangeNetmask(netmask) {
    }

    NatRange& NatRange::operator=(const NatRange& right) {
        // Check for self-assignment!
        if (this == &right) // Same object?
            return *this; // Yes, so skip assignment, and just return *this.

        interface = right.interface;
        rangeIpAddr = right.rangeIpAddr;
        rangeNetmask = right.rangeNetmask;
        return *this;
    }

    NatRange::NatRange(const NatRange& other) : interface(other.interface), rangeIpAddr(other.rangeIpAddr), rangeNetmask(other.rangeNetmask) {
    }

    NatRange::~NatRange() {
    }

    Tins::IPv4Range NatRange::calcIpRange(const Tins::IPv4Address &ip, const Tins::IPv4Address & netmask) {
        return Tins::IPv4Range::from_mask(ip, netmask);
    }

    Tins::IPv4Range NatRange::calcIpRange(const Tins::NetworkInterface::Info & interfaceInfo) {
        return calcIpRange(interfaceInfo.ip_addr, interfaceInfo.netmask);
    }

    Tins::IPv4Range NatRange::calcIpRange(const Tins::NetworkInterface & interface) {
        return calcIpRange(interface.info());
    }

    Tins::IPv4Range NatRange::calcIpRange(const NatRange & natRange) {
        return calcIpRange(natRange.rangeIpAddr, natRange.rangeNetmask);
    }

    Tins::IPv4Range NatRange::calcIpRange(const NatRange& natRange, const bool usedInterfaceIpSetting) {
        if (usedInterfaceIpSetting) {
            return calcIpRange(natRange.interface);
        }

        return calcIpRange(natRange);
    }

    Tins::IPv4Range NatRange::calcIpRange(const bool usedInterfaceIpSetting) const {
        return calcIpRange(*this, usedInterfaceIpSetting);
    }

    Tins::IPv4Address NatRange::mapIPv4Address(const Tins::IPv4Address& originIp, const Tins::NetworkInterface& interface) {
        return mapIPv4Address(originIp, interface.info());
    }

    Tins::IPv4Address NatRange::mapIPv4Address(const Tins::IPv4Address & originIp, const Tins::NetworkInterface::Info & interfaceInfo) {
        return mapIPv4Address(originIp, interfaceInfo.ip_addr, interfaceInfo.netmask);
    }

    Tins::IPv4Address NatRange::mapIPv4Address(const Tins::IPv4Address& originIp, const NatRange& natRange) {
        return mapIPv4Address(originIp, natRange.rangeIpAddr, natRange.rangeNetmask);
    }

    Tins::IPv4Address NatRange::mapIPv4Address(const Tins::IPv4Address& originIp, const Tins::IPv4Address& netIp, const Tins::IPv4Address& netmask) {
        const uint32_t networkStartIp = netIp & netmask;
        const uint32_t resultIp = (originIp & ~netmask) | networkStartIp;
        return Tins::IPv4Address(resultIp);
    }

    Tins::IPv4Address NatRange::mapIPv4Address(const Tins::IPv4Address& originIp, const NatRange& natRange, const bool usedInterfaceIpSetting) {
        if (usedInterfaceIpSetting) {
            return mapIPv4Address(originIp, natRange.interface);
        }

        return mapIPv4Address(originIp, natRange);
    }

    Tins::IPv4Address NatRange::mapIPv4Addres(const Tins::IPv4Address& originIp, const bool usedInterfaceIpSetting) const {
        return mapIPv4Address(originIp, *this, usedInterfaceIpSetting);
    }
}