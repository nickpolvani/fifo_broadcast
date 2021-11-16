#ifndef BEST_EFFORT_BROADCAST_H
#define BEST_EFFORT_BROADCAST_H

#include "packet.hpp"
#include <assert.h>
#include "perfect_link.hpp"
#include "uniform_reliable_broadcast.hpp"
#include "parser.hpp"

using namespace packet;

class PerfectLink;
class UniformReliableBroadcast;

class BestEffortBroadcast{
    private:
        PerfectLink* perfect_link;
        // packets that have source_id = this process id
        ThreadSafeQueue<Packet> packets_to_broadcast;
        // packets that have source_id != this process id, and have to be retransmitted
        ThreadSafeQueue<Packet> packets_to_re_broadcast;

        ThreadSafeQueue<Packet> packets_to_deliver;

        UniformReliableBroadcast* urb;

        // consumes packets_to_deliver queue and executes BEBDeliver defined 
        // by UniformReliableBroadcast (1 permanent Thread)
        void deliver();

        // consumes first packets_to_re_broadcast, then packets_to_broadcast
        // 1 permanent Thread
        void broadcast();

        std::vector<Parser::Host> hosts;


    public:
        BestEffortBroadcast( PerfectLink* pl, std::vector<Parser::Host> i_hosts): 
           perfect_link(pl), hosts(i_hosts){}; 


        void setURB(UniformReliableBroadcast* i_urb){
            urb = i_urb;
        }

        // contains pointers to running threads (deliver, broadcast)
        std::vector<std::thread *> threads; 

        void deliver(Packet p){
            packets_to_deliver.push(p);
        }

        void broadcast(Packet p){
            packets_to_broadcast.push(p);
        }

        void re_broadcast(Packet p){
            packets_to_re_broadcast.push(p);
        }

        // starts threads (deliver, broadcast) and adds them to threads
        void start();


};

#endif