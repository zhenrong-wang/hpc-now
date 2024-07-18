# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# arg1: # of users to be created

logfile='/root/cluster_init.log'
public_app_registry="/hpc_apps/.public_apps.reg"
time_current=`date "+%Y-%m-%d %H:%M:%S"`
app_tmp_log_root="/tmp/app_tmp_logs/"
utils_path='/tmp/utils/'
scripts_path='/tmp/scripts/'

echo -e "# $time_current Initialization started." >> ${logfile}
distro_type=`head -n 3 /etc/os-release | grep NAME= | awk -F"\"" '{print $2}' | awk '{print $1}'`
distro_vers=`head -n 3 /etc/os-release | grep VERSION= | awk -F"\"" '{print $2}' | awk '{print $1}'`
if [ $distro_type != 'CentOS' ]; then
  centos_vers='NULL'
else
  centos_vers=$distro_vers
fi
echo -e "export GNU_LINUX_DISTRO=${distro_type}" >> /etc/profile
echo -e "export GNU_LINUX_DISTRO_VERSION=${distro_vers}" >> /etc/profile
echo -e "export CENTOS_VERSION=${centos_vers}" >> /etc/profile
echo -e "alias sudo='sudo -E'" >> /etc/profile
source /etc/profile 

# The system global env INITUTILS_REPO_ROOT must be set and written in /etc/profile befor execution
if [ -z $INITUTILS_REPO_ROOT ]; then
  echo -e "# $time_current [ FATAL: ] The critical environment var INITUTILS_REPO_ROOT is not set. Init abort." >> ${logfile}
  exit 1
fi
url_utils=${INITUTILS_REPO_ROOT}

#CLOUD_A: Alicloud
#CLOUD_B: QCloud/TencentCloud
#CLOUD_C: Amazon Web Services
#CLOUD_D: Huawei Cloud
#CLOUD_E: BaiduCloud
#CLOUD_F: Azure(GLOBAL)
#CLOUD_G: Google Cloud Platform

cloud_flag=`find /root/ -name "CLOUD_*" | awk -F"/" '{print $NF}'`
if [ -z $cloud_flag ]; then
  echo -e "# $time_current [ FATAL: ] Cloud flag is missing. Initialization abort." >> ${logfile}
  exit 1
fi

# Sync Time among cluster nodes
if [ ! -z $centos_vers ] && [ $centos_vers = 7 ]; then
  yum -y install ntpdate
  ntpdate ntp.ntsc.ac.cn
fi

echo -e "\nWelcome to the HPC-NOW Cluster!\nGithub Repo: https://github.com/zhenrong-wang/hpc-now\n" > /etc/motd
sed -i 's/#   StrictHostKeyChecking ask/StrictHostKeyChecking no/g' /etc/ssh/ssh_config
echo -e "LogLevel QUIET" >> /etc/ssh/ssh_config
sed -i '/ClientAliveInterval/,+0d' /etc/ssh/sshd_config
sed -i '/ClientAliveCountMax/,+0d' /etc/ssh/sshd_config
echo -e "ClientAliveInterval 60\nClientAliveCountMax 3" >> /etc/ssh/sshd_config
systemctl restart sshd
systemctl start atd
systemctl enable atd
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current SSH setup finished" >> ${logfile}
source /etc/profile
/bin/cp /etc/hosts /etc/hosts-clean
mkdir -p /root/.cluster_secrets
mkdir -p /root/.sshkey_deleted

time1=$(date)
echo -e  "\n${time1}" >> ${logfile}
if [ -z "$1" ] || [ -z "$2" ]; then
  echo -e "Lack of Parameters.\n# arg1: #\n# arg2: db\nPLEASE PAY ATTENTION TO THE SEQUENCE OF THE PARAMETERS!\nExit now."
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "![IMPORTANT]\n# $time_current Cluster initialization failed due to lack of command parameters.\n" >> ${logfile}
  exit
fi

