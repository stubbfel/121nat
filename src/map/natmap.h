#ifndef NATMAP_H
#define NATMAP_H

#include <vector>
#include <map>
#include <queue>
#include <tins/tins.h>

namespace otonat
{
class NatMap
{
public:
    typedef std::vector<Tins::NetworkInterface> NetworkInterfaceList;
    typedef std::queue<const Tins::PDU *> PduQueue;
    typedef std::pair<Tins::IPv4Address,Tins::IPv4Address> IPv4AddressEntry;
    typedef std::map<Tins::IPv4Address, Tins::HWAddress<6>> IpAdressMacMap;
    typedef std::map<Tins::IPv4Address, Tins::IPv4Address> IpAdressMap;


    NatMap();
    NatMap(NetworkInterfaceList interfaceList);
    virtual ~NatMap();
    NatMap(const NatMap& other);
    NatMap& operator=(const NatMap& other);
    NetworkInterfaceList interfaces;
    IpAdressMacMap arpMap;
    IpAdressMap transMap;
    PduQueue pduQueue;
    void handlePdu(const Tins::PDU * pdu);

    static const Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & ip, const Tins::NetworkInterface::Info & interfaceInfo);

    static const Tins::IPv4Range calcIpRange(const Tins::NetworkInterface::Info & interfaceInfo);

protected:

private:
    void handleIp(const Tins::IP * ip);
    void handleArp(const Tins::ARP * arp);
    Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address  & originIp, const Tins::NetworkInterface::Info & interfaceInfo);
    Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address  & originIp, NetworkInterfaceList & interfaceList);
    void TranslateIpPacket(const Tins::IP * ip, const Tins::IPv4Address & transIp);
    Tins::IPv4Address zeroIp;
};
}


#endif // NATMAP_H
