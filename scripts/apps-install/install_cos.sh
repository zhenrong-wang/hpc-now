#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *HPC-NOW Netdisk (COSbrowser)* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="$HOME/.now_apps.reg"
tmp_log=/tmp/hpcmgr_install_cos.log

if [ $current_user = 'root' ]; then
  app_root="/opt/"
  app_cache="/hpc_apps/.cache/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
fi
mkdir -p $app_cache

grep cosbrowser $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] This app has been installed to all users. Please run it directly."
  exit 1
else
  grep cosbrowser $private_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] This app has been installed to the current user. Please run it directly."
    exit 3
  fi
fi

# Check whether the desktop environment is ready
yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  if [ $current_user != 'root' ]; then
    echo -e "[ FATAL: ] Desktop environment is absent. Please contact the administrator."
    exit 5
  fi
  echo -e "[ -INFO- ] This app needs desktop environment. Installing now ..."
  hpcmgr install desktop >> ${tmp_log}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit 7
  fi
fi

grep desktop_env  $public_app_registry >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "desktop_env" >> $public_app_registry
fi

echo -e "[ -INFO- ] Downloading package(s) ..."
wget https://cos5.cloud.tencent.com/cosbrowser/cosbrowser-latest-linux.zip -O ${app_cache}/cosbrowser.zip -q
unzip ${app_cache}/cosbrowser.zip -d $app_root
chmod +x ${app_root}/cosbrowser.AppImage
echo -e "[ -INFO- ] Creating a shortcut on the desktop ..."
echo -e "[Desktop Entry]" > $HOME/Desktop/cos.desktop
echo -e "Encoding=UTF-8" >> $HOME/Desktop/cos.desktop
echo -e "Version=1.0" >> $HOME/Desktop/cos.desktop
echo -e "Name=cosbrowser" >> $HOME/Desktop/cos.desktop
echo -e "Comment=cosbrowser" >> $HOME/Desktop/cos.desktop
echo -e "Exec=${app_root}cosbrowser.AppImage --no-sandbox" >> $HOME/Desktop/cos.desktop
echo -e "Icon=/opt/app.png" >> $HOME/Desktop/cos.desktop
echo -e "Terminal=false" >> $HOME/Desktop/cos.desktop
echo -e "StartupNotify=true" >> $HOME/Desktop/cos.desktop
echo -e "Type=Application" >> $HOME/Desktop/cos.desktop
echo -e "Categories=Applications;" >> $HOME/Desktop/cos.desktop

if [ $current_user = 'root' ]; then
  while read user_row
  do
    user_name=`echo $user_row | awk '{print $2}'`
    grep cosbrowser /home/$user_name/.now_apps.reg >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      /bin/cp $HOME/Desktop/cos.desktop /home/$user_name/Desktop/
      chown -R $user_name:$user_name /home/$user_name/Desktop/cos.desktop
    fi
  done < /root/.cluster_secrets/user_secrets.txt
  grep "alias cos.pub=" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias cos.pub='${app_root}cosbrowser.AppImage --no-sandbox'" >> /etc/profile
  fi
  echo -e "cosbrwoser" >> $public_app_registry
  echo -e "[ -DONE- ] COS has been installed to all users."
else
  grep "alias cos=" $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias cos='${app_root}cosbrowser.AppImage --no-sandbox'" >> $HOME/.bashrc
  fi
  echo -e "cosbrowser" >> $private_app_registry
fi
echo -e "[ -DONE- ] COS has been installed to the current user."