#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *rarlinux* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/rar/
tmp_log=/tmp/hpcmgr_install.log

echo -e "[ -INFO- ] Software: RAR for Linux "
if [[ -f /usr/local/bin/rar && -f /usr/local/bin/unrar ]]; then
  echo -e "[ -INFO- ] rarlinux is already in place. You can run 'rar' command to use it. Exit now."
  exit
fi

mkdir -p /opt/packs
if [ ! -f /opt/packs/rarlinux-x64-612.tar.gz ]; then
  echo -e "[ -INFO- ] Downloading package(s) ..."
  wget ${URL_PKGS}rarlinux-x64-612.tar.gz -O /opt/packs/rarlinux-x64-612.tar.gz -q
fi

tar zxf /opt/packs/rarlinux-x64-612.tar.gz -C /opt/packs
mkdir -p /usr/local/bin
mkdir -p /usr/local/lib
/bin/cp /opt/packs/rar/rar /opt/packs/rar/unrar /usr/local/bin
/bin/cp /opt/packs/rar/rarfiles.lst /etc
/bin/cp /opt/packs/rar/default.sfx /usr/local/lib
echo -e "[ -DONE- ] rarlinux has been installed. "