#!/bin/bash

. conf.sh
mkdir -p $log_dir

python $db_config_dir/gen_conf.py $nshard $db_config_dir
sed -i -e "s/nShard=[0-9]*/nShard=${nshard}/" $db_config_dir/config.properties

echo "Starting TimeStampServer replicas.."
$db_config_dir/start_replica.sh tss $db_config_dir/shard.tss.config "$db_bin_dir/timeserver" $log_dir

for ((i=0; i<$nshard; i++))
do
  echo "Starting shard$i replicas.."
  $db_config_dir/start_replica.sh shard$i $db_config_dir/shard$i.config "$db_bin_dir/strongstore -m occ -w $workload -v 1 -k 100000 -f $initdata -e 0 -s 0 -N $nshard -n $i -t $blocktime" $log_dir
done
