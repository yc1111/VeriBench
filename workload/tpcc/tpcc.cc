#include "tpcc.h"

#include <chrono>
#include <fstream>
#include <map>
#include <numeric>
#include <sys/time.h>
#include <iostream>

#include "boost/algorithm/string.hpp"

namespace ledgerbench {

TPCC::TPCC() {
  timeval t0;
  gettimeofday(&t0, NULL);
  srand(t0.tv_sec*1000000 + t0.tv_usec);
  randconstcid = rand()/1023;
  randconstiid = rand()/8191;
}

std::unique_ptr<Task> TPCC::NextTask() {
  std::unique_ptr<TPCCTask> task(new TPCCTask());
  auto roll = rand()/((RAND_MAX + 1u)/100);
  if (roll < 44) {
    genNewOrder(task.get());
  } else if (roll < 88) {
    genPayment(task.get());
  } else if (roll < 92) {
    genOrderStatus(task.get());
  } else if (roll < 96) {
    genDelivery(task.get());
  } else {
    genStockLevel(task.get());
  }
  return task;
}

int TPCC::ExecuteTxn(Task* task, DB* client, Promise* promise) {
  const TPCCTask* t = static_cast<const TPCCTask*>(task);
  if (t->op == 0) {
    return execNewOrder(t, client, promise);
  } else if (t->op == 1) {
    return execPayment(t, client, promise);
  } else if (t->op == 2) {
    return execOrderStatus(t, client, promise);
  } else if (t->op == 3) {
    return execDelivery(t, client, promise);
  } else {
    return execStockLevel(t, client, promise);
  }
}

int TPCC::nurand(int a, int x, int y, int constrand) {
  auto randa = rand()/a;
  auto randb = x + rand()/(y-x);
  return (((randa | randb) + constrand) % (y - x + 1)) + x;
}

void TPCC::genNewOrder(TPCCTask* task) {
  task->op = OpType::kTPCC_NEWORDER;
  auto w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
  task->keys.emplace_back(w_id);
  auto d_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(d_id);
  auto c_id = std::to_string(nurand(1023, 1, 3000, randconstcid));
  task->keys.emplace_back(c_id);
  auto num_items = 1 + rand()/((RAND_MAX + 1u)/5);
  for (size_t i = 0; i < num_items; ++i) {
    auto i_id = std::to_string(nurand(8191, 1, 100000, randconstiid));
    task->keys.emplace_back(i_id);
    auto s_w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
    task->keys.emplace_back(s_w_id);
    auto quantity = std::to_string(5 + rand()/((RAND_MAX + 1u)/10));
    task->keys.emplace_back(quantity);
  }
}

void TPCC::genPayment(TPCCTask* task) {
  task->op = OpType::kTPCC_PAYMENT;
  auto w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
  task->keys.emplace_back(w_id);
  auto d_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(d_id);
  auto c_id = std::to_string(nurand(1023, 1, 3000, randconstcid));
  task->keys.emplace_back(c_id);
  auto payment = std::to_string(double(100 + rand()/((RAND_MAX + 1u)/500000))/100);
  task->keys.emplace_back(payment);
}

void TPCC::genDelivery(TPCCTask* task) {
  task->op = OpType::kTPCC_DELIVERY;
  auto w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
  task->keys.emplace_back(w_id);
  auto o_carrier_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(o_carrier_id);
}

void TPCC::genOrderStatus(TPCCTask* task) {
  task->op = OpType::kTPCC_ORDERSTATUS;
  auto w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
  task->keys.emplace_back(w_id);
  auto d_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(d_id);
  auto c_id = std::to_string(nurand(1023, 1, 3000, randconstcid));
  task->keys.emplace_back(c_id);
}

void TPCC::genStockLevel(TPCCTask* task) {
  task->op = OpType::kTPCC_STOCKLEVEL;
  auto w_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/5));
  task->keys.emplace_back(w_id);
  auto d_id = std::to_string(1 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(d_id);
  auto threshold = std::to_string(10 + rand()/((RAND_MAX + 1u)/10));
  task->keys.emplace_back(threshold);
}

