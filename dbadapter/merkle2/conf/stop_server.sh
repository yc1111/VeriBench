. conf.sh

server=$(cat $db_config_dir/$dbconfig | sed -n 1p | awk -F'[=:]' '{print $2}')
verifier=$(cat $db_config_dir/$dbconfig | sed -n 2p | awk -F'[=:]' '{print $2}')
auditor=$(cat $db_config_dir/$dbconfig | sed -n 3p | awk -F'[=:]' '{print $2}')

ssh $server "killall -9 main"
ssh $server "killall -9 main"
ssh $server "killall -9 main"
