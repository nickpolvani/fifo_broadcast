#include "pl_receiver.hpp"
#include "parser.hpp"
#include "process_controller.hpp"
#include <thread>
#include <iostream>



void PLReceiver::listen(){
    while(true){
        DEBUG_MSG("listening\n");
        Packet received = udp_socket.receivePacket();
        DEBUG_MSG("received packet: " << received.packet_seq_num << "\n");

        received_packets.push(received);
    }
}


void PLReceiver::sendAcks(){
    while(true){
        DEBUG_MSG("sendin acks\n");
        Packet_ProcId pack_addr = acks_queue.pop();
        sockaddr_in dest_addr = process_controller->host_addresses[pack_addr.dest_proc_id];
        Packet ack = pack_addr.packet;
        udp_socket.send(ack, reinterpret_cast<sockaddr *>(&dest_addr));
    }
}


void PLReceiver::processArrivedMessages(){
    while(true){
        Packet received = received_packets.pop();
        DEBUG_MSG("processing received packet\n");

        // schedule send ack
        DEBUG_MSG("creating ack " << received.packet_seq_num << "\n");

        Packet ack = Packet::createAck(process_controller->process_id, received.packet_seq_num);
        long unsigned int dest_process_id = received.process_id;
        Packet_ProcId ack_addr = Packet_ProcId(ack, dest_process_id);
        acks_queue.push(ack_addr);

        // deliver if not already delivered
        if (delivered[received.process_id].count(received.packet_seq_num) == 0){
            delivered[received.process_id][received.packet_seq_num] = true;
            process_controller->onPacketDelivered(received);
        }
    }
}


// when this function is called the current thread stops executing and waits for
// the spawned threads to finish (which is never)
void PLReceiver::start(){
    std::thread * listener = new std::thread([this] {this -> listen();});
    std::thread * sender = new std::thread([this] {this -> sendAcks();});
    std::thread * processor = new std::thread([this] {this -> processArrivedMessages();});
    
    threads.push_back(listener);
    threads.push_back(sender);
    threads.push_back(processor);
}