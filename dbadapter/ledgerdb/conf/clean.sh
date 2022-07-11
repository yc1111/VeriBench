#!/bin/bash

. conf.sh

for host in `cat ${db_config_dir}/replicas`
do
  echo ${host}
  ssh ${host} "killall -9 strongstore; rm -rf /tmp/replica*.store; rm -rf /tmp/ustore*.store; rm -rf $log_dir; \
                  mkdir -p /tmp/testdb; mkdir -p /tmp/testledger; rm -rf /tmp/testdb/*; rm -rf /tmp/testledger/*; \
                  rm -rf /tmp/qldb.store; rm -f /tmp/wal; rm -f /tmp/index;"
done

for host in `cat ${db_config_dir}/clients`
do
  echo ${host}
  ssh ${host} "killall -9 run; rm -rf $log_dir; rm -rf /tmp/auditor*.store"
done

for host in `cat ${db_config_dir}/auditors`
do
  echo ${host}
  ssh ${host} "killall -9 run; rm -rf $log_dir; rm -rf /tmp/auditor*.store;"
done

for host in `cat ${db_config_dir}/timeservers`
do
  echo $host
  ssh ${host} "killall -9 timeserver; rm -rf /tmp/replica*.store; rm -rf /tmp/ustore*.store; rm -rf $log_dir;"
done