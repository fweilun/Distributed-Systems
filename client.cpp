#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "machine.hpp"

#define PORT 8080

int main(int argc, char** argv) {
  std::string machine_path = REMOTE_MACHINES_PATH;
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--local") == 0) {
      machine_path = LOCAL_MACHINES_PATH;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return 1;
    }
  }

  int listener, server_remain = 0, select_res;
  struct addrinfo hints;
  struct addrinfo* res;
  struct timeval max_wait = {1, 0};

  int recv_size, fdmax;
  fd_set master, read_fds;
  FD_ZERO(&read_fds);
  FD_ZERO(&master);

  std::vector<machine_config> machine_cfgs = read_all_machine_config(REMOTE_MACHINES_PATH);
  printf("machine count: %zu\n", machine_cfgs.size());

  std::string command;
  getline(std::cin, command);
  int com_size = command.size();

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  for (auto& cfg : machine_cfgs) {
    if (getaddrinfo(cfg.ip.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
      printf("getaddrinfo fails: %d\n", errno);
    }
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      printf("Socket creation fails: %d\n", errno);

    if (connect(listener, res->ai_addr, res->ai_addrlen) < 0) {
      // connection fails (the machine is down for now)
      printf("connect fails for machine %d: %d\n", cfg.id, errno);
      freeaddrinfo(res);
      close(listener);
    } else {
      freeaddrinfo(res);
      send(listener, command.c_str(), com_size, 0);
      int flags = fcntl(listener, F_GETFL);
      fcntl(listener, F_SETFL, flags | O_NONBLOCK);
      shutdown(listener, SHUT_WR);
      FD_SET(listener, &master);
      fdmax = std::max(fdmax, listener + 1);  // max() not necessary
      ++server_remain;
    }
  }

  while (server_remain) {
    read_fds = master;

    if ((select_res = select(fdmax, &read_fds, NULL, NULL, &max_wait)) <= 0) {
      printf("select fails with ret %d: errno %d\n", select_res, errno);
      continue;  // not sure what would happend
    }

    for (int i = 0; i < fdmax; ++i) {
      if (!FD_ISSET(i, &read_fds)) continue;

      char buffer[66536];
      if ((recv_size = recv(i, buffer, sizeof(buffer), 0)) < 0) {
        if (errno != EAGAIN) printf("recv fails: %d\n", errno);

      } else {
        if (recv_size)
          std::cout.write(buffer, recv_size);
        else {
          printf("Machine with socket #%d closed or finished\n", i);
          --server_remain;
          FD_CLR(i, &master);
        }
      }
    }
  }

  return 0;
}