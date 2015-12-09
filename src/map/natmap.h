#ifndef NATMAP_H
#define NATMAP_H

#include <vector>
#include <map>
#include <queue>
#include <tins/tins.h>
#include "NatRange.h"
#include <mutex>
#include <thread>

namespace otonat {
    static const Tins::IPv4Address zeroIp;
    
    class NatMap {
    public:
        typedef std::vector<NatRange> NatRangeList;
        typedef std::queue<const Tins::PDU *> PduQueue;
        typedef std::pair<Tins::IPv4Address, Tins::IPv4Address> IPv4AddressEntry;
        typedef std::map<Tins::IPv4Address, Tins::IPv4Address> IpAdressMap;
        typedef unsigned short int Checksum;
        typedef std::list<Checksum> ChecksumList;
        

        NatMap() {
        }

        NatMap(NatRangeList rangeList);
        virtual ~NatMap();
        NatMap(const NatMap& other);
        NatMap& operator=(const NatMap& other);
        NatRangeList ranges;
        IpAdressMap transMap;
        IpAdressMap reqIpMap;
        PduQueue incommingPduQueue;
        PduQueue outgoingPduQueue;
        void translate();
        std::thread * translateThread();
        void handlePdu(const Tins::PDU * pdu);
        void pushPduToIncommingPduQueue(const Tins::PDU * pdu);
        Tins::PDU * popPduIncommingPduQueue();
        void pushPduToOutgoingPduQueue(const Tins::PDU * pdu);
        Tins::PDU * popPduOutgoingPduQueue();
        void pushCheckSumToList(Checksum checksum);
        
        static Tins::PDU * popPduPduQueue(PduQueue & queue, std::mutex & mtx);
        static void pushPduToPduQueue(const Tins::PDU * pdu, PduQueue & queue, std::mutex & mtx);
        
    protected:

    private:
        bool handleIp(Tins::IP * ip, const Tins::PDU * originPDU);
        bool handleArp(Tins::ARP * arp);
        bool handleArpReq(Tins::ARP * arp);
        bool handleArpReply(Tins::ARP * arp);
        bool handleArpAndTranslateSenderIp(Tins::ARP * arp);
        void InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & transIp);
        Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const NatRange & range);
        Tins::IPv4Address InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & otherTransSameRangeIp, NatRangeList & rangeList);
        void TranslateIpPacket(Tins::IP * ip, const Tins::IPv4Address & transDstIp);
        const Tins::IPv4Address TranslateArpIp(const Tins::IPv4Address & arpIp);
        bool isForMeOrFromMeIp(const Tins::IP * ip) const;
        bool isForMeOrFromMeArp(const Tins::ARP * arp) const;
        static bool isForMeOrFromMeIp(const Tins::IP * ip, const NatRangeList & rangeList);
        bool isIpInMyRanges(const Tins::IPv4Address & ipAddr) const;
        static bool isIpInMyRanges(const Tins::IPv4Address & ipAddr, const NatRangeList & rangeList);
        void SendTranslatedArpRequest(const Tins::ARP * arp);
        void popCheckSumToList(Checksum checksum);
        bool containChecksumList(Checksum checksum);
        std::mutex incommingQueueMutex;
        std::mutex outgoingQueueMutex;
        std::mutex checksumListMutex;
        ChecksumList checksumList;
    };
}


#endif // NATMAP_H
