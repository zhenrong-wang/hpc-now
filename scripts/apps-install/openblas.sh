# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *OpenBLAS* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

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
  rm -rf ${app_root}openblas
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}openblas
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< openblas >/d' $public_app_registry
  else
    sed -e "/< openblas > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] OpenBLAS-0.3.21 has been removed successfully."
  exit 0
fi

if [ -z $3 ]; then
  echo -e "[ FATAL: ] Failed to get the location for packages repository. Exit now."
  exit 1
fi
if [ $3 != 'local' ] && [ $3 != 'web' ]; then
  echo -e "[ FATAL: ] Failed to get the location for packages repository. Exit now."
  exit 1
fi
if [ -z $4 ]; then
  echo -e "[ FATAL: ] Failed to get the location for packages repository. Exit now."
  exit 1
fi
url_pkgs=${4}
num_processors=`cat /proc/cpuinfo | grep "processor" | wc -l`

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

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Started building OpenBLAS-0.3.21."
echo -e "[ STEP 1 ] $time_current Downloading and extracting source packages ..."
if [ ! -f ${app_cache}OpenBLAS-0.3.21.tar.gz ]; then
  wget ${url_pkgs}OpenBLAS-0.3.21.tar.gz -O ${app_cache}OpenBLAS-0.3.21.tar.gz -o ${2}
fi
tar zvxf ${app_cache}OpenBLAS-0.3.21.tar.gz -C ${app_extract_cache} >> ${2}
echo -e "[ -INFO- ] Building *SINGLE_THREAD* libs now..."
cd ${app_extract_cache}OpenBLAS-0.3.21
make -j$num_processors USE_THREAD=0 USE_LOCKING=1 >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build OpenBLAS-0.3.21. Please check the log file for details. Exit now."
  exit 1
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Installing now, this step is quick ..."
make PREFIX=${app_root}openblas install >> ${2}
echo -e "#%Module1.0\nprepend-path PATH ${app_root}openblas/bin\nprepend-path LD_LIBRARY_PATH ${app_root}openblas/lib\n" > ${envmod_root}openblas
if [ $current_user = 'root' ]; then
  echo -e "< openblas >" >> $public_app_registry
else
  echo -e "< openblas > < ${current_user} >" >> $private_app_registry
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current OpenBLAS has been built to /hpc_apps/openblas ."