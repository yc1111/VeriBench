#ifndef DBADAPTER_H
#define DBADAPTER_H

#include <vector>
#include <map>
#include <string>

namespace veribench {

struct Promise {
  virtual size_t size() = 0;
};

enum OpType {
  kYCSB_RD,
  kYCSB_WR,
  kPROVENANCE,
  kRANGE,
  kTPCC_NEWORDER,
  kTPCC_PAYMENT,
  kTPCC_ORDERSTATUS,
  kTPCC_DELIVERY,
  kTPCC_STOCKLEVEL,
  kSB_AM,
  kSB_GB,
  kSB_UB,
  kSB_US,
  kSB_SP,
  kSB_WC,
  kVERIFY
};

class DB {
 public:
  virtual void Begin() = 0;

  virtual int Commit(Promise* promise) = 0;

  virtual void Abort() = 0;

  virtual void Init() = 0;

  virtual int Get(const std::vector<std::string>& keys,
                  std::vector<std::string>& vals,
                  Promise* promise) = 0;

  virtual int Get(const std::string& key,
                  std::string* vals,
                  Promise* promise) = 0;

  virtual int Put(const std::vector<std::string>& keys,
                  const std::vector<std::string>& vals) = 0;

  virtual void Provenance(const std::string& keys, int n) = 0;

  virtual int Range(const std::string& from,
                    const std::string& to,
                    std::map<std::string, std::string>& values,
                    Promise* promise) = 0;

  virtual bool Verify(Promise* promise) = 0;

  virtual int StoredProcedure(std::vector<std::string>, const OpType& type,
      Promise* promise) = 0;
};

}  // namespace veribench

#endif  // DBADAPTER_H