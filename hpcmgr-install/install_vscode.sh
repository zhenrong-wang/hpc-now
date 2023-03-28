#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *VSCode* to HPC-NOW cluster.
tmp_log=/tmp/hpcmgr_install.log

echo -e "[ -INFO- ] Software: VSCode for Linux "
yum list installed -q | grep code.x86_64 | grep @code >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -INFO- ] It seems VSCode is in place. Please run it directly. Exit now."
  exit
fi

rpm --import https://packages.microsoft.com/keys/microsoft.asc
sh -c 'echo -e "[code]\nname=Visual Studio Code\nbaseurl=https://packages.microsoft.com/yumrepos/vscode\nenabled=1\ngpgcheck=1\ngpgkey=https://packages.microsoft.com/keys/microsoft.asc" > /etc/yum.repos.d/vscode.repo'
yum check-update >> /dev/null 2>&1
yum install code -y >> $tmp_log 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -DONE- ] VSCode installed. You can run command 'code' to start using it."
else
  echo -e "[ FATAL: ] Failed to install VSCode. Please check the log file for details. Exit ow."
fi