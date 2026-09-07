#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace Metrics {

// 開檔（清空重來）。只做「準備寫入的地方」，不代表會開始記錄。
void init(const char *role, int id);

// 每個 request 切換一次。init() 之後預設是關的。
void set_enabled(bool on);

void record(const char *name, int64_t value_us, const std::string &extra = "");

void resolve_request(std::string& req, bool drop);

struct Timer {
  explicit Timer(const char *n, std::string e = "")
      : name(n), extra(std::move(e)), t0(std::chrono::steady_clock::now()) {}
  ~Timer() {
    record(name,
           std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now() - t0)
               .count(),
           extra);
  }
  const char *name;
  std::string extra;
  std::chrono::steady_clock::time_point t0;
};

} // namespace Metrics
