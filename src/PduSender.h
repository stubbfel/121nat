/* 
 * File:   PduSender.h
 * Author: dev
 *
 * Created on 16. September 2015, 20:31
 */

#ifndef PDUSENDER_H
#define	PDUSENDER_H

#include "map/natmap.h"
#include <tins/tins.h>
#include <thread>
#include <list>

namespace otonat {

    class PduSender {
    public:
        typedef std::list<unsigned short int> ChecksumList;
        
        PduSender(NatMap * map);
        PduSender(const PduSender& orig);
        PduSender& operator=(const PduSender& rhs);
        virtual ~PduSender();
        NatMap * map;
        ChecksumList checksumList;
        void SendPdusFromQueue();
        
         std::thread * SendPdusFromQueueThread();
    private:
        Tins::PacketSender sender;
    };
}

#endif	/* PDUSENDER_H */

