host=$1
primary=$2
db_config_dir=$3

sed -e "s/<host>/${host}/g;s/<primary>/${primary}/g" $db_config_dir/config/join_template.json > $db_config_dir/config/cchost_config_virtual_cpp_join.json
docker build -t ccf-app-template:cpp-virtual -f $db_config_dir/docker/ccf_app_cpp_join.virtual $db_config_dir
docker run -d --network host -v $db_config_dir/external:/external --name ccfnode ccf-app-template:cpp-virtual
