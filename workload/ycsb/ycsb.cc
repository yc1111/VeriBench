#include "ycsb.h"

#include <map>
#include <math.h>
#include <fstream>
#include <sys/time.h>
#include <iostream>

namespace veribench {

YCSB::YCSB(const char* config) {
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
  row_num = std::stoul(props["rowno"]);
  readperc = std::stoi(props["readpercentage"]);
  writeperc = std::stoi(props["writepercentage"]);
  provperc = std::stoi(props["provenancepercentage"]);
  rangesize = std::stoi(props["rangesize"]);
  theta = std::stod(props["theta"]);
  timeval t0;
  gettimeofday(&t0, NULL);
  srand(t0.tv_sec*1000000 + t0.tv_usec);
  zetan = zeta(row_num);
  zeta_2_theta = zeta(2);
}

std::unique_ptr<Task> YCSB::NextTask() {
  std::unique_ptr<YCSBTask> task(new YCSBTask());
  task->key = std::to_string(zipf(row_num));
  task->val = NextValue(20);

  auto roll = rand() % 100;
  if (roll < readperc) {
    task->op = OpType::kYCSB_RD;
    task->n = 0;
    task->from = "";
    task->to = "";
  } else if (roll < writeperc + readperc) {
    task->op = OpType::kYCSB_WR;
    task->n = 0;
    task->from = "";
    task->to = "";
  } else if (roll < writeperc + readperc + provperc) {
    task->op = OpType::kPROVENANCE;
    task->n = rand() % 10;
    task->from = "";
    task->to = "";
  } else {
    task->op = OpType::kRANGE;
    auto from = rand() % 89999 + 10000;
    auto range = rand() % 10 + rangesize;
    auto to = from + range;
    task->from = std::to_string(from);
    task->to = std::to_string(to);
    task->n = 0;
  }
  return task;
}

int YCSB::ExecuteTxn(Task* task, DB* client, Promise* promise) {
  const YCSBTask* t = static_cast<const YCSBTask*>(task);
  client->Begin();
  switch (t->op) {
    case OpType::kYCSB_WR:
    {
      auto status = client->Put({t->key}, {t->val});
      if (status > 0) return status;
      break;
    }
    case OpType::kYCSB_RD:
    {
      std::string value;
      auto status = client->Get(t->key, &value, promise);
      if (status > 0) return status;
      break;
    }
    case OpType::kPROVENANCE:
    {
      client->Provenance(t->key, t->n);
      break;
    }
    case OpType::kRANGE:
    {
      std::map<std::string, std::string> result;
      client->Range(t->from, t->to, result, promise);
      break;
    }
    default:
      break;
  }
  return client->Commit(promise);
}

int YCSB::StoredProcedure(Task* task, DB* client, Promise* promise) {
  const YCSBTask* t = static_cast<const YCSBTask*>(task);
  std::vector<std::string> params;
  params.emplace_back(std::to_string(t->op));
  params.emplace_back(t->key);
  params.emplace_back(t->val);
  params.emplace_back(std::to_string(t->n));
  params.emplace_back(t->from);
  params.emplace_back(t->to);
  return client->StoredProcedure(params, t->op, promise);
}


double YCSB::zeta(size_t n) {
  double sum = 0;
  for(uint32_t i = 1; i <= n; ++i) {
    sum += pow(1.0/i,theta);
  }
  return sum;
}

size_t YCSB::zipf(size_t n) {
  double alpha = 1/(1 - theta);
  double eta = (1 - pow(2.0 / n, 1 - theta)) /
    (1 - zeta_2_theta / zetan);
  double u = (double)(rand() % 10000000) / 10000000;
  double uz = u*zetan;
  if(uz < 1) return 1;
  if(uz < 1 + pow(0.5,theta)) return 2;
  return 1 + (uint32_t)(n * pow(eta*u - eta + 1, alpha));
}

std::string YCSB::NextValue(int n) {
  std::string res;
  for (int i = 0; i < n; ++i) {
    res += rand() % 10 + 48;
  }
  return res;
}

}  // namespace veribench
