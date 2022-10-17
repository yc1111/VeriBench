#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(void)
{
  CURL *curl;
  CURLcode res;

  std::string service_cert = "/xxx/xxx/LedgerBench/dbadapter/ccf/conf/external/service_cert.pem";
  std::string user_cert = "/xxx/xxx/LedgerBench/dbadapter/ccf/conf/script/user0_cert.pem";
  std::string user_key = "/xxx/xxx/LedgerBench/dbadapter/ccf/conf/script/user0_privk.pem";
  //std::string service_cert = "../../external/service_cert.pem";
  //std::string user_cert = "../user0_cert.pem";
  //std::string user_key = "..//user0_privk.pem";
  curl = curl_easy_init();
  struct curl_slist* headers;
  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  std::cout << "write op" << std::endl;
//  for (size_t i = 0; i < 100; ++i) {
    int i = 0;
    std::vector<std::string> params;
    params.emplace_back("1");
    params.emplace_back("k" + std::to_string(i));
    params.emplace_back("v" + std::to_string(i));
    std::string host = "10.10.10.237:8080";
    if(curl) {
      std::string readBuffer;
      std::string json = "{\"op\": " + params[0] + ", \"key\": \"" + params[1] + "\", \"val\": \"" + params[2] + "\"}";
      curl_easy_setopt(curl, CURLOPT_URL, "https://10.10.10.237:8080/app/ycsb");
      curl_easy_setopt(curl, CURLOPT_CAINFO, service_cert.c_str());
      curl_easy_setopt(curl, CURLOPT_SSLCERT, user_cert.c_str());
      curl_easy_setopt(curl, CURLOPT_SSLKEY, user_key.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      curl_easy_setopt(curl, CURLOPT_POST, 1);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json.size());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      res = curl_easy_perform(curl);
      curl_easy_reset(curl);

      std::cout << res << ": " << readBuffer << std::endl;
    }
//  }

    std::cout << "read op" << std::endl;
//  for (size_t i = 0; i < 100; ++i) {
//    std::vector<std::string> params;
    params.clear();
    params.emplace_back("0");
    params.emplace_back("k" + std::to_string(i));
    params.emplace_back("v" + std::to_string(i));
    if(curl) {
      std::string readBuffer;
      std::string json = "{\"op\": " + params[0] + ", \"key\": \"" + params[1] + "\", \"val\": \"" + params[2] + "\"}";
      curl_easy_setopt(curl, CURLOPT_URL, "https://10.10.10.237:8080/app/ycsb");
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

      std::cout << res << ": " << readBuffer << std::endl;
    }
//  }

  std::cout << "get latest txnid" << std::endl;
  long latest_commit = 30;
  size_t seq = 0;
  if (curl) {
    std::string readBuffer;
    std::string address = "https://10.10.10.237:8080/app/commit";
    curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_CAINFO, service_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, user_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, user_key.c_str());
    res = curl_easy_perform(curl);
    curl_easy_reset(curl);
    std::cout << readBuffer << std::endl;
    auto idx = readBuffer.find("transaction_id");
    std::string tid = readBuffer.substr(idx+17);
    tid = tid.substr(0, tid.size() - 2);
    auto seqstr = tid.substr(2);
    seq = stoul(seqstr);
  }

  std::cout << "get receipt" << std::endl;
  for (size_t i = latest_commit + 1; i <= seq ; ++i) {
    std::string readBuffer;
    std::string address = "https://10.10.10.237:8080/app/receipt?transaction_id=2." + std::to_string(seq);
    std::cout << address << std::endl;
    curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_CAINFO, service_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, user_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, user_key.c_str());
    res = curl_easy_perform(curl);
    curl_easy_reset(curl);
    std::cout << readBuffer << std::endl;
    latest_commit = i;
    break;
  }

  curl_easy_cleanup(curl);
  
  return 0;
}
