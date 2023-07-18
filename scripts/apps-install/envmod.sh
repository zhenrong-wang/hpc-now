#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Environment Modules* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi

if [ -f /opt/environment-modules/bin/modulecmd ]; then
  echo -e "[ -INFO- ] It seems environment-module is in place (opt/environment-modules).\n[ -INFO- ] If you really want to rebuild it. Please delete folder and retry. Exit now."
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/envmod/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING environment-module" >> ${logfile}
tmp_log=/tmp/hpcmgr_install.log
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`

yum install tcl-devel -y -q >> $tmp_log
if [ ! -f /opt/packs/modules-5.1.0.tar.gz ]; then
  wget ${URL_PKGS}modules-5.1.0.tar.gz -q -O /opt/packs/modules-5.1.0.tar.gz
fi
mkdir -p /etc/modulefiles
tar zxf /opt/packs/modules-5.1.0.tar.gz -C /opt/packs
cd /opt/packs/modules-5.1.0
./configure --prefix=/opt/environment-modules --modulefilesdir=/etc/modulefiles >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
rm -rf /etc/profile.d/modules.sh && ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.sh
rm -rf /etc/profile.d/modules.csh && ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.csh
time_current=`date "+%Y-%m-%d %H:%M:%S"`  
echo -e "[ -DONE- ] Environment Modules has been installed. "
echo -e "# $time_current Environment Modules has been installed." >> ${logfile}