#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;


 int NUM_OF_MACHINE = 10;

struct Log_Query{
  int machine_id = -1;
  string content;
};

void teardown() {
  system("pkill -f ./server");
  system("rm machine.*.log");
  system("./cleanup.sh");
}

void envSetup() {
  system("pkill -f ./server");
  system("./cleanup.sh");
  // generate log file for local version
  for (int i = 0; i < NUM_OF_MACHINE; i++) {
    string path = "./run_test/LogGenerator " + to_string(i);
    const char *command = path.c_str();
    int result = system(command);
    if (result == 0)
      cout << "file" << to_string(i) << " is generated." << endl;
  }
  system("./deploy.sh");
  sleep(1);
}

Log_Query receivedData(int socket_fd, int machine_id) {
  Log_Query result;
  result.machine_id = machine_id;
  char buffer[66536];
  ssize_t bytes_read;

  while((bytes_read = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0)  {
    result.content.append(buffer, bytes_read);
  }

  
  return result;
}


void worker_task(int machine_id, const string& query_pattern, vector<Log_Query>& results) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8081 + machine_id); 
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); 

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
    string request = query_pattern + "\n";
    send(sock, request.c_str(), request.size(), 0);
    results[machine_id] = receivedData(sock, machine_id);
    } else {
        perror(("Connect to port " + to_string(8081 + machine_id) + " failed").c_str());
    }

    close(sock);
}

int main() {
  envSetup();
  
  string PATTERN[4] = {"FATAL_CORE_DUMP_CORRUPT_BUFFER_9999",
                       "USER_SESSION_ERR_AUTH_CODE_[0-9]{4}",
                       "DATABASE_TRANSACTION_TIMEOUT_WARN",
                       "HTTP_REQUEST_GET_INDEX_SUCCESS_200"};
  string expected[5] = {
      "1\n", "0\n", "150\n", "100\n",
      "3000\n"}; // 1 for machine 7 used for rare pattern test only appear on one
             // machine, 0 for other machine with rare pattern


  


  vector<Log_Query> results(10);
  vector<thread> threads;


  

  // test1: test for rare patterns
  for (int num  = 0 ; num < 5 ; num++) {
    threads.clear();

    for (int i =  0 ; i < NUM_OF_MACHINE ; i++) {
      string command;
      if (num == 1) command = "grep -c -E \""+ PATTERN[num] +"\" ./machine." + to_string(i) + ".log";
      else if (num == 4) command = "grep \"20\" ./machine." + to_string(i) + ".log";
      else command = "grep -c \""+ PATTERN[num] +"\" ./machine." + to_string(i) + ".log";
      threads.emplace_back(worker_task, i, command, ref(results));
    }

    for (auto& t : threads) {
      if (t.joinable()) t.join();
    }

    bool is_passed = true;
    cout << "Evaluating result..." << endl;

    for (Log_Query res : results)  {
      if (num  == 0) {
        if ((res.machine_id == 7 && res.content != expected[0]) || 
            (res.machine_id != 7 && res.content != expected[1])) {
          is_passed = false;
          break;
        }
      }
      else if (num == 1) {
        if (res.content != expected[2]) {
          is_passed = false;
          break;
        }
        
      }
      else if (num == 2) {
        if (((res.machine_id == 3 || res.machine_id == 8 || res.machine_id == 9) && res.content != expected[3]) || 
            (res.machine_id != 3 && res.machine_id != 8 && res.machine_id != 9 && res.content != expected[1])) {
          is_passed = false;
          break;
        }
      }
      else if (num == 3) {
        if (res.content != expected[4]) {
          is_passed = false;
          break;
        }
      }
      else if (num == 4) {
        cout << "-----------------Below are outputs of machine " << res.machine_id << "----------------------" << res.content << endl;
      }
    }

    if (is_passed) {
      cout <<  "Test " << num + 1 << " is passed." << endl;
    }
    else {
      cout <<  "Test " << num + 1 << " is not passed." << endl;
    }
  }



  teardown();

}

