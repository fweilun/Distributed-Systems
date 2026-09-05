#include "machine.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

vector<machine_config> read_all_machine_config() {
  ifstream file("machines.txt");
  string line, ip, port;
  vector<machine_config> machine_list;
  int id;

  while (getline(file, line)) {
    stringstream ss(line);
    ss >> id >> ip >> port;
    machine_config cfg;
    cfg.id = id;
    cfg.ip = ip;
    cfg.port = port;
    machine_list.emplace_back(cfg);
  }
  return machine_list;
}

machine_config read_machine_config(int id) {
  vector<machine_config> configs = read_all_machine_config();
  return configs[id - 1];
}