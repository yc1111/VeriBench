. conf.sh

server=$(cat $db_config_dir/$dbconfig | sed -n 1p | awk -F'[=:]' '{print $2}')
verifier=$(cat $db_config_dir/$dbconfig | sed -n 2p | awk -F'[=:]' '{print $2}')
auditor=$(cat $db_config_dir/$dbconfig | sed -n 3p | awk -F'[=:]' '{print $2}')

echo "Starting MerkleSquare Main Server.."
ssh $server "source ~/.profile; mkdir -p $log_dir; go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/server/main.go > $log_dir/server.log 2>&1 &"
echo "Starting Auditing Server.."
ssh $auditor "source ~/.profile; mkdir -p $log_dir; go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/auditor/main.go > $log_dir/auditor.log 2>&1 &"
echo "Starting Verify Server.."
ssh $verifier "source ~/.profile; mkdir -p $log_dir; go run $GOPATH/src/github.com/ucbrise/MerkleSquare/demo/verifier/main.go > $log_dir/verifier.log 2>&1 &"
