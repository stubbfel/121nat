#ifndef NATMAP_H
#define NATMAP_H

#include <vector>
#include <map>
#include <queue>
#include <tins/tins.h>
#include "NatRange.h"
namespace otonat {

    class NatMap {
    public:
        typedef std::vector<NatRange> NatRangeList;
        typedef std::queue<const Tins::PDU *> PduQueue;
        typedef std::pair<Tins::IPv4Address, Tins::IPv4Address> IPv4AddressEntry;
        typedef std::map<Tins::IPv4Address, Tins::HWAddress < 6 >> IpAdressMacMap;
        typedef std::map<Tins::IPv4Address, Tins::IPv4Address> IpAdressMap;

        NatMap() {
        }

        NatMap(NatRangeList rangeList);
        virtual ~NatMap();
        NatMap(const NatMap& other);
        NatMap& operator=(const NatMap& other);
        NatRangeList ranges;
        IpAdressMacMap arpMap;
        IpAdressMap transMap;
        PduQueue incommingPduQueue;
        PduQueue outgoingPduQueue;
        void handlePdu(const Tins::PDU * pdu);

    protected:

    private:
        bool handleIp(Tins::IP * ip);
        bool handleArp(Tins::ARP * arp);
        Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const NatRange & range);
        Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & transIp, NatRangeList & rangeList);
        void TranslateIpPacket(Tins::IP * ip, const Tins::IPv4Address & transIp);
        Tins::IPv4Address zeroIp;
        bool isForMeOrFromMeIp(const Tins::IP * ip);
        static bool isForMeOrFromMeIp(const Tins::IP * ip, const NatRangeList & rangeList);
        bool isIpInMyRanges(const Tins::IPv4Address & ipAddr);
        static bool isIpInMyRanges(const Tins::IPv4Address & ipAddr, const NatRangeList & rangeList);
    };
}


#endif // NATMAP_H
