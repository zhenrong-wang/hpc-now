#!/bin/bash
# Originally created by HPC-Now Co. Ltd
# Visit https://www.hpc-now.com for more information
# This script initializes the master and compute node(s). Terraform templates run this scripts after creating the master and compute node(s).

#arg1: # of users to be created
#arg2: db - whether reinstall mariadb or not
#arg3: mpi - whether install openmpi or not
#arg4: gcc8 - whether install gcc8 - Deprecated
#arg5: of7 - Whether install OpenFOAM7 to /hpc_apps or not - Deprecated

# Define URL prefixes for the 'wget' command

URL_REPO_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_UTILS=${URL_REPO_ROOT}utils/
URL_PKGS=${URL_REPO_ROOT}packages/

logfile='/root/cluster_init.log'
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Initialization started." >> ${logfile}

CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
echo -e "export CENTOS_V=$CENTOS_V" >> /etc/profile

#CLOUD_A: Alicloud
#CLOUD_B: QCloud/TencentCloud
#CLOUD_C: Amazon Web Services
if [ -f /root/CLOUD_A ]; then
  CLOUD_FLAG="CLOUD_A"
elif [ -f /root/CLOUD_B ]; then
  CLOUD_FLAG="CLOUD_B"
elif [ -f /root/CLOUD_C ]; then
  CLOUD_FLAG="CLOUD_C"
else
  echo -e "# $time_current [ FATAL: ] Cloud flag is missing. Initialization abort." >> ${logfile}
  exit
fi

# Sync Time among cluster nodes
if [ $CENTOS_V -eq 7 ]; then
  yum -y install ntpdate
  ntpdate ntp.ntsc.ac.cn
fi

sed -i 's/#   StrictHostKeyChecking ask/StrictHostKeyChecking no/g' /etc/ssh/ssh_config
echo -e "LogLevel QUIET" >> /etc/ssh/ssh_config
cat /etc/now-pubkey.txt >> /root/.ssh/authorized_keys
systemctl restart sshd
systemctl start atd
systemctl enable atd
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current SSH setup finished" >> ${logfile}

source /etc/profile
mkdir -p /root/.cluster_secrets
time1=$(date)
echo -e  "\n${time1}" >> ${logfile}

if [ ! -n "$1" ] || [ ! -n "$2" ] || [ ! -n "$3" ]; then
  echo -e "Lack of Parameters.\n# arg1: #\n# arg2: db\n# arg3: mpi\n# arg4: gcc8\n# arg5: of7\nPLEASE PAY ATTENTION TO THE SEQUENCE OF THE PARAMETERS!\nExit now."
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

if [ $1 -gt $((0)) ] && [ $1 -le $((8)) ]; then
  echo -e "# Plan to create $1 users."
  echo -e "# Plan create $1 users." >> ${logfile} 
else
  echo -e "WARNING: The first parameter is incorrect. Will create 3 users."
  echo -e "WARNING: The first parameter is incorrect. Will create 3 users." >> ${logfile}
  exit
fi

######### define something ##############
yum -y install openssl
yum -y install openssl-devel

NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
SELINUX_STATUS=`getenforce`
APP_ROOT="/hpc_apps"

rm -rf /root/user_secrets.txt
touch /root/user_secrets.txt
echo -e "VERY IMPORTANT: PLEASE DO NOT DISCLOSE THIS FILE FOR ANY PURPOSE!\n PASSWORDS of ALL USERS. YOU CAN USE THESE PASSWORDS FOR RDP CONNECTIONS WITH IN YOUR CLUSTER." >> /root/user_secrets.txt
echo -e "source /etc/profile" >> /root/.bashrc

########## root ssh-passwd-free among nodes ############

if [ -f /root/hostfile ]; then
  if [ ! -d /hpc_data/root_data ]; then
    mkdir -p /hpc_data/root_data
  fi
  chmod -R 750 /hpc_data/root_data
fi

