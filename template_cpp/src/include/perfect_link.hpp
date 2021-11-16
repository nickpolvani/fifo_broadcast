#ifndef PERFECT_LINK_H
#define PERFECT_LINK_H

#include "thread_safe_queue.hpp"
#include "packet.hpp"
#include <map>
#include <set>
#include "udp_scocket.hpp"
#include "packet_proc_id.hpp"
#include "best_effort_broadcast.hpp"
#include <mutex>
#include "outbox.hpp"
#include "parser.hpp"
#include <thread>
#include <chrono>

using namespace packet;

class BestEffortBroadcast;

class PerfectLink{
    private:
        friend class OutBox;

        unsigned long int process_id; //id of this process

        UDPSocket udp_socket;

        ThreadSafeQueue<Packet> received_packets;

        std::map<long unsigned int, sockaddr_in> * host_addresses;

        // delivered[process_id][source_id] returns the set of sequence numbers of packets delivered
        // that were received from process_id, with original sender source_id
        std::map<long unsigned int, std::map<long unsigned int, std::set<long unsigned int>>> delivered;

        // queue of packets that have to be added to OutBox
        ThreadSafeQueue<Packet_ProcId> packets_to_send;

        // queue of acks to be sent, not added to outbox, but sent as soon as sender_lock is available 
        ThreadSafeQueue<Packet_ProcId> acks_to_send;

        // Higher abstraction, perfect link delivers to beb
        BestEffortBroadcast* beb = NULL;

        // used to synchronize Thread that sends acks and Thread that sends normal packets
        std::mutex sender_lock;

        // keeps non-ack messages that are sent periodically, messages are removed when an
        // ack is received
        OutBox outbox; 

        // sends received messages to higher abstraction (BestEffortBroadcast) when appropriate
        void deliver(Packet p);

        // waits to receive messages and populates queue received_packets (1 Thread always listening)
        void listen();

        // consumes queue of acks to send, 1 Thread always sending when sender_lock is available
        void sendAcks();

        // send Packets periodically from the OutBox, 1 Thread periodically executing this function
        void sendPackets();

        /* 1) If queue of arrived packets is non empty, take out first packet
            if the packet received was a normal message:
                2-a) populates acks_queue with the ack to be sent to the sender process
                3-a) delivers the packet on the head of the queue if it was not already delivered
            if the packet received was an ack:
                2-b) remove corresponding packet from outbox
        */
        void processArrivedMessages();
        
        // 1 Thread that consumes packets_to_send and populates outbox
        void addPacketsToOutBox();


    public:

        // hosts contains a mapping process_id, socket address
        // port_num: port number on network byte order
        PerfectLink(unsigned long int process_id, std::map<long unsigned int, sockaddr_in>* i_host_addresses, unsigned short port_num);


        void setBEB(BestEffortBroadcast * i_beb){
            beb = i_beb;
        }

        // contains running threads of Perfect Link (listen, sendAcks, processArrivedMessages, sendPackets, addPacketsToOutBox)
        std::vector<std::thread *> threads; 
        
        // starts all threads and populates variable threads
        void start();

        // called by higher abstraction to send reliably 
        // a packet (eventually the packet is delivered by the PerfectLink of the receiver)
        void send(Packet_ProcId packet_dest){
            packets_to_send.push(packet_dest);
        }

        void closeSocket(){
            udp_socket.closeConnection();
        }


        
};


#endif