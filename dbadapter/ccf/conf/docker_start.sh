#!/bin/bash

. conf.sh

primary=`sed -n "1p" $exp_dir/workers`
ssh $primary "$db_config_dir/start_primary.sh $primary $db_config_dir; rsync $db_config_dir/external/service_cert.pem $master:$db_config_dir/external/;"

for ((i=2; i<=$nshard; i++))
do
  echo "Starting shard$i replicas.."
  server=`sed -n "${i}p" $exp_dir/workers`
  echo $server
  rsync $db_config_dir/external/service_cert.pem $server:$db_config_dir/external/
  ssh $server "$db_config_dir/start_nodes.sh $server $primary $db_config_dir"
done

sleep 5
$db_config_dir/script/init.sh $primary:8080 $db_config_dir $workload

echo -e "host=$primary:8080\nservice_cert=$db_config_dir/external/service_cert.pem\nuser_cert=$db_config_dir/script/user0_cert.pem\nuser_key=$db_config_dir/script/user0_privk.pem" > $db_config_dir/config.properties
for host in `cat $exp_dir/clients`
do
  rsync $db_config_dir/config.properties $host:$db_config_dir/
  rsync $db_config_dir/external/service_cert.pem $host:$db_config_dir/external/
done