########## Spread .ssh keys ###################
echo -e "# $time_current Spawning ssh keys." >> ${logfile}
if [ -f /root/hostfile ]; then 
  cat /hpc_apps/compute_nodes_ip.txt | grep compute >> /root/hostfile
  rm -rf /hpc_apps/compute_nodes_ip.txt
  number_of_nodes=`cat /root/hostfile | grep compute | wc -l`
  sed -i '/NODE_NUM/d' /etc/profile
  echo -e "export NODE_NUM=$number_of_nodes" >> /etc/profile
  source /etc/profile
fi
echo -e "# $time_current SSH Keys spreaded." >> ${logfile}

############ Add Users ####################
for i in $( seq 1 $1 )
do
  id user${i}
  if [ $? -eq 1 ]; then
    useradd user${i}
    if [ ! -d /home/user${i} ]; then
      mkdir -p /home/user${i} && chown -R user${i}:user${i} /home/user${i}
    fi
    
    if [ -f /root/hostfile ]; then
      if [ $CENTOS_V -eq 7 ]; then
        openssl rand 8 -base64 -out /root/secret_user${i}.txt
      else
        openssl rand -base64 -out /root/secret_user${i}.txt 8
      fi
      cat /root/secret_user${i}.txt | passwd user${i} --stdin > /dev/null 2>&1
      echo -n "username: user${i}    " >> /root/user_secrets.txt
      cat /root/secret_user${i}.txt >> /root/user_secrets.txt
      if [ ! -d /home/user${i}/.ssh ]; then
        mkdir -p /home/user${i}/.ssh
      fi
# To make sure ssh password-free login for useri
      rm -rf /home/user${i}/.ssh/id_rsa.pub
      rm -rf /home/user${i}/.ssh/id_rsa
      rm -rf /home/user${i}/.ssh/authorized_keys
      rm -rf /home/user${i}/.ssh/known_hosts
      ssh-keygen -t rsa -N '' -f /home/user${i}/.ssh/id_rsa -q
      cat /home/user${i}/.ssh/id_rsa.pub >> /home/user${i}/.ssh/authorized_keys
      cat /etc/now-pubkey.txt >> /home/user${i}/.ssh/authorized_keys
      chown -R user${i}:user${i} /home/user${i}    
      if [ ! -d /hpc_data/user${i}_data ]; then
        mkdir -p /hpc_data/user${i}_data
      fi
      chmod -R 750 /hpc_data/user${i}_data
      chown -R user${i}:user${i} /hpc_data/user${i}_data
    fi
    echo -e "source /etc/profile" >> /home/user${i}/.bashrc
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "# $time_current user${i} added, password set. Please check /root/.user_secrets.txt for secrets. SSH-password free has been set." >> ${logfile}  
  fi
  echo -e "user1 ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
done

########## stop firewall and SELinux ###############
systemctl stop firewalld
systemctl disable firewalld
if [ $SELINUX_STATUS != Disabled ]; then
  sed -i 's/SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current SELINUX Disabled." >> ${logfile}

######### Yum some packages ############
# The update step really takes time, trying to avoid it.
if [ $CLOUD_FLAG != 'CLOUD_C' ]; then
  yum -y update
fi
yum -y install epel-release #epel release is really slow for China region
yum -y install gtk2 gtk2-devel
yum -y install python python3
yum -y install gcc-c++ gcc-gfortran
yum -y install htop 
yum -y install sshpass
#yum -y install munge munge-devel
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Utils installed." >> ${logfile}

