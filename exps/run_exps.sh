#!/bin/bash
. conf.sh

echo "------- Clean Server -------"
$db_config_dir/clean.sh

mkdir -p $log_dir
mkdir -p $exp_dir/results

# Print out configuration being used.
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

# Start all servers
echo "------- start Server -------"
$db_config_dir/start_server.sh

sleep $wait_boot

# Run the clients
echo "-- Running the client(s) --"
count=0
for host in ${clients[@]}
do
  echo $host
  ssh $host "source ~/.profile; mkdir -p $log_dir; \
      $exp_dir/start_multiple_clients.sh \
          \"$bin_dir/run \
              -r $request_rate -t $nthread -D $duration -d $delay -w $workload \
              -W $wl_config_file -s $database -c $db_config_file $mode \" \
          $count $nclient $log_dir"

  let count=$count+$nclient
done

# Wait for all clients to exit
echo "Waiting for client(s) to exit"
for host in ${clients[@]}
do
  ssh $host "source ~/.profile; $exp_dir/wait_client.sh run"
done

# Kill all replicas
$db_config_dir/stop_server.sh

# Process logs
$ana_dir/run_analyzer.sh

echo "------- Clean Server -------"
#$db_config_dir/clean.sh
