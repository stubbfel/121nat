#include "natmap.h"

namespace otonat
{

NatMap::NatMap(): NatMap(Tins::NetworkInterface::all())
{
    //ctor
}

NatMap::NatMap(std::vector<Tins::NetworkInterface> interfaceList) : interfaces(interfaceList)
{
}

NatMap::~NatMap()
{
    //dtor
}

NatMap::NatMap(const NatMap& other) : interfaces(other.interfaces)
{
    //copy ctor
}

NatMap& NatMap::operator=(const NatMap& rhs)
{
    if (this == &rhs) return *this; // handle self assignment

    interfaces = rhs.interfaces;
    return *this;
}

const Tins::IPv4Address NatMap::mapIPv4Address(const Tins::IPv4Address & ip, const Tins::NetworkInterface::Info & interfaceInfo)
{
    const uint32_t & netmask = interfaceInfo.netmask;
    const uint32_t & interfaceIp = interfaceInfo.ip_addr;
    const uint32_t networkStartIp = interfaceIp & netmask;
    const uint32_t resultIp = (ip & ~netmask) | networkStartIp;
    return Tins::IPv4Address(resultIp);
}

}
