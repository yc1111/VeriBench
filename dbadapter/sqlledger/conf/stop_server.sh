#!/bin/bash

. conf.sh

echo "Cleaning up"
$db_bin_dir/stop_replica.sh shard.tss.config > /dev/null 2>&1
for ((i=0; i<$nshard; i++))
do
  $db_bin_dir/stop_replica.sh shard$i.config > /dev/null 2>&1
done
