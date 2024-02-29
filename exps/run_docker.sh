#!/bin/bash
. conf.sh

echo "------- Clean Server -------"
$db_config_dir/docker_clean.sh
$exp_dir/clean_client.sh

mkdir -p $log_dir
mkdir -p $res_dir

echo "-------- Parameters --------"
echo "Configuration:"
echo "workload: $workload"
echo "workload config path: $wlconfig"
echo "database: $database"
echo "database config path: $dbconfig"
echo "#shards: $nshard"
echo "#clients: $nclient"
echo "request rate: $request_rate"
echo "delay: $delay"
echo "block time: $blocktime"
echo "init versions: $version"

echo "------- start Server -------"
$exp_dir/sync_config.sh
$db_config_dir/docker_start.sh

sleep $wait_boot

# Run the clients
echo "-- Running the client(s) --"
count=0
for host in ${clients[@]}
do
  echo $host
  ssh $host "cd $exp_dir; ./docker_start_client.sh $count $nclient"
  let count=$count+$nclient
done

# Wait for all clients to exit
echo "Waiting for client(s) to exit"
for host in ${clients[@]}
do
  ssh $host "$exp_dir/docker_wait_client.sh"
done

# Process logs
$ana_dir/run_analyzer.sh

echo "------- Clean Server -------"
$db_config_dir/docker_clean.sh
$exp_dir/clean_client.sh
