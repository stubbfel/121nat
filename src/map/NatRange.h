#ifndef NATRANGE_H
#define	NATRANGE_H

#include <tins/tins.h>
namespace otonat {

    class NatRange {
    public:
        NatRange(Tins::NetworkInterface interface, Tins::IPv4Address rangeIpAddr, Tins::IPv4Address netmask);
        NatRange(const NatRange& orig);
        NatRange& operator=(const NatRange& right);
        virtual ~NatRange();

        Tins::NetworkInterface interface;
        Tins::IPv4Address rangeIpAddr;
        Tins::IPv4Address rangeNetmask;

        Tins::IPv4Range calcIpRange(const bool usedInterfaceIpSetting) const;
        static Tins::IPv4Range calcIpRange(const Tins::IPv4Address &ip, const Tins::IPv4Address & netmask);
        static Tins::IPv4Range calcIpRange(const Tins::NetworkInterface::Info & interfaceInfo);
        static Tins::IPv4Range calcIpRange(const Tins::NetworkInterface & interface);
        static Tins::IPv4Range calcIpRange(const NatRange & natRange);
        static Tins::IPv4Range calcIpRange(const NatRange & natRange, const bool usedInterfaceIpSetting);

        Tins::IPv4Address mapIPv4Addres(const Tins::IPv4Address & originIp, const bool usedInterfaceIpSetting) const;
        static Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & originIp, const Tins::IPv4Address & netIp, const Tins::IPv4Address & netmask);
        static Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & originIp, const Tins::NetworkInterface::Info & interfaceInfo);
        static Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & originIp, const Tins::NetworkInterface & interface);
        static Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & originIp, const NatRange & natRange);
        static Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & originIp, const NatRange & natRange, const bool usedInterfaceIpSetting);

    private:
    };
}
#endif	/* NATRANGE_H */

