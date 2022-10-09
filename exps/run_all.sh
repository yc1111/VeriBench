txnrate=(10 20 30 40 50 60 70 80 90 100 110 120)
nodes=(2 4 6 8 10 12 14 16)
wperc=(read-heavy balanced write-heavy)

# for i in ${wperc[@]}
# do
#   sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
#   ./run_exps.sh
#   mv /data/yc/results/result /data/yc/results/tr$i
#   mv ../dbadapter/ccf/conf/tid /data/yc/results/tid$i
# done

for i in ${txnrate[@]}
do
  sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
  ./run_exps.sh
  mv /data/yc/results/result /data/yc/results/tr$i
  mv ../dbadapter/ccf/conf/tid /data/yc/results/tid$i
done

# for i in ${nodes[@]}
# do
#   sed -i -e "s/nshard=[0-9]*/nshard=$i/" conf.sh
#   ./run_exps.sh
#   mv /data/yc/results/result /data/yc/results/node$i
#   mv ../dbadapter/ccf/conf/tid /data/yc/results/node_tid$i
# done
