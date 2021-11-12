#include "udp_scocket.hpp"
#include "errno.h"
#include <iostream>


UDPSocket::UDPSocket(unsigned short port, int rcv_timeout=-1):timeout_sec(rcv_timeout)
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&address, 0, sizeof(address));
    // Filling server information
    address.sin_family    = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port; // function htons is already performed on the passed parameter

    if (rcv_timeout > 0){
        timeval t;
        t.tv_sec = rcv_timeout; //timeout seconds
        t.tv_usec = 0; //micro seconds
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<void *>(&t), sizeof(t)) < 0){
            perror("Could not set timeout on receive message");
            exit(EXIT_FAILURE);
            } 
    }

    if (bind(sockfd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}


packet::Packet UDPSocket::receivePacket(){
    ssize_t n;  // number of bytes received
    n = TEMP_FAILURE_RETRY(recvfrom(sockfd, buffer_received, packet::MAX_LENGTH, MSG_WAITALL, NULL, 0));
    if (n < 0){
        if (0 || errno == EAGAIN || errno == EWOULDBLOCK){
            throw TimeoutException("timeout on socket.recvfrom() has expired before receiving message\n");
        }
        else{
            std::cout << "Socket failed on recvfrom(). Error number: " << errno << "\n";
            exit(EXIT_FAILURE);
        }
    }
    packet::Packet p = packet::Packet::decodeData(buffer_received);
    return p;
}



void UDPSocket::send(packet::Packet p, const sockaddr * dest){
    p.toBytes(buffer_send);
    ssize_t n = sendto(sockfd, buffer_send, p.getLength(), MSG_CONFIRM, dest, sizeof(*dest));
    if (n < 0){
        std::cout << "Socket failed to send. Error number: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
}