std::vector<std::string> TPCC::parseFields(const std::string& row) {
  std::vector<std::string> results;
  boost::split(results, row, [](char c){return c == ',';});
  return results;
}

std::string TPCC::fieldsToString(const std::vector<std::string> fields) {
  std::string delimiter = ",";
  std::string result = std::accumulate(std::next(fields.begin()),
      fields.end(), fields[0],
      [&delimiter](std::string& a, const std::string& b) {
          return a + delimiter + b;
      });
  return result;
}

int TPCC::execNewOrder(const TPCCTask* task, DB* db, Promise* promise) {
  db->Begin();
  auto w_id = task->keys[0];
  auto d_id = task->keys[1];
  auto c_id = task->keys[2];
  std::vector<std::string> querykeys;
  std::vector<std::string> putkeys, putvals;
  querykeys.emplace_back("w_tax_" + w_id);
  querykeys.emplace_back("d_tax_" + d_id + "_" + w_id);
  querykeys.emplace_back("c_discount_" + c_id + "_" + d_id + "_" + w_id);

  std::vector<std::string> i_ids;
  std::vector<std::string> s_w_ids;
  std::vector<uint32_t> quantities;
  size_t idx = 3;
  while (idx < task->keys.size()) {
    auto i_id = task->keys[idx];
    querykeys.emplace_back("i_" + i_id);
    i_ids.emplace_back(i_id);
    ++idx;

    auto s_w_id = task->keys[idx];
    querykeys.emplace_back("s_" + i_id + "_" + s_w_id);
    s_w_ids.emplace_back(s_w_id);
    ++idx;

    auto quantity = std::stoi(task->keys[idx]);
    quantities.emplace_back(quantity);
    ++idx;
  }

  std::vector<std::string> results;
  db->Get(querykeys, results, promise);
  std::map<std::string, std::string> values;
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values[querykeys[i]] = results[i];
  }

  uint64_t order_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  auto warehouse = values["w_tax_" + w_id];
  auto district = parseFields(values["d_tax_" + d_id + "_" + w_id]);
  auto customer = values["c_discount_" + c_id + "_" + d_id + "_" + w_id];
  auto w_tax = std::stod(warehouse);
  auto d_tax = std::stod(district[0]);
  auto c_discount = std::stod(customer);
  size_t o_id = std::stol(district[1]);
  int o_all_local = 1;
  for (size_t i = 0; i < i_ids.size(); ++i) {
    auto item = parseFields(values["i_" + i_ids[i]]);
    auto stock = parseFields(values["s_" + i_ids[i] + "_" + s_w_ids[i]]);
    if (stock[1].compare(w_id) != 0) o_all_local = 0;
    auto price = std::stod(item[3]);
    auto dist_info = stock[2 + std::stod(d_id)];
    auto s_quantity = std::stol(stock[2]);
    if (s_quantity > quantities[i]) {
      s_quantity -= quantities[i];
    } else {
      s_quantity = s_quantity - quantities[i] + 100;
    }
    stock[2] = std::to_string(s_quantity);
    putkeys.emplace_back("s_" + stock[0] + "_" + stock[1]);
    putvals.emplace_back(fieldsToString(stock));
    
    auto ol_amount =
        quantities[i] * price * (1 + w_tax + d_tax) * (1 - c_discount);
    auto ol_key = "ol_" + std::to_string(o_id) + "_" + d_id + "_" + w_id +
                  "_" + std::to_string(i+1);
    std::string ol_val = std::to_string(o_id) + "," + 
                         d_id + "," +
                         w_id + "," +
                         std::to_string(i + 1) + "," +
                         i_ids[i] + "," +
                         s_w_ids[i] + ",," +
                         std::to_string(quantities[i]) + "," +
                         std::to_string(ol_amount) + "," +
                         dist_info;
    putkeys.emplace_back(ol_key);
    putvals.emplace_back(ol_val);
  }
  auto order_key = "o_" + std::to_string(o_id) + "_" + d_id + "_" + w_id;
  auto order_val = std::to_string(o_id) + "," + 
                   d_id + "," + 
                   w_id + "," + 
                   c_id + "," + 
                   std::to_string(order_time) + ",," +  // carrier_id empty
                   std::to_string(i_ids.size()) + "," + 
                   std::to_string(o_all_local);
  putkeys.emplace_back(order_key);
  putvals.emplace_back(order_val);
  putkeys.emplace_back("c_last_order_" + c_id + "_" + d_id + "_" + w_id);
  putvals.emplace_back(std::to_string(o_id));
  district[1] = std::to_string(o_id + 1);
  putkeys.emplace_back("d_tax_" + d_id + "_" + w_id);
  putvals.emplace_back(fieldsToString(district));
  db->Put(putkeys, putvals);
  return db->Commit(promise);
}

