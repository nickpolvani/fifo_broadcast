#include "fifo_broadcast.hpp"



FifoBroadcast::FifoBroadcast(ProcessController * i_process_controller, long unsigned int i_num_messages,
                                 std::vector<Parser::Host> hosts):
num_messages(i_num_messages), process_controller(i_process_controller){
    for (auto host: hosts){
        next[host.id] = 0;
    }
}


void FifoBroadcast::URBDeliver(Packet p){
    pending[p.source_id][p.packet_seq_num] = p;

    while( (pending[p.source_id].count(next[p.source_id]) == 1)){ // next seq num is in pending
        // get next packet cur_packet has same source id as p
        Packet cur_packet = pending[p.source_id][next[p.source_id]];
        next[cur_packet.source_id] = next[cur_packet.source_id] + 1;
        pending[cur_packet.source_id].erase(cur_packet.packet_seq_num);
        packets_to_deliver.push(cur_packet);
    }

}

void FifoBroadcast::fifoDeliver(){
    while(true){
        Packet p = packets_to_deliver.pop();
        DEBUG_MSG("FIFO: about to deliver packet: " <<  p.source_id << " " << p.packet_seq_num);
        // if I deliver the packet I broadcasted I can broadcast the next one
        if (p.source_id == process_controller ->process_id){
            can_broadcast = true;
            broadcast_cv.notify_all();
        }
        process_controller -> onPacketDelivered(p);
    }
}


void FifoBroadcast::broadcast(){
    assert ((urb != NULL) == true);
    DEBUG_MSG("number of packets to send: " << num_messages << "\n");
    unsigned long int process_id = process_controller -> process_id;

    std::unique_lock lock(mutex);

    // create and send Packets
    Packet curr_packet(process_id, process_id, cur_seq_num);
    for (long unsigned int i = 1; i <= num_messages; i++){
        Message curr_message(std::to_string(i));

        while(!can_broadcast){
            broadcast_cv.wait(lock);
        }

        // packet is full (leave a margin of 5 bytes to change the process_id when re-broadcasting messages)
        if (!curr_packet.canAddMessage(curr_message, 5)){  
            DEBUG_MSG("FIFO: trying to urb broadcast packet, seq_num: " << curr_packet.packet_seq_num << "\n");
            process_controller -> onPacketBroadcast(curr_packet);
            // set before because otherwise I could execute the delivery and then set this variable to false again
            // having a deadlock
            can_broadcast = false;   
            urb -> broadcast(curr_packet);
            cur_seq_num++;
            curr_packet = Packet(process_id, process_id, cur_seq_num);
        }
        curr_packet.addMessage(curr_message);

        // last message, so send packet even if not full
        if (i == num_messages){ 
            DEBUG_MSG("FIFO: trying to urb broadcast packet, seq_num: " << curr_packet.packet_seq_num << "\n");
            process_controller -> onPacketBroadcast(curr_packet);
            // set before because otherwise I could execute the delivery and then set this variable to false again
            // having a deadlock
            can_broadcast = false;
            urb -> broadcast(curr_packet);
            cur_seq_num++;
        }
    }
}



void FifoBroadcast::start(){
    std::thread * deliver_thread = new std::thread([this] {this -> fifoDeliver();});
    std::thread * broadcast_thread = new std::thread([this] {this -> broadcast();});

    threads.push_back(deliver_thread);
    threads.push_back(broadcast_thread);
}