#!/bin/bash

cmd=$1
begin=$2
copies=$3
log_dir=$4

let end=$begin+$copies

for ((i=$begin; i<$end; i++))
do
  command="$cmd > $log_dir/client.$i.log 2>&1 &"
  eval $command
done