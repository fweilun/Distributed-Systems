#include <string>
#include <vector>

struct machine_config {
  int id;
  std::string ip;
  std::string port;
};

std::vector<machine_config> read_all_machine_config();

machine_config read_machine_config(int id);