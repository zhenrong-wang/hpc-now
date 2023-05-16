#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *WPS* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
echo -e "[ -INFO- ] Software: WPS Office for Linux "
CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`

if [ $CENTOS_V -eq 7 ]; then
  yum grouplist installed -q | grep "GNOME Desktop" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] WPS needs desktop environment. Installing now ..."
    hpcmgr install desktop >> ${tmp_log}.desktop
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
      exit
    fi
  fi
  yum -y install libXScrnSaver -q
  rpm -q wps-office >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] WPS Office is in place. Exit now."
    exit
  fi
  if [ ! -f /opt/packs/wps-office-10.1.0.6634-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget ${URL_PKGS}wps-office-10.1.0.6634-1.x86_64.rpm -O /opt/packs/wps-office-10.1.0.6634-1.x86_64.rpm -q
  fi
  rpm -ivh /opt/packs/wps-office-10.1.0.6634-1.x86_64.rpm
  echo -e "[ -DONE- ] WPS Office has been installed."
elif [ $CENTOS_V -eq 9 ]; then
  yum grouplist installed -q | grep "Server with GUI" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] WPS Office needs desktop environment. Installing now ..."
    hpcmgr install desktop >> ${tmp_log}.desktop
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
      exit
    fi
  fi
  yum -y install libXScrnSaver -q
  rpm -q wps-office >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] WPS Office is in place. Exit now."
    exit
  fi
  if [ ! -f /opt/packs/wps-office-11.1.0.11664-1.x86_64.rpm ]; then
    echo -e "[ -INFO- ] Downloading package(s) ..."
    wget https://wps-linux-personal.wpscdn.cn/wps/download/ep/Linux2019/11664/wps-office-11.1.0.11664-1.x86_64.rpm -O /opt/packs/wps-office-11.1.0.11664-1.x86_64.rpm -q
  fi
  rpm -ivh /opt/packs/wps-office-11.1.0.11664-1.x86_64.rpm
  echo -e "[ -DONE- ] WPS Office has been installed."
else
  echo -e "[ FATAL: ] Unsupported OS Distribution. Installation Abort. Exit now."
  exit
fi