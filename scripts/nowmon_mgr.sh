#!/bin/bash

# This code is written and maintained by Zhenrong WANG
# mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# This code is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

. /etc/profile
statefile=/usr/hpc-now/currentstate
cluster_mon_data=/usr/hpc-now/mon_data.csv
. /usr/hpc-now/nowmon_agt.sh
line=`tail -n 1 /usr/hpc-now/mon_data.csv`
header=`echo $line | awk -F"," '{for(i=1;i<=3;i++) {printf("%s,",$i)}}'`

for i in $(seq 1 $NODE_NUM)
do
    flag=`cat $statefile | grep compute${i}_status | awk '{print $2}'`
    if [ $flag = 'Running' ] || [ $flag = 'running' ] || [ $flag = 'RUNNING' ]; then
        ssh compute$i ". /usr/hpc-now/nowmon_agt.sh"
        scp root@compute$i:/usr/hpc-now/mon_data_compute$i.csv /tmp/
        cat /tmp/mon_data_compute$i.csv >> $cluster_mon_data
    else
        echo -e "${header}compute${i},$NODE_CORES,null,null,null,null,null,null,null,null,null,null,null,null" >> $cluster_mon_data
    fi
done