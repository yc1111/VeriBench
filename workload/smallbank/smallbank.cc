#include "smallbank.h"

#include <random>
#include <map>

namespace ledgerbench {

std::unique_ptr<Task> SmallBank::NextTask() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> op_gen(1, 6);
  std::uniform_int_distribution<> acc_gen(1, 100000);
  std::uniform_int_distribution<> bal_gen(1, 10);

  std::unique_ptr<SmallBankTask> task(new SmallBankTask());
  task->op = op_gen(gen);
  task->acc1 = acc_gen(gen);
  task->acc2 = acc_gen(gen);
  task->balance = bal_gen(gen);
  return task;
}

int SmallBank::ExecuteTxn(Task* task, DB* db, Promise* promise) {
  const SmallBankTask* t = static_cast<const SmallBankTask*>(task);
  if (t->op == 0) {
    return Amalgamate(t, db, promise);
  } else if (t->op == 1) {
    return GetBalance(t, db, promise);
  } else if (t->op == 2) {
    return UpdateBalance(t, db, promise);
  } else if (t->op == 3) {
    return UpdateSaving(t, db, promise);
  } else if (t->op == 4) {
    return SendPayment(t, db, promise);
  } else {
    return WriteCheck(t, db, promise);
  }
}

int SmallBank::Amalgamate(const SmallBankTask* task, DB* db,
    Promise* promise) {
  std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;

  std::string acc1_key = saving + std::to_string(task->acc1);
  std::string acc2_key = checking + std::to_string(task->acc2);

  db->Begin();
  querykeys.emplace_back(acc1_key);
  querykeys.emplace_back(acc2_key);
  db->Get(querykeys, results, promise);
  int bal1 = stoi(results[0]);
  int bal2 = stoi(results[1]);
  putkeys.emplace_back(checking + std::to_string(task->acc1));
  putvals.emplace_back("0");
  putkeys.emplace_back(saving + std::to_string(task->acc2));
  putvals.emplace_back(std::to_string(bal1 + bal2));
  db->Put(putkeys, putvals);
  return db->Commit(promise);
}

int SmallBank::GetBalance(const SmallBankTask* task, DB* db,
    Promise* promise) {
  std::vector<std::string> querykeys, results;

  std::string sav_key = saving + std::to_string(task->acc1);
  std::string chk_key = checking + std::to_string(task->acc1);

  db->Begin();
  querykeys.emplace_back(sav_key);
  querykeys.emplace_back(chk_key);
  auto status = db->Get(querykeys, results, promise);
  if (status == 0) {
    int balance = stoi(results[0]) + stoi(results[1]);
  }
  return status;
}

int SmallBank::UpdateBalance(const SmallBankTask* task, DB* db, Promise* promise) {
  std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;

  std::string chk_key = checking + std::to_string(task->acc1);

  db->Begin();
  querykeys.emplace_back(chk_key);
  db->Get(querykeys, results, promise);
  putkeys.emplace_back(chk_key);
  putvals.emplace_back(std::to_string(stoi(results[0]) + task->balance));
  db->Put(putkeys, putvals);
  return db->Commit(promise);
}

int SmallBank::UpdateSaving(const SmallBankTask* task, DB* db, Promise* promise) {
  std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;

  std::string sav_key = saving + std::to_string(task->acc1);

  db->Begin();
  querykeys.emplace_back(sav_key);
  db->Get(querykeys, results, promise);
  putkeys.emplace_back(sav_key);
  putvals.emplace_back(std::to_string(stoi(results[0]) + task->balance));
  db->Put(putkeys, putvals);
  return db->Commit(promise);
}

int SmallBank::SendPayment(const SmallBankTask* task, DB* db, Promise* promise) {
  std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;

  std::string chk_key1 = checking + std::to_string(task->acc1);
  std::string chk_key2 = checking + std::to_string(task->acc2);

  db->Begin();
  querykeys.emplace_back(chk_key1);
  querykeys.emplace_back(chk_key2);
  db->Get(querykeys, results, promise);
  int bal1 = stoi(results[0]);
  int bal2 = stoi(results[1]);
  bal1 -= task->balance;
  bal2 += task->balance;
  putkeys.emplace_back(chk_key1);
  putvals.emplace_back(std::to_string(bal1));
  putkeys.emplace_back(chk_key2);
  putvals.emplace_back(std::to_string(bal2));
  db->Put(putkeys, putvals);
  return db->Commit(promise);
}

int SmallBank::WriteCheck(const SmallBankTask* task, DB* db, Promise* promise) {std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;

  std::string chk_key = checking + std::to_string(task->acc1);

  db->Begin();
  querykeys.emplace_back(chk_key);
  db->Get(querykeys, results, promise);
  int bal = stoi(results[0]);

  if (task->balance < bal) {
    putkeys.emplace_back(chk_key);
    putvals.emplace_back(std::to_string(bal - task->balance));
    db->Put(putkeys, putvals);
    return db->Commit(promise);
  }

  return 0;
}

}  // namespace ledgerbench