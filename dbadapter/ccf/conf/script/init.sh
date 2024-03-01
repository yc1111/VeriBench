host=$1
path=$2
workload=$3
wlconfig=$4

# activate member
curl https://${host}/gov/ack/update_state_digest -X POST \
    --cacert $path/external/service_cert.pem \
    --key $path/external/member0_privk.pem \
    --cert $path/external/member0_cert.pem \
    --silent > $path/script/state_digest.json

$path/script/scurl.sh https://${host}/gov/ack \
    --cacert $path/external/service_cert.pem \
    --signing-key $path/external/member0_privk.pem \
    --signing-cert $path/external/member0_cert.pem \
    --header 'Content-Type: application/json' \
    --data-binary @$path/script/state_digest.json

# add user
$path/script/scurl.sh https://${host}/gov/proposals \
    --cacert $path/external/service_cert.pem \
    --signing-key $path/external/member0_privk.pem \
    --signing-cert $path/external/member0_cert.pem \
    --header 'Content-Type: application/json' \
    --data-binary @$path/script/set_user.json

# open network
echo "{ \"actions\": [ {\"name\": \"transition_service_to_open\", \"args\": { \"next_service_identity\": \""$(awk '{printf "%s\\n", $0}' $path/external/service_cert.pem)"\" } } ] }" > $path/script/transition_service_to_open.json

$path/script/scurl.sh https://${host}/gov/proposals \
    --cacert $path/external/service_cert.pem \
    --signing-key $path/external/member0_privk.pem \
    --signing-cert $path/external/member0_cert.pem \
    --data-binary @$path/script/transition_service_to_open.json \
    -H "content-type: application/json"

sleep 5

if [ "$workload" = "tpcc" ]; then
  echo "init data for tpcc"
  for i in {0..51}
  do
    curl https://${host}/app/inittpcc?batchid=$i -X GET \
        --cacert $path/external/service_cert.pem \
        --key $path/script/user0_privk.pem \
        --cert $path/script/user0_cert.pem
  done
elif [ "$workload" = "smallbank" ]; then
  echo "init data for smallbank"
  curl https://${host}/app/initsb -X GET \
      --cacert $path/external/service_cert.pem \
      --key $path/script/user0_privk.pem \
      --cert $path/script/user0_cert.pem
elif [ "$workload" = "ycsb" ]; then
  echo "init data for ycsb"
  if [ "$wlconfig" = "provenance" ]; then
    let n=10
  else
    let n=1
  fi
  for i in $(seq 1 $n)
  do
    curl https://${host}/app/initycsb?version=$i -X GET \
      --cacert $path/external/service_cert.pem \
      --key $path/script/user0_privk.pem \
      --cert $path/script/user0_cert.pem
  done
fi

curl https://${host}/app/commit -X GET \
    --cacert $path/external/service_cert.pem \
    --key $path/script/user0_privk.pem \
    --cert $path/script/user0_cert.pem  > $path/tid
