. conf.sh

for host in `cat ${exp_dir}/clients ${exp_dir}/workers`
do
  rsync ${exp_dir}/conf.sh $host:${exp_dir}/
done

