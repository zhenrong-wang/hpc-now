#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Paraview 5* to HPC-NOW cluster.

current_user=`whoami`
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_paraview_${current_user}.log"

if [ $current_user = 'root' ]; then
  app_root="/opt/"
  app_cache="/hpc_apps/.cache/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
fi

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing binaries and libraries ..."
  rm -rf ${app_root}ParaView
  echo -e "[ -INFO- ] Removing environment module file ..."
  if [ $current_user = 'root' ]; then
    rm -rf /root/Desktop/ParaView.desktop
    sed -i '/< paraview >/d' $public_app_registry
    sed -i '/paraview/d' /etc/profile
    while read user_row
    do
      user_name=`echo $user_row | awk '{print $2}'`
      grep "< paraview > < $user_name >" $private_app_registry >> /dev/null 2>&1
      if [ $? -ne 0 ]; then
        rm -rf /home/$user_name/Desktop/ParaView.desktop
      fi
    done < /root/.cluster_secrets/user_secrets.txt
  else
    rm -rf /home/${current_user}/Desktop/ParaView.desktop
    sed -e "/< paraview > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
    sed -i '/paraview/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] ParaView has been removed successfully."
  exit 0
fi

mkdir -p $app_cache
centos_v=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ -z $centos_v ] || [ $centos_v -ne 7 ]; then
  echo -e "[ -INFO- ] Downloading and extracting files ..."
  if [ ! -f ${app_cache}ParaView5.tar.gz ]; then
    wget ${url_pkgs}ParaView-5.10.1-MPI-Linux-Python3.9-x86_64.tar.gz -O ${app_cache}ParaView5.tar.gz
  fi
  tar zvxf ${app_cache}ParaView5.tar.gz -C ${app_root} >> ${tmp_log}
  mv ${app_root}ParaView-5.10.1-MPI-Linux-Python3.9-x86_64 ${app_root}ParaView
else
  echo -e "[ -INFO- ] Downloading and extracting files ..."
  if [ ! -f ${app_cache}ParaView4.tar.gz ]; then
    wget ${url_pkgs}ParaView-4.0.1-Linux-64bit-glibc-2.3.6.tar.gz -O ${app_cache}ParaView4.tar.gz
  fi
  tar zvxf ${app_cache}ParaView4.tar.gz -C ${app_root} >> ${tmp_log}
  mv ${app_root}ParaView-4.0.1-Linux-64bit ${app_root}ParaView
fi

echo -e "[ -INFO- ] Adding environment variables ..."
if [ $current_user = 'root' ]; then
  grep paraview.pub /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias paraview.pub='${app_root}ParaView/bin/paraview'" >> /etc/profile
  fi
  echo -e "[Desktop Entry]\nEncoding=UTF-8\nVersion=1.0\nName=ParaView\nComment=ParaView\nExec=/opt/ParaView/bin/paraview\nIcon=/opt/app.png\nTerminal=false\nStartupNotify=true\nType=Application\nCategories=Applications;" > $HOME/Desktop/ParaView.desktop
  while read user_row
  do
    user_name=`echo $user_row | awk '{print $2}'`
    grep "< paraview > < ${user_name} >" $private_app_registry >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      /bin/cp $HOME/Desktop/ParaView.desktop /home/$user_name/Desktop/
      chown -R $user_name:$user_name /home/$user_name/Desktop/ParaView.desktop
    fi
  done < /root/.cluster_secrets/user_secrets.txt
  echo -e "< paraview >" >> $public_app_registry
  echo -e "[ -DONE- ] ParaView has been installed to all users."
else
  grep paraview $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias paraview='${app_root}ParaView/bin/paraview'" >> $HOME/.bashrc
  fi
  echo -e "[Desktop Entry]\nEncoding=UTF-8\nVersion=1.0\nName=ParaView\nComment=ParaView\nExec=${app_root}ParaView/bin/paraview\nIcon=/opt/app.png\nTerminal=false\nStartupNotify=true\nType=Application\nCategories=Applications;" > $HOME/Desktop/ParaView.desktop
  echo -e "< paraview > < ${current_user} >" >> $private_app_registry
  echo -e "[ -DONE- ] ParaView has been installed to the current user."
fi