#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *INTEL(R) HPC Toolkit* to HPC-NOW cluster.

public_app_registry="/usr/hpc-now/.public_apps.reg"
app_cache="/hpc_apps/.cache/"
mkdir -p $app_cache
tmp_log=/tmp/hpcmgr_install_intel.log

current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can install this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi

grep intelhpc $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] This app has been installed to all users. Please run it directly."
  exit 3
fi

echo -e "[ -INFO- ] Start installing Intel(R) HPC Kit latest version now ..."
if [ ! -f ${app_cache}l_HPCKit_p_2022.2.0.191.sh ]; then
  wget https://registrationcenter-download.intel.com/akdlm/irc_nas/18679/l_HPCKit_p_2022.2.0.191.sh -O ${app_cache}l_HPCKit_p_2022.2.0.191.sh
fi
sh ${app_cache}l_HPCKit_p_2022.2.0.191.sh
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to install Intel HPC Toolkit. Exit now."
  exit 7
fi
sed -i '/alias intelenv/d' /etc/profile
if [ -f /opt/intel/oneapi/setvars.sh ]; then
  echo -e "alias intelenv='source /hpc_apps/intel/oneapi/setvars.sh'" >> /etc/profile
elif [ -f /opt/intel/setvars.sh ]; then
  echo -e "alias intelenv='source /hpc_apps/intel/setvars.sh'" >> /etc/profile
else
  echo -e "[ -WARN- ] You didn't install the toolkit to the default path. Please set envvars manually." 
fi  
echo -e "[ -DONE- ] Congratulations! Intel(R) HPC Kit - Latest version has been installed."
echo -e "intelhpc" >> $public_app_registry