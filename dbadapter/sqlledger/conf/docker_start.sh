#!/bin/bash

. conf.sh

start_server () {
  shard=$1    # which shard is this
  fname=$2    # path to config file
  cmd=$3      # command to run
  
  replica=$(head -n1 ${db_config_dir}/$fname | awk '{print $2}')
  let replica=2*$replica+1
  
  for ((i=0; i<$replica; i++))
  do
    let line=$i+2 
    server=$(cat ${db_config_dir}/$fname | sed -n ${line}p | awk -F'[ :]' '{print $2}')
    command="ssh $server \"docker run -v ${db_config_dir}:/home/dbconfig \
                                      --net=host yc1111/veribench:sqlledger \
                                      sh -c '$cmd -c /home/dbconfig/${fname} -i $i > /home/$shard.replica$i.log' &\" &"
    echo $command
    eval $command
  done
}

update_config () {
  # generate replica configuration
  python $db_config_dir/gen_conf.py $nshard $exp_dir $db_config_dir

  # update config.properties
  sed -i -e "s/nShard=.*/nShard=${nshard}/" ${db_config_dir}/config.properties

  # sync across nodes
  for host in `cat ${exp_dir}/clients ${exp_dir}/workers`
  do
    rsync -r --delete ${db_config_dir} $host:${db_config_dir}/../
  done
}

update_config

echo "Starting TimeStampServer replicas.."
start_server tss shard.tss.config "/usr/local/bin/timeserver"

for ((n=0; n<$nshard; n++))
do
  echo "Starting shard$i replicas.."
  start_server shard$n shard$n.config "/usr/local/bin/strongstore -m occ -w $workload -v $nversion -k 100000 -f $initdata -e 0 -s 0 -N $nshard -n $n -t $blocktime"
done

