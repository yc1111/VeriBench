. conf.sh

#txnrate=(10 20 30 40 50 60 70 80 90 100 110 120)
txnrate=(10 20 30 40 50 60 70 80 90 100 110 120)
nodes=(2 4 6 8 10 12 14 16)
wperc=(read-heavy balanced write-heavy zipf-0.6 zipf-0.9 zipf-1.2 zipf-1.5)

#init
sed -i -e "s/nshard=[0-9]*/nshard=3/" conf.sh
sed -i -e "s/wlconfig=.*/wlconfig=provenance/" conf.sh
#mkdir -p $res_dir/$workload
mkdir -p $res_dir/provenance

#txnrate
for i in ${txnrate[@]}
do
  sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
  ./run_exps.sh
  #mv $res_dir/result $res_dir/$workload/txnrate$i
  mv $res_dir/result $res_dir/provenance/txnrate$i
done

# #node
# for i in ${nodes[@]}
# do
#   sed -i -e "s/nshard=[0-9]*/nshard=$i/" conf.sh
#   ./run_exps.sh
#   mv $res_dir/result $res_dir/$workload/node$i
# done
# sed -i -e "s/nshard=[0-9]*/nshard=3/" conf.sh

# #workload
# for i in ${wperc[@]}
# do
#   sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
#   ./run_exps.sh
#   mv $res_dir/result $res_dir/$workload/$i
#   mv ../dbadapter/ccf/conf/tid $res_dir/$workload/tid_$i
# done
# sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh

