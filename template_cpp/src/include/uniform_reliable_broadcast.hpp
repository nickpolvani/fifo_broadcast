#ifndef URB_H
#define URB_H

#include "packet.hpp"
#include <map>
#include <set>
#include "best_effort_broadcast.hpp"
#include <mutex>
#include "fifo_broadcast.hpp"
#include "thread_safe_queue.hpp"


using namespace packet;

class BestEffortBroadcast;
class FifoBroadcast;

class UniformReliableBroadcast{

    private:
        // delivered[source_id] returns the set of sequence numbers of delivered packets
        std::map<long unsigned int, std::set<long unsigned int>> delivered;

        // called by BEBDeliver(), broadcast() to access pending
        std::mutex pending_mutex;

        // pending[source_id][seq_num] returns corresponding packet
        std::map<long unsigned int, std::map<long unsigned int, Packet>> pending;
        
        // acks[source_id][seq_num] returns set of processes that have re-sent the packet with corresponding
        // source_id, packet_seq_num 
        std::map<long unsigned int, std::map<long unsigned int, std::set<long unsigned int>>> acks;

        int num_processes = -1;

        // lower level abstraction
        BestEffortBroadcast* beb = NULL;

        // higher level abstraction
        FifoBroadcast* fifo_broadcast = NULL;

        // contains packets that need to be delivered, populated by BEBDeliver 
        // that checks if packets can be URBDelivered
        ThreadSafeQueue<Packet> packets_to_deliver;

        // permanent thread that consumes p_to_check_for_delivery
        void tryDeliver();

        // checks if packet was retransmitted by a majority of processes (looking at number of acks)
        void canDeliver(long unsigned int source_id, long unsigned int seq_num);

        // permanent thread consuming packets_to_deliver
        void URBDeliver();        


    public:
        // deliver function invoked by Best Effort Broadcast 
        // (lower level abstraction)
        void BEBDeliver(Packet p);

        void setBEB(BestEffortBroadcast* i_beb){
            beb = i_beb;
        }

        void setFifoBroadcast(FifoBroadcast* i_fifo_broadcast){
            fifo_broadcast = i_fifo_broadcast;
        }

        void broadcast(Packet p);

};

#endif