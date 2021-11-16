#ifndef FIFO_BROADCAST_H
#define FIFO_BROADCAST_H

#include "packet.hpp"
#include <map>
#include "uniform_reliable_broadcast.hpp"
#include "thread_safe_queue.hpp"
#include <thread>
#include "parser.hpp"
#include "process_controller.hpp"
#include <mutex> 
#include <condition_variable>

using namespace packet;

class UniformReliableBroadcast;
class ProcessController;

class FifoBroadcast{

    private:

        std::mutex mutex;
        // condition variable to wait on broadcast until packet is delivered to not flood the network
        std::condition_variable broadcast_cv;

        bool can_broadcast = true;   

        // current packet sequence number for this process
        long unsigned int cur_seq_num = 0;
        
        // pending[source_id][seq_num] returns the corresponding packet (process_id does not matter in packets)
        std::map<long unsigned int, std::map<long unsigned int, Packet>> pending;

        // next[source_id] returns the next expected packet sequence number from process with id source_id
        std::map<long unsigned int, long unsigned int> next;

        // lower level abstraction
        UniformReliableBroadcast * urb = NULL;

        ThreadSafeQueue<Packet> packets_to_deliver;

        // permanent Thread that consumes packets_to_deliver
        void fifoDeliver();

        //  Thread creating packets containing messages with
        // seq num from 1 to num_messages included, and broadcast them to other processes
        void broadcast();

        // number of messages to broadcast
        long unsigned int num_messages;

        ProcessController * process_controller;


    public:
        // initializes next, and sets num_messags
        FifoBroadcast(ProcessController * process_controller, long unsigned int i_num_messages, std::vector<Parser::Host> hosts);


        // contains threads that execute fifoDeliver, broadcast
        std::vector<std::thread *> threads;

        // invoked by UniformReliableBroadcast when delivering
        void URBDeliver(Packet p);


        void setUrb(UniformReliableBroadcast * i_urb){
            urb = i_urb;
        }

        // begins execution of fifoDeliver, broadcast, adds thread pointers to
        // threads
        void start();
};

#endif