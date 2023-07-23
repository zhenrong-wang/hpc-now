#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *WPS* to HPC-NOW cluster.

current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
public_app_registry="/hpc_apps/.public_apps.reg"
app_cache="/hpc_apps/.cache/"

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the app ..."
  rpm -e wps-office
  sed -i '/< kswps >/d' $public_app_registry
  echo -e "[ -INFO- ] The app has been removed successfully."
  exit 0
fi

mkdir -p $app_cache
echo -e "[ -INFO- ] Software: WPS Office for Linux."
# Check whether the desktop environment is ready
yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] This app needs desktop environment. Installing now ..."
  hpcmgr install desktop >> ${2}.desktop
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Exit now."
    exit 5
  fi
fi
centos_v=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ ! -z $centos_v ] && [ $centos_v -eq 7 ]; then
  yum -y install libXScrnSaver -q
  if [ ! -f ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget ${url_pkgs}wps-office-10.1.0.6634-1.x86_64.rpm -O ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm -o ${2}
  fi
  rpm -ivh ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm
else
  yum -y install libXScrnSaver -q
  if [ ! -f ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget https://wps-linux-personal.wpscdn.cn/wps/download/ep/Linux2019/11664/wps-office-11.1.0.11664-1.x86_64.rpm -O ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm -o ${2}
  fi
  rpm -ivh ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm
fi
echo -e "< kswps >" >> $public_app_registry
echo -e "[ -DONE- ] WPS Office has been installed."