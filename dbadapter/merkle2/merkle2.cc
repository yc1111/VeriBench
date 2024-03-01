//
// Created by zch on 2022/10/7.
//

#include "merkle2.h"
#include <fstream>
#include <iostream>

namespace veribench {

Merkle2::Merkle2(const char* config, int n) {
  std::ifstream infile(config);
  std::string line;
  std::map<std::string, std::string> props;
  while (std::getline(infile, line)) {
    size_t idx = line.find("=");
    std::cout << idx << "/" << line.size() << std::endl;
    std::string key = line.substr(0, idx);
    std::string val = line.substr(idx + 1);
    props.emplace(key, val);
  }
  infile.close();

  std::string s = props["server"] + ":49563";
  std::string v = props["verifier"] + ":49562";
  std::string a = props["auditor"] + ":49564";
  goobj_ = merklesquareclient_new((char*)s.c_str(), (char*)a.c_str(), (char*)v.c_str());
  clientid = std::to_string(n) + "_";
}

Merkle2::~Merkle2() {
  merklesquareclient_delete(goobj_);
}

void Merkle2::Init() {
  std::vector<std::string> keys, vals;
  for (int i = 0; i <= 625; ++i) {
    keys.emplace_back(clientid + std::to_string(i));
    vals.emplace_back(std::to_string(i));
  }
  Put(keys, vals);
}


int Merkle2::Get(const std::vector<std::string> &keys,
                 std::vector<std::string> &vals,
                 Promise *promise) {
  int r = 0;
  size_t n = keys.size();
  for (size_t i = 0; i < n; i++) {
    char buf[512] = {0};
    auto key = clientid + keys[i];
    auto rc = merklesquareclient_get(goobj_, (char *) key.c_str(), buf);
    vals.emplace_back(buf);
    r += rc;
  }
  return r;
}

int Merkle2::Get(const std::string &key,
                 std::string *vals,
                 Promise *promise) {
  char buf[512] = {0};
  std::string k = clientid + key;
  auto rc = merklesquareclient_get(goobj_, (char*)k.c_str(), buf);
  vals->clear();
  vals->append(buf);
  return rc;
}

int Merkle2::Put(const std::vector<std::string> &keys,
                  const std::vector<std::string> &vals) {
  int r = 0;
  for (size_t i = 0; i < keys.size(); i++) {
    auto key = clientid + keys[i];
    auto rc = merklesquareclient_set(goobj_, (char *) key.c_str(), (char *) vals[i].c_str());
    r += rc;
  }
  return r;
}

void Merkle2::Provenance(const std::string &keys, int n) {
  // not support
}

int Merkle2::Range(const std::string &from, const std::string &to,
                   std::map<std::string, std::string> &values,
                   Promise *promise) {
  // not support
  return 0;
}

bool Merkle2::Verify(Promise *promise) {
    // no need
    return true;
}

}  // namespace veribench
