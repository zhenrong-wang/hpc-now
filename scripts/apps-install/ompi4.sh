#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenMPI-4.1.2* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_ompi4_${current_user}.log"

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`

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
  rm -rf ${app_root}ompi-4.1.2
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}ompi-4.1.2
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< ompi4 >/d' $public_app_registry
  else
    sed -e "/< ompi4 > < ${user_name} >/d" $private_app_registry > /tmp/sed_${user_name}.tmp
    cat /tmp/sed_${user_name}.tmp > $private_app_registry
    rm -rf /tmp/sed_${user_name}.tmp
  fi
  echo -e "[ -INFO- ] OpenMPI-4.1.2 has been removed successfully."
  exit 0
fi

gcc_vers=('gcc12' 'gcc9' 'gcc8' 'gcc4')
gcc_code=('gcc-12.1.0' 'gcc-9.5.0' 'gcc-8.2.0' 'gcc-4.9.2')
systemgcc='true'
if [ ! -z $centos_ver ] && [ $centos_ver -eq 7 ]; then
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

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Started building OpenMPI-4.1.2."
echo -e "[ STEP 1 ] $time_current Downloading and extracting source packages ..."
if [ ! -f ${app_cache}openmpi-4.1.2.tar.gz ]; then
  wget ${url_pkgs}openmpi-4.1.2.tar.gz -O ${app_cache}openmpi-4.1.2.tar.gz -o ${tmp_log}
fi
tar zvxf ${app_cache}openmpi-4.1.2.tar.gz -C ${app_extract_cache} >> $tmp_log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 2 ] $time_current Building libraries and binaries from the source packages ..."
cd ${app_extract_cache}openmpi-4.1.2
./configure --prefix=${app_root}ompi-4.1.2 --enable-mpi-cxx >> $tmp_log
make -j$num_processors >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build OpenMPI-4.1.2. Please check the log file for details. Exit now."
  exit 1
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Installing now, this step is quick ..."
make install >> $tmp_log
echo -e "#%Module1.0\nprepend-path PATH ${app_root}ompi-4.1.2/bin\nprepend-path LD_LIBRARY_PATH ${app_root}ompi-4.1.2/lib\nprepend-path C_INCLUDE_PATH ${app_root}ompi-4.1.2/include\nprepend-path CPLUS_INCLUDE_PATH ${app_root}ompi-4.1.2/include\n" > ${envmod_root}ompi-4.1.2
echo -e "setenv OMPI_ALLOW_RUN_AS_ROOT 1\nsetenv OMPI_ALLOW_RUN_AS_ROOT_CONFIRM 1" >> ${envmod_root}ompi-4.1.2
if [ $current_user = 'root' ]; then
  echo -e "< ompi4 >" >> $public_app_registry
else
  echo -e "< ompi4 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] OpenMPI-4.1.2 has been built. "