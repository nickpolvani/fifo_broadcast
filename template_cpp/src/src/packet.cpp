#include "packet.hpp"
#include <assert.h>
#include "debug.h"

using namespace packet;


void Packet::toBytes(char * buffer){
    char* cur_pointer = &buffer[0];

    // encode header
    std::string source_id_str = std::to_string(source_id);
    std::string process_id_str = std::to_string(process_id);
    std::string packet_seq_num_str = std::to_string(packet_seq_num);
    std::string first_msg_seq_num_str = std::to_string(first_msg_seq_num);
    std::string is_ack_str = std::to_string(static_cast<unsigned int>(is_ack));
    std::string payload_length_str = std::to_string(payload_length);

    // write header to buffer
    memcpy(cur_pointer, source_id_str.c_str(), source_id_str.size() + 1);
    cur_pointer += source_id_str.size() + 1;

    memcpy(cur_pointer, process_id_str.c_str(), process_id_str.size() + 1);
    cur_pointer += process_id_str.size() + 1;

    memcpy(cur_pointer, packet_seq_num_str.c_str(), packet_seq_num_str.size() + 1);
    cur_pointer += packet_seq_num_str.size() + 1;

    memcpy(cur_pointer, first_msg_seq_num_str.c_str(), first_msg_seq_num_str.size() + 1);
    cur_pointer += first_msg_seq_num_str.size() + 1;

    memcpy(cur_pointer, is_ack_str.c_str(), is_ack_str.size() + 1);
    assert((is_ack_str.size() == 1) == true);
    cur_pointer += is_ack_str.size() + 1;

    memcpy(cur_pointer, payload_length_str.c_str(), payload_length_str.size() + 1);
    cur_pointer += payload_length_str.size() + 1;

    // write all the messages (string that terminates with \0)
    for (Message message : messages){
        int message_length = message.get_length();
        message.toBytes(cur_pointer);
        cur_pointer += message_length;
    }
}


/* appends characters from src[0] to src[i] into dest, where data[i] is a NULL character
   returns the number of characters copied INCLUDING the NULL character 
*/
static int copyString(char * src, std::string * dest){
    int i = 0;
    char cur_char = src[i];
    while (cur_char != '\0'){
        *dest += cur_char;  //append cur char to dest
        i++;
        cur_char = src[i];
    }
    return i + 1;
}

Packet Packet::decodeData(char * data){
    // decode header
    char * cur_pointer = &data[0];

    std::string source_id_str;
    std::string process_id_str;
    std::string packet_seq_num_str;
    std::string first_msg_seq_num_str;
    std::string is_ack_str;
    std::string payload_length_str;

    int i = copyString(cur_pointer, &source_id_str);
    assert((i > 1) == true);
    cur_pointer += i;

    i = copyString(cur_pointer, &process_id_str);
    assert((i > 1) == true);
    cur_pointer += i;

    i = copyString(cur_pointer, &packet_seq_num_str);
    assert((i > 1) == true);
    cur_pointer += i;

    i = copyString(cur_pointer, &first_msg_seq_num_str);
    assert((i > 1) == true);
    cur_pointer += i;

    i = copyString(cur_pointer, &is_ack_str);
    assert ((i == 2) == true);  //check boolean is only 1 value
    cur_pointer += i;

    i = copyString(cur_pointer, &payload_length_str);
    assert((i >= 2) == true);
    cur_pointer += i;
    
   // DEBUG_MSG("meaning of numbers: source_id, process_id, packet_seq_num, first_msg_seq_num, is_ack, payload_length");
    // DEBUG_MSG("PERFECT_LINK READING " << source_id_str << " " << process_id_str << " " << packet_seq_num_str << " " << first_msg_seq_num_str << " " << is_ack_str << " " << payload_length_str << "\n");

    long unsigned int i_source_id = std::stoul(source_id_str);
    long unsigned int i_process_id = std::stoul(process_id_str);
    long unsigned int i_packet_seq_num = std::stoul(packet_seq_num_str);
    long unsigned int i_first_msg_seq_num = std::stoul(first_msg_seq_num_str);
    bool is_ack = static_cast<bool>(std::stoi(is_ack_str));
    if (is_ack){
        return createAck(i_process_id, i_source_id,  i_packet_seq_num);
    }
    else{
        long unsigned int payload_length = std::stoul(payload_length_str);
        // parse messages
        Packet p = Packet(i_process_id, i_source_id, i_packet_seq_num); 
        while (p.payload_length < payload_length){
            Message cur_message = Message::decodeData(cur_pointer); //retrieve current message
            if (!p.canAddMessage(cur_message)){
                std::cerr << "Error when trying to reconstruct Packet !!!\n";
                std::cerr << "Expected payload length: " << payload_length << "\n";
            }
            p.addMessage(cur_message);
            cur_pointer += cur_message.get_length();
        }
        return p;
    }
}