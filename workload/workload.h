#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <memory>
#include "dbadapter.h"

namespace veribench {

struct Task {
  OpType op;
};

class Workload {
 public:
  virtual std::unique_ptr<Task> NextTask() = 0;
  virtual int ExecuteTxn(Task* t, DB* db, Promise* promise) = 0;
  virtual int StoredProcedure(Task* task, DB* db, Promise* promise) = 0;
};

}  // namespace veribench

#endif  // WORKLOAD_H