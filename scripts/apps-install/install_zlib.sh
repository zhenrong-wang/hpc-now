#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *zlib-1.2.13* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="$HOME/.now_apps.reg"
tmp_log=/tmp/hpcmgr_install_zlib.log

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo | grep "processor" | wc -l`

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_cache="/hpc_apps/.cache/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
fi
mkdir -p $app_cache

grep zlib $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] This app has been installed to all users. Please run it directly."
  exit 1
else
  grep zlib $private_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] This app has been installed to the current user. Please run it directly."
    exit 3
  fi
fi

echo -e "[ START: ] Downloading and Extracting source code ..."

if [ ! -f ${app_cache}zlib-1.2.13.tar.gz ]; then
  wget ${url_pkgs}zlib-1.2.13.tar.gz -O ${app_cache}zlib-1.2.13.tar.gz
fi
tar zxf ${app_cache}zlib-1.2.13.tar.gz -C ${app_cache} >> /dev/null 2>&1

echo -e "[ STEP 1 ] Building zlib-1.2.13 ... This step usually takes seconds."
cd ${app_cache}zlib-1.2.13
./configure --prefix=${app_root}zlib-1.2.13 >> $tmp_log
make -j${num_processors} >> $tmp_log
make install >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build zlib-1.2.13. Please check the log file for more details. Exit now."
  exit 5
fi

if [ $current_user = 'root' ]; then
  sed -i '/zlib-1.2.13/d' /etc/profile
  echo -e "export LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib:\$LD_LIBRARY_PATH" >> /etc/profile
  echo -e "export C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include:\$C_INCLUDE_PATH" >> /etc/profile
  echo -e "zlib" >> $public_app_registry
else
  sed -i '/zlib-1.2.13/d' $HOME/.bashrc
  echo -e "export LD_LIBRARY_PATH=/hpc_apps/zlib-1.2.13/lib:\$LD_LIBRARY_PATH" >> $HOME/.bashrc
  echo -e "export C_INCLUDE_PATH=/hpc_apps/zlib-1.2.13/include:\$C_INCLUDE_PATH" >> $HOME/.bashrc
  echo -e "zlib" >> $private_app_registry
fi
echo -e "[ -DONE- ] zlib-1.2.13 has been successfully installed to ${app_root}."