#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *rarlinux* to HPC-NOW cluster.

current_user=`whoami`
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/rar/
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_rar_${current_user}.log"

if [ $current_user = 'root' ]; then
  app_root="/opt/"
  app_cache="/hpc_apps/.cache/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
fi

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the binaries and libraries ..."
  if [ $current_user = 'root' ]; then
    rm -rf /usr/local/bin/rar
    rm -rf /usr/local/bin/unrar
    rm -rf /etc/rarfiles.lst
    rm -rf /usr/local/lib/default.sfx
    sed -i '/< rar >/d' $public_app_registry
  else
    rm -rf ${app_root}rar
    sed -i '/rar/d' $HOME/.bashrc
    sed -e "/< rar > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] App removed."
  exit 0
fi

mkdir -p $app_cache
echo -e "[ -INFO- ] Software: RAR for Linux "
if [ ! -f ${app_cache}rarlinux-x64-612.tar.gz ]; then
  echo -e "[ -INFO- ] Downloading package(s) ..."
  wget ${url_pkgs}rarlinux-x64-612.tar.gz -O ${app_cache}rarlinux-x64-612.tar.gz >> ${tmp_log}
fi
echo -e "[ -INFO- ] Extracting packages ..."
tar zvxf ${app_cache}rarlinux-x64-612.tar.gz -C ${app_root} >> ${tmp_log}
if [ $current_user = 'root' ]; then
  mkdir -p /usr/local/bin
  mkdir -p /usr/local/lib
  /bin/cp ${app_root}rar/rar ${app_root}rar/unrar /usr/local/bin
  /bin/cp ${app_root}rar/rarfiles.lst /etc
  /bin/cp ${app_root}rar/default.sfx /usr/local/lib
  echo -e "< rar >" >> $public_app_registry
else
  echo -e "export PATH=${app_root}rar/:\$PATH" >> $HOME/.bashrc
  echo -e "< rar > < ${current_user} >" >> $private_app_registry
fi

echo -e "[ -DONE- ] rarlinux has been installed. "