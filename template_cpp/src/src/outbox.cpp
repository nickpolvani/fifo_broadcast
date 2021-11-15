#include "outbox.hpp"


/* adds packet to outbox, if it is full
    wait until some other thread (consumer) removes a packet
*/
void OutBox::addPacket(Packet_ProcId const pack_and_dest){
    std::unique_lock<std::mutex> lock(mutex); //creates lock and calls mutex.lock()
    while (curr_size == max_size){
        //Atomically unlocks lock, blocks the current executing thread, 
        //and adds it to the list of threads waiting on *this
        //The thread will be unblocked when notify_all() or notify_one() is executed.
        //It may also be unblocked spuriously
        cv_add.wait(lock);
    }
    curr_size += 1;
    long unsigned int dest_id = pack_and_dest.dest_proc_id;
    long unsigned int source_id = pack_and_dest.packet.source_id;
    long unsigned int seq_num = pack_and_dest.packet.packet_seq_num;
    packets[dest_id][source_id][seq_num] = pack_and_dest.packet; 

    //destructor of lock releases the mutex
}


// returns true if successfully removed packet, false if there was not
// the specified packet, waits only to own the lock of the outbox
bool OutBox::removePacket(unsigned long int dest_proc_id, unsigned long int source_id, unsigned long int seq_num){
    std::unique_lock<std::mutex> lock(mutex);
    // check if packet is in the container
    if(//(packets.count(dest_proc_id) == 1) && 
        //(packets[dest_proc_id].count(source_id) == 1) && 
        (packets[dest_proc_id][source_id].count(seq_num) == 1) ) {
        
        size_t num_removed = packets[dest_proc_id][source_id].erase(seq_num);
        assert((num_removed == 1) == true);
        curr_size--;

        cv_add.notify_all();
        return true;
    }
    else{
        return false;
    }
}



void OutBox::sendPackets(UDPSocket * udp_socket){
    std::unique_lock<std::mutex> lock(mutex);
    // iterate destination process ids
    for (auto it_dest_proc_id = packets.begin(); it_dest_proc_id != packets.end(); ++it_dest_proc_id){
        SourceId_2_SeqNum_2_Packet source2seq2pack = it_dest_proc_id -> second;
        long unsigned int dest_id = it_dest_proc_id -> first;
        sockaddr_in dest_addr = (*host_addresses)[dest_id];
        // iterate source id
        for (auto it_source_id = source2seq2pack.begin(); it_source_id != source2seq2pack.end(); ++ it_source_id){
            std::map<long unsigned int, Packet> seq2pack = it_source_id -> second;
            // iterate sequence number
            for (auto it_seq = seq2pack.begin(); it_seq != seq2pack.end(); ++it_seq){
                udp_socket -> send(it_seq -> second, reinterpret_cast<sockaddr*> (&dest_addr));
            }
        }
    }
}

void OutBox::debug(){
    std::unique_lock<std::mutex> lock(mutex);
    // iterate destination process ids
    for (auto it_dest_proc_id = packets.begin(); it_dest_proc_id != packets.end(); ++it_dest_proc_id){
        SourceId_2_SeqNum_2_Packet source2seq2pack = it_dest_proc_id -> second;
        long unsigned int dest_id = it_dest_proc_id -> first;
        sockaddr_in dest_addr = (*host_addresses)[dest_id];
        // iterate source id
        for (auto it_source_id = source2seq2pack.begin(); it_source_id != source2seq2pack.end(); ++ it_source_id){
            std::map<long unsigned int, Packet> seq2pack = it_source_id -> second;
            // iterate sequence number
            for (auto it_seq = seq2pack.begin(); it_seq != seq2pack.end(); ++it_seq){
                std::cout << "dest: " << dest_id << " source: " << it_source_id->first << " seq_num: " << it_seq->first << "\n";
            }
        }
    }
}
