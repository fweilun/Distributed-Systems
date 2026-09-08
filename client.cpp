#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "machine.hpp"
#include "metrics.hpp"

#define PORT 9080

struct Log_Query {
  int machine_id = 0;
  int line_num = 0;
  std::string content;
};


Log_Query receivedData(int socket_fd, int machine_id) {
  Log_Query result;
  result.machine_id = machine_id;
  char buffer[65536];
  ssize_t bytes_read;

  while ((bytes_read = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) {
    result.content.append(buffer, bytes_read);
    for (int i = 0 ; i < bytes_read ; i++) {
      if (buffer[i] == '\n') result.line_num++;
    }
  }

  result.content = "Machine " + std::to_string(machine_id) + "\nFind lines: " + std::to_string(result.line_num) + "\n" + result.content;

  return result;
}

void worker_task(int machine_id, const std::string &query_pattern,
                 std::vector<machine_config> &machine_cfg, std::vector<Log_Query> &results) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
      return;
  
  timeval timeout = timeval{2, 0};
  
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(std::stoi(machine_cfg[machine_id].port));
  inet_pton(AF_INET, machine_cfg[machine_id].ip.c_str(), &server_addr.sin_addr);

  // build Log_Query
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      0) {
    std::string request = query_pattern + "\n";
    send(sock, request.c_str(), request.size(), 0);
    shutdown(sock, SHUT_WR);
    results[machine_id] = receivedData(sock, machine_id + 1);
  } else {
    perror((machine_cfg[machine_id].ip + " failed\n").c_str());
    results[machine_id] = Log_Query{machine_id + 1, 0, "Machine " + std::to_string(machine_id + 1) + " failed\n"};
  }

  close(sock);
}

int main(int argc, char **argv) {
  std::string machine_path = REMOTE_MACHINES_PATH;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--local") == 0) {
      machine_path = LOCAL_MACHINES_PATH;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return 1;
    }
  }
  Metrics::init("client", -1);

  // int listener, server_remain = 0, select_res;
  // struct addrinfo hints;
  // struct addrinfo *res;
  // struct timeval max_wait = {1, 0};

  // int recv_size;
  // int fdmax;
  // fd_set master, read_fds;
  // FD_ZERO(&read_fds);
  // FD_ZERO(&master);

  std::vector<machine_config> machine_cfgs =
      read_all_machine_config(REMOTE_MACHINES_PATH);
  printf("machine count: %zu\n", machine_cfgs.size());
  std::vector<Log_Query> results(machine_cfgs.size());

  std::string command;
  getline(std::cin, command);
  int com_size = command.size();

  // memset(&hints, 0, sizeof(hints));
  // hints.ai_family = AF_UNSPEC;
  // hints.ai_socktype = SOCK_STREAM;

  std::string tag = command;
  std::vector<std::thread> threads;

  for (char &c : tag)
    if (c == ',')
      c = ';'; // for CSV decoding
  Metrics::resolve_request(command, false);
  Metrics::Timer full_timer("full", "cmd=" + tag);
  

  for (int i = 0 ; i < machine_cfgs.size() ; i++) {
    threads.emplace_back(worker_task, i, command, std::ref(machine_cfgs), std::ref(results));
  }

  for (auto &t : threads) {
    if (t.joinable())
    t.join();
  }

  for (const auto &res : results) {
    std::cout << res.content;
  }

  // for (auto &cfg : machine_cfgs) {
  //   if (getaddrinfo(cfg.ip.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
  //     printf("getaddrinfo fails: %d\n", errno);
  //   }
  //   if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  //     printf("Socket creation fails: %d\n", errno);

  //   if (connect(listener, res->ai_addr, res->ai_addrlen) < 0) {
  //     // connection fails (the machine is down for now)
  //     printf("connect fails for machine %d: %d\n", cfg.id, errno);
  //     freeaddrinfo(res);
  //     close(listener);
  //   } else {
  //     freeaddrinfo(res);
  //     send(listener, command.c_str(), com_size, 0);
  //     int flags = fcntl(listener, F_GETFL);
  //     fcntl(listener, F_SETFL, flags | O_NONBLOCK);
  //     shutdown(listener, SHUT_WR);
  //     FD_SET(listener, &master);
  //     fdmax = std::max(fdmax, listener + 1); // max() not necessary
  //     ++server_remain;
  //   }
  // }

  // while (server_remain) {
  //   read_fds = master;

  //   if ((select_res = select(fdmax, &read_fds, NULL, NULL, &max_wait)) <= 0) {
  //     printf("select fails with ret %d: errno %d\n", select_res, errno);
  //     continue; // not sure what would happend
  //   }

  //   for (int i = 0; i < fdmax; ++i) {
  //     if (!FD_ISSET(i, &read_fds))
  //       continue;

  //     char buffer[66536];
  //     if ((recv_size = recv(i, buffer, sizeof(buffer), 0)) < 0) {
  //       if (errno != EAGAIN)
  //         printf("recv fails: %d\n", errno);

  //     } else {
  //       if (recv_size)
  //         std::cout.write(buffer, recv_size);
  //       else {
  //         // printf("Machine with socket #%d closed or finished\n", i);
  //         --server_remain;
  //         FD_CLR(i, &master);
  //       }
  //     }
  //   }
  // }

  return 0;
}