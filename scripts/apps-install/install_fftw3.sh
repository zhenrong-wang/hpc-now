#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *fftw3* to HPC-NOW cluster.

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
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING FFTW3" >> ${logfile}
tmp_log=/tmp/hpcmgr_install.log
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`

if [ -f $APP_ROOT/fftw3/bin/fftw-wisdom ]; then
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "[ -INFO- ] It seems FFTW3 is in place.\n[ -INFO- ] If you really want to rebuild it. Please delete the /hpc_apps/fftw3 folder and retry. Exit now."
  echo -e "[ -INFO- ] # $time_current It seems FFTW3 is in place.\n[ -INFO- ] # $time_current If you really want to rebuild it. Please delete the /hpc_apps/fftw3 folder and retry. Exit now." >> ${logfile}
  echo -e "#%Module1.0\nprepend-path PATH $APP_ROOT/fftw3/bin\nprepend-path LD_LIBRARY_PATH $APP_ROOT/fftw3/lib\n" > /etc/modulefiles/fftw3
  exit
fi

hpcmgr install envmod >> $tmp_log

if [ ! -f /opt/packs/fftw-3.3.10.tar.gz ]; then
  echo -e "[ -INFO- ] Downloading and extracting packages."
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "[ -INFO- ] # $time_current Downloading and extracting packages." >> ${logfile}
  wget ${URL_PKGS}fftw-3.3.10.tar.gz -q -O /opt/packs/fftw-3.3.10.tar.gz
  rm -rf /opt/packs/fftw-3.3.10
  cd /opt/packs && tar zxf fftw-3.3.10.tar.gz
fi
echo -e "[ -INFO- ] Building binaries and libs to /hpc_apps/fftw3 now..."
cd /opt/packs/fftw-3.3.10 && ./configure --prefix=$APP_ROOT/fftw3 --enable-sse2 --enable-avx --enable-avx2 --enable-shared >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build FFTW-3. Please check the log file for details. Exit now."
  exit
fi
echo -e "#%Module1.0\nprepend-path PATH $APP_ROOT/fftw3/bin\nprepend-path LD_LIBRARY_PATH $APP_ROOT/fftw3/lib\n" > /etc/modulefiles/fftw3
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] FFTW-3 has been built to /hpc_apps/fftw3 ."
echo -e "[ -DONE- ] # $time_current FFTW-3 has been built to /hpc_apps/fftw3 ." >> ${logfile}
