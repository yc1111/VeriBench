#include "ccf.h"

#include <fstream>
#include <iostream>

namespace ledgerbench {

CCF::CCF(const char* config) {
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
  host = props["host"];
  service_cert = props["service_cert"];
  user_cert = props["user_cert"];
  user_key = props["user_key"];
  curl = curl_easy_init();
  verifier = curl_easy_init();
  latest_commit = GetLatestCommit();
}

CCF::~CCF() {
  curl_easy_cleanup(curl);
  curl_easy_cleanup(verifier);
}

size_t CCF::GetLatestCommit() {
  CURLcode res;
  struct curl_slist* headers;
  std::string readBuffer;
  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  std::string address = "https://" + host + "/app/commit";
  curl_easy_setopt(verifier, CURLOPT_URL, address.c_str());
  curl_easy_setopt(verifier, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(verifier, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(verifier, CURLOPT_CAINFO, service_cert.c_str());
  curl_easy_setopt(verifier, CURLOPT_SSLCERT, user_cert.c_str());
  curl_easy_setopt(verifier, CURLOPT_SSLKEY, user_key.c_str());
  res = curl_easy_perform(verifier);
  curl_easy_reset(verifier);

  auto idx = readBuffer.find("transaction_id");
  std::string tid = readBuffer.substr(idx + 19);
  tid = tid.substr(0, tid.size() - 2);
  auto seq = stoul(tid);
  return seq;
}

bool CCF::Verify(Promise* promise) {
  if (!verifier) return 1;
  auto seq = GetLatestCommit();
  
  CURLcode res;
  struct curl_slist* headers;
  std::string readBuffer;
  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  
  //int count = 0;
  for (size_t i = latest_commit + 160; i <= seq; i = i + 160) {
    std::string address = "https://" + host +
        "/app/receipt?transaction_id=2." + std::to_string(i);
    curl_easy_setopt(verifier, CURLOPT_URL, address.c_str());
    curl_easy_setopt(verifier, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(verifier, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(verifier, CURLOPT_CAINFO, service_cert.c_str());
    curl_easy_setopt(verifier, CURLOPT_SSLCERT, user_cert.c_str());
    curl_easy_setopt(verifier, CURLOPT_SSLKEY, user_key.c_str());
    res = curl_easy_perform(verifier);
    curl_easy_reset(verifier);
    //++count;
    latest_commit = i;
  }

  return res;
}

int CCF::StoredProcedure(std::vector<std::string> params, const OpType& type,
    Promise* promise) {
  if (!curl) return 1;
  CURLcode res;
  struct curl_slist* headers;
  std::string readBuffer;
  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  switch(type) {
    case OpType::kYCSB: {
      std::string address = "https://" + host + "/app/ycsb";
      std::string json = "{\"op\": " + params[0] + ", \"key\": \"" + params[1] + "\", \"val\": \"" + params[2] + "\"}";
      curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      curl_easy_setopt(curl, CURLOPT_CAINFO, service_cert.c_str());
      curl_easy_setopt(curl, CURLOPT_SSLCERT, user_cert.c_str());
      curl_easy_setopt(curl, CURLOPT_SSLKEY, user_key.c_str());
      curl_easy_setopt(curl, CURLOPT_POST, 1);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json.size());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      res = curl_easy_perform(curl);
      curl_easy_reset(curl);
      break;
    }
    default:
      return 1;
  }
  return res;
}

size_t CCF::WriteCallback(void *contents, size_t size,
    size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

}  // namespace ledgerbench
