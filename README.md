# LedgerBench

[Home page](https://www.comp.nus.edu.sg/~dbsystem/fintech-Ledgerbench/#/)

## Overview

This repository implements LedgerBench, a benchmark framework for ledger databases outlined in IEEE Data Engineering Bulletin [paper](http://sites.computer.org/debull/A22june/A22JUNE-CD.pdf#page=61) (June 2022). LedgerBench evaluates all main components of a ledger database, namely ledger storage, query processing, verification, and auditing. It provides comprehensive performance analysis with verification-aware micro- and macro-benchmarks. Meanwhile, LedgerBench provides user friendly extensible APIs. Users can easily add their customized workloads and new ledger databases for benchmarking.

## Code structure

- `/workload` - Contains workload interface and implementations. The implementation of workload is placed under specific folder and implementing common APIs specified in `workload.h`. In this repository, we provide default implementation for YCSB, SmallBank, and TPCC. 

- `/analyzer` - Provides scripts to process the log generated during workload execution.

- `/dbadapter` - Contains DB adapter interface and implementations. Users can add new adapter for the database to be evaluated by implementing a list of common database APIs defined in `dbadapter.h`. Users shall put their implementation files in a separate folder, and provide `start_server.sh` and `stop_server.sh` in the `conf` folder.

- `/dbs` - (Optional) Contains executables and shared libraries of the database under evaluation. The database can be installed anywhere else with the `db_bin_dir` specidified in the `conf.sh`
  - ledgerdb - We provide with an example that is built from [LedgerDatabase](https://github.com/nusdbsystem/LedgerDatabase).
    ```
    $ gitclone https://github.com/nusdbsystem/LedgerDatabase
    $ cd LedgerDatabase
    $ mkdir build && cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=<ledgerbench_root_dir>/dbs/ledgerdb .. && make
    $ make install
    ```

- `/exps` - Contains the program and scripts to start the experiments.
   - `conf.sh` - The configurations and environment to run the experiments.
   - `run_exps.sh` - Script to start the experiments

## Dependency
* gcc (&geq; 5.5)
* cmake (&geq; 3.12)
* Intel Threading Building Block (tested with tbb_2020 version)

## Quick Start
### Build
```
$ git clone https://github.com/nusdbsystem/LedgerBench
$ cd LedgerBench
$ mkdir build && cd build
$ cmake .. && make
```
### Run benchmark
Fill in the configurations in `/exps/conf.sh`
```
$ cd exps
$ ./run_exps.sh
```
