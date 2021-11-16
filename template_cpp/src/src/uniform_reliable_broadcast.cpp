#include "uniform_reliable_broadcast.hpp"


bool UniformReliableBroadcast::canDeliver(long unsigned int source_id, long unsigned int seq_num){
    return acks[source_id][seq_num].size() > num_processes / 2;
}


void UniformReliableBroadcast::URBDeliver(){
    assert((fifo_broadcast != NULL) == true);
    while(true){
        Packet p = packets_to_deliver.pop();
        DEBUG_MSG("URBDeliver: packet source: " <<  p.source_id << " sender: " << p.process_id << " seq_num: "  << p.packet_seq_num);
        fifo_broadcast -> URBDeliver(p);
    }
}


void UniformReliableBroadcast::BEBDeliver(Packet p){
    acks[p.source_id][p.packet_seq_num].insert(p.process_id);

    DEBUG_MSG("BEBDeliver: packet source: " <<  p.source_id << " sender: " << p.process_id << " seq_num: "  << p.packet_seq_num);

    pending_mutex.lock();

    if ((pending[p.source_id].count(p.packet_seq_num) == 0) // packet not in pending 
             && (delivered[p.source_id].count(p.packet_seq_num) == 0)){ // packet not delivered

        pending[p.source_id].insert(p.packet_seq_num);
        pending_mutex.unlock(); // free lock because you could wait on the next instruction

        DEBUG_MSG("BEBDeliver: about to RE-Broadcast, source " <<  p.source_id << " previous sender: " << p.process_id << " seq_num: "  << p.packet_seq_num);
        // change sender process to this one 
        p.changeSenderId(process_id);
        beb -> re_broadcast(p);
        DEBUG_MSG("successfully added to packets_to_re_broadcast");
        pending_mutex.lock(); //regain lock for next part of execution
    }

    // see if packet can be URBDelivered 
    if (canDeliver(p.source_id, p.packet_seq_num) && // majority BEBdelivered packet
        (pending[p.source_id].count(p.packet_seq_num) == 1) && // packet is in pending
        (delivered[p.source_id].count(p.packet_seq_num) == 0)){ // packet is not delivered 

        delivered[p.source_id].insert(p.packet_seq_num);
        pending[p.source_id].erase(p.packet_seq_num);
        
        pending_mutex.unlock(); // free lock because you could wait on the next instruction
        packets_to_deliver.push(p);
    }
    else{
        pending_mutex.unlock();
    }
}


void UniformReliableBroadcast::broadcast(Packet p){
    pending_mutex.lock();
    pending[p.source_id].insert(p.packet_seq_num);
    pending_mutex.unlock();

    DEBUG_MSG("URB Broadcasting: packet seq_num: "  << p.packet_seq_num);
    beb -> broadcast(p);
}

void UniformReliableBroadcast::start(){
    std::thread * deliver_thread = new std::thread([this] {this -> URBDeliver();});
    threads.push_back(deliver_thread);
}