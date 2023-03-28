#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *LAPACK-3.11.0* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING LAPACK-3.11.0" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
echo -e "\n# $time_current SOFTWARE: LAPACK-3.11.0"

if [[ -f /hpc_apps/lapack-3.11/libcblas.a && -f /hpc_apps/lapack-3.11/liblapack.a && -f /hpc_apps/lapack-3.11/liblapacke.a && -f /hpc_apps/lapack-3.11/librefblas.a && -f /hpc_apps/lapack-3.11/libtmglib.a ]]; then
  echo -e "[ -INFO- ] It seems LAPACK-3.11.0 libraries are already in place (/hpc_apps/lapack-3.11)."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please remove the previous libraries and retry. Exit now.\n"
  exit 
fi
rm -rf /hpc_apps/lapack-3.11/libcblas.a
rm -rf /hpc_apps/lapack-3.11/liblapack.a
rm -rf /hpc_apps/lapack-3.11/liblapacke.a
rm -rf /hpc_apps/lapack-3.11/librefblas.a
rm -rf /hpc_apps/lapack-3.11/libtmglib.a
echo -e "[ -INFO- ] LAPACK-3.11.0 will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/lapack-3.11.tar.gz ]; then
  wget ${URL_PKGS}lapack-3.11.tar.gz -q -O /opt/packs/lapack-3.11.tar.gz
fi
tar zvxf /opt/packs/lapack-3.11.tar.gz -C /hpc_apps/ >> $tmp_log 2>&1
echo -e "[ STEP 1 ] Building LAPACK, BLAS, CBLAS, LAPACKE ... This step usually takes minutes."
cd /hpc_apps/lapack-3.11 && /bin/cp make.inc.example make.inc
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
cd /hpc_apps/lapack-3.11/LAPACKE && make -j$NUM_PROCESSORS >> $tmp_log 2>&1
cd /hpc_apps/lapack-3.11/BLAS && make -j$NUM_PROCESSORS >> $tmp_log 2>&1
cd /hpc_apps/lapack-3.11/CBLAS && make -j$NUM_PROCESSORS >> $tmp_log 2>&1
echo -e "[ -INFO- ] LAPACK-3.11 has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
cat /etc/profile | grep "LIBRARY_PATH=/hpc_apps/lapack-3.11" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LIBRARY_PATH=/hpc_apps/lapack-3.11:\$LIBRARY_PATH" >> /etc/profile
fi
cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/lapack-3.11/LAPACKE/include" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export C_INCLUDE_PATH=/hpc_apps/lapack-3.11/LAPACKE/include:/hpc_apps/lapack-3.11/CBLAS/include:\$C_INCLUDE_PATH" >> /etc/profile
fi
source /etc/profile
echo -e "[ -DONE- ] LAPACK-3.11 has been successfully installed to your cluster." 