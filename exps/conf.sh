#!/bin/bash

# workload configurations
workload="ycsb"
wlconfig="balanced"

# database configurations
database="ledgerdb"
dbconfig="config.properties"

# experiment configurations
nshard=16
nclient=20
nthread=10
txnrate=120
duration=120
delay=1000
blocktime=100

# project root
root_dir=`pwd`/..
# ledgerbench binary
bin_dir="$root_dir/build/bin"
# database binary path
db_bin_dir="$root_dir/dbs/$database/bin"
# database config path
db_config_dir="$root_dir/dbadapter/$database/conf"
# database config file full path
db_config_file="$root_dir/dbadapter/$database/conf/$dbconfig"
# workload config file full path
wl_config_file="$root_dir/workload/$workload/conf/$wlconfig"
# experiment path
exp_dir="$root_dir/exps"
# log path
log_dir="/data/yc/logs"
# result path
res_dir="/data/yc/results"
# analyzer path
ana_dir="$root_dir/analyzer/$workload"

# Machines running.
clients=`cat $db_config_dir/clients`
master=10.10.10.206
