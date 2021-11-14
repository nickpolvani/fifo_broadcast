#ifndef PROCESS_CONTROLLER_H
#define PROCESS_CONTROLLER_H


#include "parser.hpp"
#include "packet.hpp"
#include <map>

#include <iostream>
#include <fstream>
#include "perfect_link.hpp"
#include "best_effort_broadcast.hpp"
#include "uniform_reliable_broadcast.hpp"
#include "fifo_broadcast.hpp"

// the process either sends o receives messages,
// so it will have only one between PLSender and PLReceiver

class PerfectLink;
class FifoBroadcast;
class BestEffortBroadcast;
class UniformReliableBroadcast;


class ProcessController{
    private:

        std::vector<Parser::Host> hosts;
        
        std::ofstream output_file;

        std::ifstream config_file;

        long unsigned int num_messages; // number of messages to broadcast

        PerfectLink * perfect_link = NULL;

        BestEffortBroadcast * beb = NULL;

        UniformReliableBroadcast * urb = NULL;

        FifoBroadcast * fifo_broadcast = NULL;

    
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
        void onPacketBroadcast(packet::Packet p);

        void stopProcess();

        // initialize lower level abstractions and start broadcasting/delivering packets
        void start();
};

#endif