########## Build munge #################
yum -y install rpm-build 
yum -y install bzip2-devel
yum -y install zlib-devel m4 libxml2-devel
cd /root
if ! command -v munge >/dev/null 2>&1; then
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "# $time_current Start building munge." >> ${logfile}
  if [ ! -f munge-0.5.14* ]; then
    wget ${URL_UTILS}munge/dun.gpg
    wget ${URL_UTILS}munge/munge-0.5.14.tar.xz
    wget ${URL_UTILS}munge/munge-0.5.14.tar.xz.asc
  fi
  rpmbuild -tb munge-0.5.14.tar.xz
  cd /rpmbuild/RPMS/x86_64 && rpm -ivh munge*
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`  
echo -e "# $time_current Munge installed." >> ${logfile}

########## Add user slurm ################
id -u slurm
if [ $? -eq 1 ]; then
  useradd slurm
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current User slurm added." >> ${logfile}

############## Re-Install mariadb Be careful! ########################
if [ -f /root/hostfile ]; then
  yum remove -y `rpm -aq mariadb*`
  rm -rf /etc/my.cnf
  rm -rf /var/lib/mysql
  if [ $CENTOS_V -eq 7 ]; then
    yum -y install mariadb mariadb-devel mariadb-server
    yum -y install mariadb-libs
  else
    yum -y install mariadb-*
  fi
  if [ -f /root/mariadb_private_ip.txt ]; then
    time_current=`date "+%Y-%m-%d %H:%M:%S"
    mv /root/mariadb_root_passwd.txt /root/.cluster_secrets/
    mv /root/mariadb_slurm_acct_db_pw.txt /root/.cluster_secrets/`
    db_address=`cat /root/mariadb_private_ip.txt | awk -F"\t" '{print $1}'`
    echo -e "# $time_current Mariadb has been installed to the host $db_address. Will not build mariadb on localhost." >> ${logfile}
  else
    if [ $2 != db ]; then
      time_current=`date "+%Y-%m-%d %H:%M:%S"`
      echo -e "# $time_current IMPORTANT: No dedicated MariaDB Server found. Automatically install Mariadb Server on localhost." >> ${logfile}
    fi
    systemctl start mariadb.service
    db_address="LOCALHOST"
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "# $time_current Mariadb installation on localhost started." >> ${logfile}
    
    if [ $CENTOS_V -eq 7 ]; then
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
########### Move sensative file to .cluster_secrets folder #############

mv /root/secret_user* /root/.cluster_secrets/
mv /root/user_secret* /root/.cluster_secrets/
mv /root/master_passwd.txt /root/.cluster_secrets/
mv /root/compute_passwd.txt /root/.cluster_secrets/

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "ALL the secrets are stored in the directory /root/.cluster_secrets/ ."
echo -e "# $time_current ALL the secrets are stored in the directory /root/.cluster_secrets/ ." >> ${logfile}

########### Change owners of some directories ################
if [ ! -d /run/munge ]; then
  mkdir /run/munge
fi
chown -R slurm:slurm /run/munge
chown -R slurm:slurm /etc/munge
chown -R slurm:slurm /var/run/munge
chown -R slurm:slurm /var/lib/munge
chown -R slurm:slurm /var/log/munge

########## munge #################
if [ -f /root/hostfile ]; then
  mungekey
  chown -R slurm:slurm /etc/munge/munge.key
fi

########### Build SLURM #####################
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Started building Slurm 21.08.8." >> ${logfile}
cd /root
if [ ! -f slurm-21.08.8-2.tar.bz2 ]; then
  wget ${URL_UTILS}slurm/slurm-21.08.8-2.tar.bz2
fi
tar xf slurm-21.08.8-2.tar.bz2
cd slurm-21.08.8-2
./configure --prefix=/opt/slurm --sysconfdir=/opt/slurm/etc
make -j$NUM_PROCESSORS && make install
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

if [ ! -d /opt/slurm/etc ]; then
  mkdir -p /opt/slurm/etc/
fi

if [ -f /root/hostfile ]; then
  if [ ! -f /opt/slurm/etc/slurm.conf.128 ]; then
    wget ${URL_UTILS}slurm/slurm.conf.128 -O /opt/slurm/etc/slurm.conf.128
  fi
  if [ ! -f /opt/slurm/etc/slurmdbd.conf ]; then
    wget ${URL_UTILS}slurm/slurmdbd.conf -O /opt/slurm/etc/slurmdbd.conf
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
  if [ ! -d /var/spool/slurmctld ]; then
    mkdir -p /var/spool/slurmctld
  fi
  chown -R slurm:slurm /var/spool/slurmctld
  systemctl start slurmdbd
  systemctl enable slurmdbd
  if [ -f /etc/munge/munge.key ]; then
    chown -R slurm:slurm /etc/munge/munge.key
  fi

  if [ ! -d /opt/slurm/archive ]; then
    mkdir /opt/slurm/archive
  fi

  if [ ! -f /etc/hosts-clean ]; then
    cp /etc/hosts /etc/hosts-clean
  fi
  if [ $CENTOS_V -eq 7 ]; then
    wget ${URL_UTILS}hpcmgr-gcc4 -O /usr/bin/hpcmgr
  else
    wget ${URL_UTILS}hpcmgr-gcc11 -O /usr/bin/hpcmgr
  fi
  chmod +x /usr/bin/hpcmgr
  
  yum -y install git python-devel
  if [ $CLOUD_FLAG = 'CLOUD_A' ]; then
    wget ${URL_UTILS}ossutil64 -O /usr/bin/ossutil64 && chmod 755 /usr/bin/ossutil64
  elif [ $CLOUD_FLAG = 'CLOUD_B' ]; then
    pip install coscmd
  elif [ $CLOUD_FLAG = 'CLOUD_C' ]; then 
    yum -y install s3cmd
  fi
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Slurm built and configured in path /opt/slurm." >> ${logfile}

############## install environment-module ######################
cd /root
if ! command -v module >/dev/null 2>&1; then
  yum install tcl-devel -y
  if [ ! -f modules-5.1.0.tar.gz ]; then
    wget ${URL_UTILS}modules-5.1.0.tar.gz
  fi
  tar zvxf modules-5.1.0.tar.gz
  cd modules-5.1.0
  mkdir -p /etc/modulefiles
  ./configure --prefix=/opt/environment-modules --modulefilesdir=/etc/modulefiles
  make -j$NUM_PROCESSORS && make install
  ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.sh
  ln -s /opt/environment-modules/init/profile.sh /etc/profile.d/modules.csh
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`  
echo -e "# $time_current Environment Module has been installed." >> ${logfile}

