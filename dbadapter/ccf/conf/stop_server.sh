. conf.sh

echo "stopping server"

curl -X GET "https://10.10.10.237:8080/app/commit" --cacert $db_config_dir/external/service_cert.pem --key $db_config_dir/script/user0_privk.pem --cert $db_config_dir/script/user0_cert.pem  >> $db_config_dir/tid
