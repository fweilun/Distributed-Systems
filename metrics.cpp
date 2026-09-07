#include "metrics.hpp"

#include <sys/stat.h>

#include <cstdio>

static std::string g_path, g_tag;
static bool g_enabled = false;

void Metrics::init(const char *role, int id) {
  mkdir("metrics", 0755);
  g_tag = std::string(role) + "," + std::to_string(id);
  g_path = "metrics/" + std::string(role) + "." + std::to_string(id) +
           ".metrics.log";
  // append：client 是一次 query 一個 process，用 "w" 會洗掉前幾次的資料。
  // 要重來就砍掉 metrics/ 目錄。
  if (FILE *f = fopen(g_path.c_str(), "a"))
    fclose(f);
}

void Metrics::set_enabled(bool on) { g_enabled = on; }

void Metrics::record(const char *name, int64_t value_us,
                     const std::string &extra) {
  if (!g_enabled || g_path.empty())
    return;
  FILE *f = fopen(g_path.c_str(), "a");
  if (!f)
    return;
  fprintf(f, "%lld,%s,%s,%lld,%s\n",
          (long long)std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count(),
          g_tag.c_str(), name, (long long)value_us, extra.c_str());
  fclose(f); // flush
}