if [ -f /root/hostfile ]; then
  if [ ! -f /root/master_passwd.txt ] || [ ! -f /root/compute_passwd.txt ]; then
    echo -e "UNKNOWN master node password or compute node pasword. Please make sure the 2 files are in /root directory: master_passwd.txt & compute_passwd.txt.\nExit now."
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "UNKNOWN master node password or compute node pasword. Please make sure the 2 files are in /root directory: master_passwd.txt & compute_passwd.txt.\nExit now." >> ${logfile}
    exit
  fi
fi

echo -e "# Plan to create $1 users."
echo -e "# Plan create $1 users." >> ${logfile} 

num_processors=`cat /proc/cpuinfo| grep "processor" | wc -l`
selinux_status=`getenforce`
echo -e "source /etc/profile" >> /root/.bashrc

if [ -f /root/hostfile ]; then
  mkdir -p /hpc_apps/root_apps
  mkdir -p /hpc_data/cluster_data
  chmod -R 644 /hpc_data/cluster_data
  chmod 755 /hpc_data/cluster_data
  if [ $cloud_flag = 'CLOUD_E' ]; then
    mkdir -p /hpc_data/cluster_data/.bucket_creds
    chmod -R 644 /hpc_data/cluster_data/.bucket_creds
  fi
  mkdir -p /hpc_data/public
  chmod -R 777 /hpc_data/public
  mkdir -p ${app_tmp_log_root}
  chmod 777 ${app_tmp_log_root}
fi

mkdir -p /usr/hpc-now/
touch $public_app_registry # Only root user can modify this file

# Add user slurm 
id -u slurm
if [ $? -ne 0 ]; then
  useradd slurm
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current User slurm added." >> ${logfile}

