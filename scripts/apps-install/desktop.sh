#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to install *Desktop* to HPC-NOW cluster.

if [ $1 = 'remove' ]; then
  echo -e "[ FATAL: ] This is an internal & global component. Cannot be removed."
  exit 0
fi
current_user=`whoami`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can install this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/desktop/
public_app_registry="/hpc_apps/.public_apps.reg"

yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  echo -e "[ -WARN- ] It seem the desktop environment has already been installed. Reinstalling now ..."
fi

centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ ! -z $centos_ver ] && [ $centos_ver -eq 7 ]; then
  yum -y install ntpdate >> ${2} 2>&1
  ntpdate ntp.ntsc.ac.cn >> ${2} 2>&1
  cp -r /etc/profile /etc/profile.orig
  yum grouplist installed -q | grep "GNOME Desktop" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] OS: CentOS 7. Installing Desktop now ... "
    yum -y update >> ${2} 2>&1 && yum -y groupinstall "GNOME Desktop" >> ${2} 2>&1
  fi
  echo -e "[ -INFO- ] Setting desktop now ... "
  systemctl disable firewalld >> ${2} 2>&1
  systemctl stop firewalld
  systemctl set-default graphical.target >> ${2} 2>&1
  echo -e "[ -INFO- ] Installing RDP service now ... "
  yum -y install tigervnc tigervnc-server xrdp gcc-c++ gfortran -q >> ${2} 2>&1
  yum -y install ibus ibus-libpinyin -q >> ${2} 2>&1
  yum -y install kde-l10n-Chinese -q >> ${2} 2>&1
  systemctl start ntpd.service && systemctl enable ntpd.service >> ${2} 2>&1
  sed -i 's/; (1 = ExtendedDesktopSize)/ (1 = ExtendedDesktopSize)/g' /etc/xrdp/xrdp.ini
  sed -i 's/#xserverbpp=24/xserverbpp=24/g' /etc/xrdp/xrdp.ini
  #localectl set-locale LANG=zh_CN.UTF-8
  systemctl start xrdp
  systemctl enable xrdp >> ${2} 2>&1
  echo -e "[ -DONE- ] Desktop environment installed."
  echo -e "#! /bin/bash\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true" > /etc/g_ini.sh
  chmod +x /etc/g_ini.sh
  sed -i '/gini/d' /etc/profile
  echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
  echo -e "[ -DONE- ] *IMPORTANT*: Please set password for the users in order to log into the desktop by using RDP."
  wget ${url_pkgs}libstdc++.so.6.0.26 -O /usr/lib64/libstdc++.so.6.0.26 -q
  rm -rf /usr/lib64/libstdc++.so.6
  ln -s /usr/lib64/libstdc++.so.6.0.26 /usr/lib64/libstdc++.so.6
  grep "source /etc/profile" /root/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "source /etc/profile" >> /root/.bashrc
  fi
  find /home -name ".bashrc" > /tmp/bashrc_dirs.txt
  while read rows
  do 
    grep "source /etc/profile" ${rows} >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "source /etc/profile" >> ${rows}
    fi
  done < /tmp/bashrc_dirs.txt
  rm -rf /tmp/bashrc_dirs.txt
else
  yum -y install chrony >> ${2} 2>&1
  systemctl start chronyd && systemctl enable chronyd
  cp -r /etc/profile /etc/profile.orig
  echo -e "[ -INFO- ] OS: CentOS Stream 9. "
  echo -e "[ -INFO- ] Updating the system now ..."
  yum -y update >> ${2} 2>&1 && yum -y install epel-release >> ${2} 2>&1
  yum grouplist installed -q | grep "Server with GUI" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ -INFO- ] Installing Desktop now ..."
    yum -y groupinstall "Server with GUI" >> ${2} 2>&1
  fi
  systemctl enable gdm --now
  systemctl disable firewalld >> ${2} 2>&1
  systemctl stop firewalld
  systemctl set-default graphical.target >> ${2} 2>&1
  echo -e "[ -INFO- ] Installing RDP service now ... "
  yum -y install tigervnc tigervnc-server xrdp gcc-c++ gfortran -q >> ${2} 2>&1
  yum -y install langpacks-zh_CN ibus ibus-libpinyin -q >> ${2} 2>&1
  #echo LANG=zh_CN.UTF-8 > /etc/locale.conf
  source /etc/locale.conf
  sed -i 's/; (1 = ExtendedDesktopSize)/ (1 = ExtendedDesktopSize)/g' /etc/xrdp/xrdp.ini
  sed -i 's/#xserverbpp=24/xserverbpp=24/g' /etc/xrdp/xrdp.ini
  systemctl start xrdp
  systemctl enable xrdp >> ${2} 2>&1
  if [ ! -f /usr/share/backgrounds/wallpapers.zip ]; then
    rm -rf /usr/share/backgrounds/*.png
    rm -rf /usr/share/backgrounds/*.jpg
    wget -q ${url_pkgs}wallpapers.zip -O /usr/share/backgrounds/wallpapers.zip
    cd /usr/share/backgrounds && unzip -o -q wallpapers.zip
  fi
# Make sure the desktop is user-friendly
  sed -i 's/#WaylandEnable=false/WaylandEnable=false/g' /etc/gdm/custom.conf
  yum -y install gnome-tweaks gnome-extensions-app.x86_64 -q
  echo -e "#! /bin/bash\ngnome-extensions enable background-logo@fedorahosted.org\ngnome-extensions enable window-list@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable apps-menu@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable desktop-icons@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable launch-new-instance@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable places-menu@gnome-shell-extensions.gcampax.github.com\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true\ngsettings set org.gnome.desktop.background picture-options centered\ngsettings set org.gnome.desktop.background picture-uri /usr/share/backgrounds/day.jpg" > /etc/g_ini.sh
  chmod +x /etc/g_ini.sh
  sed -i '/gini/d' /etc/profile
  echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
  echo -e "[ -DONE- ] Desktop environment installed."
  echo -e "[ -DONE- ] *IMPORTANT*: Please set password for the users in order to log into the desktop by using RDP."
  grep "source /etc/profile" /root/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "source /etc/profile" >> /root/.bashrc
  fi
  find /home -name ".bashrc" > /tmp/bashrc_dirs.txt
  while read rows
  do 
    grep "source /etc/profile" ${rows} >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "source /etc/profile" >> ${rows}
    fi
  done < /tmp/bashrc_dirs.txt
  rm -rf /tmp/bashrc_dirs.txt
fi
grep "< desktop_env >"  $public_app_registry >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "< desktop_env >" >> $public_app_registry
fi