#ifndef WORKLOAD_YCSB_H
#define WORKLOAD_YCSB_H

#include <vector>
#include <string>

#include "workload.h"

namespace veribench {

struct YCSBTask : public Task {
  std::string key;
  std::string val;
  std::string from;
  std::string to;
  int n;
};

class YCSB : public Workload {
 public:
  YCSB(const char* config);
  ~YCSB() = default;

  std::unique_ptr<Task> NextTask();
  int ExecuteTxn(Task* task, DB* db, Promise* promise);
  int StoredProcedure(Task* task, DB* db, Promise* promise);

 private:
  double zeta(size_t n);
  size_t zipf(size_t n);
  std::string NextValue(int n);

  double theta;
  size_t row_num;
  double zetan;
  double zeta_2_theta;
  int readperc;
  int writeperc;
  int provperc;
  int rangesize;
};

}  // namespace veribench

#endif  // WORKLOAD_YCSB_H
