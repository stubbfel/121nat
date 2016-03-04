/*
 * File:   PduSniffer.h
 * Author: dev
 *
 * Created on 29. September 2015, 21:29
 */

#ifndef PDUSNIFFER_H
#define	PDUSNIFFER_H

#include "natmap.h"
#include <tins/tins.h>
#include <thread>

namespace otonat {

    class PduSniffer {
    public:
        typedef std::vector<Tins::Sniffer *> SnifferList;

        PduSniffer(NatMap * map);
        PduSniffer(const PduSniffer& orig);
        PduSniffer& operator=(const PduSniffer& rhs);
        virtual ~PduSniffer();
        void SniffInterface(const Tins::NetworkInterface & interface);
        std::thread * SniffInterfaceInNewThread(const Tins::NetworkInterface & interface);
        void Start();
        void Stop();
    private:
        SnifferList snifferList;
        NatMap * map;
        Tins::SnifferConfiguration config;
        bool isRunnig;
        bool sniffPdu(const Tins::PDU &pdu);
        NatMap::NetworkInterfaceId interfaceId;
    };

}

#endif	/* PDUSNIFFER_H */

