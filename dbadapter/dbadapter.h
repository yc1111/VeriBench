#ifndef DBADAPTER_H
#define DBADAPTER_H

#include <vector>
#include <string>

namespace ledgerbench {

struct Promise {
  virtual size_t size() = 0;
};

class DB {
 public:
  virtual void Begin() = 0;

  virtual int Commit(Promise* promise) = 0;

  virtual void Abort() = 0;

  virtual int Get(const std::vector<std::string>& keys,
                  std::vector<std::string>& vals,
                  Promise* promise) = 0;

  virtual int Get(const std::string& key,
                  std::string* vals,
                  Promise* promise) = 0;

  virtual void Put(const std::vector<std::string>& keys,
                   const std::vector<std::string>& vals) = 0;

  virtual void Provenance(const std::string& keys, int n) = 0;

  virtual bool Verify(Promise* promise) = 0;
};

}  // namespace ledgerbench

#endif  // DBADAPTER_H