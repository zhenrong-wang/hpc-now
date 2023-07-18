#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Baidu Netdisk* to HPC-NOW cluster.

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/baidu/
current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi

public_app_registry="/usr/hpc-now/.public_apps.reg"
app_cache="/hpc_apps/.cache/"
tmp_log=/tmp/hpcmgr_install_baidu.log

if [ $1 = 'remove' ]; then
  rpm -e baidunetdisk-patch
  rpm -e baidunetdisk
  rm -rf /opt/baidunetdisk
  sed -i '/< baidu >/d' $public_app_registry
  sed -i '#/opt/baidunetdisk/baidunetdisk#d' /etc/profile

  rm -rf /root/Desktop/baidu.desktop
  while read user_row
  do
    user_name=`echo $user_row | awk '{print $2}'`
    rm -rf /home/$user_name/Desktop/baidu.desktop
  done < /root/.cluster_secrets/user_secrets.txt
  echo -e "[ -INFO- ] Baidunetdisk has been removed successfully."
  exit 0
fi

mkdir -p $app_cache
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
  echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
  yum -y install libXScrnSaver -q
  if [ ! -f ${app_cache}baidunetdisk-4.3.0.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-4.3.0.x86_64.rpm -O ${app_cache}baidunetdisk-4.3.0.x86_64.rpm -q
  fi
  rpm -ivh ${app_cache}baidunetdisk-4.3.0.x86_64.rpm
  grep "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
  fi
  echo -e "[ -DONE- ] Baidu Netdisk has been installed. You can either run 'baidu' command or click the shortcut to use it. "
  wget ${url_pkgs}baidu.desktop -O /opt/baidunetdisk/baidu.desktop -q
  echo -e "[ -INFO- ] Creating a shortcut on the desktop ..."
  if [ -d /root/Desktop ]; then
    /bin/cp /opt/baidunetdisk/baidu.desktop /root/Desktop
  fi
  find /home -name "Desktop" > /tmp/desktop_dirs.txt
  while read rows
  do 
    user_row=`echo $rows | awk -F"/" '{print $3}'`
    /bin/cp /opt/baidunetdisk/baidu.desktop ${rows}
    chown -R ${user_row}:${user_row} ${rows}
  done < /tmp/desktop_dirs.txt
  rm -rf /tmp/desktop_dirs.txt
else
  yum -y install libXScrnSaver libcloudproviders -q
  echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
  if [ ! -f ${app_cache}baidunetdisk-4.15.5.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-4.15.5.x86_64.rpm -O ${app_cache}baidunetdisk-4.15.5.x86_64.rpm -q
  fi
  if [ ! -f ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-patch-1.0.1-1.x86_64.rpm -O ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm  -q
  fi
  rpm -ivh ${app_cache}baidunetdisk-4.15.5.x86_64.rpm 
  rpm -ivh ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm
  wget ${url_pkgs}libgdkpatch.so -O /opt/baidunetdisk/libgdkpatch.so -q
  grep "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
  fi
  echo -e "[ -DONE- ] Baidu Netdisk has been installed. You can either run 'baidu' command or click the shortcut to use it. " 
  echo -e "#! /bin/bash\nLD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox" > /opt/baidunetdisk/baidu.sh && chmod +x /opt/baidunetdisk/baidu.sh
  wget ${url_pkgs}baidu.desktop -O /opt/baidunetdisk/baidu.desktop -q
  sed -i 's@Exec=/opt/baidunetdisk/baidunetdisk --no-sandbox@Exec=/opt/baidunetdisk/baidu.sh@g' /opt/baidunetdisk/baidu.desktop
  if [ -d /root/Desktop ]; then
    /bin/cp /opt/baidunetdisk/baidu.desktop /root/Desktop
  fi
  find /home -name "Desktop" > /tmp/desktop_dirs.txt
  while read rows
  do 
    user_row=`echo $rows | awk -F"/" '{print $3}'`
    /bin/cp /opt/baidunetdisk/baidu.desktop ${rows}
    chown -R ${user_row}:${user_row} ${rows}
  done < /tmp/desktop_dirs.txt
  rm -rf /tmp/desktop_dirs.txt
fi
echo -e "< baidu >" >> $public_app_registry