int TPCC::execPayment(const TPCCTask* task, DB* db, Promise* promise) {
  db->Begin();
  auto w_id = task->keys[0];
  auto d_id = task->keys[1];
  auto c_id = task->keys[2];
  auto payment = std::stod(task->keys[3]);
  std::vector<std::string> querykeys;
  std::vector<std::string> putkeys, putvals;
  querykeys.emplace_back("w_" + w_id);
  querykeys.emplace_back("d_" + d_id + "_" + w_id);
  querykeys.emplace_back("c_" + c_id + "_" + d_id + "_" + w_id);
  std::map<std::string, std::string> values;
  std::vector<std::string> results;
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values[querykeys[i]] = results[i];
  }

  auto warehouse = parseFields(values["w_" + w_id]);
  auto district = parseFields(values["d_" + d_id + "_" + w_id]);
  auto customer = parseFields(values["c_" + c_id + "_" + d_id + "_" + w_id]);

  auto w_ytd = std::stod(warehouse[7]);
  auto d_ytd = std::stod(district[8]);
  auto c_balance = std::stod(customer[15]);
  auto c_ytd_payment = std::stod(customer[16]);
  auto c_payment_cnt = std::stod(customer[17]);

  warehouse[7] = std::to_string(w_ytd + payment);
  district[8] = std::to_string(d_ytd + payment);
  customer[15] = std::to_string(c_balance - payment);
  customer[16] = std::to_string(c_ytd_payment + payment);
  customer[17] = std::to_string(c_payment_cnt + 1);
  
  putkeys.emplace_back("w_" + w_id);
  putvals.emplace_back(fieldsToString(warehouse));
  putkeys.emplace_back("d_" + d_id + "_" + w_id);
  putvals.emplace_back(fieldsToString(district));
  putkeys.emplace_back("c_" + c_id + "_" + d_id + "_" + w_id);
  putvals.emplace_back(fieldsToString(customer));
  uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  putkeys.emplace_back("h_" + c_id + "_" + d_id + "_" + w_id + "_" +
      std::to_string(time));
  putvals.emplace_back(task->keys[3]);
  return db->Commit(promise);
}

int TPCC::execOrderStatus(const TPCCTask* task, DB* db, Promise* promise) {
  db->Begin();
  auto w_id = task->keys[0];
  auto d_id = task->keys[1];
  auto c_id = task->keys[2];
  std::vector<std::string> querykeys;
  std::vector<std::string> putkeys, putvals;
  querykeys.emplace_back("w_" + w_id);
  querykeys.emplace_back("d_" + d_id + "_" + w_id);
  querykeys.emplace_back("c_" + c_id + "_" + d_id + "_" + w_id);
  querykeys.emplace_back("c_last_order_" + c_id + "_" + d_id + "_" + w_id);
  std::map<std::string, std::string> values;
  std::vector<std::string> results;
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values[querykeys[i]] = results[i];
  }
  querykeys.clear();
  results.clear();

  auto warehouse = parseFields(values["w_" + w_id]);
  auto district = parseFields(values["d_" + d_id + "_" + w_id]);
  auto customer = parseFields(values["c_" + c_id + "_" + d_id + "_" + w_id]);
  auto o_id = values["c_last_order_" + c_id + "_" + d_id + "_" + w_id];
  if (o_id.compare("") == 0) {
    return 1;
  }

  querykeys.emplace_back("o_" + o_id + "_" + d_id + "_" + w_id);
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  auto order = parseFields(values["o_" + o_id + "_" + d_id + "_" + w_id]);
  auto ol_cnt = std::stoi(order[6]);
  for (int i = 1; i <= ol_cnt; ++i) {
    querykeys.emplace_back("ol_" + o_id + "_" + d_id + "_" + w_id + "_" +
        std::to_string(i));
  }
  
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  return 0;
}

