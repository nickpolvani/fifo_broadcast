#ifndef OUTBOX_H
#define OUTBOX_H

#include <map>
#include "packet.hpp"
#include <mutex>
#include <condition_variable>
#include "packet_proc_id.hpp"
#include "udp_scocket.hpp"
#include <assert.h>

using namespace packet;


typedef std::map<long unsigned int, std::map<long unsigned int, Packet>> SourceId_2_SeqNum_2_Packet;


class OutBox{
    friend class PerfectLink;

    private:
        size_t curr_size;
        size_t max_size = 500;
        //condition variables for add operation
        std::condition_variable cv_add;  
        std::mutex mutex;  // lock for the outbox

        // packets kept in the outbox
        // packet.process_id is the process owning the outbox, 
        // so it is the same for all packets
        std::map<long unsigned int, // destination process id
                    std::map<long unsigned int, // source process id
                        std::map<long unsigned int, // packet sequence number
                            Packet>>> packets;

        std::map<long unsigned int, sockaddr_in> * host_addresses;

    public:

        explicit OutBox(std::map<long unsigned int, sockaddr_in> * host_addresses) :
            host_addresses(host_addresses){}

        /* adds packet to outbox, if it is full
           wait until some other thread (consumer) removes a packet
        */
        void addPacket(Packet_ProcId const pack_and_dest);

        // returns true if successfully removed packet, false if there was not
        // the specified packet, waits only to own the lock of the outbox
        bool removePacket(unsigned long int dest_proc_id, unsigned long int source_id, unsigned long int seq_num);


        void sendPackets(UDPSocket * udp_socket);

        void debug();

};


#endif