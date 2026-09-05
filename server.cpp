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
#define BACKLOG 20

// The users assign a single machine id to this program.
int main(int argc, char** argv) {
  int machine_id = -1;
  std::string machine_path = REMOTE_MACHINES_PATH;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0) {
      if (i + 1 >= argc) {
        printf("-i requires a machine id\n");
        return 1;
      }
      machine_id = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--local") == 0) {
      machine_path = LOCAL_MACHINES_PATH;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return 1;
    }
  }
  if (machine_id < 0) {
    printf("Usage: %s -i <machine_id> [--remote]\n", argv[0]);
    return 1;
  }

  // machine_config: returns the id, ip, port for each machine_id
  struct machine_config cfg = read_machine_config(machine_id, machine_path);

  int sockfd, target_fd, bytes;
  struct addrinfo hints;
  struct addrinfo* res;
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof(their_addr);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  // NULL + AI_PASSIVE => 分配0.0.0.0:port 內外網來的封包都收
  int rv = getaddrinfo(NULL, cfg.port.c_str(), &hints, &res);
  if (rv != 0) {
    printf("getaddrinfo fails: %d\n", rv);
    return 1;
  }

  // socket is represented by a file that is fd, stdin:1, ...
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) printf("Socket creation fails: %d\n", errno);

  // bind: register the destination addr to kernel space, connected by sockfd.
  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    printf("Socket bind fails: %d\n", errno);
  }
  freeaddrinfo(res);

  // Open the listening process for kernel.
  listen(sockfd, BACKLOG);
  if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) == -1) printf("fcntl FD_CLOEXEC fails: %d\n", errno);

  while (true) {
    target_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
    char command[1024];
    memset(command, 0, sizeof(command));
    char buffer[65536];
    recv(target_fd, command, 1024, 0);
    FILE* grep_result = popen(command, "r");
    while ((bytes = fread(buffer, 1, sizeof(buffer), grep_result)) > 0) {
      send(target_fd, buffer, bytes, 0);
    }
    printf("Machine %d complete.\n", machine_id);
    pclose(grep_result);
    close(target_fd);
  }

  return 0;
}