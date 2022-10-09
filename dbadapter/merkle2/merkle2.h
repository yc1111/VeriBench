//
// Created by zch on 2022/10/7.
//

#ifndef LEDGERBENCH_MERKLE2_H
#define LEDGERBENCH_MERKLE2_H


#include "dbadapter.h"

extern "C" {
#include "merklesquare_go.h"
}

namespace ledgerbench {
    class MerkleSquare : public DB {
    public:
        MerkleSquare(const char* config);

        ~MerkleSquare();

        int Get(const std::vector<std::string> &keys,
                std::vector<std::string> &vals,
                Promise *promise);

        int Get(const std::string &key,
                std::string *vals,
                Promise *promise);

        void Put(const std::vector<std::string> &keys,
                 const std::vector<std::string> &vals);

        void Provenance(const std::string &keys, int n);

        int Range(const std::string &from,
                  const std::string &to,
                  std::map<std::string, std::string> &values,
                  Promise *promise);

        bool Verify(Promise *promise);

    private:
        int rc;
        char* server;
        char* verifier;
        char* auditor;
        merklesquareclient_handle_t goobj_;
    };
}


#endif //LEDGERBENCH_MERKLE2_H