######### Install Desktop Env-NECESSARY- ##############
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Started installing Desktop Environment." >> ${logfile}
if [ $CENTOS_V -eq 7 ]; then
  yum -y groupinstall "GNOME Desktop"
  if [ -f /root/hostfile ]; then
    wget ${URL_UTILS}libstdc++.so.6.0.26 -O /usr/lib64/libstdc++.so.6.0.26
    rm -rf /usr/lib64/libstdc++.so.6
    ln -s /usr/lib64/libstdc++.so.6.0.26 /usr/lib64/libstdc++.so.6
  fi
  systemctl disable firewalld
  systemctl stop firewalld
  echo -e "#! /bin/bash\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true" > /etc/g_ini.sh
  chmod +x /etc/g_ini.sh
  sed -i '/gini/d' /etc/profile
  echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
else
  echo -e "# $time_current CENTOS VERSION $CENTOS_V. Installing GUI now." >> ${logfile}
  yum grouplist installed -q | grep "Server with GUI" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    yum -y groupinstall "Server with GUI"
  fi
  systemctl enable gdm --now
  systemctl disable firewalld
  systemctl stop firewalld
fi
systemctl set-default graphical.target
yum -y install tigervnc tigervnc-server xrdp
sed -i 's/; (1 = ExtendedDesktopSize)/ (1 = ExtendedDesktopSize)/g' /etc/xrdp/xrdp.ini
sed -i 's/#xserverbpp=24/xserverbpp=24/g' /etc/xrdp/xrdp.ini
systemctl start xrdp
systemctl enable xrdp

