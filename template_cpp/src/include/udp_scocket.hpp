#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "packet.hpp"

class TimeoutException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class UDPSocket{
    private:
        int sockfd; //socket file descriptor
        char buffer_received[packet::MAX_LENGTH];
        char buffer_send[packet::MAX_LENGTH];
        struct sockaddr_in address;
        int timeout_sec;

    public:
        /* initializes UDP socket listening and sending messages on port, using IPv4
           port: port number on network byte order
           rcv_timeout: timeout in seconds to be set on the recvfrom function, after which the socket 
                        stops waiting to receive messages and returns. If <= 0 it is not considered
                        and the socket waits indefinetely to receive a packet
        */
        UDPSocket(unsigned short port, int rcv_timeout);

        /* blocks execution until a packet arrives */
        packet::Packet receivePacket();

        void send(packet::Packet p, const sockaddr * dest);


        void closeConnection(){
          close(sockfd);
        }

};

#endif