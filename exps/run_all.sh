txnrate=(120)
nodes=(2 8 16)
wperc=(read-heavy balanced write-heavy)
zipf=(zipf-0.9 zipf-1.5)
range=(range-10 range-100 range-1000)
#systems=(qldb sqlledger ledgerdb glassdb)
systems=(merkle2)

res_dir=$(grep -Eo 'res_dir=.*' conf.sh | sed 's/"//g' | sed 's/\$database//g' | awk -F [=:] '{print $2}')

for db in ${systems[@]}
do
  #init
  mkdir -p $res_dir/$db/ycsb
  mkdir -p $res_dir/$db/tpcc
  mkdir -p $res_dir/$db/smallbank
  mkdir -p $res_dir/$db/range
  mkdir -p $res_dir/$db/provenance
  mkdir -p $res_dir/$db/delay
  mkdir -p $res_dir/$db/audit
done

run_fig9 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=1/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi

    single_txnrate=(1 2 3 4 5 6 7 8 9 10)
    #9 a,b,c
    for i in ${single_txnrate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/txnrate$i
    done

    #9 d
    for i in ${wperc[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/$i
    done
    sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  done
}

run_fig10 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi

    #10 a,b
    for i in ${txnrate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/txnrate$i
    done

    #10 c
    for i in ${nodes[@]}
    do
      sed -i -e "s/nshard=[0-9]*/nshard=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/node$i
    done
    sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh

    #10 d
    for i in ${wperc[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/$i
    done
    sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  done
}

run_fig11 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi
    for i in ${zipf[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/ycsb/$i
    done
  done
}

run_fig12 () {
  sed -i -e "s/workload=.*/workload=smallbank/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi
    for i in ${txnrate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/smallbank/txnrate$i
    done
  done
}

run_fig13 () {
  sed -i -e "s/workload=.*/workload=tpcc/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  small_rate=(20)

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi
    for i in ${small_rate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/tpcc/txnrate$i
    done
  done
}

run_fig14 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi
    for i in ${range[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/range/$i
    done
  done
}

run_fig15 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=provenance/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi
    for i in ${txnrate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      if [ $db = "ccf" ] || [ $db = "merkle2" ]; then
        ./run_exps.sh
      else
        ./run_docker.sh
      fi
      mv $res_dir/$db/result $res_dir/$db/provenance/$i
    done
  done
}

run_fig9
#run_fig10
#run_fig11
#run_fig12
#run_fig13
#run_fig14
#run_fig15
