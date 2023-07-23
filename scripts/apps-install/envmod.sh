#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
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
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs_envmod=${url_root}packages/envmod/
public_app_registry="/hpc_apps/.public_apps.reg"
app_cache="/hpc_apps/.cache/"
mkdir -p $app_cache
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
yum install tcl-devel -y >> ${2}

echo -e "[ -INFO- ] Downloading and Extracting files ..."
if [ ! -f ${app_cache}modules-5.1.0.tar.gz ]; then
  wget ${url_pkgs_envmod}modules-5.1.0.tar.gz -O ${app_cache}modules-5.1.0.tar.gz -o ${2}
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