. conf.sh

for i in `cat $exp_dir/workers`
do
  ssh $i "docker kill ccfnode; docker container prune -f;"
done
