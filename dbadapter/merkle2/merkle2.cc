//
// Created by zch on 2022/10/7.
//

#include "merkle2.h"
#include <fstream>

ledgerbench::MerkleSquare::MerkleSquare(const char* config) {
    std::ifstream infile(config);
    std::string line;
    std::map<std::string, std::string> props;
    while (std::getline(infile, line)) {
      size_t idx = line.find("=");
      std::string key = line.substr(0, idx);
      std::string val = line.substr(idx + 1);
      props.emplace(key, val);
    }
    infile.close();

    std::string s = props["server"] + ":49563";
    server = new char[s.size()];
    std::copy(s.begin(), s.end(), server);

    std::string v = props["verifier"] + ":49563";
    verifier = new char[v.size()];
    std::copy(v.begin(), v.end(), verifier);

    std::string a = props["auditor"] + ":49563";
    auditor = new char[a.size()];
    std::copy(a.begin(), a.end(), auditor);

    merklesquareclient_new(server, auditor, verifier);
}

ledgerbench::MerkleSquare::~MerkleSquare() {
    merklesquareclient_delete(goobj_);
    delete server;
    delete verifier;
    delete auditor;
}


int ledgerbench::MerkleSquare::Get(const std::vector<std::string> &keys, std::vector<std::string> &vals,
                                   ledgerbench::Promise *promise) {
    int r = 0;
    size_t n = keys.size();
    for (size_t i = 0; i < n; i++) {
        char buf[512];
        rc = merklesquareclient_get(goobj_, (char *) keys[i].c_str(), buf);
        vals.emplace_back(buf);
        r += rc;
    }
    return r;
}

int ledgerbench::MerkleSquare::Get(const std::string &key, std::string *vals, ledgerbench::Promise *promise) {
    char buf[512];
    rc = merklesquareclient_get(goobj_, (char *) key.c_str(), buf);
    vals->clear();
    vals->append(buf);
    return rc;
}

void ledgerbench::MerkleSquare::Put(const std::vector<std::string> &keys, const std::vector<std::string> &vals) {
    size_t n = keys.size();
    for (size_t i = 0; i < n; i++) {
        merklesquareclient_set(goobj_, (char *) (keys[i].c_str()), (char *) vals[i].c_str());
    }
}

void ledgerbench::MerkleSquare::Provenance(const std::string &keys, int n) {
    // not support
}

int ledgerbench::MerkleSquare::Range(const std::string &from, const std::string &to,
                                     std::map<std::string, std::string> &values, ledgerbench::Promise *promise) {
    // not support
    return 0;
}

bool ledgerbench::MerkleSquare::Verify(ledgerbench::Promise *promise) {
    // no need
    return true;
}
