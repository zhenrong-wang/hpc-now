#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Paraview 5* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ -f /opt/ParaView/bin/paraview ]; then
  echo -e "[ -INFO- ] It seems ParaView is already in place. If you do want to reinstall it, pease delete the /opt/ParaView folder and retry. Exit now.\n"
  cat /etc/profile | grep paraview >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias paraview='/opt/ParaView/bin/paraview'" >> /etc/profile
  fi
  exit
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING PARAVIEW" >> ${logfile}
tmp_log=/tmp/hpcmgr_install.log
CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ $CENTOS_V -eq 9 ]; then
  echo -e "[ -INFO- ] Downloading and extracting files ..."
  if [ ! -f /opt/packs/ParaView5.tar.gz ]; then
    wget ${URL_PKGS}ParaView-5.10.1-MPI-Linux-Python3.9-x86_64.tar.gz -q -O /opt/packs/ParaView5.tar.gz
  fi
  if [ -d /opt/ParaView ]; then
    rm -rf /opt/ParaView
  fi
  cd /opt && tar zxf /opt/packs/ParaView5.tar.gz
  mv /opt/ParaView-5.10.1-MPI-Linux-Python3.9-x86_64 /opt/ParaView
  echo -e "[ -INFO- ] Adding environment variables ..."
  cat /etc/profile | grep paraview >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias paraview='/opt/ParaView/bin/paraview'" >> /etc/profile
  fi
  echo -e "[ -INFO- ] ParaView-Version 5.10 has been installed into this cluster. Please run 'paraview' command to start ParaView."
elif [ $CENTOS_V -eq 7 ]; then
  echo -e "[ -INFO- ] Downloading and extracting files ..."
  if [ ! -f /opt/packs/ParaView4.tar.gz ]; then
    wget ${URL_PKGS}ParaView-4.0.1-Linux-64bit-glibc-2.3.6.tar.gz -q -O /opt/packs/ParaView4.tar.gz
  fi
  if [ -d /opt/ParaView ]; then
    rm -rf /opt/ParaView
  fi
  cd /opt && tar zxf /opt/packs/ParaView4.tar.gz
  mv /opt/ParaView-4.0.1-Linux-64bit /opt/ParaView
  echo -e "[ -INFO- ] Adding environment variables ..."
  cat /etc/profile | grep paraview >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias paraview='/opt/ParaView/bin/paraview'" >> /etc/profile
  fi
  echo -e "[ -INFO- ] ParaView-Version 4.01 has been installed into this cluster. Please run 'paraview' command to start ParaView."
else
  echo -e "[ FATAL: ] Unknown CentOS Version. Exit Now.\n"
  exit
fi
echo -e "[Desktop Entry]\nEncoding=UTF-8\nVersion=1.0\nName=ParaView\nComment=ParaView\nExec=/opt/ParaView/bin/paraview\nIcon=/opt/app.png\nTerminal=false\nStartupNotify=true\nType=Application\nCategories=Applications;" > /root/Desktop/ParaView.desktop
find /home -name "Desktop" > /tmp/desktop_dirs.txt
while read rows
do 
  user_row=`echo $rows | awk -F"/" '{print $3}'`
  /bin/cp /root/Desktop/ParaView.desktop ${rows}
  chown -R ${user_row}:${user_row} ${rows}
done < /tmp/desktop_dirs.txt
rm -rf /tmp/desktop_dirs.txt