int TPCC::execDelivery(const TPCCTask* task, DB* db, Promise* promise) {
  db->Begin();
  auto w_id = task->keys[0];
  auto carrier_id = task->keys[1];
  std::vector<std::string> querykeys;
  std::vector<std::string> putkeys, putvals;

  for (size_t i = 0; i < 10; ++i) {
    querykeys.emplace_back("d_tax_" + std::to_string(i+1) + "_" + w_id);
    querykeys.emplace_back("next_d_id_" + std::to_string(i+1) + "_" + w_id);
  }
  std::map<std::string, std::string> values;
  std::vector<std::string> results;
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

  std::vector<std::string> o_ids;
  for (size_t i = 0; i < 10; ++i) {
    auto district = parseFields(values["d_tax_" + std::to_string(i+1) + "_" + w_id]);
    size_t next_o_id = std::stol(district[1]);
    size_t d_id = 1;
    std::string next_d_id_key = "next_d_id_" + std::to_string(i+1) + "_" + w_id;
    if (values.find(next_d_id_key) != values.end() && values[next_d_id_key].size() > 0) {
      d_id = std::stol(values["next_d_id_" + std::to_string(i+1) + "_" + w_id]);
    }
    if (d_id >= next_o_id) {
      o_ids.push_back("");
      continue;
    }
    std::string o_id = std::to_string(d_id);
    o_ids.push_back(o_id);
    ++d_id;
    putkeys.emplace_back("next_d_id_" + std::to_string(i+1) + "_" + w_id);
    putvals.emplace_back(std::to_string(d_id));
    querykeys.emplace_back("o_" + o_id + "_" + std::to_string(i+1) + "_" + w_id);
  }
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  std::vector<std::string> c_ids;
  std::vector<size_t> ol_cnts;
  for (size_t i = 0; i < 10; ++i) {
    if (o_ids[i].size() == 0) {
      ol_cnts.push_back(0);
      c_ids.push_back("0");
      continue;
    }
    auto order = parseFields(values["o_" + o_ids[i] + "_" + std::to_string(i+1) + "_" + w_id]);
    auto ol_cnt = std::stoi(order[6]);
    ol_cnts.push_back(ol_cnt);
    order[5] = carrier_id;
    putkeys.emplace_back("o_" + order[0] + "_" + order[1] + "_" + order[2]);
    putvals.emplace_back(fieldsToString(order));
    for (int j = 1; j <= ol_cnt; ++j) {
      querykeys.emplace_back("ol_" + o_ids[i] + "_" + std::to_string(i+1) + "_" +
          w_id + "_" + std::to_string(j));
    }
    auto c_id = order[3];
    c_ids.push_back(c_id);
    querykeys.emplace_back("c_" + c_id + "_" + std::to_string(i+1) + "_" + w_id);
  }
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  for (size_t i = 0; i < 10; ++i) {
    if (o_ids[i].size() == 0) continue;
    double total = 0;
    for (size_t j = 1; j <= ol_cnts[i]; ++j) {
      auto orderline = parseFields(values["ol_" + o_ids[i] + "_" + std::to_string(i+1) + "_" +
          w_id + "_" + std::to_string(j)]);
      orderline[6] = std::to_string(time);
      putkeys.emplace_back("ol_" + o_ids[i] + "_" + std::to_string(i+1) + "_" +
          w_id + "_" + std::to_string(j));
      putvals.emplace_back(fieldsToString(orderline));
      auto ol_amount = std::stod(orderline[8]);
      total += ol_amount;
    }
    auto customer = parseFields(values["c_" + c_ids[i] + "_" + std::to_string(i+1) + "_" + w_id]);
    customer[15] = std::to_string(std::stod(customer[15]) + total);
    customer[18] = std::to_string(std::stod(customer[18]) + 1);
    putkeys.emplace_back("c_" + c_ids[i] + "_" + std::to_string(i+1) + "_" + w_id);
    putvals.emplace_back(fieldsToString(customer));
  }
  return db->Commit(promise);
}

