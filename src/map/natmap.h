#ifndef NATMAP_H
#define NATMAP_H

#include <vector>
#include <tins/tins.h>

namespace otonat
{
class NatMap
{
public:
    NatMap();
    NatMap(std::vector<Tins::NetworkInterface> interfaceList);
    virtual ~NatMap();
    NatMap(const NatMap& other);
    NatMap& operator=(const NatMap& other);
    std::vector<Tins::NetworkInterface> interfaces;
    static const Tins::IPv4Address mapIPv4Address(const Tins::IPv4Address & ip, const Tins::NetworkInterface::Info & interfaceInfo);

protected:

private:

};
}


#endif // NATMAP_H
