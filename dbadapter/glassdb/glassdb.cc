#include "glassdb.h"

#include <fstream>

namespace veribench {

GlassDB::GlassDB(const char* config) {
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

void GlassDB::Begin() {
  client->Begin();
}

int GlassDB::Commit(Promise* promise) {
  std::map<int, std::map<uint64_t, std::vector<std::string>>> unverified_list;
  auto status = client->Commit(unverified_list);
  Merge(unverified_list, promise);
  return status? 0 : 1;
}

void GlassDB::Abort() {
  client->Abort();
}

int GlassDB::Get(const std::vector<std::string>& keys,
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

  // for (size_t i = 0; i < keys.size(); ++i) {
  //   client->Get(keys[i]);
  // }
  return status;
}

int GlassDB::Get(const std::string& key, std::string* vals,
                  Promise* promise) {
  return client->Get(key);
}

int GlassDB::Put(const std::vector<std::string>& keys,
                  const std::vector<std::string>& vals) {
  for (size_t i = 0; i < keys.size(); ++i) {
    client->Put(keys[i], vals[i]);
  }
  return 0;
}

void GlassDB::Provenance(const std::string& key, int n) {
  client->GetNVersions(key, n);
}

int GlassDB::Range(const std::string& from,
                     const std::string& to,
                     std::map<std::string, std::string>& values,
                     Promise* promise) {
  std::map<int, std::map<uint64_t, std::vector<std::string>>> unverified_list;
  auto status = client->GetRange(from, to, values, unverified_list);
  Merge(unverified_list, promise);
  return status;
}

bool GlassDB::Verify(Promise* promise) {
  GlassDBPromise* p = static_cast<GlassDBPromise*>(promise);
  if (p->verify_map.size() == 0) return false;
  std::map<int, std::map<uint64_t, std::vector<std::string>>> unverified_keys;
  { 
    std::unique_lock<std::mutex> lock(lck);
    for (auto& replica : p->verify_map) {
      std::map<uint64_t, std::vector<std::string>> blocks;
      for (auto& block : replica.second) {
        std::vector<std::string> keys;
        for (auto& k : block.second) {
          keys.push_back(k);
        }
        blocks.emplace(block.first, keys);
      }
      unverified_keys.emplace(replica.first, blocks);
    }
  }
  client->Verify(unverified_keys);
  { 
    std::unique_lock<std::mutex> lock(lck);
    for (auto& replica : unverified_keys) {
      for (auto& block : replica.second) {
        p->verify_map[replica.first].erase(block.first);
      }
      if (p->verify_map[replica.first].empty()) {
        p->verify_map.erase(replica.first);
      }
    }
  }

  return true;
}

void GlassDB::Merge(const
    std::map<int, std::map<uint64_t, std::vector<std::string>>>& newmap,
    Promise* promise) {
  GlassDBPromise* p = reinterpret_cast<GlassDBPromise*>(promise);
  std::unique_lock<std::mutex> write(lck);
  for (auto& replicas : newmap) {
    if (p->verify_map.find(replicas.first) != p->verify_map.end()) {
      for (auto& blocks : replicas.second) {
        if (p->verify_map[replicas.first].find(blocks.first) !=
            p->verify_map[replicas.first].end()) {
          for (auto& keys : blocks.second) {
            p->verify_map[replicas.first][blocks.first].emplace(keys);
          }
        } else {
          std::set<std::string> ks(blocks.second.begin(),
              blocks.second.end());
          p->verify_map[replicas.first].emplace(blocks.first, ks);
        }
      }
    } else {
      std::map<uint64_t, std::set<std::string>> blocks;
      for (auto& block : replicas.second) {
        std::set<std::string> keys;
        for (auto& k : block.second) {
          keys.emplace(k);
        }
        blocks.emplace(block.first, keys);
      }
      p->verify_map.emplace(replicas.first, blocks);
    }
  }
}

}
