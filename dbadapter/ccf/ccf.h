#ifndef DBADAPTER_CCF_H
#define DBADAPTER_CCF_H

#include <curl/curl.h>
#include "dbadapter.h"

namespace ledgerbench {

struct CCFPromise : public Promise {
  size_t size() {return 1;}
};

class CCF : public DB {
 public:
  CCF(const char* config);
  ~CCF();

  void Begin() {}

  int Commit(Promise* promise) { return 0; }

  void Abort() {}

  int Get(const std::vector<std::string>& keys,
          std::vector<std::string>& vals,
          Promise* promise) { return 0; }

  int Get(const std::string& key,
          std::string* vals,
          Promise* promise) { return 0; }

  void Put(const std::vector<std::string>& keys,
           const std::vector<std::string>& vals) {}

  void Provenance(const std::string& keys, int n) {}

  int Range(const std::string& from,
            const std::string& to,
            std::map<std::string, std::string>& values,
            Promise* promise) { return 0; }

  bool Verify(Promise* promise);

  int StoredProcedure(std::vector<std::string> params, const OpType& type,
      Promise* promise);

 private:
  static size_t WriteCallback(void *contents, size_t size,
      size_t nmemb, void* userp);
  
  size_t GetLatestCommit();

  CURL* curl;
  CURL* verifier;
  std::string host;
  std::string service_cert;
  std::string user_cert;
  std::string user_key;
  long latest_commit;
};

}  // namespace ledgerbench

#endif  // DBADAPTER_CCF_H
