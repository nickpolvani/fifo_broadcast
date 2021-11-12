
#include "process_controller.hpp"
#include <string>
#include <exception>

ProcessController::ProcessController(long unsigned int id, Parser parser): process_id(id)
{
    //populate host_addresses
    for (Parser::Host host : parser.hosts()){
        sockaddr_in host_addr;
        host_addr.sin_family = AF_INET;
        in_addr ip;
        ip.s_addr = host.ip;
        host_addr.sin_addr = ip;
        host_addr.sin_port = host.port;
        host_addresses[host.id] = host_addr;
    }
    config_file.open(parser.configPath());
    if (config_file.bad()){
        std::cerr << "Error: could not open config file\n";
        exit(EXIT_FAILURE);
    }
    config_file >> num_messages;
    config_file >> receiver_id;
    config_file.close();

    output_file.open(parser.outputPath());
    if (output_file.bad()){
        std::cerr << "Error: could not open output file\n";
        exit(EXIT_FAILURE);
    }

}


void ProcessController::listen(){
    unsigned short port = host_addresses[process_id].sin_port;
    pl_receiver = new PLReceiver(port, this);
    pl_receiver->start();
    for (auto thread: pl_receiver->threads){
        thread->join();
    }
}

void ProcessController::sendPackets(){
    // create Perfect Link Sender
    unsigned short port = host_addresses[process_id].sin_port;
    pl_sender = new PLSender(port, this, 2);
    pl_sender->start();

    DEBUG_MSG("number of packets to send: " << num_messages << "\n");
    unsigned long int packet_seq_num = 0;
    // create and send Packets
    Packet curr_packet(process_id, packet_seq_num);
    for (long unsigned int i = 1; i <= num_messages; i++){
        Message curr_message(std::to_string(i));

        // packet is full
        if (!curr_packet.canAddMessage(curr_message)){  
            DEBUG_MSG("trying to add packet to queue, messages processed:" << i << "\n");
            pl_sender->addPacket(Packet_ProcId(curr_packet, receiver_id));
            packet_seq_num++;
            curr_packet = Packet(process_id, packet_seq_num);
        }
        curr_packet.addMessage(curr_message);

        // last message, so send packet even if not full
        if (i == num_messages){ 
            pl_sender->addPacket(Packet_ProcId(curr_packet, receiver_id));
            packet_seq_num++;
        }
    }
    for (auto thread: pl_sender->threads){
        thread->join();
    }
}


void ProcessController::onPacketDelivered(packet::Packet p){
    std::string out_content;
    // packet contains multiple messages
    for (long unsigned int i = p.first_msg_seq_num; i < p.first_msg_seq_num + p.getNumMessages(); i++){
        out_content += "d " + std::to_string(p.process_id) + " " +  std::to_string(i) + "\n";
    }
    output_file << out_content;
}


void ProcessController::onPacketSent(packet::Packet p){
    std::string out_content;
    for (long unsigned int i = p.first_msg_seq_num; i < p.first_msg_seq_num + p.getNumMessages(); i++){
        out_content += "b " + std::to_string(i) + "\n";
    }
    output_file << out_content;
}


void ProcessController::stopProcess(){
    if (pl_receiver != NULL){
        pl_receiver->closeSocket();
        std::cout << "Closed socket for Receiver";
    }
    if (pl_sender != NULL){
        pl_sender->closeSocket();
        std::cout << "Closed socket for Sender";
    }
    std::cout.flush();
    output_file.flush();
    output_file.close();
} 
