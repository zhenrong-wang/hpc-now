#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *WPS* to HPC-NOW cluster.

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can install this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi
public_app_registry="/usr/hpc-now/.public_apps.reg"
app_root="/hpc_apps/"
app_cache="/hpc_apps/.cache/"
mkdir -p $app_cache
tmp_log=/tmp/hpcmgr_install.log

cat $public_app_registry | grep kingsoft_wps >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] This app has been installed to all users. Please run it directly."
  exit 3
fi

echo -e "[ -INFO- ] Software: WPS Office for Linux."
# Check whether the desktop environment is ready
yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] This app needs desktop environment. Installing now ..."
  hpcmgr install desktop >> ${tmp_log}.desktop
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit 5
  fi
fi

centos_v=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ ! -z $centos_v ] && [ $centos_v -eq 7 ]; then
  yum -y install libXScrnSaver -q
  if [ ! -f ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget ${url_pkgs}wps-office-10.1.0.6634-1.x86_64.rpm -O ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm -q
  fi
  rpm -ivh ${app_cache}wps-office-10.1.0.6634-1.x86_64.rpm
  echo -e "[ -DONE- ] WPS Office has been installed."
else
  yum -y install libXScrnSaver -q
  if [ ! -f ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget https://wps-linux-personal.wpscdn.cn/wps/download/ep/Linux2019/11664/wps-office-11.1.0.11664-1.x86_64.rpm -O ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm -q
  fi
  rpm -ivh ${app_cache}wps-office-11.1.0.11664-1.x86_64.rpm
  echo -e "[ -DONE- ] WPS Office has been installed."
fi
echo -e "kingsoft_wps" >> $public_app_registry