int TPCC::execStockLevel(const TPCCTask* task, DB* db, Promise* promise) {
  db->Begin();
  auto w_id = task->keys[0];
  auto d_id = task->keys[1];
  auto threshold = std::stol(task->keys[2]);
  std::vector<std::string> querykeys, results;
  std::vector<std::string> putkeys, putvals;
  std::map<std::string, std::string> values;

  querykeys.emplace_back("d_tax_" + d_id + "_" + w_id);
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  auto district = parseFields(values["d_tax_" + d_id + "_" + w_id]);
  auto o_id = std::stol(district[1]);

  for (auto i = o_id - 20; i < o_id; ++i) {
    querykeys.emplace_back("o_" + std::to_string(i) + "_" + d_id+ "_" + w_id);
  }
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  std::vector<size_t> ol_cnts;
  for (auto i = o_id - 20; i < o_id; ++i) {
    std::string order_key = "o_" + std::to_string(i) + "_" + d_id+ "_" + w_id;
    if (values[order_key].size() == 0) {
      return 1;
    }
    auto order = parseFields(values[order_key]);
    size_t ol_cnt = std::stoi(order[6]);
    ol_cnts.push_back(ol_cnt);
    for (size_t j = 0; j < ol_cnt; ++j) {
      querykeys.emplace_back("ol_" + std::to_string(i) + "_" +
          d_id + "_" + w_id + "_" + std::to_string(j+1));
    }
  }
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  size_t ol_cnt_idx = 0;
  std::vector<std::string> i_ids;
  for (auto i = o_id - 20; i < o_id; ++i) {
    for (size_t j = 0; j < ol_cnts[ol_cnt_idx]; ++j) {
      std::string ol_key = "ol_" + std::to_string(i) + "_" + d_id + "_" +
          w_id + "_" + std::to_string(j+1);
      if (values[ol_key].size() == 0) {
        return 1;
      }
      auto orderline = parseFields(values[ol_key]);
      auto i_id = orderline[4];
      i_ids.push_back(i_id);
      auto s_w_id = orderline[5];
      if (s_w_id.compare(w_id) == 0) {
        querykeys.emplace_back("s_" + i_id + "_" + w_id);
      }
    }
    ol_cnt_idx++;
  }
  db->Get(querykeys, results, promise);
  for (size_t i = 0; i < querykeys.size(); ++i) {
    values.emplace(querykeys[i], results[i]);
  }
  querykeys.clear();
  results.clear();

  ol_cnt_idx = 0;
  std::set<std::string> result;
  for (auto i = o_id - 20; i < o_id; ++i) {
    for (auto i_id : i_ids) {
      auto stockstr = values["s_" + i_id + "_" + w_id];
      if (stockstr.size() > 0) {
        auto stock = parseFields(stockstr);
        auto quantity = std::stol(stock[2]);
        if (quantity < threshold) {
          result.emplace(stock[0]);
        }
      }
    }
    ol_cnt_idx++;
  }
  return 0;
}

int TPCC::StoredProcedure(Task* task, DB* db, Promise* promise) {
  const TPCCTask* t = static_cast<const TPCCTask*>(task);
  return db->StoredProcedure(t->keys, task->op, promise);
}

}  // namespace ledgerbench
