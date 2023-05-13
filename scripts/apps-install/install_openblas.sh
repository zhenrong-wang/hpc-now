#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenBLAS* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING OpenBLAS-0.3.21" >> ${logfile}
tmp_log=/tmp/hpcmgr_install.log
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`

if [ -f $APP_ROOT/openblas/lib/libopenblas.a ]; then
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "[ -INFO- ] It seems OpenBLAS is in place.\n[ -INFO- ] If you really want to rebuild it. Please delete the /hpc_apps/openblas folder and retry. Exit now.\n"
  echo -e "[ -INFO- ] # $time_current It seems OpenBLAS is in place.\n[ -INFO- ] # $time_current If you really want to rebuild it. Please delete the /hpc_apps/openblas folder and retry. Exit now.\n" >> ${logfile}
  echo -e "#%Module1.0\nprepend-path PATH $APP_ROOT/openblas/bin\nprepend-path LD_LIBRARY_PATH $APP_ROOT/openblas/lib\n" > /etc/modulefiles/openblas
  exit
fi

hpcmgr install envmod >> $tmp_log

if [ ! -f /opt/packs/OpenBLAS-0.3.21.tar.gz ]; then
  echo -e "[ -INFO- ] Downloading and extracting packages."
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "[ -INFO- ] # $time_current Downloading and extracting packages." >> ${logfile}
  wget ${URL_PKGS}OpenBLAS-0.3.21.tar.gz -q -O /opt/packs/OpenBLAS-0.3.21.tar.gz
  rm -rf /opt/packs/OpenBLAS-0.3.21
  cd /opt/packs && tar zxf OpenBLAS-0.3.21.tar.gz
fi

echo -e "[ -INFO- ] Building *SINGLE_THREAD* libs to /hpc_apps/openblas now..."

cd /opt/packs/OpenBLAS-0.3.21 && make -j$NUM_PROCESSORS USE_THREAD=0 USE_LOCKING=1 >> $tmp_log 2>&1
make PREFIX=/hpc_apps/openblas install >> $tmp_log 2>&1
echo -e "#%Module1.0\nprepend-path PATH $APP_ROOT/openblas/bin\nprepend-path LD_LIBRARY_PATH $APP_ROOT/openblas/lib\n" > /etc/modulefiles/openblas
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] OpenBLAS has been built to /hpc_apps/openblas ."
echo -e "[ -DONE- ] # $time_current OpenBLAS has been built to /hpc_apps/openblas ." >> ${logfile}