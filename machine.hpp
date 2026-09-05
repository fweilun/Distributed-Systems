#include <string>
#include <vector>

struct machine_config {
  int id;
  std::string ip;
  std::string port;
};
const std::string LOCAL_MACHINES_PATH = "machines.txt";
const std::string REMOTE_MACHINES_PATH = "machines-remote.txt";

std::vector<machine_config> read_all_machine_config();

machine_config read_machine_config(int id);