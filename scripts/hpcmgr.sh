# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command line tool.

function help_info() {
  echo -e "[ -INFO- ] Valid options/commands:"
  echo -e "           quick   - quick config"
  echo -e "           master  - refresh only master node"
  echo -e "           connect - check cluster connectivity"
  echo -e "           all     - refresh the whole cluster"
  echo -e "           clear   - clear the hostfile_dead_nodes list"
  echo -e "           applist - List out the apps in the store"
  echo -e "           build   - build software from source"
  echo -e "           install - install software"
  echo -e "           remove  - remove software"
  echo -e "           submit  - submit a job"
}

function node_invalid_info() {
  echo -e "[ FATAL: ] It seems you are *NOT* working on the master node of a NOW Cluster."
  echo -e "           In this case, *ONLY* 'hpcmgr install' command is valid."
  echo -e "           Exit now."
}

function add_a_user() {
  mkdir -p /home/$1/.ssh
  rm -rf /home/$1/.ssh/*
  ssh-keygen -t rsa -N '' -f /home/$1/.ssh/id_rsa -q
  cat /home/$1/.ssh/id_rsa.pub >> /home/$1/.ssh/authorized_keys
  cat /etc/now-pubkey.txt >> /home/$1/.ssh/authorized_keys
  rm -rf /home/$1/.ssh/id_rsa.pub
  mkdir -p /home/$1/Desktop
  cp -r /root/Desktop/*.desktop /home/$1/Desktop/
  mkdir -p /hpc_data/${1}_data
  chmod -R 750 /hpc_data/${1}_data
  mkdir -p /hpc_apps/${1}_apps
  touch /hpc_apps/${1}_apps/.private_apps.reg
  chmod -R 750 /hpc_apps/${1}_apps
  mkdir -p /hpc_apps/envmod/${1}_env
  chmod -R 750 /hpc_apps/envmod/${1}_env
  chown -R $1:$1 /hpc_data/${1}_data
  chown -R $1:$1 /hpc_apps/${1}_apps
  chown -R $1:$1 /hpc_apps/envmod/${1}_env
  ln -s /hpc_data/${1}_data /home/$1/Desktop/ >> /dev/null 2>&1
  ln -s /hpc_apps/${1}_apps /home/$1/Desktop/ >> /dev/null 2>&1
  chown -R $1:$1 /home/$1
  for i in $(seq 1 $NODE_NUM )
  do
    ping -c 1 -W 1 -q compute${i} >> ${logfile} 2>&1
    if [ $? -eq 0 ]; then
      ssh -n compute${i} "useradd $1 -m"
      ssh -n compute${i} "rm -rf /home/$1/.ssh"
      scp -r -q /home/$1/.ssh root@compute${i}:/home/$1/
      ssh -n compute${i} "chown -R $1:$1 /home/$1"
    fi
  done
}

if [ -z "$1" ]; then
  help_info
  exit 3
fi

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
app_tmp_log_root="/tmp/app_tmp_logs/"
#if [ $current_user != 'root' ]; then
private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
#fi

main_menu=('quick' 'master' 'connect' 'all' 'clear' 'applist' 'build' 'install' 'remove' 'submit' 'users')
command_flag='false'
for i in $(seq 0 10)
do
  if [ $1 = ${main_menu[i]} ]; then
    command_flag='true'
    break
  fi
done

if [ $command_flag = 'false' ]; then
  echo -e "[ FATAL: ] The command $1 is incorrect. Exit now."
  help_info
  exit 51
fi

if [ $current_user != 'root' ] && [ $1 != 'applist' ] && [ $1 != 'build' ] && [ $1 != 'install' ] && [ $1 != 'remove' ] && [ $1 != 'submit' ]; then
  echo -e "[ FATAL: ] *ONLY* root user can run the command 'hpcmgr $1'. "
  echo -e "           Please make sure you use either user1 with 'sudo' privilege, OR"
  echo -e "           use root (NOT recommend!) to run 'hpcmgr'. Exit now."
  exit 1
fi
#CRITICAL: Environment Variable $NODE_NUM and $NODE_CORES MUST be written to /etc/profile IN ADVANCE!
source /etc/profile
if [ ! -z $APPS_INSTALL_SCRIPTS_URL ]; then
  url_instscripts_root=$APPS_INSTALL_SCRIPTS_URL
fi
rm -rf ~/.ssh/known_hosts
user_registry=/root/.cluster_secrets/user_secrets.txt
if [ $current_user = 'root' ]; then
  logfile="/var/log/hpcmgr.log"
else
  logfile="$HOME/.hpcmgr.log"
fi
time1=$(date)
echo -e "${time1}: HPC-NOW Cluster Manager task started." >> ${logfile}

if [ $1 = 'clear' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 5
  fi
  rm -rf /root/hostfile_dead_nodes && touch /root/hostfile_dead_nodes
  echo -e "[ -INFO- ] The hostfile_dead_nodes list had been emptied. Exit now."
  echo -e "[ -INFO- ] The hostfile_dead_nodes list had been emptied." >> ${logfile}
  exit 0
elif [ $1 = 'users' ]; then
  mkdir -p /root/.sshkey_deleted >> ${logfile} 2>&1
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 5
  fi
  if [ -z "$2" ]; then
    echo -e "Usage: \n\thpcmgr users list\n\thpcmgr users add your_user_name your_password\n\thpcmgr users delete your_user_name\nPlease check your parameters. Exit now.\n"
    exit 3
  fi
  if [[ $2 != 'list' && $2 != 'add' && $2 != 'delete' && $2 != 'rebuild' ]]; then
    echo -e "Usage: \n\thpcmgr users list\n\thpcmgr users add your_user_name your_password\n\thpcmgr users delete your_user_name\nPlease check your parameters. Exit now.\n"
    exit 3
  fi
  if [ $2 = 'list' ]; then
    sacctmgr list user
    exit 0
  fi
  if [ $2 = 'add' ]; then
    if [ -z "$3" ]; then
      echo -e "Usage: \n\thpcmgr users add your_user_name your_password\nPlease check your parameters. Exit now.\n"
      exit 3
    else
      id $3 >> ${logfile} 2>&1
      if [ $? -eq 0 ]; then
        sacctmgr list user $3 | grep hpc_users >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
          echo -e "User $3 already exists in this cluster. Please specify another username. Exit now. \n"
          exit 11
        else
          echo -e "User $3 already exists in this OS but not in this cluster. Adding to the cluster now.\n"
          echo "y" | sacctmgr add user $3 account=hpc_users
          sed -i "/$3/,+0 s/DISABLED/ENABLED/g" ${user_registry}
          mv /root/.sshkey_deleted/id_rsa.$3 /home/$3/.ssh/id_rsa >> ${logfile} 2>&1
          exit 0
        fi
      else
        useradd $3 -m
        if [ -n "$4" ]; then
          echo "$4" | passwd $3 --stdin > /dev/null 2>&1
          echo -e "username: $3 $4 ENABLED" >> ${user_registry}
        else
          echo -e "Generate random string for password." 
          if [ ! -z $CENTOS_VERSION ] && [ $CENTOS_VERSION -eq 7 ]; then
            openssl rand 8 -base64 -out /root/.cluster_secrets/secret_$3.txt
          else
            openssl rand -base64 -out /root/.cluster_secrets/secret_$3.txt 8
          fi
          cat /root/.cluster_secrets/secret_$3.txt | passwd $3 --stdin > /dev/null 2>&1
          echo -n "username: $3 " >> ${user_registry}
          new_passwd=`cat /root/.cluster_secrets/secret_$3.txt`
          echo -e "$new_passwd ENABLED" >> ${user_registry}
          rm -rf /root/.cluster_secrets/secret_$3.txt
        fi
        add_a_user $3
        #bucket_conf $3
        echo "y" | sacctmgr add user $3 account=hpc_users
        exit 0
      fi
    fi
  fi
  if [ $2 = 'delete' ]; then
    if [ -z "$3" ]; then
      echo -e "Usage: \n\thpcmgr users delete your_user_name\n\thpcmgr users delete your_user_name os\nPlease check your parameters. Exit now."
      exit 3
    fi
    if [[ $3 = 'root' || $3 = 'user1' ]]; then
      echo -e "ROOT USER and User1 cannot be deleted! Exit now."
      exit 13
    fi
    sacctmgr list user $3 | grep hpc_users >> /dev/null 2>&1
    if [ -z "$4" ] || [ $4 != 'os' ]; then
      if [ $? -ne 0 ]; then
        echo -e "$3 is not in the cluster. Nothing deleted, exit now."
        exit 15
      fi
    fi
    if [ $? -eq 0 ]; then
      echo "y" | sacctmgr delete user $3
    else
      echo -e "$3 is not in the cluster. Nothing deleted, exit now."
      exit 15
    fi
    if [[ -z "$4" || $4 != "os" ]]; then
      echo -e "User $3 has been deleted from the cluster, but still in the OS."
      sed -i "/$3/,+0 s/ENABLED/DISABLED/g" ${user_registry}
      mv /home/$3/.ssh/id_rsa /root/.sshkey_deleted/id_rsa.$3 >> ${logfile} 2>&1
      exit 0
    else
      echo -e "[ -WARN- ] User $3 will be erased from the Operating System permenantly!"
      userdel -f -r $3
      mv /hpc_data/${3}_data /hpc_data/${3}_data_deleted_user
      rm -rf /hpc_apps/${3}_apps
      rm -rf /hpc_apps/envmod/${3}_env
      for i in $(seq 1 $NODE_NUM )
      do
        ping -c 1 -W 1 -q compute${i} >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
          ssh -n compute${i} "userdel -f -r $3"
        fi
      done
      sed -i "/$3/d" ${user_registry}
      rm -rf /root/.sshkey_deleted/id_rsa.$3 >> ${logfile} 2>&1
      echo -e "[ -DONE- ] User $3 Deleted permenantly."
      exit 0
    fi 
  fi
  if [ $2 = 'rebuild' ]; then
    if [ ! -f ${user_registry} ]; then
      echo -e "[ FATAL: ] The user registry is absent. Exit now."
      exit 16
    fi
    while read user_row
    do
      user_name=`echo $user_row | awk '{print $2}'`
      user_passwd=`echo $user_row | awk '{print $3}'`
      user_status=`echo $user_row | awk '{print $4}'`
      echo $user_name $user_passwd $user_status
      id $user_name >> /dev/null 2>&1
      if [ $? -ne 0 ]; then
        useradd $user_name -m
        add_a_user $user_name
      fi
      echo "${user_passwd}" | passwd $user_name --stdin > /dev/null 2>&1
      #bucket_conf $user_name
      if [ -z $user_status ] || [ $user_status = 'ENABLED' ]; then
        echo "y" | sacctmgr add user $user_name account=hpc_users
      else
        cp -r /home/$user_name/.ssh/id_rsa /root/.sshkey_deleted/id_rsa.$user_name
        echo "y" | sacctmgr delete user $user_name
      fi
    done < ${user_registry}
  fi
  exit 0
elif [ $1 = 'quick' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 3
  fi
  sacct >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    sacct | grep running  >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      echo -e "[ -WARN- ] There are still running tasks in current cluster. You need to cancel all the tasks first."
      echo -e "[ -WARN- ] Exit now.\n"
      exit 17
    fi
  fi  
  echo -e "[ -INFO- ] Welcome to the quick mode. All services will be restarted now."
  echo -e "[ -INFO- ] Please make sure you've already run the command 'hpcmgr all' "
  echo -e "[ STEP 1 ] Restarting services on the master node ..."
  mkdir -p /run/munge && chown -R slurm:slurm /run/munge && sudo -u slurm munged >> $logfile 2>&1
  systemctl restart slurmdbd >> $logfile 2>&1
  systemctl enable slurmdbd >> $logfile 2>&1
  for i in $(seq 1 2 )
  do
    sleep 1
  done
  systemctl restart slurmctld >> $logfile 2>&1
  systemctl enable slurmctld >> $logfile 2>&1
  systemctl status slurmdbd >> ${logfile}
  systemctl status slurmctld >> ${logfile}
  echo -e "[ STEP 2 ] Restarting services on the compute node(s) ..."
  for i in $( seq 1 $NODE_NUM )
  do
    ping -c 1 -W 1 -q compute${i} >> ${logfile} 2>&1
    if [ $? -ne 0 ]; then
      echo -e "\n Node ${i} is unreachable." >> ${logfile}
      continue
    fi
    ssh compute${i} "mkdir -p /run/munge && chown -R slurm:slurm /run/munge && sudo -u slurm munged" >> $logfile 2>&1 
    ssh compute${i} "systemctl restart slurmd && systemctl enable slurmd" >> $logfile 2>&1
    scontrol update NodeName=compute${i} State=DOWN Reason=hung_completing
    scontrol update NodeName=compute${i} State=RESUME
    echo -e "\nSlurmd Status of Node ${i}:" >> ${logfile}
    ssh compute${i} "systemctl status slurmd" >> ${logfile}
    echo -e "\n" >> ${logfile}
  done
  echo -e "[ -DONE- ] HPC-NOW Cluster Status:\n"
  sinfo -N
  exit 0
elif [ $1 = 'connect' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 3
  fi
  compute_passwd=`cat /root/.cluster_secrets/compute_passwd.txt`
  echo -e "[ STEP 1 ] Checking the network connection now ... "
  if [ ! -f /root/hostfile_dead_nodes ]; then
    touch /root/hostfile_dead_nodes
  fi
  cat /root/hostfile | grep compute > /root/hostfile-tmp
  cd /root
  while read iprow
  do
    private_ip=`echo -e "$iprow" | awk -F"\t" '{print $1}'`
    node_name=`echo -e "$iprow" | awk -F"\t" '{print $2}'`
    echo -e "[ STEP 2 ] Pinging $node_name with private IP $private_ip now .... "
    ping -c 1 -W 1 -q $private_ip >> ${logfile} 2>&1
    if [ $? -ne 0 ]; then
      sed -i "/$private_ip/d" /root/hostfile
      echo -e "[ STEP 2 ] Exclude $node_name from the cluster with private IP $private_ip."
      echo -e "[ STEP 2 ] Exclude $node_name from the cluster with private IP $private_ip because it is not responding.\nThe hostfile has been updated." >> ${logfile}
      cat /root/hostfile_dead_nodes | grep -w $node_name
      if [ $? -ne 0 ]; then
        echo -e "$iprow" >> /root/hostfile_dead_nodes_tmp
      else
        sed -i "/$node_name/d" /root/hostfile_dead_nodes
        echo -e "$iprow" >> /root/hostfile_dead_nodes_tmp
      fi
    else
      echo -e "[ STEP 2 ] $node_name with private IP $private_ip connected."
      sshpass -p $compute_passwd scp -r /root/.ssh root@$private_ip:/root/
      echo -e "[ STEP 2 ] $node_name with private IP $private_ip connected." >> ${logfile}
      cat /root/hostfile_dead_nodes | grep -w $node_name >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        echo -e "[ STEP 2 ] $node_name has been deleted from the hostfile_dead_nodes list."
        echo -e "[ STEP 2 ] $node_name has been deleted from the hostfile_dead_nodes list." >> ${logfile}
        sed -i "/$node_name/d" /root/hostfile_dead_nodes
      fi  
    fi
  done < hostfile-tmp
  rm -rf hostfile-tmp  
  while read iprow_dead_nodes
  do
    private_ip=`echo -e "$iprow_dead_nodes" | awk -F"\t" '{print $1}'`
    node_name=`echo -e "$iprow_dead_nodes" | awk -F"\t" '{print $2}'`
    echo -e "[ STEP 2 ] Pinging $node_name with private IP $private_ip now .... "
    ping -c 1 -W 1 -q $private_ip >> ${logfile} 2>&1
    if [ $? -eq 0 ]; then
      cat /root/hostfile | grep -w $node_name >> /dev/null
      if [ $? -ne 0 ]; then
        echo -e "$iprow_dead_nodes" >> /root/hostfile
        sed -i "/$node_name/d" /root/hostfile_dead_nodes
        sshpass -p $compute_passwd scp -r /root/.ssh root@$private_ip:/root/
      else
        echo -e "[ -WARN- ] Node name conflict occurred, please check the IP address of the node $node_name. The HOSTFILE is unchanged."
      fi
    else
      cat /root/hostfile | grep -w $node_name >> /dev/null
      if [ $? -eq 0 ]; then
        echo -e "[ STEP 2 ] The node $node_name is in the hostfile."
        sed -i "/$node_name/d" /root/hostfile_dead_nodes
      else
        echo -e "[ STEP 2 ] The node $node_name with private IP address $private_ip is still not responing."
      fi
    fi  
  done < /root/hostfile_dead_nodes
  if [ -f /root/hostfile_dead_nodes_tmp ]; then
    cat /root/hostfile_dead_nodes_tmp >> /root/hostfile_dead_nodes && rm -rf /root/hostfile_dead_nodes_tmp
  fi
  echo -e "[ STEP 2 ] Connectivity check finished."
  echo -e "[ STEP 3 ] Updating the cluster environment variables now ... "
  sample_ip=`cat /root/hostfile | grep compute | head -n1 | awk -F"\t" '{print $1}'`
  number_of_cores=`ssh $sample_ip "lscpu | grep CPU\(s\): | head -n1" | awk -F":" '{print $2}' | awk '{print $1}'`
  number_of_nodes=`cat /root/hostfile | grep compute | wc -l`
  threads_per_core=`ssh $sample_ip "lscpu | grep \"Thread(s) per core:\"" | awk -F":" '{print $2}' | awk '{print $1}'`
  cores_per_socket=`ssh $sample_ip "lscpu | grep \"Core(s) per socket:\"" | awk -F":" '{print $2}' | awk '{print $1}'`
  sockets=`ssh $sample_ip "lscpu | grep \"Socket(s)\"" | awk -F":" '{print $2}' | awk '{print $1}'`  
  sed -i '/NODE_NUM/d' /etc/profile
  echo -e "export NODE_NUM=$number_of_nodes" >> /etc/profile
  sed -i '/NODE_CORES/d' /etc/profile
  echo -e "export NODE_CORES=$number_of_cores" >> /etc/profile
  sed -i '/THREADS_PER_CORE/d' /etc/profile
  echo -e "export THREADS_PER_CORE=$threads_per_core" >> /etc/profile
  sed -i '/CORES_PER_SOCKET/d' /etc/profile
  echo -e "export CORES_PER_SOCKET=$cores_per_socket" >> /etc/profile
  sed -i '/SOCKETS/d' /etc/profile
  echo -e "export SOCKETS=$sockets" >> /etc/profile
  echo -e "[ STEP 3 ] Environment Variable NODE_NUM has been updated to $number_of_nodes\n[ STEP 3 ] Environment Variable NODE_CORES has been updated to $number_of_cores"
  echo -e "[ STEP 3 ] Environment Variable NODE_NUM has been updated to $number_of_nodes\n[ STEP 3 ] Environment Variable NODE_CORES has been updated to $number_of_cores" >> ${logfile}
  source /etc/profile
  echo -e "[ STEP 4 ] Updating the cluster configuration now ..."
  /bin/cp /etc/hosts-clean /etc/hosts
  cat /root/hostfile >> /etc/hosts
  /bin/cp /opt/slurm/etc/slurm.conf.128 /opt/slurm/etc/slurm.conf
  sed -i 's/NodeName=compute\[1-2\]/NodeName=compute\[1-'$NODE_NUM'\]/g' /opt/slurm/etc/slurm.conf
  sed -i 's/CPUs=128/CPUs='$NODE_CORES'/g' /opt/slurm/etc/slurm.conf
  sed -i 's/SCKTS/'$SOCKETS'/g' /opt/slurm/etc/slurm.conf
  sed -i 's/C_P_S/'$CORES_PER_SOCKET'/g' /opt/slurm/etc/slurm.conf
  sed -i 's/T_P_C/'$THREADS_PER_CORE'/g' /opt/slurm/etc/slurm.conf
  echo -e "[ STEP 4 ] The cluster configuration file has been updated.\n" >>  ${logfile}
  echo -e "[ STEP 5 ] Setting up users of compute nodes ... "
  for i in $(seq 1 $NODE_NUM )
  do
    ping -c 1 -W 1 -q compute${i} >> ${logfile} 2>&1
    if [ $? -ne 0 ]; then
      echo -e "[ -WARN- ] Failed to set up users for compute $i ."
      continue
    fi
    scp -r -q /etc/munge/munge.key root@compute${i}:/etc/munge
    while read hpc_user_row
    do
      username=`echo -e $hpc_user_row | awk '{print $2}'`
      ssh -n compute${i} "useradd ${username} -m >> /dev/null 2>&1 && mkdir -p /home/$username >> /dev/null 2>&1"
      scp -r -q /home/$username/.ssh root@compute${i}:/home/$username/
      ssh -n compute${i} "chown -R $username:$username /home/$username >> /dev/null 2>&1"
    done < ${user_registry}
  done
  echo -e "[ STEP 5 ] Users are ready."
  echo -e "[ -DONE- ] Connectivety check finished! \n[ -DONE- ] You need to run the command 'hpcmgr all' on the master node.\n[ -DONE- ] Exit now."
  exit 0
elif [ $1 = 'master' ] || [ $1 = 'all' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 3
  fi
  if [ $((NODE_NUM)) -lt $((1)) ]; then
    echo -e "[ FATAL: ] There is no compute nodes in the current cluster. You need to add at least 1 compute node to start the cluster.\n[ FATAL: ] Exit now.\n"
    exit 21
  fi 
  sacct >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    sacct | grep running  >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      echo -e "[ -WARN- ] There are still running tasks in current cluster. You need to cancel all the tasks first.\n[ -WARN- ] Exit now.\n"
      exit 17
    fi
  fi
  # To make sure the munged is up
  echo -e "[ STEP 1 ] Checking the authentication of the master node ..."
  ps -aux | grep slurm | grep munged >> ${logfile}
  if [ $? -ne 0 ]; then
    mkdir -p /run/munge && chown -R slurm:slurm /run/munge
    mkdir -p /etc/munge && chown -R slurm:slurm /etc/munge
    mkdir -p /var/run/munge && chown -R slurm:slurm /var/run/munge
    mkdir -p /var/lib/munge && chown -R slurm:slurm /var/lib/munge
    mkdir -p /var/log/munge && chown -R slurm:slurm /var/log/munge
    sudo -u slurm munged
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] FAILED to start munge daemon on Master Node! Please check the installation of munge.\nExit now."
      echo -e "[ FATAL: ] FAILED to start munge daemon on Master Node! Please check the installation of munge.\nExit now." >> ${logfile}
      exit 33
    fi
  fi
  echo -e "[ STEP 1 ] Authentication of the master node is OK."
  echo -e "[ STEP 2 ] Checking the authentication of Compute node(s) ..."
  for i in $(seq 1 $NODE_NUM )
  do
    ssh compute${i} "ps -aux | grep slurm | grep munged | cut -c 9-16 | xargs kill -9 >> /dev/null 2>&1"
    ssh compute${i} "mkdir -p /run/munge && chown -R slurm:slurm /run/munge && mkdir -p /etc/munge && chown -R slurm:slurm /etc/munge && mkdir -p /var/run/munge && chown -R slurm:slurm /var/run/munge && mkdir -p /var/lib/munge && chown -R slurm:slurm /var/lib/munge && mkdir -p /var/log/munge && chown -R slurm:slurm /var/log/munge && sudo -u slurm munged" >> $logfile 2>&1  
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] FAILED to start munge daemon on Compute${i} Node! Please check the installation of munge.\nExit now."
      echo -e "[ FATAL: ] FAILED to start munge daemon on Compute${i} Node! Please check the installation of munge.\nExit now." >> ${logfile}
      exit 33
    fi
  done
  echo -e "[ STEP 2 ] Authentication of Compute node(s) is OK."
  echo -e "[ STEP 3 ] Pulling the master node up ..."  
  systemctl restart slurmdbd
  systemctl enable slurmdbd >> ${logfile}
  systemctl status slurmdbd | grep "Active: active" >> ${logfile}
  if [ $? -eq 0 ]; then
    for i in $( seq 1 2)
    do
      sleep 1
    done
    systemctl restart slurmctld
    systemctl enable slurmctld >> ${logfile}
    echo -e "[ STEP 3 ] The master node is up."
  else
    echo -e "[ FATAL: ] SLURMDBD is not properly started. Exit now."
    echo -e "[ FATAL: ] SLURMDBD is not properly started.\n" >> ${logfile}
    exit 33
  fi
  systemctl status slurmctld | grep "Active: failed" >> ${logfile}
  if [ $? -eq 0 ]; then
    echo -e "[ FATAL: ] SLURMCTLD is not properly started. Exit now."
    echo -e "[ FATAL: ] SLURMCTLD is not properly started.\n" >> ${logfile}
    exit 33
  fi
   
  if [ $1 = 'all' ]; then
    echo -e "[ STEP 4 ] Pulling the compute node(s) up ..."
    for i in $( seq 1 $NODE_NUM )
    do
      scp -q /root/hostfile root@compute${i}:/etc/
      scp -q /opt/slurm/etc/slurm.conf root@compute${i}:/opt/slurm/etc/
      ssh compute${i} "sed -i '/master/d' /etc/hosts && sed -i '/compute/d' /etc/hosts"
      ssh compute${i} "cat /etc/hostfile >> /etc/hosts && rm -rf /etc/hostfile && systemctl restart slurmd && systemctl enable slurmd" >> ${logfile}
      echo -e "\nSlurmd Status of Node ${i}:" >> ${logfile}
      ssh compute${i} "systemctl status slurmd" >> ${logfile}
      echo -e "\n" >> ${logfile}
      scontrol update NodeName=compute${i} State=DOWN Reason=hung_completing
      scontrol update NodeName=compute${i} State=RESUME
    done
    echo -e "[ STEP 4 ] Compute nodes are ready."
    sinfo -N >> ${logfile}
  fi
  echo -e "[ STEP 5 ] Checking cluster users ... "
  sacctmgr list account hpc_users | grep -w hpc_users >> ${logfile} 2>&1
  if [ $? -ne 0 ]; then
    echo "y" | sacctmgr add account hpc_users >> ${logfile}
  fi
  while read hpc_user_row
  do
    user_name=`echo -e $hpc_user_row | awk '{print $2}'`
    user_status=`echo -e $hpc_user_row | awk '{print $4}'`
    if [ ! -z $user_status ] && [ $user_status = "DISABLED" ]; then
      echo "y" | sacctmgr delete user $user_name >> ${logfile} 2>&1
      mv /home/$user_name/.ssh/id_rsa /root/.sshkey_deleted/id_rsa.$user_name >> ${logfile} 2>&1
    else
      if [ $user_name = 'user1' ]; then
        echo "y" | sacctmgr add user ${user_name} account=hpc_users adminlevel=admin >> ${logfile} 2>&1
      else
        echo "y" | sacctmgr add user ${user_name} account=hpc_users >> ${logfile} 2>&1
      fi
      mv /root/.sshkey_deleted/id_rsa.$user_name /home/$user_name/.ssh/id_rsa >> ${logfile} 2>&1
    fi
  done < ${user_registry}
  echo -e "[ STEP 5 ] Cluster users are ready."
  echo -e "[ -DONE- ] HPC-NOW Cluster Status:\n"
  sinfo -N
  exit 0
elif [ $1 = 'applist' ]; then
  if [ -z $url_instscripts_root ]; then
    echo -e "[ FATAL: ] Failed to connect to a valid appstore repo. Exit now."
    exit 34
  fi
  if [ -z $2 ]; then
    echo -e "1. Applications:"
    echo -e "\tof7       - OpenFOAM-v7"
    echo -e "\tof9       - OpenFOAM-v9"
    echo -e "\tof1912    - OpenFOAM-v1912"
    echo -e "\tof2112    - OpenFOAM-v2112"
    echo -e "\tlammps    - LAMMPS dev latest"
    echo -e "\tgromacs   - GROMACS"
    echo -e "\twrf       - WRF & WPS -4.4"
    echo -e "\tvasp5     - VASP-5.4.4 (BRING YOUR OWN SOURCE AND LICENSE)"
    echo -e "\tvasp6.1   - VASP-6.1.0 (BRING YOUR OWN SOURCE AND LICENSE)"
    echo -e "\tvasp6.3   - VASP-6.1.0 (BRING YOUR OWN SOURCE AND LICENSE)"
    echo -e "\tR         - R & RStudio (in development)"
    echo -e "\tparaview  - ParaView-5"
    echo -e "\thdf5      - HDF5-1.10.9"
    echo -e "\tnetcdf4   - netCDF-C-4.9.0 & netCDF-fortran-4.5.3"
    echo -e "\tabinit    - ABINIT-9.6.2"
    echo -e "2. MPI Toolkits:"
    echo -e "\tmpich3    - MPICH-3.2.1"
    echo -e "\tmpich4    - MPICH-4.0.2"
    echo -e "\tompi3     - OpenMPI-3.1.6"
    echo -e "\tompi4     - OpenMPI-4.1.2"
    echo -e "3. Compilers:"
    echo -e "\tgcc4      - GNU Compiler Collections - 4.9.2"
    echo -e "\tgcc8      - GNU Compiler Collections - 8.2.0"
    echo -e "\tgcc9      - GNU Compiler Collections - 9.5.0."
    echo -e "\tgcc12     - GNU Compiler Collections - 12.1.0"
    echo -e "\tintel     - Intel(R) HPC Toolkit Latest"
    echo -e "4. Important Libraries:"
    echo -e "\tfftw3     - FFTW 3"
    echo -e "\tlapack311 - LAPACK-3.11.0"
    echo -e "\tzlib      - zlib-1.2.13"
    echo -e "\tslpack2   - ScaLAPACK-2.1.0"
    echo -e "\topenblas  - OpenBLAS 0.3.15 *SINGLE THREAD*"
    echo -e "5. Other Tools:"
    echo -e "\tdesktop   - Desktop Environment" 
    echo -e "\tbaidu     - Baidu Netdisk"
    echo -e "\tcos       - COSBrowser (RECOMMENDED)" 
    echo -e "\trar       - RAR for Linux (RECOMMENDED)" 
    echo -e "\tkswps     - WPS Office Suite for Linux (RECOMMENDED)" 
    echo -e "\tenvmod    - Environment Modules" 
    echo -e "\tvscode    - Visual Studio Code"
    exit 0
  elif [ $2 = 'avail' ]; then
    echo -e "|       +- Available(Installed) Apps ~ Public:"
    while read public_reg_row
    do
      echo -e "|          ${public_reg_row}"
    done < $public_app_registry
    echo -e "|       +- Available(Installed) Apps ~ Private:"
    if [ $current_user = 'root' ]; then
      while read user_row
      do
        user_name_tmp=`echo $user_row | awk '{print $2}'`
        while read private_reg_row
        do
          echo -e "|          ${private_reg_row}"
        done < /hpc_apps/${user_name_tmp}_apps/.private_apps.reg
      done < /root/.cluster_secrets/user_secrets.txt
    else
      while read private_reg_row
      do
        echo -e "|          ${private_reg_row}"
      done < /hpc_apps/${current_user}_apps/.private_apps.reg
    fi
    exit 0
  elif [ $2 = 'check' ]; then
    if [ -z $3 ]; then
      echo -e "[ FATAL: ] Please provide an app name to check."
      exit 35
    else
      grep "< $3 >" $public_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        echo -e "[ -INFO- ] The app $3 is available for all users."
        exit 0
      else
        if [ $current_user = 'root' ]; then
          while read user_row
          do
            user_name_tmp=`echo $user_row | awk '{print $2}'`
            grep "< $3 >" /hpc_apps/${user_name_tmp}_apps/.private_apps.reg
            if [ $? -eq 0 ]; then
              echo -e "[ -INFO- ] The app $3 is available for ${user_name_tmp}."
              exit 0
            fi
          done < /root/.cluster_secrets/user_secrets.txt
          echo -e "[ -INFO- ] The app $3 is not available for any users."
          exit 0
        else
          grep "< $3 >" ${private_app_registry}
          if [ $? -eq 0 ]; then
            echo -e "[ -INFO- ] The app $3 is available for the current user ${current_user}."
          else
            echo -e "[ -INFO- ] The app $3 is not available for the current user ${current_user}"
          fi
          exit 0
        fi
      fi
    fi
  else
    echo -e "[ FATAL: ] Invalid applist sub-commands. Valid commands: avail, check."
    exit 36
  fi
elif [ $1 = 'install' ] || [ $1 = 'remove' ] || [ $1 = 'build' ]; then
  if [ -z "$2" ]; then
    echo -e "[ -INFO- ] Please specify an app to $1 ."
    exit 37
  fi
  if [ $1 = 'install' ] || [ $1 = 'build' ]; then
    curl -s ${url_instscripts_root}_app_list.txt | grep "< ${2} >" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] The software ${2} is not in the store. Exit now."
      exit 39
    fi
    grep "< $2 >" $public_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      echo -e "[ -INFO- ] This app has been installed to all users. Please run it directly."
      exit 0
    else
      grep "< $2 > < ${current_user} >" $private_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        echo -e "[ -INFO- ] This app has been installed to the current user. Please run it directly."
        exit 0
      fi
    fi
  else
    if [ $current_user = 'root' ]; then
      grep "< $2 >" $public_app_registry >> /dev/null 2>&1
      if [ $? -ne 0 ]; then
        echo -e "[ -INFO- ] This app has not been installed to all users."
        exit 4
      fi
    else
      grep "< $2 > < ${current_user} >" $private_app_registry >> /dev/null 2>&1
      if [ $? -ne 0 ]; then
        echo -e "[ -INFO- ] This app has not been installed to the current user."
        exit 4
      fi
    fi
  fi
  app_tmp_log="${app_tmp_log_root}${current_user}_${1}_${2}.log"
  touch ${app_tmp_log} && chmod 644 ${app_tmp_log}
  curl -s ${url_instscripts_root}${2}.sh | bash -s $1 ${app_tmp_log}
  exit 0
elif [ $1 = 'submit' ]; then
  if [ -z $2 ]; then
    job_info_tmp="/tmp/job_submit_info_${current_user}.tmp"
  else
    job_info_tmp=$2
  fi
  if [ ! -f ${job_info_tmp} ]; then
    echo -e "[ FATAL: ] Job submit info file $2 is absent. Exit now."
    exit 51
  fi
  dos2unix ${job_info_tmp}
  app_name=`grep "App Name" ${job_info_tmp} | awk -F"::" '{print $2}'`
  job_nodes=`grep "Job Nodes" ${job_info_tmp} | awk -F"::" '{print $2}'`
  cores_per_node=`grep "Cores Per Node" ${job_info_tmp} | awk -F"::" '{print $2}'`
  total_cores=`grep "Total Cores" ${job_info_tmp} | awk -F"::" '{print $2}'`
  job_name=`grep "Job Name" ${job_info_tmp} | awk -F"::" '{print $2}'`
  duration_hours=`grep "Duration Hours" ${job_info_tmp} | awk -F"::" '{print $2}'`
  job_exec=`grep "Job Executable" ${job_info_tmp} | awk -F"::" '{print $2}'`
  data_directory=`grep "Data Directory" ${job_info_tmp} | awk -F"::" '{print $2}'`
  if [ ! -d ${data_directory} ]; then
    echo -e "[ FATAL: ] Failed to find/open the Data Directory: ${data_directory}. Exit now."
    exit 53
  fi
  echo -e '#!/bin/bash' > ${data_directory}/job_submit.sh
  echo -e "#SBATCH --account=hpc_users\n#SBATCH --cluster=cluster\n#SBATCH --partition=debug" >> ${data_directory}/job_submit.sh
  echo -e "#SBATCH --nodes=${job_nodes}\n#SBATCH --ntasks-per-node=${cores_per_node}" >> ${data_directory}/job_submit.sh
  echo -e "#SBATCH --job-name=${job_name}\n#SBATCH --output=output.%j.${job_name}.out" >> ${data_directory}/job_submit.sh
  echo -e "#SBATCH --time=${duration_hours}:00:00\n" >> ${data_directory}/job_submit.sh
  echo -e "mpirun -np ${total_cores} -bind-to numa ${job_exec} -parallel > ${data_directory}/${job_name}_run.log 2>&1" >> ${data_directory}/job_submit.sh
  cd ${data_directory}
  grep "< ${app_name} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    app_env=`grep ${app_name}.env /etc/profile | awk -F"'" '{print $2}' | awk '{print $2}'`
  else
    app_env=`grep ${app_name}.env ${HOME}/.bashrc | awk -F"'" '{print $2}'| awk '{print $2}'`
  fi
  source ${app_env}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to load the running environment for ${app_name}. Exit now."
    exit 55
  fi
  sbatch job_submit.sh
  if [ $? -eq 0 ]; then
    echo -e "[ -INFO- ] Job submitted successfully. Console output: ${data_directory}/${job_name}_run.log ."
    exit 0
  else
    echo -e "[ -INFO- ] Failed to submit the job."
    exit 55
  fi
else
  echo -e "[ FATAL ] The command $1 is invalid."
  help_info
  exit 3
fi