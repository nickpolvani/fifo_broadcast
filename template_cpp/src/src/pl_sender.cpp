#include "pl_sender.hpp"


void PLSender::sendPackets(){
    while(true){
        // get current packet
        DEBUG_MSG("trying to get packet to send from queue\n");
        DEBUG_MSG("size of queue is: " << packets_to_send.getSize() << "\n");
        Packet_ProcId curr_packet_addr = packets_to_send.pop();
        DEBUG_MSG("Got packet from queue\n");
        Packet curr_packet = curr_packet_addr.packet;
        long unsigned int dest_proc_id = curr_packet_addr.dest_proc_id;
        sockaddr_in dest_addr = process_controller->host_addresses[curr_packet_addr.dest_proc_id];

        // let other thread process packet sent
        packets_sent.push(curr_packet_addr.packet);

        // send packet
        bool ack_received = false;
        while(!ack_received){
            udp_socket.send(curr_packet_addr.packet, reinterpret_cast<sockaddr*>(&dest_addr));
            DEBUG_MSG("packet sent, packet seq num: " << curr_packet_addr.packet.packet_seq_num << " first message seq num: " << curr_packet_addr.packet.first_msg_seq_num << "\n");
            try{
                Packet ack = udp_socket.receivePacket();
                // check the ack corresponds to the packet sent
                if (ack.process_id == dest_proc_id && ack.packet_seq_num == curr_packet.packet_seq_num){
                    ack_received = true;
                    DEBUG_MSG("ack received\n");
                }
            } catch(TimeoutException&){  // ack not received in time, so resend packet
                continue;
            }
        }
    }
}


void PLSender::processSentPackets(){
    while(true){
        Packet packet = packets_sent.pop();
        DEBUG_MSG("Processing packet sent\n");
        process_controller->onPacketSent(packet);
    }
}


void PLSender::start(){
    std::thread * sender =  new std::thread([this] {this -> sendPackets();});
    std::thread * processor = new std::thread([this]{this -> processSentPackets();});
    threads.push_back(sender);
    threads.push_back(processor);
}