# Add System Users 
echo -e "user1 ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
sed -i 's/env_reset/!env_reset/g' /etc/sudoers
sed -i 's/Defaults    secure_path/#Defaults    secure_path/g' /etc/sudoers # Granted sudo permissions to user1
chmod 511 /usr/bin/passwd # Disable ordinary users to change its own password
while read user_row
do
  user_name=`echo $user_row | awk '{print $2}'`
  user_passwd=`echo $user_row | awk '{print $3}'`
  user_status=`echo $user_row | awk '{print $4}'`
  private_app_registry="/hpc_apps/${user_name}_apps/.private_apps.reg"
  useradd ${user_name} -m
  mkdir -p /home/${user_name} && chown -R ${user_name}:${user_name} /home/${user_name}
  echo -e "source /etc/profile" >> /home/${user_name}/.bashrc
  if [ -f /root/hostfile ]; then
    echo ${user_passwd} | passwd ${user_name} --stdin >> /dev/null 2>&1
    mkdir -p /home/${user_name}/.ssh && rm -rf /home/${user_name}/.ssh/*
    ssh-keygen -t rsa -N '' -f /home/${user_name}/.ssh/id_rsa -q
    cat /home/${user_name}/.ssh/id_rsa.pub >> /home/${user_name}/.ssh/authorized_keys
    cat /etc/now-pubkey.txt >> /home/${user_name}/.ssh/authorized_keys
    rm -rf /home/${user_name}/.ssh/id_rsa.pub
    mkdir -p /hpc_data/${user_name}_data
    mkdir -p /hpc_apps/${user_name}_apps
    mkdir -p /hpc_apps/envmod/${user_name}_env
    touch ${private_app_registry}
    chmod -R 750 /hpc_data/${user_name}_data
    chmod -R 750 /hpc_apps/${user_name}_apps
    chmod -R 750 /hpc_apps/envmod/${user_name}_env
    chown -R ${user_name}:${user_name} /hpc_data/${user_name}_data
    chown -R ${user_name}:${user_name} /hpc_apps/${user_name}_apps
    chown -R ${user_name}:${user_name} /hpc_apps/envmod/${user_name}_env
  fi
  chown -R ${user_name}:${user_name} /home/${user_name}
done < /root/user_secrets.txt
mv /root/user_secrets.txt /root/.cluster_secrets/
mv /root/master_passwd.txt /root/.cluster_secrets/
mv /root/compute_passwd.txt /root/.cluster_secrets/

yum -y install wget
if [ -f /root/hostfile ]; then
  if [ ! -f /hpc_apps/root_apps/init_master.tar.gz ]; then
    mkdir -p /hpc_apps/root_apps
    wget ${url_utils}init_master.tar.gz -O /hpc_apps/root_apps/init_master.tar.gz
  fi
  rm -rf /tmp/utils
  tar zvxf /hpc_apps/root_apps/init_master.tar.gz -C /tmp
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to extract init utils to master node."
    exit
  fi
  if [ ! -f /hpc_apps/root_apps/init_compute.tar.gz ]; then
    wget ${url_utils}init_compute.tar.gz -O /hpc_apps/root_apps/init_compute.tar.gz
  fi
else
  rm -rf /tmp/utils
  tar zvxf /hpc_apps/root_apps/init_compute.tar.gz -C /tmp
  if [ $? -ne 0 ]; then
    rm -rf /tmp/utils
    wget ${url_utils}init_compute.tar.gz -O /root/init_compute.tar.gz
    tar zvxf /root/init_compute.tar.gz -C /tmp
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Failed to extract init utils to compute node."
      exit
    fi
  fi
fi

/bin/cp -r ${scripts_path}* /usr/hpc-now/
chmod +x /usr/hpc-now/*.sh
if [ -f /root/hostfile ]; then
  wget ${url_utils}hpcmgr.sh -O /usr/hpc-now/.hpcmgr_main.sh
fi

yum -y install gcc bc openssl openssl-devel unzip curl make perl sshpass gtk2 gtk2-devel
# stop firewall and SELinux 
systemctl stop firewalld
systemctl disable firewalld
if [ $selinux_status != 'Disabled' ]; then
  sed -i 's/SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current SELINUX Disabled." >> ${logfile}

# The update step really takes time, trying to avoid it.
if [ $cloud_flag = 'CLOUD_B' ]; then
  yum -y install https://mirrors.cloud.tencent.com/epel/epel-release-latest-9.noarch.rpm
  sed -i 's|^#baseurl=https://download.example/pub|baseurl=https://mirrors.cloud.tencent.com|' /etc/yum.repos.d/epel*
  sed -i 's|^metalink|#metalink|' /etc/yum.repos.d/epel*
elif [ $cloud_flag = 'CLOUD_A' ]; then
  yum -y install https://mirrors.aliyun.com/epel/epel-release-latest-9.noarch.rpm
  sed -i 's|^#baseurl=https://download.example/pub|baseurl=https://mirrors.aliyun.com|' /etc/yum.repos.d/epel*
  sed -i 's|^metalink|#metalink|' /etc/yum.repos.d/epel*
else
  yum -y install epel-release 
fi
yum -y install python 
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Utils installed." >> ${logfile}

# Build munge
yum -y install rpm-build bzip2-devel zlib-devel m4 libxml2-devel net-tools
cd /root
if ! command -v munge >/dev/null 2>&1; then
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "# $time_current Start building munge." >> ${logfile}
  if [ ! -f munge-0.5.14* ]; then
    /bin/cp -r ${utils_path}munge/* .
  fi
  rpmbuild -tb munge-0.5.14.tar.xz
  cd /rpmbuild/RPMS/x86_64 && rpm -ivh munge*
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`  
echo -e "# $time_current Munge installed." >> ${logfile}

# Re-Install mariadb
if [ -f /root/hostfile ]; then
  yum remove -y `rpm -aq mariadb`
  rm -rf /etc/my.cnf
  rm -rf /var/lib/mysql
  if [ ! -z $centos_vers ] && [ $centos_vers = 7 ]; then
    yum -y install mariadb mariadb-devel mariadb-server
    yum -y install mariadb-libs
  else
    yum -y install mariadb-*
  fi
  if [ -f /root/mariadb_private_ip.txt ]; then
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    mv /root/mariadb_root_passwd.txt /root/.cluster_secrets/
    mv /root/mariadb_slurm_acct_db_pw.txt /root/.cluster_secrets/
    db_address=`cat /root/mariadb_private_ip.txt | awk -F"\t" '{print $1}'`
    echo -e "# $time_current MariaDB Server address: $db_address. Installing MariaDB Client ..." >> ${logfile}
  else
    if [ $2 != db ]; then
      time_current=`date "+%Y-%m-%d %H:%M:%S"`
      echo -e "# $time_current IMPORTANT: No dedicated MariaDB Server found. Automatically install Mariadb Server on localhost." >> ${logfile}
    fi
    systemctl start mariadb.service
    db_address="LOCALHOST"
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "# $time_current Mariadb installation on localhost started." >> ${logfile}
    
    if [ ! -z $centos_vers ] && [ $centos_vers = 7 ]; then
      openssl rand 8 -base64 -out /root/mariadb_root_passwd.txt
      openssl rand 8 -base64 -out /root/mariadb_slurm_acct_db_pw.txt
    else
      openssl rand -base64 -out /root/mariadb_root_passwd.txt 8
      openssl rand -base64 -out /root/mariadb_slurm_acct_db_pw.txt 8
    fi
    root_passwd=`cat /root/mariadb_root_passwd.txt`
    slurm_acct_db_pwd=`cat /root/mariadb_slurm_acct_db_pw.txt`
    echo -e "Reinstalling MariadbNow ...\n"
    yum -y install expect
    systemctl enable mariadb.service
    echo -e "#!/usr/bin/expect\nset passwd $root_passwd\nspawn mysql_secure_installation" > dbconfig.sh
    echo -e "expect {\n\t\t\"Enter current password\" { send \"\\\r\"; exp_continue }\n\t\t\"Y/n\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"New password\" { send \"\$passwd\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"Re-enter new password\" { send \"\$passwd\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"Remove anonymous users\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"Disallow root login remotely\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"Remove test database and access to it\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
    echo -e "\t\t\"Reload privilege tables now\" { send \"Y\\\r\" }\n}" >> dbconfig.sh
    chmod +x dbconfig.sh
    ./dbconfig.sh
    rm -rf dbconfig.sh
    mysql -hlocalhost -uroot -p$root_passwd -e"create database IF NOT EXISTS slurm_acct_db"
    hash=`mysql -hlocalhost -uroot -p$root_passwd -e"select password('$slurm_acct_db_pwd')" | tail -1`
    mysql -hlocalhost -uroot -p$root_passwd -e"GRANT ALL PRIVILEGES ON slurm_acct_db.* TO 'slurm'@'localhost' IDENTIFIED BY PASSWORD '$hash'"
    mysql -hlocalhost -uroot -p$root_passwd -e"flush privileges"
    systemctl restart mariadb
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "# $time_current MariaDB ON LOCALHOST reinstalled." >> ${logfile}
    mv /root/mariadb_root_passwd.txt /root/.cluster_secrets/
    mv /root/mariadb_slurm_acct_db_pw.txt /root/.cluster_secrets/
  fi
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current MariaDB installed." >> ${logfile}

# Change owners of directories 
mkdir -p /run/munge && chown -R slurm:slurm /run/munge
mkdir -p /etc/munge && chown -R slurm:slurm /etc/munge
mkdir -p /var/run/munge && chown -R slurm:slurm /var/run/munge
mkdir -p /var/lib/munge && chown -R slurm:slurm /var/lib/munge
mkdir -p /var/log/munge && chown -R slurm:slurm /var/log/munge

# munge 
if [ -f /root/hostfile ]; then
  mungekey
  chown -R slurm:slurm /etc/munge/munge.key
fi

# Build SLURM 
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Started building Slurm 21.08.8." >> ${logfile}
cd /root
if [ ! -f slurm-21.08.8-2.tar.bz2 ]; then
  /bin/cp -r ${utils_path}slurm/slurm-21.08.8-2.tar.bz2 .
fi
tar xf slurm-21.08.8-2.tar.bz2
cd slurm-21.08.8-2
./configure --prefix=/opt/slurm --sysconfdir=/opt/slurm/etc
make -j$num_processors && make install
/bin/cp etc/{slurmctld.service,slurmdbd.service,slurmd.service} /usr/lib/systemd/system
cat /etc/profile | grep slurm/bin
if [ $? -ne 0 ]; then
  export PATH=/opt/slurm/bin:$PATH
  export LD_LIBRARY_PATH=/opt/slurm/lib:$LD_LIBRARY_PATH
  echo -e "export PATH=/opt/slurm/bin:\$PATH\nexport LD_LIBRARY_PATH=/opt/slurm/lib:\$LD_LIBRARY_PATH\nexport PATH=/opt/slurm/sbin:\$PATH" >> /etc/profile
  source /etc/profile
fi
ln -s /opt/slurm/bin/* /usr/bin/
ln -s /opt/slurm/sbin/* /usr/sbin/
mkdir -p /opt/slurm/etc/

if [ -f /root/hostfile ]; then
  if [ ! -f /opt/slurm/etc/slurm.conf.128 ]; then
    /bin/cp -r ${utils_path}slurm/slurm.conf.128 /opt/slurm/etc/
  fi
  if [ ! -f /opt/slurm/etc/slurmdbd.conf ]; then
    /bin/cp -r ${utils_path}slurm/slurmdbd.conf /opt/slurm/etc/
  fi
  if [ $db_address != "LOCALHOST" ]; then
    sed -i "s@STORAGE_HOST@$db_address@g" /opt/slurm/etc/slurmdbd.conf
  else
    sed -i "s@STORAGE_HOST@localhost@g" /opt/slurm/etc/slurmdbd.conf
  fi
  local_address=`ifconfig | grep inet | head -n1 | awk '{print $2}'`
  dbd_passwd=`cat /root/.cluster_secrets/mariadb_slurm_acct_db_pw.txt`  
  sed -i "s@DBD_HOST@$local_address@g" /opt/slurm/etc/slurmdbd.conf
  sed -i "s@clarkwin2019@$dbd_passwd@g" /opt/slurm/etc/slurmdbd.conf
  chmod 600 /opt/slurm/etc/slurmdbd.conf
  chown -R slurm:slurm /opt/slurm/etc/slurmdbd.conf
  mkdir -p /var/spool/slurmctld
  chown -R slurm:slurm /var/spool/slurmctld
  systemctl start slurmdbd
  systemctl enable slurmdbd
  if [ -f /etc/munge/munge.key ]; then
    chown -R slurm:slurm /etc/munge/munge.key
  fi
  mkdir -p /opt/slurm/archive
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "# $time_current Slurm built and configured in path /opt/slurm." >> ${logfile}
  if [ ! -z $centos_vers ] && [ $centos_vers = 7 ]; then
     # This is a workaround. CentOS-7 will be deprecated in the future
     mv /usr/hpc-now/.hpcmgr_main.sh /usr/bin/hpcmgr && chmod +x /usr/bin/hpcmgr
  else
    /bin/cp -r ${utils_path}hpcmgr.exe /usr/bin/hpcmgr && chmod +x /usr/bin/hpcmgr
  fi
  yum -y install git python-devel
fi

# install environment-module
cd /root
if ! command -v module >/dev/null 2>&1; then
  yum install tcl-devel -y
  if [ ! -f modules-5.1.0.tar.gz ]; then
    /bin/cp -r ${utils_path}modules-5.1.0.tar.gz .
  fi
  tar zvxf modules-5.1.0.tar.gz
  cd modules-5.1.0
  mkdir -p /etc/modulefiles
  ./configure --prefix=/opt/environment-modules --modulefilesdir=/hpc_apps/envmod
  make -j$num_processors && make install
  ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.sh
  ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.csh
else
  #if module is already available, add the /hpc_apps/envmod to system envvar
  echo -e "export MODULEPATH=/hpc_apps/envmod:\$MODULEPATH" >> /etc/profile
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`  
echo -e "# $time_current Environment Module has been installed." >> ${logfile}
if [ -f /root/hostfile ]; then
  grep "< envmod >" $public_app_registry
  if [ $? -ne 0 ]; then
    echo -e "< envmod >" >> $public_app_registry
  fi
fi

# Install Desktop Env-NECESSARY
time_current=`date "+%Y-%m-%d %H:%M:%S"`
if [ -f /root/hostfile ]; then
  echo -e "# $time_current Started installing Desktop Environment." >> ${logfile}
  if [ $distro_type != 'CentOS' ] && [ $distro_type != 'Rocky' ] && [ $distro_type != 'Oracle' ]; then
    echo -e "# $time_current GNU/Linux Distro: ${distro_type}. Installing GUI now." >> ${logfile}
    yum -y install gnome-shell gdm gnome-session gnome-terminal gnome-system-monitor gnome-tweaks gnome-terminal-nautilus gnome-classic-session gnome-disk-utility
    yum -y install gnome-shell-*
    yum -y install firefox
    yum -y install nautilus
    yum -y install ibus-table-chinese texlive-collection-langchinese google-noto-sans-cjk-sc-fonts
    yum -y install gedit
    systemctl enable gdm.service --now
  else
    echo -e "# $time_current CENTOS VERSION $centos_vers. Installing GUI now." >> ${logfile}
    if [ ! -z $centos_vers ] && [ $centos_vers = 7 ]; then
      yum -y groupinstall "GNOME Desktop"
      /bin/cp -r ${utils_path}libstdc++.so.6.0.26 /usr/lib64/
      rm -rf /usr/lib64/libstdc++.so.6
      ln -s /usr/lib64/libstdc++.so.6.0.26 /usr/lib64/libstdc++.so.6
      systemctl disable firewalld
      systemctl stop firewalld
      echo -e "#! /bin/bash\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true\ngsettings set org.gnome.desktop.wm.preferences button-layout \":minimize,maximize,close\"\n" > /etc/g_ini.sh
      chmod +x /etc/g_ini.sh
      sed -i '/gini/d' /etc/profile
      echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
    else
#      if [ $cloud_flag != "CLOUD_G" ]; then
#        yum -y groupinstall "Server with GUI"
#      else
        # For some reasons, Google Compute Instance fails to restart after installing "Server with GUI". 
        # Therefore, we have to avoid installing "Server with GUI"
      yum -y install gnome-shell gdm gnome-session gnome-terminal gnome-system-monitor gnome-tweaks gnome-terminal-nautilus gnome-classic-session gnome-disk-utility
      yum -y install gnome-shell-*
      yum -y install firefox
      yum -y install nautilus
      yum -y install ibus-table-chinese texlive-collection-langchinese google-noto-sans-cjk-sc-fonts
      yum -y install gedit
#      fi
      systemctl enable gdm.service --now
    fi
  fi
  systemctl disable firewalld
  systemctl stop firewalld
  systemctl set-default graphical.target
  yum -y install tigervnc tigervnc-server
  grep "< desktop >" $public_app_registry
  if [ $? -ne 0 ]; then
    echo -e "< desktop >" >> $public_app_registry
  fi
# yum -y install xrdp 
# FATAL: xrdp-0.9.22 fails to work. We have to build xrdp from source.
  yum -y remove xrdp # For Amazon Machines, xrdp may have been installed. Here we need to remove and rebuild.
  yum -y install autoconf libtool automake pam-devel
  cd /root
  if [ ! -f /root/xrdp-0.9.zip ]; then
    /bin/cp -r ${utils_path}xrdp-0.9.zip .
  fi
  unzip -o xrdp-0.9.zip && rpm -ivh nasm-2.16.rpm
  chmod +x /root/xrdp-0.9/bootstrap && chmod +x /root/xrdp-0.9/librfxcodec/src/nasm_lt.sh && chmod +x /root/xrdp-0.9/instfiles/pam.d/mkpamrules
  cd /root/xrdp-0.9/ && ./bootstrap && ./configure
  make -j$num_processors && make install
  rm -rf /root/xrdp-0.9*
  rm -rf /root/nasm-2.16.rpm
  /bin/cp /etc/xrdp/xrdp.ini /etc/xrdp/xrdp.ini.bkup
  sed -i '/\[Xorg\]/,+7d' /etc/xrdp/xrdp.ini
  sed -i '/\[vnc-any\]/,+7d' /etc/xrdp/xrdp.ini
  sed -i '/\[neutrinordp-any\]/,+8d' /etc/xrdp/xrdp.ini
  sed -i 's/; (1 = ExtendedDesktopSize)/ (1 = ExtendedDesktopSize)/g' /etc/xrdp/xrdp.ini
  sed -i 's/#xserverbpp=24/xserverbpp=24/g' /etc/xrdp/xrdp.ini

  yum -y install rpcbind flex GConf2 cmake cmake3 tcsh
  yum -y install ibus libXScrnSaver
  yum -y install gmp-devel mpfr-devel 
  yum -y install ibus-pinyin
  if [ $cloud_flag = 'CLOUD_B' ]; then
    wget https://cos5.cloud.tencent.com/cosbrowser/cosbrowser-latest-linux.zip -O /opt/cosbrowser.zip
    cd /opt && unzip -o cosbrowser.zip && rm -rf cosbrowser.zip
    cat /etc/profile | grep cosbrowser
    if [ $? -ne 0 ]; then
      echo -e "alias cos='/opt/cosbrowser.AppImage --no-sandbox'" >> /etc/profile
    fi
    grep "< cos >" $public_app_registry
    if [ $? -ne 0 ]; then
      echo -e "< cos >" >> $public_app_registry
    fi
  elif [ $cloud_flag = 'CLOUD_A' ]; then
    wget https://gosspublic.alicdn.com/oss-browser/1.16.0/oss-browser-linux-x64.zip -O /opt/oss.zip
    cd /opt && unzip -o oss.zip && rm -rf oss.zip 
    cat /etc/profile | grep ossbrowser
    if [ $? -ne 0 ]; then
      echo -e "alias oss='/opt/oss-browser-linux-x64/oss-browser'" >> /etc/profile
    fi
    grep "< oss >" $public_app_registry
    if [ $? -ne 0 ]; then
      echo -e "< oss >" >> $public_app_registry
    fi
  fi
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "# $time_current Desktop Environment and RDP has been installed." >> ${logfile}
  systemctl restart xrdp
  systemctl enable xrdp
fi

# Download scripts & Desktop shortcuts 
if [ -f /root/hostfile ]; then
  mkdir -p /root/Desktop
  ln -s /hpc_apps /root/Desktop/
  ln -s /hpc_data /root/Desktop/
  ln -s /hpc_data/cluster_data /root/Desktop/
  /bin/cp -r ${utils_path}pics/* /opt/
  if [ $cloud_flag = 'CLOUD_A' ]; then
    /bin/cp -r ${utils_path}shortcuts/oss-.desktop /root/Desktop/
  elif [ $cloud_flag = 'CLOUD_B' ]; then
    /bin/cp -r ${utils_path}shortcuts/cos.desktop /root/Desktop/
  fi
  while read user_row
  do
    user_name=`echo $user_row | awk '{print $2}'`
    mkdir -p /home/${user_name}/Desktop
    ln -s /hpc_apps/${user_name}_apps /home/${user_name}/Desktop/
    ln -s /hpc_data/${user_name}_data /home/${user_name}/Desktop/
    cp /root/Desktop/*.desktop /home/${user_name}/Desktop
    chown -R ${user_name}:${user_name} /home/${user_name}
  done < /root/.cluster_secrets/user_secrets.txt
  rm -rf /usr/share/backgrounds/*.png
  rm -rf /usr/share/backgrounds/*.jpg
  /bin/cp -r ${utils_path}pics/wallpapers.zip /usr/share/backgrounds/
  cd /usr/share/backgrounds && unzip -o wallpapers.zip
  chmod 644 *.jpg
  if [ -z $centos_vers ] || [ $centos_vers != 7 ]; then
    sed -i 's/#WaylandEnable=false/WaylandEnable=false/g' /etc/gdm/custom.conf
    yum -y install gnome-tweaks gnome-extensions-app.x86_64
    echo -e "#! /bin/bash\ngnome-extensions enable background-logo@fedorahosted.org\ngnome-extensions enable window-list@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable apps-menu@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable desktop-icons@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable launch-new-instance@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable places-menu@gnome-shell-extensions.gcampax.github.com\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true\ngsettings set org.gnome.desktop.background picture-options zoom\ngsettings set org.gnome.desktop.background picture-uri /usr/share/backgrounds/hpc-now-default.jpg\ngsettings set org.gnome.desktop.wm.preferences button-layout \":minimize,maximize,close\"\n" > /etc/g_ini.sh
    chmod +x /etc/g_ini.sh
    echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
  fi
  if [ ! -f /hpc_data/sbatch_sample.sh ]; then
    /bin/cp -r ${utils_path}slurm/sbatch_sample.sh /hpc_data/
  fi
  
  if [ $cloud_flag = 'CLOUD_A' ]; then
    sudo -v ; curl https://gosspublic.alicdn.com/ossutil/install.sh | sudo bash
  elif [ $cloud_flag = 'CLOUD_B' ]; then
    #pip install coscmd
    curl https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-linux -o /usr/bin/coscli
    chmod +x /usr/bin/coscli
    chmod 755 /usr/bin/coscli
  elif [ $cloud_flag = 'CLOUD_C' ]; then 
    #yum -y install s3cmd
    curl https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip -q -o /tmp/awscli.zip
    unzip -o /tmp/awscli.zip -d /tmp
    /tmp/aws/install
  elif [ $cloud_flag = 'CLOUD_D' ]; then
    curl https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_linux_amd64.tar.gz -o /tmp/obsutil_linux_amd64.tar.gz
    tar zvxf /tmp/obsutil_linux_amd64.tar.gz -C /tmp/
    /bin/cp -r /tmp/obsutil_linux_amd64*/obsutil /usr/local/bin/
    chmod +x /usr/local/bin/obsutil
    chmod 755 /usr/local/bin/obsutil
  elif [ $cloud_flag = 'CLOUD_E' ]; then
    curl https://doc.bce.baidu.com/bce-documentation/BOS/linux-bcecmd-0.4.1.zip -o /tmp/bcecmd.zip
    unzip -o /tmp/bcecmd.zip -d /tmp
    mv /tmp/linux-bcecmd-0.4.1/bcecmd /usr/local/bin/
    chmod +x /usr/local/bin/bcecmd
    chmod 755 /usr/local/bin/bcecmd
  elif [ $cloud_flag = 'CLOUD_F' ]; then
    curl https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_linux_amd64_10.20.1.tar.gz -o /tmp/azcopy.tar.gz
    tar zvxf /tmp/azcopy.tar.gz -C /tmp/
    /bin/cp -r /tmp/azcopy_linux_amd64_10.20.1/azcopy /usr/local/bin/
    chmod 755 /usr/local/bin/azcopy
  elif [ $cloud_flag = 'CLOUD_G' ]; then
    curl https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-449.0.0-linux-x86_64.tar.gz -o /tmp/gcloud.tar.gz
    tar zvxf /tmp/gcloud.tar.gz -C /opt/
    echo -e "export PATH=/opt/google-cloud-sdk/bin:\$PATH" >> /etc/profile
  elif [ $cloud_flag = 'CLOUD_H' ]; then
    curl https://tos-tools.tos-cn-beijing.volces.com/linux/amd64/tosutil -o /usr/bin/tosutil
    chmod +x /usr/bin/tosutil
    chmod 755 /usr/bin/tosutil
  else
    echo "[ FATAL: ] The cloud flag is incorrect. This is not supposed to happen."
    exit 1
  fi
fi

yum -y update
systemctl stop firewalld
systemctl disable firewalld
systemctl mask firewalld

yum -y install gcc-c++ gcc-gfortran 
yum -y install htop 
yum -y install python3 python3-devel 
yum -y install hostname dos2unix bash-completion

if [ -f /root/hostfile ]; then
  yum -y install evince # The PDF viewer
  yum -y install eog # The image viewer
fi

if [ $cloud_flag = 'CLOUD_B' ]; then
  echo 1 > /sys/block/sr0/device/delete
fi

echo -e "Cleaning Up ..."
rm -rf /root/slurm*
rm -rf /root/munge*
rm -rf /root/modules*
rm -rf /root/dun.gpg
rm -rf /rpmbuild
echo -e "Installation Finished."
echo "*/1 * * * *  /usr/hpc-now/nowmon_mgr.sh " >> /var/spool/cron/root
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Initialization Finished." >> ${logfile}