if [ -f /root/hostfile ]; then
  yum -y install remmina
  yum -y install rpcbind flex
  yum -y install GConf2
  yum -y install cmake
  yum -y install ibus
  yum -y install libXScrnSaver
  yum -y install ibus-pinyin
  if [ $CLOUD_FLAG = 'CLOUD_B' ]; then
    wget https://cos5.cloud.tencent.com/cosbrowser/cosbrowser-latest-linux.zip -O /opt/cosbrowser.zip
    cd /opt && unzip -o cosbrowser.zip && rm -rf cosbrowser.zip
    cat /etc/profile | grep cosbrowser
    if [ $? -ne 0 ]; then
      echo -e "alias cos='/opt/cosbrowser.AppImage --no-sandbox'" >> /etc/profile
    fi
  elif [ $CLOUD_FLAG = 'CLOUD_A' ]; then
    wget ${URL_UTILS}oss-browser-linux-x64.zip -O /opt/oss.zip
    cd /opt && unzip -o oss.zip && rm -rf oss.zip 
    cat /etc/profile | grep ossbrowser
    if [ $? -ne 0 ]; then
      echo -e "alias oss='/opt/oss-browser-linux-x64/oss-browser'" >> /etc/profile
    fi
  fi
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Desktop Environment and RDP has been installed." >> ${logfile}

####################### Download scripts & Desktop shortcuts ##########################
if [ ! -d /root/Desktop ]; then
  mkdir -p /root/Desktop
fi
ln -s /hpc_apps /root/Desktop/
ln -s /hpc_data/root_data /root/Desktop/
wget ${URL_UTILS}pics/app.png -O /opt/app.png
wget ${URL_UTILS}pics/logo.png -O /opt/logo.png

if [ -f /root/hostfile ]; then
  if [ $CENTOS_V -eq 7 ]; then
    wget ${URL_UTILS}shortcuts/baidu.desktop -O /root/Desktop/baidu.desktop
  fi
  if [ $CLOUD_FLAG = 'CLOUD_A' ]; then
    wget ${URL_UTILS}shortcuts/oss-.desktop -O /root/Desktop/oss-.desktop
  elif [ $CLOUD_FLAG = 'CLOUD_B' ]; then
    wget ${URL_UTILS}shortcuts/cos.desktop -O /root/Desktop/cos.desktop
  fi
fi

