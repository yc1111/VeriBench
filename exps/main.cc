#include <time.h>
#include <future>

#include "tbb/concurrent_queue.h"

#include "dbadapter.h"
#include "ledgerdb/ledgerdb.h"
#include "workload.h"
#include "ycsb/ycsb.h"
#include "tpcc/tpcc.h"

using namespace std;

bool running = true;

tbb::concurrent_queue<std::unique_ptr<ledgerbench::Task>> task_queue;
std::unique_ptr<ledgerbench::Promise> promises;

void millisleep(size_t t) {
  timespec req;
  req.tv_sec = (int) t/1000;
  req.tv_nsec = (int64_t)((t%1000)*1e6); 
  nanosleep(&req, NULL); 
}

int taskGenerator(ledgerbench::Workload* wl, int timeout) {
  while (1) {
    millisleep(timeout);
    if (!running) {
      cout << "taskgen thread terminated" << endl;
      break;
    }

    auto task = wl->NextTask();
    task_queue.emplace(std::move(task));
  }
  return 0;
}

int verifyThread(ledgerbench::DB* db, size_t delay) {
  while (1) {
    millisleep(delay);
    if (!running) {
      cout << "verify thread terminated" << endl;
      return 0;
    }
    if (promises->size() == 0) continue;

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    bool vs = db->Verify(promises.get());

    gettimeofday(&t1, NULL);
    long latency = (t1.tv_sec - t0.tv_sec)*1000000 +
                   (t1.tv_usec - t0.tv_usec);

    fprintf(stderr, "%ld %ld.%06ld %ld.%06ld %ld %d %d\n", promises->size(),
        t0.tv_sec, t0.tv_usec, t1.tv_sec, t1.tv_usec, latency, vs?1:0, 9);
  }
  return 0;
}

int txnThread(ledgerbench::Workload* wl, ledgerbench::DB* db, int duration) {
  size_t nTransactions = 0;
  timeval t0, t1, t2;
  gettimeofday(&t0, NULL);

  while (1) {
    std::unique_ptr<ledgerbench::Task> task;
    while (!task_queue.try_pop(task));

    gettimeofday(&t1, NULL);
    int status = wl->ExecuteTxn(task.get(), db, promises.get());
    gettimeofday(&t2, NULL);

    long latency = (t2.tv_sec - t1.tv_sec)*1000000 +
                   (t2.tv_usec - t1.tv_usec);

    ++nTransactions;

    fprintf(stderr, "%ld %ld.%06ld %ld.%06ld %ld %d %d\n", nTransactions,
        t1.tv_sec, t1.tv_usec, t2.tv_sec, t2.tv_usec, latency, status, task->op);

    if (((t2.tv_sec-t0.tv_sec)*1000000 + (t2.tv_usec-t0.tv_usec)) >
        duration*1000000) {
      cout << "txn thread terminated" << endl;
      running = false;
      break;
    }
  }
  return 0;
}

void UsageMessage(const char *command) {
  cout << "Usage: " << command << " [options]" << endl;
  cout << "Options:" << endl;
  cout << "  -r specify request rate for each thread" << endl;
  cout << "  -t number of threads used to generate workload" << endl;
  cout << "  -D duration in seconds to run experiment" << endl;
  cout << "  -d delay time in milliseconds for deferred verification" << endl;
  cout << "  -w workload type: ycsb, tpcc, smallbank, etc." << endl;
  cout << "  -W workload config file path" << endl;
  cout << "  -s system name" << endl;
  cout << "  -c database config file path" << endl;
}

int main(int argc, char **argv) {
  // workload config
  string workloadType;
  const char *workloadConfigPath = NULL;

  // experimental config
  int txnRate = 10;
  int nThread = 10;
  int duration = 10;
  size_t delay = 0;

  // database config
  string system;
  const char *dbConfigPath = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "r:t:D:d:w:W:c:s:")) != -1) {
    switch (opt) {
    case 'r': // request rate
    {
      char *strtolPtr;
      txnRate = strtol(optarg, &strtolPtr, 10);
      if ((*optarg == '\0') || (*strtolPtr != '\0') || (txnRate < 0)) {
        UsageMessage(argv[0]);
        return 0;
      }
      break; 
    }
    case 't': // number of threads
    { 
      char *strtolPtr;
      nThread = strtol(optarg, &strtolPtr, 10);
      if ((*optarg == '\0') || (*strtolPtr != '\0') || (nThread <= 0)) {
        UsageMessage(argv[0]);
        return 0;
      }
      break;
    }
    case 'D': // Duration in seconds to run.
    { 
      char *strtolPtr;
      duration = strtol(optarg, &strtolPtr, 10);
      if ((*optarg == '\0') || (*strtolPtr != '\0') || (duration <= 0)) {
        UsageMessage(argv[0]);
        return 0;
      }
      break;
    }
    case 'd': // delay time in milliseconds
    {
      char *strtolPtr;
      delay = strtol(optarg, &strtolPtr, 10);
      if ((*optarg == '\0') || (*strtolPtr != '\0') || (delay < 0)) {
        UsageMessage(argv[0]);
        return 0;
      }
      break; 
    }
    case 'w': // workload configuration path
    { 
      workloadType = optarg;
      break;
    }
    case 'W': // workload configuration path
    { 
      workloadConfigPath = optarg;
      break;
    }
    case 's': // system name
    { 
      system = optarg;
      break;
    }
    case 'c': // DB configuration path
    { 
      dbConfigPath = optarg;
      break;
    }
    default:
      UsageMessage(argv[0]);
      return 0;
    }
  }
  
  std::unique_ptr<ledgerbench::DB> db;
  if (system.compare("ledgerdb") == 0) {
    db.reset(new ledgerbench::LedgerDB(dbConfigPath));
    promises.reset(new ledgerbench::LDBPromise());
  }

  std::unique_ptr<ledgerbench::Workload> wl;
  if (workloadType.compare("ycsb") == 0) {
    wl.reset(new ledgerbench::YCSB(workloadConfigPath));
  } else if (workloadType.compare("tpcc") == 0) {
    wl.reset(new ledgerbench::TPCC());
  }

  std::vector<std::future<int>> actual_ops;
  int interval = 1000 / txnRate;
  for (int i = 0; i < nThread; ++i) {
    actual_ops.emplace_back(std::async(std::launch::async, taskGenerator, wl.get(), interval));
  }

  actual_ops.emplace_back(std::async(std::launch::async, txnThread, wl.get(), db.get(), duration));

  // actual_ops.emplace_back(std::async(std::launch::async, verifyThread, db.get(), delay));
}
