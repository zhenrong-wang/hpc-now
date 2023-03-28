#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *zlib-1.2.13* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

if [ -f /hpc_apps/zlib-1.2.3/lib/libz.so ]; then
  echo -e "[ -INFO- ] It seems the zlib-1.2.13 libraries are in place (/hpc_apps/zlib-1.2.13/lib). If you want to rebuild it, please delete previous libraries and retry. Exit now."
  cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib:\$LD_LIBRARY_PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include:\$C_INCLUDE_PATH" >> /dev/null 2>&1
  fi
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING zlib-1.2.13" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`

echo -e "[ -INFO- ] zlib-1.2.13 will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/zlib-1.2.13.tar.gz ]; then
  wget ${URL_PKGS}zlib-1.2.13.tar.gz -q -O /opt/packs/zlib-1.2.13.tar.gz
fi
tar zxf /opt/packs/zlib-1.2.13.tar.gz -C /opt/packs/ >> /dev/null 2>&1
echo -e "[ STEP 1 ] Building zlib-1.2.13 ... This step usually takes seconds."
cd /opt/packs/zlib-1.2.13 && ./configure --prefix=$APP_ROOT/zlib-1.2.13 >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build zlib-1.2.13. Please check the log file for more details. Exit now."
  exit
fi
echo -e "[ -INFO- ] zlib-1.2.13 has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib:\$LD_LIBRARY_PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include:\$C_INCLUDE_PATH" >> /dev/null 2>&1
  fi
source /etc/profile
echo -e "[ -DONE- ] zlib-1.2.13 has been successfully installed to your cluster." 