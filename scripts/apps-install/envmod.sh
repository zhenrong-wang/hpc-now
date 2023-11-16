# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *Environment Modules* to HPC-NOW cluster.

if [ $1 = 'remove' ]; then
  echo -e "[ FATAL: ] This is an internal & global component. Cannot be removed."
  exit 3
fi
current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
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

url_pkgs=${4}packages/envmod/
public_app_registry="/hpc_apps/.public_apps.reg"
app_cache="/hpc_apps/.cache/"
mkdir -p $app_cache
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
yum install tcl-devel -y >> ${2}

echo -e "[ -INFO- ] Downloading and Extracting files ..."
if [ ! -f ${app_cache}modules-5.1.0.tar.gz ]; then
  if [ $3 = 'local' ]; then
    /bin/cp -r ${url_pkgs}modules-5.1.0.tar.gz ${app_cache}modules-5.1.0.tar.gz >> ${2}
  else
    wget ${url_pkgs}modules-5.1.0.tar.gz -O ${app_cache}modules-5.1.0.tar.gz -o ${2}
  fi
fi
tar zvxf ${app_cache}modules-5.1.0.tar.gz -C ${app_cache} >> ${2}
cd ${app_cache}modules-5.1.0
echo -e "[ -INFO- ] Configuring building options ..."
./configure --prefix=/opt/environment-modules --modulefilesdir=/hpc_apps/envmod >> ${2} 2>&1
echo -e "[ -INFO- ] Compiling in progress ..."
make -j$num_processors >> ${2}
make install >> ${2}
rm -rf /etc/profile.d/modules.sh && ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.sh
rm -rf /etc/profile.d/modules.csh && ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.csh
echo -e "[ -DONE- ] Environment Modules has been installed. "
echo -e "< envmod >" >> $public_app_registry