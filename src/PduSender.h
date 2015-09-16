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

namespace otonat {

    class PduSender {
    public:
        PduSender(NatMap * map);
        PduSender(const PduSender& orig);
        PduSender& operator=(const PduSender& rhs);
        virtual ~PduSender();
        NatMap * map;
        
        void SendPdusFromQueue();
    private:
        Tins::PacketSender sender;
    };
}

#endif	/* PDUSENDER_H */

