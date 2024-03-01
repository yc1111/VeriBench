. conf.sh

cd ..
dir=`pwd | sed 's|\(.*\)/.*|\1|'`
cd exps

for host in `cat ${exp_dir}/clients ${exp_dir}/workers`
do
  echo $host
  rsync -a $dir/VeriBench $host:$dir
  ssh $host "docker pull yc1111/veribench:glassdb; docker pull yc1111/veribench:ledgerdb; \
             docker pull yc1111/veribench:sqlledger; docker pull yc1111/veribench:qldb; \
             docker pull yc1111/veribench:ccf; docker pull yc1111/veribench:merkle2;"
done

