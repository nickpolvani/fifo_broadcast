#include "best_effort_broadcast.hpp"



void BestEffortBroadcast::deliver(){
    while(true){
        Packet p = packets_to_deliver.pop();
        urb -> BEBDeliver(p);
    }
}


void BestEffortBroadcast::broadcast(){
    while(true){
        // it works because this is the only consumer of packets_to_rebroadcast
        if (packets_to_re_broadcast.getSize() > 0){
            Packet cur_packet = packets_to_re_broadcast.pop();
            DEBUG_MSG("BEB RE-Broadcasting: packet source: " <<  cur_packet.source_id << " sender: " << cur_packet.process_id << " seq_num: "  << cur_packet.packet_seq_num);
            for (auto host : hosts){
                perfect_link -> send(Packet_ProcId(cur_packet, host.id));
            }
        }
        // it works because this is the only consumer of packets_to_broadcast
        if(packets_to_broadcast.getSize() > 0){
            Packet cur_packet = packets_to_broadcast.pop();
            DEBUG_MSG("BEB Broadcasting: packet seq_num: "  << cur_packet.packet_seq_num);
            for (auto host : hosts){
                perfect_link -> send(Packet_ProcId(cur_packet, host.id));
            }
        }
    }
}

void BestEffortBroadcast::start(){
    std::thread * deliver_thread = new std::thread([this] {this -> deliver();});
    std::thread * broadcast_thread = new std::thread([this] {this -> broadcast();});

    threads.push_back(deliver_thread);
    threads.push_back(broadcast_thread);
}

