#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *fftw3* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log=/tmp/hpcmgr_install_fftw3_${current_user}.log

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

if [ $1 = 'remove' ]; then
  rm -rf ${app_root}fftw3
  if [ $current_user = 'root' ]; then
    sed -i '/< fftw3 >/d' $public_app_registry
    sed -i '/fftw3/d' /etc/profile
  else
    sed -e "/< fftw3 > < ${user_name} >/d" $private_app_registry > /tmp/sed_${user_name}.tmp
    cat /tmp/sed_${user_name}.tmp > $private_app_registry
    rm -rf /tmp/sed_${user_name}.tmp
    sed -i '/fftw3/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] Cosbrowser has been removed successfully."
  exit 0
fi

mkdir -p $app_cache

echo -e "[ -INFO- ] Downloading and extracting packages ..."
if [ ! -f ${app_cache}fftw-3.3.10.tar.gz ]; then
  wget ${url_pkgs}fftw-3.3.10.tar.gz -O ${app_cache}fftw-3.3.10.tar.gz -o ${tmp_log}
fi
rm -rf ${app_cache}fftw-3.3.10
tar zvxf ${app_cache}fftw-3.3.10.tar.gz -C ${app_cache} >> ${tmp_log}

cd ${app_cache}fftw-3.3.10
echo -e "[ -INFO- ] Configuring build options ..."
./configure --prefix=${app_root}fftw3 --enable-sse2 --enable-avx --enable-avx2 --enable-shared >> ${tmp_log}
echo -e "[ -INFO- ] Compiling in progress ..."
make -j${num_processors} >> $tmp_log
echo -e "[ -INFO- ] Installing the binaries and libraries to the destination ..."
make install >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build FFTW-3. Please check the log file for details. Exit now."
  exit
fi

if [ $current_user = 'root' ]; then
  grep fftw3 /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export PATH=${app_root}fftw3/bin/:\$PATH\nexport LD_LIBRARY_PATH=${app_root}fftw3/lib/:\$LD_LIBRARY_PATH" >> /etc/profile
  fi
  echo -e "< fftw3 >" >> $public_app_registry
else
  grep fftw3 $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export PATH=${app_root}fftw3/bin/:\$PATH\nexport LD_LIBRARY_PATH=${app_root}fftw3/lib/:\$LD_LIBRARY_PATH" >> $HOME/.bashrc
  fi
  echo -e "< fftw3 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] FFTW-3 has been built to ${app_root}fftw3."