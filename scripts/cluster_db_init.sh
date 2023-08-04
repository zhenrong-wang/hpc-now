# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is for database initialization.

CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
echo -e "export CENTOS_V=$CENTOS_V" >> /etc/profile
logfile=/tmp/cluster_db_init.log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current Mariadb installation started." >> ${logfile} 2>&1
root_passwd_temp=`cat /root/mariadb_root_passwd.txt`
slurm_acct_db_pwd=`cat /root/mariadb_slurm_acct_db_pw.txt`
echo -e "Reinstalling MariadbNow ...\n" >> ${logfile} 2>&1
yum remove -y `rpm -aq mariadb*` >> ${logfile} 2>&1
rm -rf /etc/my.cnf >> ${logfile} 2>&1
rm -rf /var/lib/mysql/* >> ${logfile} 2>&1
if [ $CENTOS_V -eq 7 ]; then
  yum -y install mariadb mariadb-devel mariadb-server >> ${logfile} 2>&1
  yum -y install mariadb-libs >> ${logfile} 2>&1
else
  yum -y install mariadb-* >> ${logfile} 2>&1
fi
yum -y install expect >> ${logfile} 2>&1
systemctl start mariadb.service >> ${logfile} 2>&1
systemctl enable mariadb.service >> ${logfile} 2>&1
root_passwd=`echo -e "$root_passwd_temp" | md5sum | awk '{print $1}' | cut -c 1-16`
echo -e "#!/usr/bin/expect\nset passwd $root_passwd\nspawn mysql_secure_installation" > dbconfig.sh
echo -e "expect {\n\t\t\"Enter current password\" { send \"\\\r\"; exp_continue }\n\t\t\"Y/n\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"New password\" { send \"\$passwd\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"Re-enter new password\" { send \"\$passwd\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"Remove anonymous users\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"Disallow root login remotely\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"Remove test database and access to it\" { send \"Y\\\r\"; exp_continue }" >> dbconfig.sh
echo -e "\t\t\"Reload privilege tables now\" { send \"Y\\\r\" }\n}" >> dbconfig.sh
chmod +x dbconfig.sh
./dbconfig.sh >> ${logfile} 2>&1
#rm -rf dbconfig.sh
mysql -hlocalhost -uroot -p$root_passwd -e"create database IF NOT EXISTS slurm_acct_db"
hash=`mysql -hlocalhost -uroot -p$root_passwd -e"select password('$slurm_acct_db_pwd')" | tail -1`
mysql -hlocalhost -uroot -p$root_passwd -e"GRANT ALL PRIVILEGES ON slurm_acct_db.* TO 'slurm'@'%' IDENTIFIED BY PASSWORD '$hash'"
mysql -hlocalhost -uroot -p$root_passwd -e"flush privileges"
systemctl restart mariadb >> ${logfile} 2>&1
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "# $time_current MariaDB reinstalled." >> ${logfile} 2>&1