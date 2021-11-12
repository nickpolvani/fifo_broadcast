#ifndef PL_SENDER_H
#define PL_SENDER_H

#include "thread_safe_queue.hpp"
#include "packet.hpp"
#include "udp_scocket.hpp"
#include "pl_receiver.hpp"
#include "process_controller.hpp"
#include "packet_proc_id.hpp"
#include "debug.h"

using namespace packet;



/* Perfect Link Sender 
   1 Thread sends message and waits for the ack of the receiver
   1 Thread processes sent messages
*/
class ProcessController;

class PLSender{
    private:
        // underlying udp socket to send packets, receive acks.
        // timeout is set on the port to wait for an ack, if timeout then re-send message
        UDPSocket udp_socket;

        // populated by process_controller, consumed by sendPackages()
        ThreadSafeQueue<Packet_ProcId> packets_to_send;

        // populated by sendPackets() and consumed by processSentPackets()
        ThreadSafeQueue<Packet> packets_sent;


        ProcessController * process_controller;

        // function to send packets, receive acks
        // for each packet, declare package as sent at first trial of sending it
        // send each packet infinitely many times until you receive ack from receiver
        void sendPackets();

        // function to process sent packets        
        void processSentPackets();

    public:
        // running threads: 1 to send packets/receive acks, 1 to process sent messages
        std::vector<std::thread *> threads;

        // creates UDPSocket with timeout on receive function
        PLSender(unsigned short port, ProcessController * controller, int timeout_sec) : 
                udp_socket(port, timeout_sec), process_controller(controller){}

        
        // add packet to queue of messages to send
        void addPacket(Packet_ProcId packet_addr){
            packets_to_send.push(packet_addr);
            DEBUG_MSG("successfully added packet to queue\n");
        }

        void start();

        void closeSocket(){
            udp_socket.closeConnection();
        }

};


#endif