for i in $( seq 1 $1 )
do
  if [ ! -d /home/user${i}/Desktop ]; then
    mkdir -p /home/user${i}/Desktop
  fi
  ln -s /hpc_apps /home/user${i}/Desktop/
  ln -s /hpc_data/user${i}_data /home/user${i}/Desktop/
  cp /root/Desktop/*.desktop /home/user${i}/Desktop
  if [ $CLOUD_FLAG = 'CLOUD_A' ]; then
    cp /root/.ossutilconfig /home/user${i}/
    chown -R user${i}:user${i} /home/user${i}/.ossutilconfig
  elif [ $CLOUD_FLAG = 'CLOUD_B' ]; then
    cp /root/.cos.conf /home/user${i}/
    chown -R user${i}:user${i} /home/user${i}/.cos.conf
  elif [ $CLOUD_FLAG = 'CLOUD_C' ]; then
    cp /root/.s3cfg /home/user${i}/
    chown -R user${i}:user${i} /home/user${i}/.s3cfg
  fi
  chown -R user${i}:user${i} /home/user${i}/Desktop
done

#Set the wallpaper of HPC_NOW
rm -rf /usr/share/backgrounds/*.png
rm -rf /usr/share/backgrounds/*.jpg
wget ${URL_UTILS}pics/wallpapers.zip -O /usr/share/backgrounds/wallpapers.zip
cd /usr/share/backgrounds && unzip wallpapers.zip
if [ $CENTOS_V -ne 7 ]; then
  sed -i 's/#WaylandEnable=false/WaylandEnable=false/g' /etc/gdm/custom.conf
  yum -y install gnome-tweaks gnome-extensions-app.x86_64
  echo -e "#! /bin/bash\ngnome-extensions enable background-logo@fedorahosted.org\ngnome-extensions enable window-list@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable apps-menu@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable desktop-icons@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable launch-new-instance@gnome-shell-extensions.gcampax.github.com\ngnome-extensions enable places-menu@gnome-shell-extensions.gcampax.github.com\ngsettings set org.gnome.desktop.lockdown disable-lock-screen true\ngsettings set org.gnome.desktop.background picture-options centered\ngsettings set org.gnome.desktop.background picture-uri /usr/share/backgrounds/day.jpg" > /etc/g_ini.sh
  chmod +x /etc/g_ini.sh
  echo -e "alias gini='/etc/g_ini.sh'" >> /etc/profile
fi

if [ ! -f /hpc_data/sbatch_sample.sh ]; then
  wget ${URL_UTILS}slurm/sbatch_sample.sh -O /hpc_data/sbatch_sample.sh
fi

# Tencent Cloud exposes sensitive information in /dev/sr0. The block device must be deleted.
if [ $CLOUD_FLAG = 'CLOUD_B' ]; then
  echo 1 > /sys/block/sr0/device/delete
fi

#############################  
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Necassary scripts has been downloaded to /root." >> ${logfile}
rm -rf /root/slurm*
rm -rf /root/munge*
rm -rf /root/modules*
rm -rf /root/dun.gpg
rm -rf /rpmbuild

if [[ $3 = 'mpi' && -f /root/hostfile ]]; then
 ########################## build openmpi-4.1.2 ################################
  time_current=`date "+%Y-%m-%d %H:%M:%S"`
  echo -e "# $time_current Started building OpenMPI-4.1.2." >> ${logfile}
  if [ ! -f /hpc_apps/ompi-4.1.2/bin/mpirun ]; then
    rm -rf /hpc_apps/ompi-4.1.2/*
    cd /root
    if [ ! -f openmpi-4.1.2.tar.gz ]; then
      wget ${URL_PKGS}openmpi-4.1.2.tar.gz
    fi
    tar zvxf openmpi-4.1.2.tar.gz
    cd openmpi-4.1.2
    ./configure --prefix=/hpc_apps/ompi-4.1.2 --enable-mpi-cxx
    make -j$NUM_PROCESSORS && make install
    if [ $? -eq 0 ]; then
      cat /etc/profile | grep OMPI_ALLOW_RUN_AS_ROOT
      if [ $? -eq 1 ]; then
        export OMPI_ALLOW_RUN_AS_ROOT=1
        export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
        echo -e "export OMPI_ALLOW_RUN_AS_ROOT=1\nexport OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1\n" >> /etc/profile
      fi
      echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/ompi-4.1.2/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/ompi-4.1.2/lib\n" > /etc/modulefiles/ompi-4.1.2
      time_current=`date "+%Y-%m-%d %H:%M:%S"`
      echo -e "# $time_current OpenMPI-4.1.2 has been built." >> ${logfile}
    else
      time_current=`date "+%Y-%m-%d %H:%M:%S"`
      echo -e "# $time_current Error: Failed to OpenMPI-4.1.2. Please retry after the cluster initialization."
    fi
  else
    echo -e "#%Module1.0\nprepend-path PATH /hpc_apps/ompi-4.1.2/bin\nprepend-path LD_LIBRARY_PATH /hpc_apps/ompi-4.1.2/lib\n" > /etc/modulefiles/ompi-4.1.2
    time_current=`date "+%Y-%m-%d %H:%M:%S"`
    echo -e "# $time_current OpenMPI-4.1.2 has been built." >> ${logfile}
  fi
fi

# Clean up
echo -e "Cleaning Up ..."
rm -rf /root/openmpi*
echo -e "Installation Finished."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Extra packages have been removed." >> ${logfile}