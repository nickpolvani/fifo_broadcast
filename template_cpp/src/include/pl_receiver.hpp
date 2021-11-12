#ifndef PL_RECEIVER_H
#define PL_RECEIVER_H

#include <map>
#include "packet.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "thread_safe_queue.hpp"
#include "process_controller.hpp"
#include "udp_scocket.hpp"
#include "packet_proc_id.hpp"

using namespace packet;

class ProcessController;



/* Perfect Link Receiver */
class PLReceiver{
    private:
        unsigned short port;
        // underlying UDP socket to receive packets / send acks
        UDPSocket udp_socket; 

        // structure to keep track of delivered packets (packet content not stored), key to outer map is process_id,
        // key to inner map is the sequence number of the packet, accessed only by 
        // processArrivedMessages, it does not contain any value, I decided to use map for the
        // inner structure to have logarithmic access to the sequence numbers
        std::map<long unsigned int, 
                    std::map<long unsigned int, bool>> delivered;


        // queue of acks to be sent
        ThreadSafeQueue<Packet_ProcId> acks_queue; 

        // used to deal with delivered/sent packets calling onPackageDelivered()/onPackageSent()
        ProcessController * process_controller;

        // queue populated by Thread executing listen(), 
        // emptied by Thread executing processArrivedMessages
        ThreadSafeQueue<Packet> received_packets;

        void deliver(Packet p);

        // waits to receive messages and populates queue received_packets (1 Thread always listening)
        void listen();

        // send confirmation from acks_queue after receiving a message
        void sendAcks();

        /* 1) If queue of arrived packets is non empty, take out first packet
           2) populates acks_queue with the ack to be sent to the sender process
           3) delivers the packet on the head of the queue if it was not already delivered
        */
        void processArrivedMessages();
        
    public:
        // contains running threads of the PLReceiver (listener, sender, processor)
        std::vector<std::thread *> threads;

        //port must be given in network byte order
        
        PLReceiver(unsigned short i_port, ProcessController * i_controller): 
                port(i_port), udp_socket(i_port, -1), process_controller(i_controller){}

        // default copy constructor

        // spawns 3 Threads to execute listen(), processArrivedMessages(), sendAcks()
        // stored in threads
        void start();

        
        void closeSocket(){
            udp_socket.closeConnection();
        }

};

#endif