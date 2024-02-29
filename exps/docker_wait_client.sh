#/bin/bash

check=1
while [ $check -gt 0 ]
do
    check=`docker ps | grep veribench | wc -l`
    sleep 1
done
