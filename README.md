# VeriBench

## Overview

This repository implements VeriBench, a benchmark framework for database systems with verifiability. VeriBench evaluates all main components of a verifiability-enabled database, namely authenticated data structure, query processing, verification, and auditing. It provides comprehensive performance analysis with verification-aware micro- and macro-benchmarks. Meanwhile, VeriBench provides user friendly extensible APIs. Users can easily add their customized workloads and new ledger databases for benchmarking.

## Code structure

- `/workload` - Contains workload interface and implementations. The implementation of workload is placed under specific folder and implementing common APIs specified in `workload.h`. In this repository, we provide default implementation for YCSB, SmallBank, and TPCC. 

- `/analyzer` - Provides scripts to process the log generated during workload execution.

- `/dbadapter` - Contains DB adapter interface and implementations. Users can add new adapter for the database to be evaluated by implementing a list of common database APIs defined in `dbadapter.h`. Users shall put their implementation files in a separate folder, and provide `start_server.sh` and `stop_server.sh` in the `conf` folder.

- `/dbs` - (Optional) Contains executables and shared libraries of the database under evaluation. The database can be installed anywhere else with the `db_bin_dir` specidified in the `conf.sh`

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
$ git clone https://github.com/yc1111/VeriBench
$ cd VeriBench
$ mkdir build && cd build
$ cmake .. && make
```
### Run benchmark
Fill in the configurations in `/exps/conf.sh`
```
$ cd exps
$ ./run_exps.sh
```
