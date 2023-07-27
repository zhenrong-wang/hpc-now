#!/bin/bash

# This code is written and maintained by Zhenrong WANG
# mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# This code is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

# This script is used when rebuilding the clusters
# $1: backup / restore: backup or restore cluster system and user envrionment variables

current_user=`whoami`
if [ $current_user != 'root' ]; then
    echo -e "[ FATAL: ] Only the root user can execute this script. Exit now."
    exit 1
fi
if [ -z $1 ]; then
    echo -e "[ FATAL: ] Please specify 'backup' or 'restore' as the first parameter. Exit now."
    exit 3
else
    if [ $1 != 'backup' ] && [ $1 != 'restore' ]; then
        echo -e "[ FATAL: ] Please specify 'backup' or 'restore' as the first parameter. Exit now."
        exit 3
    fi
fi

backup_dir="/hpc_apps/.profile_backup/"
backup_dir_trash="/hpc_apps/.profile_backup.trash/"
if [ $1 = 'backup' ]; then
    mkdir -p $backup_dir
    rm -rf ${backup_dir}*
    /bin/cp -r /etc/profile ${backup_dir}
    /bin/cp -r /root/.bashrc ${backup_dir}root_bashrc
    while read user_row
    do
      user_name=`echo $user_row | awk '{print $2}'`
      mkdir -p ${backup_dir}${user_name}/
      /bin/cp -r /home/${user_name}/.bashrc ${backup_dir}${user_name}/
    done < /root/.cluster_secrets/user_secrets.txt
    exit 0
else
    if [ ! -d ${backup_dir} ]; then
        echo -e "[ FATAL: ] Backup directory not found. Have you backed up yet? Exit now."
        exit 5
    fi
    /bin/cp -r ${backup_dir}profile /etc/
    /bin/cp -r ${backup_dir}root_bashrc /root/.bashrc
    while read user_row
    do
      user_name=`echo $user_row | awk '{print $2}'`
      /bin/cp -r ${backup_dir}${user_name}/.bashrc /home/${user_name}/.bashrc
    done < /root/.cluster_secrets/user_secrets.txt
    mkdir -p ${backup_dir_trash}
    mv ${backup_dir} ${backup_dir_trash}
    exit 0
fi