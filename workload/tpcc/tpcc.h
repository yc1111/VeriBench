#ifndef WORKLOAD_TPCC_H
#define WORKLOAD_TPCC_H

#include <vector>
#include <string>

#include "workload.h"

namespace ledgerbench {

struct TPCCTask : public Task {
  std::vector<std::string> keys;
};

class TPCC : public Workload {
 public:
  TPCC();
  ~TPCC() = default;

  std::unique_ptr<Task> NextTask();
  int ExecuteTxn(Task* task, DB* db, Promise* promise);

 private:
  int nurand(int a, int x, int y, int constrand);
  void genNewOrder(TPCCTask* task);
  void genPayment(TPCCTask* task);
  void genOrderStatus(TPCCTask* task);
  void genDelivery(TPCCTask* task);
  void genStockLevel(TPCCTask* task);
  std::vector<std::string> parseFields(const std::string& row);
  std::string fieldsToString(const std::vector<std::string> fields);
  int execNewOrder(const TPCCTask* task, DB* db, Promise* promise);
  int execPayment(const TPCCTask* task, DB* db, Promise* promise);
  int execOrderStatus(const TPCCTask* task, DB* db, Promise* promise);
  int execDelivery(const TPCCTask* task, DB* db, Promise* promise);
  int execStockLevel(const TPCCTask* task, DB* db, Promise* promise);

  int randconstcid;
  int randconstiid;
};

}  // namespace ledgerbench

#endif  // WORKLOAD_TPCC_H