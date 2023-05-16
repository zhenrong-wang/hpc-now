#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *INTEL(R) HPC Toolkit* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

find /hpc_apps/intel -name "icc" >> /dev/null 2>&1

if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] It seems Intel is already in place. If you really want to reinstall it, please delete /hpc_apps/intel and run this command again. Exit now."
  cat /etc/profile | grep "alias intelenv" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then 
    if [ -f /hpc_apps/intel/oneapi/setvars.sh ]; then
      echo -e "alias intelenv='source /hpc_apps/intel/oneapi/setvars.sh'" >> /etc/profile
    elif [ -f /hpc_apps/intel/setvars.sh ]; then
      echo -e "alias intelenv='source /hpc_apps/intel/setvars.sh'" >> /etc/profile
    fi
  fi
  exit
fi

echo -e "[ -INFO- ] Start installing Intel(R) HPC Kit latest version now ..."
echo -e "[ NOTICE ] IMPORTANT: YOU *MUST* CHANGE THE INSTALLATION PATH TO /hpc_apps/intel . \n[ NOTICE ] IMPORTANT: DO NOT USE THE DEFAULT PATH (/opt/path)"
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
wget https://registrationcenter-download.intel.com/akdlm/irc_nas/18679/l_HPCKit_p_2022.2.0.191.sh -O /opt/packs/l_HPCKit_p_2022.2.0.191.sh -q
bash /opt/packs/l_HPCKit_p_2022.2.0.191.sh
sed -i '/alias intelenv/d' /etc/profile
if [ -f /hpc_apps/intel/oneapi/setvars.sh ]; then
  echo -e "alias intelenv='source /hpc_apps/intel/oneapi/setvars.sh'" >> /etc/profile
elif [ -f /hpc_apps/intel/setvars.sh ]; then
  echo -e "alias intelenv='source /hpc_apps/intel/setvars.sh'" >> /etc/profile
else
  echo -e "[ FATAL: ] It seems you didn't install the intel HPC kit to the required path /hpc_apps/intel . Please reinstall to the required path." 
  exit
fi  
echo -e "[ -DONE- ] Congratulations! Intel(R) HPC Kit - Latest version has been installed. \n[ -INFO- ] Please run 'intelenv' command to load environment for your session."