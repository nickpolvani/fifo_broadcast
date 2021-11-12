#ifndef PROCESS_CONTROLLER_H
#define PROCESS_CONTROLLER_H


#include "parser.hpp"
#include "packet.hpp"
#include <map>
#include "pl_receiver.hpp"
#include "pl_sender.hpp"
#include <iostream>
#include <fstream>

// the process either sends o receives messages,
// so it will have only one between PLSender and PLReceiver

class PLSender;
class PLReceiver;

class ProcessController{
    private:
        std::vector<Parser::Host> hosts;

        std::ofstream output_file;

        std::ifstream config_file;

        PLSender * pl_sender = NULL; 

        PLReceiver * pl_receiver = NULL;

        long unsigned int receiver_id;  // id of the process that receives messages
                                        // if receiver_id == process_id, then wait for messages

        long unsigned int num_messages; // number of messages to send to the process with receiver_id

        void sendPackets();  // creates a PLSender, creates and sends packets to the receiver

        void listen(); // in case this process is the receiver, create PLReceiver and wait for packets

    
    public:

        /* store addresses of hosts, key is process_id (accessed only in read after creation)
            so it is Thread safe
        */
        std::map<long unsigned int, sockaddr_in> host_addresses;

        long unsigned int process_id;
        
        // initializes member variables
        ProcessController(long unsigned int id, Parser parser);

        // write packet delivered to output file
        void onPacketDelivered(packet::Packet p);

        // write packet sent to output file
        void onPacketSent(packet::Packet p);

        void stopProcess();

        // starts listening/sending packets
        void start(){
            if (receiver_id == process_id){
                listen();
            }
            else{
                sendPackets();
            }
        } 
};

#endif