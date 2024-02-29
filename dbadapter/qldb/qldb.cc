#include "qldb/qldb.h"

#include <fstream>

namespace veribench {

QLDB::QLDB(const char* config) {
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
  std::string clusterConfigs = props["clusterConfigs"];
  int nShard = std::stoi(props["nShard"]);
  int closestReplica = std::stoi(props["closestReplica"]);
  uint64_t skew = std::stoul(props["skew"]);
  uint64_t error = std::stoul(props["error"]);
  client = new strongstore::Client(strongstore::MODE_OCC, clusterConfigs,
      nShard, closestReplica, TrueTime(skew, error));
}

void QLDB::Begin() {
  client->Begin();
}

int QLDB::Commit(Promise* promise) {
  std::map<int, std::map<uint64_t, std::vector<std::string>>> unverified_list;
  auto status = client->Commit(unverified_list);
  return status? 0 : 1;
}

void QLDB::Abort() {
  client->Abort();
}

int QLDB::Get(const std::vector<std::string>& keys,
              std::vector<std::string>& vals,
              Promise* promise) {
  for (size_t i = 0; i < keys.size(); ++i) {
    client->BufferKey(keys[i]);
  }
  std::map<std::string, std::string> values;
  auto status = client->BatchGet(values);
  for (size_t i = 0; i < keys.size(); ++i) {
    vals.emplace_back(values[keys[i]]);
  }
  return status;
}

int QLDB::Get(const std::string& key, std::string* vals,
              Promise* promise) {
  return client->Get(key);
}

int QLDB::Put(const std::vector<std::string>& keys,
              const std::vector<std::string>& vals) {
  for (size_t i = 0; i < keys.size(); ++i) {
    client->Put(keys[i], vals[i]);
  }
  return 0;
}

void QLDB::Provenance(const std::string& key, int n) {
  client->GetNVersions(key, n);
}

int QLDB::Range(const std::string& from,
                const std::string& to,
                std::map<std::string, std::string>& values,
                Promise* promise) {
  std::map<int, std::map<uint64_t, std::vector<std::string>>> unverified_list;
  auto status = client->GetRange(from, to, values, unverified_list);
  return status;
}

bool QLDB::Verify(Promise* promise) { return true; }

}