#!/bin/bash

. conf.sh

primary=`sed -n "1p" $db_config_dir/replicas`
ssh $primary "$db_config_dir/start_primary.sh $primary $db_config_dir"

for ((i=2; i<=$nshard; i++))
do
  echo "Starting shard$i replicas.."
  server=`sed -n "${i}p" $db_config_dir/replicas`
  echo $server
  ssh $server "$db_config_dir/start_nodes.sh $server $primary $db_config_dir"
done

sleep 5
$db_config_dir/script/init.sh $primary:8080 $db_config_dir $workload
