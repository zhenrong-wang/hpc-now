#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenMPI-3.1.6* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/

time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING OpenMPI - 3.1.6" >> ${logfile}
echo -e "\n# $time_current INSTALLING OpenMPI - 3.1.6"
tmp_log=/tmp/hpcmgr_install.log
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
yum -y install perl >> $tmp_log 2>&1

if [ -f $APP_ROOT/ompi-3.1.6/bin/mpicc ]; then
  echo -e "[ -INFO- ] It seems OpenMPI-3.1.6 is in place.\n[ -INFO- ] If you really want to rebuild it. Please delete the ompi-3.1.6 folder and retry. Exit now."
  echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/ompi-3.1.6/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/ompi-3.1.6/lib\nprepend-path C_INCLUDE_PATH /hpc_apps/ompi-3.1.6/include\nprepend-path CPLUS_INCLUDE_PATH /hpc_apps/ompi-3.1.6/include\n" > /etc/modulefiles/ompi-3.1.6
  exit
fi

echo -e "[ START: ] $time_current Started building OpenMPI-3.1.6."
echo -e "[ STEP 1 ] $time_current Downloading and extracting source packages ..."
if [ ! -f /opt/packs/openmpi-3.1.6.zip ]; then
  wget ${URL_PKGS}openmpi-3.1.6.zip -q -O /opt/packs/openmpi-3.1.6.zip
fi
cd /opt/packs && unzip -o -q openmpi-3.1.6.zip
echo -e "[ STEP 2 ] $time_current Building libraries and binaries from the source packages ..."
cd /opt/packs/openmpi-3.1.6 && ./configure --prefix=/hpc_apps/ompi-3.1.6 --enable-mpi-cxx >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
if [ $? -eq 0 ]; then
  echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/ompi-3.1.6/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/ompi-3.1.6/lib\nprepend-path C_INCLUDE_PATH /hpc_apps/ompi-3.1.6/include\nprepend-path CPLUS_INCLUDE_PATH /hpc_apps/ompi-3.1.6/include\n" > /etc/modulefiles/ompi-3.1.6
  echo -e "[ -DONE- ] $time_current OpenMPI-3.1.6 has been built. "
  echo -e "# $time_current OpenMPI-3.1.6 has been built." >> ${logfile}
else
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  rm -rf /etc/modulefiles/ompi-3.1.6
  echo -e "[ FATAL: ] Failed to build OpenMPI-3.1.6."
  echo -e "# $time_current Error: Failed to build OpenMPI-3.1.6." >> ${logfile}
fi