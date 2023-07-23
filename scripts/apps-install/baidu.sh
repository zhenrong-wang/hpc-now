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
public_app_registry="/hpc_apps/.public_apps.reg"
app_cache="/hpc_apps/.cache/"

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
  hpcmgr install desktop >> $2.desktop
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit 5
  fi
fi

centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ ! -z $centos_ver ] && [ $centos_ver -eq 7 ]; then
  echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
  yum -y install libXScrnSaver >> ${2}
  if [ ! -f ${app_cache}baidunetdisk-4.3.0.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-4.3.0.x86_64.rpm -O ${app_cache}baidunetdisk-4.3.0.x86_64.rpm -o ${2}
  fi
  rpm -ivh ${app_cache}baidunetdisk-4.3.0.x86_64.rpm
  grep "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias baidu='/opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
  fi
  echo -e "#! /bin/bash\n/opt/baidunetdisk/baidunetdisk --no-sandbox" > /opt/baidunetdisk/baidu.sh
  chmod +x /opt/baidunetdisk/baidu.sh
else
  yum -y install libXScrnSaver libcloudproviders >> ${2}
  echo -e "[ -INFO- ] Downloading and installing Baidu Netdisk now ..."
  if [ ! -f ${app_cache}baidunetdisk-4.15.5.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-4.15.5.x86_64.rpm -O ${app_cache}baidunetdisk-4.15.5.x86_64.rpm -o ${2}
  fi
  if [ ! -f ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm ]; then
    wget ${url_pkgs}baidunetdisk-patch-1.0.1-1.x86_64.rpm -O ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm -o ${2}
  fi
  rpm -ivh ${app_cache}baidunetdisk-4.15.5.x86_64.rpm 
  rpm -ivh ${app_cache}baidunetdisk-patch-1.0.1-1.x86_64.rpm
  if [ ! -f ${app_cache}libgdkpatch.so ]; then
    wget ${url_pkgs}libgdkpatch.so -O ${app_cache}libgdkpatch.so -o ${2}
  fi
  /bin/cp ${app_cache}libgdkpatch.so /opt/baidunetdisk/
  grep "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias baidu='LD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox'" >> /etc/profile
  fi 
  echo -e "#! /bin/bash\nLD_PRELOAD=/opt/baidunetdisk/libgdkpatch.so /opt/baidunetdisk/baidunetdisk --no-sandbox" > /opt/baidunetdisk/baidu.sh
  chmod +x /opt/baidunetdisk/baidu.sh
fi
echo -e "[Desktop Entry]" > $HOME/Desktop/baidu.desktop
echo -e "Encoding=UTF-8" >> $HOME/Desktop/baidu.desktop
echo -e "Version=1.0" >> $HOME/Desktop/baidu.desktop
echo -e "Name=baidu" >> $HOME/Desktop/baidu.desktop
echo -e "Comment=baidunetdisk" >> $HOME/Desktop/baidu.desktop
echo -e "Exec=/opt/baidunetdisk/baidu.sh" >> $HOME/Desktop/baidu.desktop
echo -e "Icon=/opt/app.png" >> $HOME/Desktop/baidu.desktop
echo -e "Terminal=false" >> $HOME/Desktop/baidu.desktop
echo -e "StartupNotify=true" >> $HOME/Desktop/baidu.desktop
echo -e "Type=Application" >> $HOME/Desktop/baidu.desktop
echo -e "Categories=Applications;" >> $HOME/Desktop/baidu.desktop
while read user_row
do
  user_name=`echo $user_row | awk '{print $2}'`
  cp -r $HOME/Desktop/baidu.desktop /home/${user_name}/Desktop/
  chown -R ${user_name}:${user_name} /home/${user_name}/Desktop/
done < /root/.cluster_secrets/user_secrets.txt
echo -e "< baidu >" >> $public_app_registry
echo -e "[ -DONE- ] Baidu Netdisk has been installed. You can either run 'baidu' command or click the shortcut to use it. " 