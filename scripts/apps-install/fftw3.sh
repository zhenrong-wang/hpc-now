# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *fftw3* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

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
  rm -rf ${app_root}fftw3
  rm -rf ${envmod_root}fftw3
  if [ $current_user = 'root' ]; then
    sed -i '/< fftw3 >/d' $public_app_registry
  else
    sed -e "/< fftw3 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] FFTW-3.3.10 has been removed successfully."
  exit 0
fi

if [ $1 = 'install' ]; then
  echo -e "[ -INFO- ] Downloading the prebuilt package ..."
  if [ ! -f ${app_cache}fftw3.tar.gz ]; then
    wget ${url_pkgs}prebuilds-9/fftw3.tar.gz -O ${app_cache}fftw3.tar.gz -o ${2}
  fi
  echo -e "[ -INFO- ] Extracting the binaries and libraries ..."
  tar zvxf ${app_cache}fftw3.tar.gz -C ${app_root} >> ${2}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install FFTW-3.3.10. Please check the log file for details. Exit now."
    exit 1
  fi
  echo -e "#%Module1.0\nprepend-path PATH ${app_root}fftw3/bin\nprepend-path LD_LIBRARY_PATH ${app_root}fftw3/lib\n" > ${envmod_root}fftw3
  if [ $current_user = 'root' ]; then
    echo -e "< fftw3 >" >> $public_app_registry
  else
    echo -e "< fftw3 > < ${current_user} >" >> $private_app_registry
  fi
  echo -e "[ -DONE- ] FFTW-3.3.10 has been installed."
  exit 0
fi

echo -e "[ -INFO- ] Downloading and extracting packages ..."
if [ ! -f ${app_cache}fftw-3.3.10.tar.gz ]; then
  wget ${url_pkgs}fftw-3.3.10.tar.gz -O ${app_cache}fftw-3.3.10.tar.gz -o ${2}
fi
rm -rf ${app_cache}fftw-3.3.10
tar zvxf ${app_cache}fftw-3.3.10.tar.gz -C ${app_extract_cache} >> ${2}
cd ${app_extract_cache}fftw-3.3.10
echo -e "[ -INFO- ] Configuring build options ..."
./configure --prefix=${app_root}fftw3 --enable-sse2 --enable-avx --enable-avx2 --enable-shared >> ${2} 2>&1
echo -e "[ -INFO- ] Compiling in progress ..."
make -j${num_processors} >> ${2}
echo -e "[ -INFO- ] Installing the binaries and libraries to the destination ..."
make install >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build FFTW-3. Please check the log file for details. Exit now."
  exit
fi
echo -e "#%Module1.0\nprepend-path PATH ${app_root}fftw3/bin\nprepend-path LD_LIBRARY_PATH ${app_root}fftw3/lib\n" > ${envmod_root}fftw3
if [ $current_user = 'root' ]; then
  echo -e "< fftw3 >" >> $public_app_registry
else
  echo -e "< fftw3 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] FFTW-3 has been built to ${app_root}fftw3."