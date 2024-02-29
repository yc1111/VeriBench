#!/bin/bash

. conf.sh

mkdir -p $log_dir


begin=$1
copies=$2
let end=$begin+$copies

for ((i=$begin; i<$end; i++))
do
  docker run \
      -v $db_config_dir:/home/dbconfig \
      -v $log_dir:/home/logs \
      -v $wl_config_dir:/home/wlconfig \
      --net=host -d yc1111/veribench:$database \
      sh -c "/home/VeriBench/build/bin/run \
          -r $request_rate -t $nthread -D $duration -d $delay \
          -w $workload -W /home/wlconfig/$wlconfig -s $database \
          -c /home/dbconfig/config.properties $mode -n $i > /home/logs/client.$i.log 2>&1"
done
