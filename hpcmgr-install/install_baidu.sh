#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Baidu Netdisk* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_UTILS=${URL_ROOT}utils/
tmp_log=/tmp/hpcmgr_install.log

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi

CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ $CENTOS_V -eq 7 ]; then
  yum grouplist installed -q | grep "GNOME Desktop" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] Baidu Netdisk needs desktop environment. Installing now."
    hpcmgr install desktop >> ${tmp_log}.desktop
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
      exit
    fi
  fi
  yum -y install libXScrnSaver -q
  wget ${URL_UTILS}pics/app.png -O /opt/app.png -q
  rpm -q baidunetdisk >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] Baidu Netdisk is already in place. Exit now."
    cat /etc/profile | grep "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
    fi
    exit
  else
    echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
    if [ ! -f /opt/packs/baidunetdisk-4.3.0.x86_64 ]; then
      wget ${URL_UTILS}baidunetdisk-4.3.0.x86_64.rpm -O /opt/packs/baidunetdisk-4.3.0.x86_64.rpm -q
    fi
    rpm -ivh /opt/packs/baidunetdisk-4.3.0.x86_64.rpm
    cat /etc/profile | grep "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
    fi
  echo -e "[ -DONE- ] Baidu Netdisk has been installed. You can either run 'baidu' command or click the shortcut to use it. "
  fi
  wget ${URL_UTILS}shortcuts/baidu.desktop -O /opt/baidunetdisk/baidu.desktop -q
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
elif [ $CENTOS_V -eq 9 ]; then
  yum grouplist installed -q | grep "Server with GUI" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] Baidu Netdisk needs desktop environment. Installing now."
    hpcmgr install desktop >> ${tmp_log}.desktop
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
      exit
    fi
  fi
  yum -y install libXScrnSaver libcloudproviders -q
  wget ${URL_UTILS}pics/app.png -O /opt/app.png -q
  rpm -q baidunetdisk-patch >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] Baidu Netdisk is already in place. Exit now."
    if [ ! -f /opt/baidunetdisk/libgdkpatch.so ]; then
      wget ${URL_UTILS}libgdkpatch.so -O /opt/baidunetdisk/libgdkpatch.so -q 
    fi
    cat /etc/profile | grep "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
    fi
  else
    echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
    if [ ! -f /opt/packs/baidunetdisk-4.15.5.x86_64.rpm ]; then
      wget ${URL_UTILS}baidunetdisk-4.15.5.x86_64.rpm -O /opt/packs/baidunetdisk-4.15.5.x86_64.rpm -q
    fi
    if [ ! -f /opt/packs/baidunetdisk-patch-1.0.1-1.x86_64.rpm ]; then
      wget ${URL_UTILS}baidunetdisk-patch-1.0.1-1.x86_64.rpm -O /opt/packs/baidunetdisk-patch-1.0.1-1.x86_64.rpm  -q
    fi
    rpm -ivh /opt/packs/baidunetdisk-4.15.5.x86_64.rpm 
    rpm -ivh /opt/packs/baidunetdisk-patch-1.0.1-1.x86_64.rpm
    wget ${URL_UTILS}libgdkpatch.so -O /opt/baidunetdisk/libgdkpatch.so -q
    cat /etc/profile | grep "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
    fi
    echo -e "[ -DONE- ] Baidu Netdisk has been installed. You can either run 'baidu' command or click the shortcut to use it. " 
  fi
  echo -e "#! /bin/bash\nLD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox" > /opt/baidunetdisk/baidu.sh && chmod +x /opt/baidunetdisk/baidu.sh
  wget ${URL_UTILS}shortcuts/baidu.desktop -O /opt/baidunetdisk/baidu.desktop -q
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
else
  echo -e "[ FATAL: ] Unsupported OS Distribution. Installation Abort. Exit now."
  exit
fi
