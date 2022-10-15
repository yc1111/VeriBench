// To test `merklesquare_go.h`
// run `g++ -o test _test.cpp ../lib/merklesquare_go.a -lpthread -lcrypto` to compile

extern "C" {
#include "./merklesquare_go.h"
}
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

class merkle2 {
 public:
  merkle2() {
    goobj_ = merklesquareclient_new("10.10.10.213:49563","10.10.10.213:49564","10.10.10.213:49562");
  }
  ~merkle2() {
    merklesquareclient_delete(goobj_);
  }
  void get(const std::string& key) {
    char buf[512] = {0};
    auto rc = merklesquareclient_get(goobj_, (char*)key.c_str(), buf);
    if (rc == 0){
      std::cout<< buf <<std::endl;
    } else {
      std::cout<< "Read failed!" <<std::endl;
    }
  }

  void put(const std::vector<std::string>& key, const std::vector<std::string>& val) {
    for (size_t i = 0; i < key.size(); ++i) {
      auto rc = merklesquareclient_set(goobj_, (char*)key[i].c_str(), (char*)val[i].c_str());
      if (rc == 0) {
        std::cout << "Insert OK!" << std::endl;
      } else {
        std::cout << "Insert failed for " << key[i] << std::endl;
      }
    }
  }

 private:
  merklesquareclient_handle_t goobj_;
};

int main() {
  merkle2 m;
  std::vector<std::string> keys, vals;
  keys.emplace_back("key1");
  keys.emplace_back("key2");
  vals.emplace_back("val1");
  vals.emplace_back("val2");
  m.put(keys, vals);
  sleep(2);
  m.get("key1");
  m.get("key2");
}
