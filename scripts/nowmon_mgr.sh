#!/bin/bash

# This code is written and maintained by Zhenrong WANG
# mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# This code is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

. /etc/profile
statefile=/usr/hpc-now/currentstate
sed -i 's/\r//g' $statefile
cluster_mon_data=/usr/hpc-now/mon_data.csv
cluster_core_summary=/usr/hpc-now/mon_cores.dat
. /usr/hpc-now/nowmon_agt.sh
line=`tail -n 1 /usr/hpc-now/mon_data.csv`
header=`echo $line | awk -F"," '{for(i=1;i<=3;i++) {printf("%s,",$i)}}'`
date_time=`echo $line | awk -F"," '{printf("%s %s",$1,$2)}'`
idle_cores=0
low_cores=0
for i in $(seq 1 $NODE_NUM)
do
    flag=`cat $statefile | grep compute${i}_status | awk '{print $2}'`
    if [ $flag = 'Running' ] || [ $flag = 'running' ] || [ $flag = 'RUNNING' ]; then
        ssh compute$i ". /usr/hpc-now/nowmon_agt.sh"
        scp root@compute$i:/usr/hpc-now/mon_data_compute$i.csv /tmp/ >> /dev/null 2>&1
        cat /tmp/mon_data_compute$i.csv >> $cluster_mon_data
	idle_cores_i=`awk -F"," '{print $12}' /tmp/mon_data_compute$i.csv`
    low_cores_i=`awk -F"," '{print $13}' /tmp/mon_data_compute$i.csv`
	idle_cores=$((idle_cores+idle_cores_i))
    low_cores=$((low_cores+low_cores_i))
    else
        echo -e "${header}compute${i},$NODE_CORES,null,null,null,null,null,null,null,null,null,null,null,null" >> $cluster_mon_data
    fi
done
node_cores=`grep compute_node_cores: $statefile | awk '{print $2}'`
running_nodes=`grep running_compute_nodes: $statefile | awk '{print $2}'`
total_nodes=`grep total_compute_nodes: $statefile | awk '{print $2}'`
running_cores=$((node_cores*running_nodes))
total_cores=$((node_cores*total_nodes))
echo -e "|          Date Time: $date_time\tTotal|Running|*IDLE|~LOW Cores : ${total_cores}|${running_cores}|*${idle_cores}|~${low_cores}" >> $cluster_core_summary