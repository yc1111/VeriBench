#!/bin/bash

. conf.sh

for host in `cat ${exp_dir}/clients`
do
  echo ${host}
  result=($(ssh -q ${host} "docker ps -a | grep veribench | awk '{print \$1}'"))
  if [ ${#result[@]} -gt 0 ]; then
    ssh ${host} "docker kill ${result[@]}; docker container rm ${result[@]};"
  fi
done
