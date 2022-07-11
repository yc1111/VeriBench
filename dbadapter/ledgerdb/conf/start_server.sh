#!/bin/bash

. conf.sh

echo "Starting TimeStampServer replicas.."
$db_bin_dir/start_replica.sh tss $db_config_dir/shard.tss.config "$db_bin_dir/timeserver" $log_dir

for ((i=0; i<$nshard; i++))
do
  echo "Starting shard$i replicas.."
  $db_bin_dir/start_replica.sh shard$i $db_config_dir/shard$i.config "$db_bin_dir/strongstore -m occ -w ycsb -v 1 -k 100000 -e 0 -s 0 -N $nshard -n $i -t $blocktime" $log_dir
done