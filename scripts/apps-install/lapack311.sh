#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *LAPACK-3.11.0* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`

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
  rm -rf ${app_root}lapack-3.11
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}lapack-3.11
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< lapack311 >/d' $public_app_registry
  else
    sed -e "/< lapack311 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] LAPACK-3.11.0 has been removed successfully."
  exit 0
fi

gcc_vers=('gcc12' 'gcc9' 'gcc8' 'gcc4')
gcc_code=('gcc-12.1.0' 'gcc-9.5.0' 'gcc-8.2.0' 'gcc-4.9.2')
systemgcc='true'
if [ ! -z $CENTOS_VERSION ] && [ $CENTOS_VERSION = '7' ]; then
  for i in $(seq 0 3)
  do
	  grep "< ${gcc_vers[i]} >" $public_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      module load ${gcc_code[i]}
      gcc_env="${gcc_code[i]}"
      systemgcc='false'
      break
    fi
    if [ $current_user != 'root' ]; then
      grep "< ${gcc_vers[i]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        module load ${current_user}_apps/${gcc_code[i]}
        gcc_env="${current_user}_env/${gcc_code[i]}"
        systemgcc='false'
        break
      fi
    fi
  done
else
  grep "< ${gcc_vers[0]} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load ${gcc_code[0]}
    gcc_env="${gcc_code[0]}"
    systemgcc='false'
  else
    if [ $current_user != 'root' ]; then
      grep "< ${gcc_vers[0]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        module load ${current_user}_env/${gcc_code[0]}
        gcc_env="${current_user}_env/${gcc_code[0]}"
        systemgcc='false'
      fi
    fi
  fi
fi
gcc_v=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`

mkdir -p ${app_root}lapack-3.11/
rm -rf ${app_root}lapack-3.11/*

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Started building LAPACK-3.11.0."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -f ${app_cache}lapack-3.11.tar.gz ]; then
  wget ${url_pkgs}lapack-3.11.tar.gz -O ${app_cache}lapack-3.11.tar.gz -o ${2}
fi
tar zvxf ${app_cache}lapack-3.11.tar.gz -C ${app_extract_cache} >> ${2}
echo -e "[ STEP 1 ] Building LAPACK, BLAS, CBLAS, LAPACKE ... This step usually takes minutes."
cd ${app_extract_cache}lapack-3.11/
/bin/cp make.inc.example make.inc
make -j$num_processors >> ${2}
cd ${app_extract_cache}lapack-3.11/LAPACKE && make -j$num_processors >> ${2}
cd ${app_extract_cache}lapack-3.11/BLAS && make -j$num_processors >> ${2}
cd ${app_extract_cache}lapack-3.11/CBLAS && make -j$num_processors >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ScaLAPACK. Please check the log file for more details. Exit now."
  exit 3
fi
rm -rf ${app_root}lapack-3.11
mkdir -p ${app_root}lapack-3.11
/bin/cp -r ${app_extract_cache}lapack-3.11/*.a ${app_root}lapack-3.11
/bin/cp -r ${app_extract_cache}lapack-3.11/LAPACKE ${app_root}lapack-3.11/
/bin/cp -r ${app_extract_cache}lapack-3.11/CBLAS ${app_root}lapack-3.11/
echo -e "#%Module1.0\nprepend-path LIBRARY_PATH ${app_root}lapack-3.11" > ${envmod_root}lapack-3.11
echo -e "prepend-path C_INCLUDE_PATH ${app_root}lapack-3.11/LAPACKE/include" >> ${envmod_root}lapack-3.11
echo -e "prepend-path C_INCLUDE_PATH ${app_root}lapack-3.11/CBLAS/include" >> ${envmod_root}lapack-3.11
if [ $current_user = 'root' ]; then
  echo -e "< lapack311 >" >> $public_app_registry
else
  echo -e "< lapack311 > < ${current_user} >" >> $private_app_registry
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current LAPACK-3.11.0 has been built."