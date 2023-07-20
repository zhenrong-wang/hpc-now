#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *INTEL(R) HPC Toolkit* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the app ..."
  yum -y remove intel-hpckit
  sed -i '/alias intelenv/d' /etc/profile
  sed -i '/< intel >/d' $public_app_registry
  echo -e "[ -INFO- ] The app has been removed successfully."
  exit 0
fi

echo -e "[ -INFO- ] Start installing Intel(R) HPC Kit latest version now ..."
if [ ! -f /etc/yum.repos.d/oneAPI.repo ]; then
  echo -e "[oneAPI]\nname=Intel® oneAPI repository\nbaseurl=https://yum.repos.intel.com/oneapi\nenabled=1\ngpgcheck=1\nrepo_gpgcheck=1\ngpgkey=https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB" > /etc/yum.repos.d/oneAPI.repo
fi
yum -y install intel-hpckit
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to install Intel HPC Toolkit. Exit now."
  exit 7
fi
sed -i '/alias intelenv/d' /etc/profile
echo -e "alias intelenv='source /opt/intel/oneapi/setvars.sh'" >> /etc/profile
echo -e "[ -DONE- ] Congratulations! Intel(R) HPC Kit - Latest version has been installed."
echo -e "< intel >" >> $public_app_registry