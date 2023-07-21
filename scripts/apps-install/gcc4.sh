#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *GNU Compiler Collections-4.9.2* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi
tmp_log="/tmp/hpcmgr_install_gcc4_${current_user}.log"

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo | grep "processor" | wc -l`

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_cache="/hpc_apps/.cache/"
  app_extract_cache="/root/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
  app_extract_cache="/home/${current_user}/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/${current_user}_env/"
fi
mkdir -p ${app_cache}
mkdir -p ${app_extract_cache}

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing binaries and libraries ..."
  rm -rf ${app_root}gcc-4.9.2
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}gcc-4.9.2
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< gcc4 >/d' $public_app_registry
  else
    sed -e "/< gcc4 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] GCC-4.9.2 has been removed successfully."
  exit 0
fi

unset LIBRARY_PATH #Only for gcc, we have to do this
unset LD_LIBRARY_PATH
unset CPATH

centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ -z $centos_ver ] || [ $centos_ver -ne 7 ]; then
  echo -e "[ -INFO- ] GNU Compiler Collections - Version 4.9.2 is *ONLY* for CentOS 7.x to upgrade. Exit now."
  exit 3
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Building GNU Compiler Collections - Version 4.9.2  now ... "
echo -e "[ STEP 1 ] $time_current Downloading and extracting source packages, this step usually takes minutes ... "
if [ ! -f ${app_cache}gcc-4.9.2-full.tar.gz ]; then
  wget ${url_pkgs}gcc-4.9.2-full.tar.gz -O ${app_cache}gcc-4.9.2-full.tar.gz -o $tmp_log
fi
tar zvxf ${app_cache}gcc-4.9.2-full.tar.gz -C ${app_extract_cache} >> $tmp_log
cd ${app_extract_cache}gcc-4.9.2
./configure --prefix=${app_root}gcc-4.9.2 --enable-checking=release --enable-languages=c,c++,fortran --disable-multilib >> $tmp_log 2>&1
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 2 ] $time_current Making gcc-4.9.2 now, this step usually takes more than 2 hours with 8 cores..."
make -j$num_processors >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build gcc-4.9.2. Please check the log file for details. Exit now."
  exit 1
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Installing gcc-4.9.2 now, this step is quick ..."
make install >> $tmp_log 
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "#%Module1.0\nprepend-path PATH ${app_root}gcc-4.9.2/bin\nprepend-path LD_LIBRARY_PATH ${app_root}gcc-4.9.2/lib64\n" > ${envmod_root}gcc-4.9.2
if [ $current_user = 'root' ]; then
  echo -e "< gcc4 >" >> $public_app_registry
else
  echo -e "< gcc4 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ STEP 4 ] $time_current Comgratulations! GCC-4.9.2 has been built."