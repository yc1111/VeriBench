. conf.sh

server=$(cat $db_config_dir/$dbconfig | sed -n 1p | awk -F'[=:]' '{print $2}')
verifier=$(cat $db_config_dir/$dbconfig | sed -n 2p | awk -F'[=:]' '{print $2}')
auditor=$(cat $db_config_dir/$dbconfig | sed -n 3p | awk -F'[=:]' '{print $2}')

ssh $server "killall -9 main; rm -rf $log_dir/server.log"
ssh $server "killall -9 main; rm -rf $log_dir/verifier.log"
ssh $server "killall -9 main; rm -rf $log_dir/auditor.log"

for host in `cat ${db_config_dir}/clients`
do
  echo ${host}
  ssh ${host} "killall -9 run; rm -rf $log_dir;"
done