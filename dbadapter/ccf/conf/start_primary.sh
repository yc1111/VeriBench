host=$1
db_config_dir=$2

sed -e "s/<host>/${host}/g" $db_config_dir/config/start_template.json > $db_config_dir/config/cchost_config_virtual_cpp.json
docker build -t ccf-app-template:cpp-virtual -f $db_config_dir/docker/ccf_app_cpp.virtual $db_config_dir
docker run -d --network host -v $db_config_dir/external:/external --name ccfnode ccf-app-template:cpp-virtual
