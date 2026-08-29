#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "machine.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <fcntl.h>

#define PORT 8080



int main(int _, char** argv) {
    int listener;
    struct addrinfo hints;
    struct addrinfo *res;

    int recv_size, fdmax;
    fd_set master, read_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    
    std::vector<machine_config> machine_cfgs = read_all_machine_config();
    printf("machine count: %d\n", machine_cfgs.size());
    int server_cnt = machine_cfgs.size();
    
    std::string command;
    std::cin >> command;
    

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    for (auto &cfg: machine_cfgs) {
        if (getaddrinfo(cfg.ip.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
            printf("getaddrinfo fails: %d\n", errno);
        }
        if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
            printf("Socket creation fails: %d\n", errno);

        if (connect(listener, res->ai_addr, res->ai_addrlen) < 0)
            printf("connect fails: %d\n", errno);

        send(listener, command.c_str(), sizeof(command, 0), 0);

        int flags = fcntl(listener, F_GETFL);
        fcntl(listener, F_SETFL, flags | O_NONBLOCK); // returns?
        shutdown(listener, SHUT_WR);
        FD_SET(listener, &master);
        fdmax = listener; // max fd
    }
    
    while (server_cnt--) {
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0) {
            printf("select fails: %d\n", errno);
            continue; // not sure what would happend
        }

        for (int i = 0; i < fdmax; ++i) {
            if (!FD_ISSET(i, &read_fds)) continue;
            char buffer[66536];
            if ((recv_size = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                if (recv_size == 0) {

                } else {

                }
            } else {
                std::cout << buffer << std::endl;
            }
        }
    }

    return 0;
}