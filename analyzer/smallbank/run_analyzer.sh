#!/bin/bash

. conf.sh

mkdir -p $res_dir
for host in ${clients[@]}                                                       
do                                                                              
  ssh $host "source ~/.profile; cat $log_dir/client.*.log | sort -g -k 3 > $log_dir/client.log; \
             rm -f $log_dir/client.*.log; mkdir -p $res_dir; \
             python $ana_dir/process_smallbank.py $log_dir/client.log $duration $res_dir/result;
             rsync $res_dir/result $master:$res_dir/client.$host.log;"
done

echo "Processing logs"
ssh $master "source ~/.profile; python $ana_dir/aggregate_smallbank.py $res_dir $res_dir/result; \
             rm -f $res_dir/client.*.log;"
