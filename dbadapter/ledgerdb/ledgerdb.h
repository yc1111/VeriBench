#ifndef DBADAPTER_LEDGERBD_H
#define DBADAPTER_LEDGERBD_H

#include <vector>
#include <map>
#include <string>
#include <mutex>

#include "dbadapter.h"
#include "distributed/store/strongstore/client.h"

namespace ledgerbench {

struct LDBPromise : public Promise {
  std::map<int, std::map<uint64_t, std::set<std::string>>> verify_map;
  size_t size() { return verify_map.size(); }
};

class LedgerDB : public DB {
 public:
  LedgerDB(const char* config);
  ~LedgerDB() { delete client; };

  void Begin();

  int Commit(Promise* promise);

  void Abort();

  int Get(const std::vector<std::string>& keys,
          std::vector<std::string>& vals,
          Promise* promise);

  void Put(const std::vector<std::string>& keys,
           const std::vector<std::string>& vals);

  void Provenance(const std::string& keys, int n);

  bool Verify(Promise* promise);
 private:
  void Merge(const
      std::map<int, std::map<uint64_t, std::vector<std::string>>>& newmap,
      Promise* promise);

  strongstore::Client* client;
  std::mutex lck;
};

}  // namespace ledgerbench

#endif  // DBADAPTER_LEDGERBD_H