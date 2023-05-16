#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *MPICH-4.0.2* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING MPICH - 4.0.2" >> ${logfile} 2>&1
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
if [ -f $APP_ROOT/mpich-4.0.2/bin/mpicc ]; then
  echo -e "[ -INFO- ] It seems MPICH-4.0.2 is in place.\n[ -INFO- ] If you really want to rebuild it. Please delete the mpich-4.0.2 folder and retry. Exit now."
  echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/mpich-4.0.2/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/mpich-4.0.2/lib\nprepend-path C_INCLUDE_PATH /hpc_apps/mpich-4.0.2/include\nprepend-path CPLUS_INCLUDE_PATH /hpc_apps/mpich-4.0.2/include\n" > /etc/modulefiles/mpich-4.0.2
  exit
fi
#source /opt/environment-modules/init/bash
hpcmgr install envmod >> $tmp_log 2>&1
module ava -t | grep gcc >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  gcc_version=`module ava -t | grep gcc | tail -n1 | awk '{print $1}'`
  gcc_vnum=`echo $gcc_version | awk -F"-" '{print $2}' | awk -F"." '{print $1}'`
  module load $gcc_version
  echo -e "[ -INFO- ] $time_current MPICH4 will be built with GNU C Compiler (gcc) - $gcc_version."
else
  gcc_version=`gcc --version | head -n1`
  gcc_vnum=`echo $gcc_version | awk '{print $3}' | awk -F"." '{print $1}'`
  echo -e "[ -INFO- ] $time_current MPICH4 will be built with System GNU C Compiler - $gcc_version."
fi

echo -e "[ START: ] $time_current Started building MPICH-4.0.2."
echo -e "# $time_current Started building MPICH-4.0.2." >> ${logfile}
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
echo -e "[ STEP 1 ] $time_current Downloading and extracting source packages ..."
if [ ! -f /opt/packs/mpich-4.0.2.tar.gz ]; then
  wget -q ${URL_PKGS}mpich-4.0.2.tar.gz -O /opt/packs/mpich-4.0.2.tar.gz
fi
tar zvxf /opt/packs/mpich-4.0.2.tar.gz -C /opt/packs >> $tmp_log 2>&1
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 2 ] $time_current Configuring and making binaries ... "
if [ $gcc_vnum -gt 10 ]; then
  export FFLAGS="-w -fallow-argument-mismatch -O2"
  export FCFLAGS="-w -fallow-argument-mismatch -O2"
fi
cd /opt/packs/mpich-4.0.2 && ./configure --prefix=/hpc_apps/mpich-4.0.2 --enable-cxx --enable-fortran=all >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/mpich-4.0.2/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/mpich-4.0.2/lib\nprepend-path C_INCLUDE_PATH /hpc_apps/mpich-4.0.2/include\nprepend-path CPLUS_INCLUDE_PATH /hpc_apps/mpich-4.0.2/include\n" > /etc/modulefiles/mpich-4.0.2
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current MPICH-4.0.2 has been built. "
echo -e "# $time_current MPICH-4.0.2 has been built." >> ${logfile}
