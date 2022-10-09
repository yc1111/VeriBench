. conf.sh

for i in `cat $db_config_dir/replicas`
do
  ssh $i "docker kill ccfnode; docker container prune -f;"
done
