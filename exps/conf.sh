#!/bin/bash

###########################
# workload configurations
###########################

# workload name (same as the folder name under workload/)
workload=ycsb

# configuration file name under workload/<workload>/conf/ folder, if any
wlconfig=balanced

# workload init data file path, if any
initdata="/home/tpcc/tpcc"

###########################
# database configurations
###########################

# database name (same as the folder name under dbadaptor/)
database=merkle2

# configuratino file name under dbadapter/<database>/conf/ folder, if any
dbconfig="config.properties"

# execution mode of database: interactive (-i) or stored procedure (-p)
mode=-i

###########################
# experiment configurations
###########################

# number of shards
nshard=1

# number of client process per client node
nclient=20

# number of thread per client process for task generation
nthread=10

# task generation rate for each thread
request_rate=1

# experiment duration
duration=120

# verification interval for deferred verification
delay=100

# block creation interval
blocktime=100

# wait time when the cluster is booted
wait_boot=10

# node to start exp and collect results
master=10.10.10.206

###########################
# path configurations
###########################

# log path
log_dir="/data/yc/veribench/logs"

# result path
res_dir="/data/yc/veribench/results/$database"

###########################
# default environment value
###########################

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
wl_config_dir="$root_dir/workload/$workload/conf/"
wl_config_file="$root_dir/workload/$workload/conf/$wlconfig"

# experiment path
exp_dir="$root_dir/exps"

# analyzer path
ana_dir="$root_dir/analyzer/$workload"

# Machines running.
clients=`cat $exp_dir/clients`
