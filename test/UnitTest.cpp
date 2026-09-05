#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

int NUM_OF_MACHINE;
int SERVER_PORT = 8080;
vector<string> MACHINE_IPS;

struct Log_Query {
  int machine_id = 0;
  string content;
};

void loadMachineIps(string filename) {
  ifstream file(filename);
  if (! file.is_open()) {
    cerr << "Error: Cannot open " << filename << endl;
    exit(1);
  }

  string ip;
  while(file >> ip) {
    while (!ip.empty() && (ip.back() == '\r' || ip.back() == ' ' || ip.back() == '\n')) {
      ip.pop_back();
    }
    if (!ip.empty()) {
      MACHINE_IPS.push_back(ip);
    }
  }

  NUM_OF_MACHINE = MACHINE_IPS.size();
  if (NUM_OF_MACHINE == 0) {
    cerr << "Error : No machine IP found in " << filename << endl;
    exit(1);
  }
}

void teardown() {
  system("./cleanup.sh");
  system("rm -r ./logs");
}

void envSetup() {
  system("./cleanup.sh");

  system("mkdir -p ./logs");
  // generate log file from local
  for (int i = 1; i <= NUM_OF_MACHINE; i++) {
    string path = "./bin/LogGenerator " + to_string(i);
    const char* command = path.c_str();
    int result = system(command);
    if (result == 0) cout << "file" << to_string(i) << " is generated." << endl;
  }
  system("./remote.sh push_logs");
  system("./remote.sh start");
  sleep(3);
}

Log_Query receivedData(int socket_fd, int machine_id) {
  Log_Query result;
  result.machine_id = machine_id;
  char buffer[65536];
  ssize_t bytes_read;

  while ((bytes_read = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) {
    result.content.append(buffer, bytes_read);
  }

  return result;
}

void worker_task(int machine_id, const string& query_pattern, vector<Log_Query>& results) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return;

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, MACHINE_IPS[machine_id].c_str(), &server_addr.sin_addr);

  if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
    string request = query_pattern + "\n";
    send(sock, request.c_str(), request.size(), 0);
    results[machine_id] = receivedData(sock, machine_id);
  } else {
    perror((MACHINE_IPS[machine_id] + " failed").c_str());
    // 確保即使失敗，物件也是合法且乾淨的空字串
    results[machine_id] = Log_Query{machine_id, ""};
  }

  close(sock);
}

int main() {
  loadMachineIps("machines.txt");
  envSetup();

  string PATTERN[5] = {"FATAL_CORE_DUMP_CORRUPT_BUFFER_9999", "USER_SESSION_ERR_AUTH_CODE_[0-9]{4}",
                       "DATABASE_TRANSACTION_TIMEOUT_WARN", "HTTP_REQUEST_GET_INDEX_SUCCESS_200",  ""};
  string expected[5] = {"1\n", "0\n", "150\n", "100\n",
                        "3000\n"};  // 1 for machine 7 used for rare pattern test only appear on
                                    // one machine, 0 for other machine with rare pattern

  vector<Log_Query> results(NUM_OF_MACHINE);

  string filename = "test_result.txt";
  ofstream file(filename);
  if (! file.is_open()) {
    cerr << "Error: Cannot open " << filename << endl;
    exit(1);
  }
  

  // test1: test for rare patterns
  for (int num = 0; num < 5; num++) {
    vector<thread> threads;
    for (int i = 0; i < NUM_OF_MACHINE; i++) {
      int machine_id = i + 1;
      string log_file = "./logs/machine." + to_string(machine_id) + ".log";
      string command;
      if (num == 1)
        command = "grep -c -E '" + PATTERN[num] + "' " + log_file;
      else if (num == 4)
        command = "grep '20' " + log_file;
      else
        command = "grep -c '" + PATTERN[num] + "' " + log_file;

      threads.emplace_back(worker_task, i, command, ref(results));
    }

    for (auto& t : threads) {
      if (t.joinable()) t.join();
    }

    bool is_passed = true;
    cout << "Evaluating result..." << endl;

    for (const auto& res : results) {
      if (num == 0) {
        if ((res.machine_id == 6 && res.content != expected[0]) ||
            (res.machine_id != 6 && res.content != expected[1])) {
          is_passed = false;
          break;
        }
      } else if (num == 1) {
        if (res.content != expected[2]) {
          is_passed = false;
          break;
        }

      } else if (num == 2) {
        if (((res.machine_id == 2 || res.machine_id == 7 || res.machine_id == 8) &&
             res.content != expected[3]) ||
            (res.machine_id != 2 && res.machine_id != 7 && res.machine_id != 8 &&
             res.content != expected[1])) {
          is_passed = false;
          break;
        }
      } else if (num == 3) {
        if (res.content != expected[4]) {
          is_passed = false;
          break;
        }
      } else if (num == 4) {
        cout << "-----------------Below are outputs of machine " << res.machine_id
             << "----------------------" << res.content << endl;
      }
    }

    if (is_passed) {
      file << "Test " << num + 1 << " is passed." << endl;
    } 
    else {
      file << "Test " << num + 1 << " is not passed." << endl;
      for (const auto& res : results) {
        file << "[DEBUG] Machine " << res.machine_id << " returned: [" << res.content << "]" << endl;
      }
    }
    
  }

  file.close();
  teardown();
}
