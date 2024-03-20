wperc=(read-heavy balanced write-heavy)

res_dir=$(grep -Eo 'res_dir=.*' conf.sh | sed 's/"//g' | sed 's/\$database//g' | awk -F [=:] '{print $2}')
mkdir -p $res_dir

clean () {
  systems=(qldb ledgerdb sqlledger glassdb ccf merkle2)
  for db in ${systems[@]}
  do
    rm $res_dir/${db}*
  done
}

run_fig9 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=1/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=10/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=300/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=300/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf merkle2)
  txnrate=(1 4 7 10)

  sys=$( IFS=$','; echo "${systems[*]}" )
  txns=$( IFS=$','; echo "${txnrate[*]}" )
  wpers=$( IFS=$','; echo "${wperc[*]}" )

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi

    #9 a,b,c
    for i in ${txnrate[@]}
    do
      sed -i -e "s/request_rate=[0-9]*/request_rate=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done

    #9 d
    for i in ${wperc[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
    sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  done
  python parse_tps.py $res_dir $sys $txns fig9a
  python parse_lat.py $res_dir $sys $txns fig9b
  python parse_lat_op.py $res_dir $sys $txns read,write,verify 5,6,10 fig9c
  python parse_wl.py $res_dir $sys $wpers fig9d
  clean
}

run_fig10 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  txnrate=(10 50 90 120)
  nshard=(2 8 16)

  sys=$( IFS=$','; echo "${systems[*]}" )
  txns=$( IFS=$','; echo "${txnrate[*]}" )
  nodes=$( IFS=$','; echo "${nshard[*]}" )
  wpers=$( IFS=$','; echo "${wperc[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done

    #10 c
    for i in ${nshard[@]}
    do
      sed -i -e "s/nshard=[0-9]*/nshard=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_node${i}
    done
    sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh

    #10 d
    for i in ${wperc[@]}
    do
      sed -i -e "s/wlconfig=.*/wlconfig=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
    sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  done
  python parse_tps.py $res_dir $sys $txns fig10a
  python parse_lat_op.py $res_dir $sys $txns read,write,verify 5,6,10 fig10b
  python parse_scale.py $res_dir $sys $nodes fig10c
  python parse_wl.py $res_dir $sys $wpers fig10d
  clean
}

run_fig11 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  zipf=(zipf-0.9 zipf-1.5)

  sys=$( IFS=$','; echo "${systems[*]}" )
  zipfs=$( IFS=$','; echo "${zipf[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
  done
  python parse_wl.py $res_dir $sys $zipfs fig11a
  python parse_wl_abort.py $res_dir $sys $zipfs fig11b
  clean
}

run_fig12 () {
  sed -i -e "s/workload=.*/workload=smallbank/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  txnrate=(10 50 90 120)

  sys=$( IFS=$','; echo "${systems[*]}" )
  txns=$( IFS=$','; echo "${txnrate[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
  done
  python parse_tps.py $res_dir $sys $txns fig12a
  python parse_lat_op.py $res_dir $sys $txns AM,GB,UB,US,SP,WC 5,6,7,8,9,10 fig12b
  clean
}

run_fig13 () {
  sed -i -e "s/workload=.*/workload=tpcc/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  txnrate=(5 10 15 20)

  sys=$( IFS=$','; echo "${systems[*]}" )
  txns=$( IFS=$','; echo "${txnrate[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
  done
  python parse_tps.py $res_dir $sys $txns fig13a
  python parse_lat_op.py $res_dir $sys $txns NewOrder,Payment,OrderStat,Deliv,StockLvl 5,6,7,8,9 fig13b
  clean
}

run_fig14 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=16/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  range=(range-10 range-100 range-1000)

  sys=$( IFS=$','; echo "${systems[*]}" )
  ranges=$( IFS=$','; echo "${range[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
  done
  python parse_wl.py $res_dir $sys $ranges fig14a
  python parse_lat_op.py $res_dir $sys $ranges range,verify 8,10 fig14b
  clean
}

run_fig15 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=provenance/" conf.sh
  sed -i -e "s/nversion=[0-9]*/nversion=20/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=100/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=100/" conf.sh

  systems=(qldb ledgerdb sqlledger glassdb ccf)
  txnrate=(8 16 24 32)

  sys=$( IFS=$','; echo "${systems[*]}" )
  txns=$( IFS=$','; echo "${txnrate[*]}" )

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
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_${i}
    done
  done
  sed -i -e "s/nversion=[0-9]*/nversion=1/" conf.sh
  python parse_tps.py $res_dir $sys $txns fig15a
  python parse_lat_op.py $res_dir $sys $txns provenance,verify 7,10 fig15b
  clean
}

run_fig16 () {
  sed -i -e "s/workload=.*/workload=ycsb/" conf.sh
  sed -i -e "s/wlconfig=.*/wlconfig=balanced/" conf.sh
  sed -i -e "s/nversion=[0-9]*/nversion=1/" conf.sh
  sed -i -e "s/nshard=[0-9]*/nshard=16/" conf.sh
  sed -i -e "s/nclient=[0-9]*/nclient=20/" conf.sh
  sed -i -e "s/nthread=[0-9]*/nthread=10/" conf.sh
  sed -i -e "s/delay=[0-9]*/delay=10/" conf.sh
  sed -i -e "s/blocktime=[0-9]*/blocktime=10/" conf.sh
  sed -i -e "s/request_rate=[0-9]*/request_rate=120/" conf.sh

  systems=(ledgerdb sqlledger glassdb)
  delay=(10 20 30 40 50)
  blocktime=(10 20 30 40 50)

  sys=$( IFS=$','; echo "${systems[*]}" )

  for db in ${systems[@]}
  do
    sed -i -e "s/database=.*/database=${db}/" conf.sh
    if [ $db = "ccf" ]; then
      sed -i -e "s/mode=.*/mode=-p/" conf.sh
    else
      sed -i -e "s/mode=.*/mode=-i/" conf.sh
    fi

    sed -i -e "s/blocktime=[0-9]*/blocktime=10/" conf.sh
    for i in ${delay[@]}
    do
      sed -i -e "s/delay=[0-9]*/delay=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_delay${i}
    done

    sed -i -e "s/delay=[0-9]*/delay=1280/" conf.sh
    for i in ${blocktime[@]}
    do
      sed -i -e "s/blocktime=[0-9]*/blocktime=$i/" conf.sh
      ./run_docker.sh
      mv $res_dir/result $res_dir/${db}_blocktime${i}
    done
  done
}

run_fig9
run_fig10
run_fig11
run_fig12
run_fig13
run_fig14
run_fig15
run_fig16
