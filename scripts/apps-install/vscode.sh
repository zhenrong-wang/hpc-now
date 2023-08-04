# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *VSCode* to HPC-NOW cluster.

current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi
public_app_registry="/hpc_apps/.public_apps.reg"

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the package ..."
  yum remove code -y
  sed -i '/< code >/d' $public_app_registry
  echo -e "[ -INFO- ] App removed successfully."
  exit 0
fi

yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] This app needs desktop environment. Installing now ..."
  hpcmgr install desktop >> ${2}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit 3
  fi
fi

echo -e "[ -INFO- ] Installing Visual Studio Code for Linux now ..."
rpm --import https://packages.microsoft.com/keys/microsoft.asc
sh -c 'echo -e "[code]\nname=Visual Studio Code\nbaseurl=https://packages.microsoft.com/yumrepos/vscode\nenabled=1\ngpgcheck=1\ngpgkey=https://packages.microsoft.com/keys/microsoft.asc" > /etc/yum.repos.d/vscode.repo'
yum check-update >> /dev/null 2>&1
yum install code -y >> ${2} 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -DONE- ] VSCode installed. You can run command 'code' to start using it."
  echo -e "< vscode >" >> $public_app_registry
else
  echo -e "[ FATAL: ] Failed to install VSCode. Please check the log file for details. Exit now."
  exit 7
fi

echo -e "[Desktop Entry]" > $HOME/Desktop/code.desktop
echo -e "Encoding=UTF-8" >> $HOME/Desktop/code.desktop
echo -e "Version=1.0" >> $HOME/Desktop/code.desktop
echo -e "Name=VSCode" >> $HOME/Desktop/code.desktop
echo -e "Comment=VSCode" >> $HOME/Desktop/code.desktop
echo -e "Exec=/bin/code" >> $HOME/Desktop/code.desktop
echo -e "Icon=/opt/app.png" >> $HOME/Desktop/code.desktop
echo -e "Terminal=false" >> $HOME/Desktop/code.desktop
echo -e "StartupNotify=true" >> $HOME/Desktop/code.desktop
echo -e "Type=Application" >> $HOME/Desktop/code.desktop
echo -e "Categories=Applications;" >> $HOME/Desktop/code.desktop

find /home -name "Desktop" > /tmp/desktop_dirs.vscode.txt
while read rows
do 
  user_row=`echo $rows | awk -F"/" '{print $3}'`
  /bin/cp $HOME/Desktop/code.desktop ${rows}
  chown -R ${user_row}:${user_row} ${rows}
done < /tmp/desktop_dirs.vscode.txt
rm -rf /tmp/desktop_dirs.vscode.txt