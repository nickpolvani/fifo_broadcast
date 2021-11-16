#include "perfect_link.hpp"
#ifndef DEBUG
static bool debug_mode = false;
#else
static bool debug_mode = true;
#endif

// hosts contains a mapping process_id, socket address
// port_num: port number on network byte order
PerfectLink::PerfectLink(unsigned long int i_process_id, std::map<long unsigned int, sockaddr_in>* i_host_addresses, unsigned short port_num) :
    process_id(i_process_id), udp_socket(port_num, -1), host_addresses(i_host_addresses), outbox(NULL)
{
    outbox.host_addresses = i_host_addresses;
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
    while(true){
        Packet received = received_packets.pop();
        if (received.is_ack){
            DEBUG_MSG("PERFECT-LINK received ACK: source: " <<  received.source_id << " sender: " << received.process_id << " seq_num: "  << received.packet_seq_num);
            bool remove_success = outbox.removePacket(received.process_id, received.source_id, received.packet_seq_num);
            DEBUG_MSG("PERFECT-LINK removed packet from outbox: " << remove_success);

        }
        else{
            DEBUG_MSG("PERFECT-LINK received packet: source" <<  received.source_id << " sender: " << received.process_id << " seq_num: "  << received.packet_seq_num);
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
}



void PerfectLink::sendAcks(){
    while (true){
        Packet_ProcId ack_dest = acks_to_send.pop();
        sockaddr_in dest_addr = (*host_addresses)[ack_dest.dest_proc_id];
        sender_lock.lock();
        DEBUG_MSG("PERFECT-LINK sending ACK: dest: " << ack_dest.dest_proc_id << " source: " <<  ack_dest.packet.source_id << " sender: " << ack_dest.packet.process_id << " seq_num: "  << ack_dest.packet.packet_seq_num);
        udp_socket.send(ack_dest.packet, reinterpret_cast<sockaddr*> (&dest_addr));
        sender_lock.unlock();
    }
}

void PerfectLink::sendPackets(){
    while(true){
        sender_lock.lock();
        DEBUG_MSG("PERFECT-LINK sending packets from outbox");
        if (debug_mode){
            outbox.debug();
        }
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



