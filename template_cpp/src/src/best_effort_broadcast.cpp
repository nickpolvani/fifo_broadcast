#include "best_effort_broadcast.hpp"



void BestEffortBroadcast::deliver(){
    while(true){
        Packet p = packets_to_deliver.pop();
        urb -> BEBDeliver(p);
    }
}


void BestEffortBroadcast::broadcast(){
    while(true){
        Packet cur_packet;
        if (packets_to_re_broadcast.getSize() > 0){
            cur_packet = packets_to_re_broadcast.pop();
        }
        else{
            cur_packet = packets_to_broadcast.pop();
        }
        for (auto host : hosts){
            perfect_link -> send(Packet_ProcId(cur_packet, host.id));
        }
    }
}

void BestEffortBroadcast::start(){
    std::thread * deliver_thread = new std::thread([this] {this -> deliver();});
    std::thread * broadcast_thread = new std::thread([this] {this -> broadcast();});

    threads.push_back(deliver_thread);
    threads.push_back(broadcast_thread);
}

