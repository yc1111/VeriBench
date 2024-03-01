#include "ccf.h"

#include <fstream>
#include <iostream>

namespace veribench {

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
  
  int count = 0;
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
    ++count;
    latest_commit = i;
  }
  std::cout << "verifynkeys " << count << std::endl;

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
  std::string json, address;
  switch(type) {
    case OpType::kYCSB_RD:
    case OpType::kYCSB_WR:
    case OpType::kRANGE:
      address = "https://" + host + "/app/ycsb";
      json = "{\"op\": " + params[0] + 
            ", \"key\": \"" + params[1] +
            "\", \"val\": \"" + params[2] +
            "\", \"n\": " + params[3] +
            ", \"from\": \"" + params[4] +
            "\", \"to\": \"" + params[5] + "\"}";
      break;
    case OpType::kPROVENANCE:
    {
      int latest = latest_commit;
      int i = stoi(params[3]);
      while (i > 0 && latest > 0) {
        auto status = HandleProvenance(params[1], latest);
        i--;
        latest--;
      }
      return 0;
    }
    case OpType::kTPCC_NEWORDER:
      address = "https://" + host + "/app/tpccneworder";
      json = "{\"keys\": [";
      for (size_t i = 0; i < params.size(); ++i) {
        json += i > 0? ", " : "";
        json += "\"" + params[i] + "\"";
      }
      json += "]}";
      break;
    case OpType::kTPCC_PAYMENT:
      address = "https://" + host + "/app/tpccpayment";
      json = "{\"keys\": [";
      for (size_t i = 0; i < params.size(); ++i) {
        json += i > 0? ", " : "";
        json += "\"" + params[i] + "\"";
      }
      json += "]}";
      break;
    case OpType::kTPCC_ORDERSTATUS:
      address = "https://" + host + "/app/tpccorderstatus";
      json = "{\"keys\": [";
      for (size_t i = 0; i < params.size(); ++i) {
        json += i > 0? ", " : "";
        json += "\"" + params[i] + "\"";
      }
      json += "]}";
      break;
    case OpType::kTPCC_DELIVERY:
      address = "https://" + host + "/app/tpccdelivery";
      json = "{\"keys\": [";
      for (size_t i = 0; i < params.size(); ++i) {
        json += i > 0? ", " : "";
        json += "\"" + params[i] + "\"";
      }
      json += "]}";
      break;
    case OpType::kTPCC_STOCKLEVEL:
      address = "https://" + host + "/app/tpccstocklevel";
      json = "{\"keys\": [";
      for (size_t i = 0; i < params.size(); ++i) {
        json += i > 0? ", " : "";
        json += "\"" + params[i] + "\"";
      }
      json += "]}";
      break;
    case OpType::kSB_AM:
      address = "https://" + host + "/app/sb_amalgamate";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    case OpType::kSB_GB:
      address = "https://" + host + "/app/sb_getbalance";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    case OpType::kSB_UB:
      address = "https://" + host + "/app/sb_updatebalance";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    case OpType::kSB_US:
      address = "https://" + host + "/app/sb_updatesav";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    case OpType::kSB_SP:
      address = "https://" + host + "/app/sb_sendpayment";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    case OpType::kSB_WC:
      address = "https://" + host + "/app/sb_writecheck";
      json = "{\"acc1\": " + params[0] + ", \"acc2\": " + params[1] + ", \"balance\": " + params[2] + "}";
      break;
    default:
      return 1;
  }
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
  return res;
}

size_t CCF::WriteCallback(void *contents, size_t size,
    size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

long CCF::HandleProvenance(std::string key, int seq) {
  CURLcode res;
  struct curl_slist* headers;
  std::string readBuffer;
  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  std::string tid_header = "x-ms-ccf-transaction-id: 2." + std::to_string(seq);
  headers = curl_slist_append(headers, tid_header.c_str());
  std::string address = "https://" + host + "/app/historical?key=" + key;
  long http_code = 0;

  curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(curl, CURLOPT_CAINFO, service_cert.c_str());
  curl_easy_setopt(curl, CURLOPT_SSLCERT, user_cert.c_str());
  curl_easy_setopt(curl, CURLOPT_SSLKEY, user_key.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  res = curl_easy_perform(curl);
  // while (http_code != 200 && http_code != 204) {
  //   res = curl_easy_perform(curl);
  // }
  curl_easy_reset(curl);
  return http_code;
}

}  // namespace veribench
