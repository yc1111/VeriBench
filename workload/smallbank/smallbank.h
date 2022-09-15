#ifndef WORKLOAD_SMALLBANK_H
#define WORKLOAD_SMALLBANK_H

#include <vector>
#include <string>

#include "workload.h"

namespace ledgerbench {

struct SmallBankTask : public Task {
  int acc1;
  int acc2;
  int balance;
};

class SmallBank : public Workload {
 public:
  SmallBank() = default;
  ~SmallBank() = default;

  std::unique_ptr<Task> NextTask();
  int ExecuteTxn(Task* task, DB* db, Promise* promise);

 private:
  int Amalgamate(const SmallBankTask* task, DB* db, Promise* promise);
  int GetBalance(const SmallBankTask* task, DB* db, Promise* promise);
  int UpdateBalance(const SmallBankTask* task, DB* db, Promise* promise);
  int UpdateSaving(const SmallBankTask* task, DB* db, Promise* promise);
  int SendPayment(const SmallBankTask* task, DB* db, Promise* promise);
  int WriteCheck(const SmallBankTask* task, DB* db, Promise* promise);

  const std::string saving = "savingStore_";
  const std::string checking = "checkingStore_";
};

}  // namespace ledgerbench

#endif  // WORKLOAD_SMALLBANK_H