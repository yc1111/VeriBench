#ifndef DBADAPTER_GLASSDB_H
#define DBADAPTER_GLASSDB_H

#include <vector>
#include <map>
#include <string>
#include <mutex>

#include "dbadapter.h"
#include "store/strongstore/client.h"

namespace veribench {

struct GlassDBPromise : public Promise {
  std::map<int, std::map<uint64_t, std::set<std::string>>> verify_map;
  size_t size() { return verify_map.size(); }
};

class GlassDB : public DB {
 public:
  GlassDB(const char* config);
  ~GlassDB() { delete client; };

  void Begin();

  int Commit(Promise* promise);

  void Abort();

  void Init() {}

  int Get(const std::vector<std::string>& keys,
          std::vector<std::string>& vals,
          Promise* promise);

  int Get(const std::string& key,
          std::string* vals,
          Promise* promise);

  int Put(const std::vector<std::string>& keys,
           const std::vector<std::string>& vals);

  void Provenance(const std::string& keys, int n);

  int Range(const std::string& from,
            const std::string& to,
            std::map<std::string, std::string>& values,
            Promise* promise);

  bool Verify(Promise* promise);

  int StoredProcedure(std::vector<std::string> task, const OpType& type,
      Promise* promise) { return 0; }

 private:
  void Merge(const
      std::map<int, std::map<uint64_t, std::vector<std::string>>>& newmap,
      Promise* promise);

  strongstore::Client* client;
  std::mutex lck;
};

}  // namespace veribench

#endif  // DBADAPTER_GLASSDB_H
