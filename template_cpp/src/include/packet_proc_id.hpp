#ifndef PACKET_PROC_ID_H
#define PACKET_PROC_ID_H

#include "packet.hpp"
// packet to be sent, needs also process id of the destination host
struct Packet_ProcId{
    packet::Packet packet;
    long unsigned int dest_proc_id;
    Packet_ProcId(packet::Packet p, long unsigned int dest_id): packet(p), dest_proc_id(dest_id){}
};

#endif