#include "natmap.h"
#include <iostream>
namespace otonat {

    NatMap::NatMap(NatRangeList rangeList) : ranges(rangeList) {
    }

    NatMap::~NatMap() {
        ranges.clear();
        transMap.clear();
        reqIpMap.clear();

        this->checksumListMutex.lock();
        this->checksumList.clear();
        this->checksumListMutex.unlock();
        this->incommingQueueMutex.lock();
        while (!incommingPduQueue.empty()) {
            incommingPduQueue.pop();
        }

        this->incommingQueueMutex.unlock();

        this->outgoingQueueMutex.lock();
        while (!outgoingPduQueue.empty()) {
            outgoingPduQueue.pop();
        }

        this->outgoingQueueMutex.unlock();
    }

    NatMap::NatMap(const NatMap& other) : ranges(other.ranges), transMap(other.transMap), reqIpMap(other.reqIpMap), incommingPduQueue(other.incommingPduQueue), outgoingPduQueue(other.outgoingPduQueue), checksumList(other.checksumList) {
    }

    NatMap& NatMap::operator=(const NatMap& rhs) {
        if (this == &rhs) return *this; // handle self assignment

        ranges = rhs.ranges;
        transMap = rhs.transMap;
        this->incommingQueueMutex.lock();
        incommingPduQueue = rhs.incommingPduQueue;
        this->incommingQueueMutex.unlock();
        this->outgoingQueueMutex.lock();
        outgoingPduQueue = rhs.outgoingPduQueue;
        this->outgoingQueueMutex.unlock();
        this->checksumListMutex.lock();
        this->checksumList = rhs.checksumList;
        this->checksumListMutex.unlock();
        return *this;
    }

    void NatMap::handlePdu(const Tins::PDU * pdu) {
        std::cout << "handle pdu:" << pdu->size() << std::endl;
        if (pdu == nullptr) {
            return;
        }

        Tins::PDU * pduCopy = pdu->clone();
        delete pdu;
        Tins::ARP * arp = pduCopy->find_pdu<Tins::ARP>();
        if (arp != nullptr) {
            if (handleArp(arp)) {
                this->pushPduToOutgoingPduQueue(pduCopy);
            }
            return;
        }

        Tins::IP * ip = pduCopy->find_pdu<Tins::IP>();
        if (ip != nullptr && handleIp(ip, pduCopy)) {

            this->pushPduToOutgoingPduQueue(pduCopy);
        }
    }

