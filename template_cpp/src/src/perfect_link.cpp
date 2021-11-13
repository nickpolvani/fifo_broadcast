#include "perfect_link.hpp"


// hosts contains a mapping process_id, socket address
// port_num: port number on network byte order
PerfectLink::PerfectLink(unsigned long int i_process_id, BestEffortBroadcast* i_beb, std::vector<Parser::Host> hosts, unsigned short port_num) :
    process_id(i_process_id), udp_socket(port_num, -1), beb(i_beb), outbox(NULL)
{
    //populate host_addresses
    for (Parser::Host host : hosts){
        sockaddr_in host_addr;
        host_addr.sin_family = AF_INET;
        in_addr ip;
        ip.s_addr = host.ip;
        host_addr.sin_addr = ip;
        host_addr.sin_port = host.port;
        host_addresses[host.id] = host_addr;
    }
    outbox.host_addresses = &host_addresses;

}


inline void PerfectLink::deliver(Packet p){
    beb -> deliver(p);
}

void PerfectLink::listen(){
    while(true){
        Packet received = udp_socket.receivePacket();
        received_packets.push(received);
    }
}

/* 1) If queue of arrived packets is non empty, take out first packet
    if the packet received was a normal message:
        2-a) populates acks_queue with the ack to be sent to the sender process
        3-a) delivers the packet on the head of the queue if it was not already delivered
    if the packet received was an ack:
        2-b) remove corresponding packet from outbox
*/
void PerfectLink::processArrivedMessages(){
    Packet received = received_packets.pop();
    if (received.is_ack){
        outbox.removePacket(received.process_id, received.source_id, received.packet_seq_num);
    }
    else{
        Packet ack = Packet::createAck(process_id, received.source_id, received.packet_seq_num);
        long unsigned int dest_proc_id = received.process_id;
        Packet_ProcId ack_and_dest = Packet_ProcId(ack, dest_proc_id);
        acks_to_send.push(ack_and_dest);

        // deliver if not already delivered
        if (delivered[received.process_id][received.source_id].count(received.packet_seq_num) == 0){
            delivered[received.process_id][received.source_id].insert(received.packet_seq_num);
            deliver(received);
        }
    }
}



void PerfectLink::sendAcks(){
    while (true){
        Packet_ProcId ack_dest = acks_to_send.pop();
        sockaddr_in dest_addr = host_addresses[ack_dest.dest_proc_id];
        sender_lock.lock();
        udp_socket.send(ack_dest.packet, reinterpret_cast<sockaddr*> (&dest_addr));
        sender_lock.unlock();
    }
}

void PerfectLink::sendPackets(){
    while(true){
        sender_lock.lock();
        outbox.sendPackets(&udp_socket);
        sender_lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void PerfectLink::addPacketsToOutBox(){
    while(true){
        Packet_ProcId cur_packet_dest = packets_to_send.pop();
        outbox.addPacket(cur_packet_dest);
    }
}

// when this function is called the current thread stops executing and waits for
// the spawned threads to finish (which is when the whole program stops)
void PerfectLink::start(){
    std::thread * listener = new std::thread([this] {this -> listen();});
    std::thread * ack_sender = new std::thread([this] {this -> sendAcks();});
    std::thread * processor = new std::thread([this] {this -> processArrivedMessages();});
    std::thread * packet_sender = new std::thread([this] {this -> sendPackets();});
    std::thread * outbox_dealer = new std::thread([this] {this -> addPacketsToOutBox();});
    
    threads.push_back(listener);
    threads.push_back(ack_sender);
    threads.push_back(processor);
    threads.push_back(packet_sender);
    threads.push_back(outbox_dealer);
}
