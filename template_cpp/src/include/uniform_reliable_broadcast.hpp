#ifndef URB_H
#define URB_H

#include "packet.hpp"
#include <map>
#include <set>
#include "best_effort_broadcast.hpp"
#include <mutex>
#include "fifo_broadcast.hpp"
#include "thread_safe_queue.hpp"
#include <thread>


using namespace packet;

class BestEffortBroadcast;
class FifoBroadcast;

class UniformReliableBroadcast{

    private:
        long unsigned int process_id;

        // delivered[source_id] returns the set of sequence numbers of delivered packets
        std::map<long unsigned int, std::set<long unsigned int>> delivered;

        // called by BEBDeliver(), broadcast() to access pending
        std::mutex pending_mutex;

        // pending[source_id] returns set of packet_seq_num of pending packets
        std::map<long unsigned int, std::set<long unsigned int>> pending;
        
        // acks[source_id][seq_num] returns set of processes that have re-sent the packet with corresponding
        // source_id, packet_seq_num 
        std::map<long unsigned int, std::map<long unsigned int, std::set<long unsigned int>>> acks;

        long unsigned int num_processes;

        // lower level abstraction
        BestEffortBroadcast* beb = NULL;

        // higher level abstraction
        FifoBroadcast* fifo_broadcast = NULL;

        // contains packets that need to be delivered, populated by BEBDeliver 
        // that checks if packets can be URBDelivered
        ThreadSafeQueue<Packet> packets_to_deliver;

        // checks if packet was retransmitted by a majority of processes (looking at number of acks)
        bool canDeliver(long unsigned int source_id, long unsigned int seq_num);

        // permanent thread consuming packets_to_deliver
        void URBDeliver();        


    public:

        UniformReliableBroadcast(long unsigned int i_process_id, long unsigned int i_num_processes) : 
        process_id(i_process_id), num_processes(i_num_processes){}

        
        // contains active threads (URBDeliver)
        std::vector<std::thread *> threads;

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

        // begins execution of threads and adds them to threads
        void start();

};

#endif