    bool NatMap::handleIp(Tins::IP * ip, const Tins::PDU * originPDU) {
        Checksum checksum = ip->checksum();

        if (containChecksumList(checksum)) {
            this->popCheckSumToList(checksum);
            return false;
        }

        if (isForMeOrFromMeIp(ip)) {
            return false;
        }

        const Tins::IPv4Address originDstIp = ip->dst_addr();
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
            const Tins::EthernetII * eth = originPDU->find_pdu<Tins::EthernetII>();
            if (eth != nullptr) {

                Tins::EthernetII fakeArp = Tins::ARP::make_arp_request(ip->dst_addr(), ip->src_addr(), eth->src_addr());
                SendTranslatedArpRequest(fakeArp.find_pdu<Tins::ARP>());
            }

            return false;
        }
    }

    void NatMap::TranslateIpPacket(Tins::IP * ip, const Tins::IPv4Address & transDstIp) {

        // set translated dst address
        ip->dst_addr(transDstIp);

        // translate src adress
        const Tins::IPv4Address originSrc = ip->src_addr();

        IpAdressMap::const_iterator transIpIter = transMap.find(originSrc);
        Tins::IPv4Address transSrcAttr;
        if (transIpIter != transMap.end()) {
            // set translated src address
            transSrcAttr = transIpIter->second;
        } else {

            transSrcAttr = InsertOrUdpateTranslateIpAddress(originSrc, transDstIp, ranges);
        }

        ip->src_addr(transSrcAttr);
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & otherTransSameRangeIp, NatRangeList & rangeList) {
        // calc translated ip address for first up and not same interfaces
        for (NatRange & range : rangeList) {
            // insert or update translated ip address
            const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            const Tins::IPv4Range ipRange = range.calcIpRange(true);
            if (!ipRange.contains(otherTransSameRangeIp)) {

                continue;
            }

            return InsertOrUdpateTranslateIpAddress(originIp, range);
        }

        return zeroIp;
    }

    Tins::IPv4Address NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const NatRange & range) {
        // translated ip address
        const Tins::IPv4Address transAddr = range.mapIPv4Addres(originIp, false);

        // insert forward translation
        InsertOrUdpateTranslateIpAddress(originIp, transAddr);

        // insert or update backward translation
        InsertOrUdpateTranslateIpAddress(transAddr, originIp);

        return transAddr;
    }

    void NatMap::InsertOrUdpateTranslateIpAddress(const Tins::IPv4Address & originIp, const Tins::IPv4Address & transIp) {
        IpAdressMap::const_iterator transIpIter = transMap.find(transIp);
        if (transIpIter != transMap.end()) {
            transMap[transIp] = originIp;
        } else {

            transMap.insert(IPv4AddressEntry(transIp, originIp));
        }
    }

    bool NatMap::handleArp(Tins::ARP * arp) {
        if (isForMeOrFromMeArp(arp)) {
            return false;
        }

        const Tins::IPv4Address & originDstIp = arp->target_ip_addr();
        if (!isIpInMyRanges(originDstIp)) {
            return false;
        }

        switch (arp->opcode()) {
            case Tins::ARP::REQUEST:
                return this->handleArpReq(arp);
            case Tins::ARP::REPLY:
                return this->handleArpReply(arp);

            default:
                return false;
        }
    }

    bool NatMap::isForMeOrFromMeIp(const Tins::IP* ip) const {

        return isForMeOrFromMeIp(ip, ranges);
    }

    bool NatMap::isForMeOrFromMeArp(const Tins::ARP * arp) const {
        Tins::IP fakeIp(arp->target_ip_addr(), arp->sender_ip_addr());

        return isForMeOrFromMeIp(&fakeIp);
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

    bool NatMap::isIpInMyRanges(const Tins::IPv4Address & ipAddr) const {

        return isIpInMyRanges(ipAddr, ranges);
    }

    bool NatMap::isIpInMyRanges(const Tins::IPv4Address & ipAddr, const NatRangeList & rangeList) {
        for (NatRange range : rangeList) {
            if (range.calcIpRange(false).contains(ipAddr)) {

                return true;
            }
        }

        return false;
    }

    bool NatMap::handleArpReq(Tins::ARP* arp) {
        const Tins::IPv4Address targetIp = arp->target_ip_addr();
        IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(targetIp);
        if (transSenderIpReqIter != reqIpMap.end()) {
            return false;
        }

        IpAdressMap::const_iterator transTargetIpIter = this->transMap.find(targetIp);
        if (transTargetIpIter == transMap.end()) {
            SendTranslatedArpRequest(arp);
            return false;
        }

        const Tins::IPv4Address transTargetIp = transTargetIpIter->second;
        arp->target_ip_addr(transTargetIp);
        IpAdressMap::const_iterator transSenderIpIter = this->transMap.find(arp->sender_ip_addr());
        if (transSenderIpIter != transMap.end()) {
            arp->sender_ip_addr(transSenderIpIter->second);
            this->reqIpMap.insert(IPv4AddressEntry(transTargetIp, targetIp));

            return true;
        }

        return handleArpAndTranslateSenderIp(arp);
    }

    bool NatMap::handleArpReply(Tins::ARP* arp) {
        const Tins::IPv4Address targetIp = arp->target_ip_addr();
        IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(targetIp);
        if (transSenderIpReqIter == reqIpMap.end()) {
            return false;
        }

        const Tins::IPv4Address transTargetIp = TranslateArpIp(targetIp);
        if (transTargetIp == zeroIp) {
            return false;
        }

        const Tins::IPv4Address senderIp = arp->sender_ip_addr();
        const Tins::IPv4Address transSenderIp = TranslateArpIp(senderIp);
        if (transSenderIp == zeroIp) {
            return false;
        }

        arp->target_ip_addr(transTargetIp);
        arp->sender_ip_addr(transSenderIp);

        return true;
    }

    const Tins::IPv4Address NatMap::TranslateArpIp(const Tins::IPv4Address & arpIp) {
        Tins::IPv4Address transArpIp;
        IpAdressMap::const_iterator transArpIpIter = this->transMap.find(arpIp);
        if (transArpIpIter == transMap.end()) {
            IpAdressMap::const_iterator transReqArpIpIter = this->reqIpMap.find(arpIp);
            if (transReqArpIpIter == reqIpMap.end()) {
                return zeroIp;
            }

            transArpIp = transReqArpIpIter->second;
            this->InsertOrUdpateTranslateIpAddress(arpIp, transArpIp);
            this->InsertOrUdpateTranslateIpAddress(transArpIp, arpIp);
            this->reqIpMap.erase(transReqArpIpIter);

            return transArpIp;
        }

        return transArpIpIter->second;
    }

    bool NatMap::handleArpAndTranslateSenderIp(Tins::ARP* arp) {
        for (NatRange & range : this->ranges) {
            const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            const Tins::IPv4Range ipRange = range.calcIpRange(true);
            if (!ipRange.contains(arp->target_ip_addr())) {
                continue;
            }

            const Tins::IPv4Address senderIp = arp->sender_ip_addr();
            const Tins::IPv4Address transSenderIp = range.mapIPv4Addres(senderIp, true);
            arp->sender_ip_addr(transSenderIp);

            IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(transSenderIp);
            if (transSenderIpReqIter == reqIpMap.end()) {

                this->reqIpMap.insert(IPv4AddressEntry(transSenderIp, senderIp));
            }

            return true;
        }

        return false;
    }

    void NatMap::SendTranslatedArpRequest(const Tins::ARP * arp) {
        const Tins::IPv4Address targetIp = arp->target_ip_addr();
        for (NatRange & range : this->ranges) {
            const Tins::NetworkInterface::Info & interfaceInfo = range.interface.info();
            if (!interfaceInfo.is_up) {
                continue;
            }

            const Tins::IPv4Range ipRange = range.calcIpRange(true);
            if (ipRange.contains(targetIp)) {
                continue;
            }

            const Tins::IPv4Address senderIp = arp->sender_ip_addr();
            const Tins::IPv4Address transSenderIp = range.mapIPv4Addres(senderIp, false);
            const Tins::IPv4Address transTargetIp = range.mapIPv4Addres(targetIp, false);
            IpAdressMap::const_iterator transSenderIpReqIter = this->reqIpMap.find(transSenderIp);
            if (transSenderIpReqIter == reqIpMap.end()) {
                this->reqIpMap.insert(IPv4AddressEntry(transSenderIp, senderIp));
            }

            IpAdressMap::const_iterator transTargetIpReqIter = this->reqIpMap.find(transTargetIp);
            if (transTargetIpReqIter == reqIpMap.end()) {

                this->reqIpMap.insert(IPv4AddressEntry(transTargetIp, targetIp));
            }

            const Tins::EthernetII transArp = Tins::ARP::make_arp_request(transTargetIp, transSenderIp, arp->sender_hw_addr());
            this->pushPduToOutgoingPduQueue(transArp.clone());
        }
    }

    void NatMap::pushPduToIncommingPduQueue(const Tins::PDU* pdu) {

        pushPduToPduQueue(pdu, this->incommingPduQueue, this ->incommingQueueMutex);
    }

    Tins::PDU * NatMap::popPduIncommingPduQueue() {

        return popPduPduQueue(this->incommingPduQueue, this->incommingQueueMutex);
    }

    void NatMap::pushPduToOutgoingPduQueue(const Tins::PDU* pdu) {

        pushPduToPduQueue(pdu, this->outgoingPduQueue, this->outgoingQueueMutex);
    }

    Tins::PDU * NatMap::popPduOutgoingPduQueue() {

        return popPduPduQueue(this->outgoingPduQueue, this->outgoingQueueMutex);
    }

    Tins::PDU * NatMap::popPduPduQueue(PduQueue & queue, std::mutex & mtx) {
        mtx.lock();
        if (queue.empty()) {
            mtx.unlock();
            return nullptr;
        }

        const Tins::PDU * result = queue.front();
        Tins::PDU * outPut = result->clone();
        queue.pop();
        mtx.unlock();
        delete result;

        return outPut;
    }

    void NatMap::pushPduToPduQueue(const Tins::PDU* pdu, PduQueue& queue, std::mutex& mtx) {

        mtx.lock();
        queue.push(pdu);
        mtx.unlock();
    }

    void NatMap::translate() {
        while (true) {
            Tins::PDU * pdu = this->popPduIncommingPduQueue();
            if (pdu == nullptr) {

                continue;
            }

            this->handlePdu(pdu);
        }
    }

    void NatMap::pushCheckSumToList(Checksum checksum) {
        checksumListMutex.lock();
        if (!containChecksumList(checksum)) {

            this->checksumList.push_back(checksum);
        }

        checksumListMutex.unlock();
    }

    void NatMap::popCheckSumToList(Checksum checksum) {
        checksumListMutex.lock();
        this->checksumList.remove(checksum);
        checksumListMutex.unlock();
    }

    bool NatMap::containChecksumList(Checksum checksum) {
        ChecksumList::const_iterator endIter = this->checksumList.end();
        ChecksumList::const_iterator beginIter = this->checksumList.begin();
        ChecksumList::const_iterator findIter = std::find(beginIter, endIter, checksum);
        return endIter != findIter;
    }

    std::thread * NatMap::translateThread() {
        std::thread * newThread = new std::thread(std::bind(&NatMap::translate, this));
        return newThread;
    }
}
