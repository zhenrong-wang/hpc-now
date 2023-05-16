#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *HPC-NOW Netdisk (COSbrowser)* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/cosbrowser/

yum list installed -q | grep gnome >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] NOW Disk needs desktop environment. Installing now."
  hpcmgr install desktop >> ${tmp_log}.desktop
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit
  fi
fi

if [ ! -f /opt/cosbrowser.AppImage ]; then
  echo -e "[ -INFO- ] Downloading package(s) ..."
  wget https://cos5.cloud.tencent.com/cosbrowser/cosbrowser-latest-linux.zip -O /opt/cosbrowser.zip -q
  cd /opt && unzip cosbrowser.zip && rm -rf /opt/cosbrowser.zip
fi
chmod +x /opt/cosbrowser.AppImage
if [ ! -f /opt/app.png ]; then
  wget ${URL_PKGS}app.png -O /opt/app.png -q
fi
echo -e "[ -INFO- ] Creating a shortcut on the desktop ..."
wget ${URL_PKGS}cos.desktop -O /opt/cos.desktop -q
if [ -d /root/Desktop ]; then
  /bin/cp /opt/cos.desktop /root/Desktop
fi
find /home -name "Desktop" > /tmp/desktop_dirs.txt
while read rows
do 
  user_row=`echo $rows | awk -F"/" '{print $3}'`
  /bin/cp /opt/cos.desktop ${rows}
  chown -R ${user_row}:${user_row} ${rows}
done < /tmp/desktop_dirs.txt
rm -rf /tmp/desktop_dirs.txt
cat /etc/profile | grep "alias cos=" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias cos='/opt/cosbrowser.AppImage --no-sandbox'" >> /etc/profile
fi
echo -e "[ -DONE- ] COS has been installed to your system."