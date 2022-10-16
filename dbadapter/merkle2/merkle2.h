//
// Created by zch on 2022/10/7.
//

#ifndef DBADAPTER_MERKLE2_H
#define DBADAPTER_MERKLE2_H


#include "dbadapter.h"

extern "C" {
#include "merklesquare_go.h"
}

namespace ledgerbench {

  struct Merkle2Promise : public Promise {
    size_t size() { return 0; };
  };

  class Merkle2 : public DB {
   public:
    Merkle2(const char* config, int n);

    ~Merkle2();

    void Begin() {};

    int Commit(Promise* promise) { return 0; };

    void Abort() {};

    void Init();

    int Get(const std::vector<std::string> &keys,
            std::vector<std::string> &vals,
            Promise *promise);

    int Get(const std::string &key,
            std::string *vals,
            Promise *promise);

    int Put(const std::vector<std::string> &keys,
            const std::vector<std::string> &vals);

    void Provenance(const std::string &keys, int n);

    int Range(const std::string &from,
              const std::string &to,
              std::map<std::string, std::string> &values,
              Promise *promise);

    bool Verify(Promise *promise);

    int StoredProcedure(std::vector<std::string>, const OpType& type,
        Promise* promise) { return 0; };

   private:
    std::string clientid;
    merklesquareclient_handle_t goobj_;
  };
}


#endif  // DBADAPTER_MERKLE2_H
