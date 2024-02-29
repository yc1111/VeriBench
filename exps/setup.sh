. conf.sh

cd ..
dir=`pwd | sed 's|\(.*\)/.*|\1|'`
cd exps

for host in `cat ${exp_dir}/clients ${exp_dir}/workers`
do
  echo $host
  rsync -a $dir/VeriBench $host:$dir
  ssh $host "docker pull yc1111/veribench:glassdb"
  ssh $host "docker pull yc1111/veribench:ledgerdb"
  ssh $host "docker pull yc1111/veribench:sqlledger"
  ssh $host "docker pull yc1111/veribench:qldb"
  ssh $host "docker pull yc1111/veribench:ccf"
  ssh $host "docker pull yc1111/veribench:merkle2"
done

