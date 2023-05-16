#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command line tool.

CURRENT_USER=`whoami`
if [ $CURRENT_USER != 'root' ]; then
  echo -e "[ FATAL: ] *ONLY* root user can run the command 'hpcmgr'. "
  echo -e "           Please make sure you use either user1 with 'sudo' privilege, OR"
  echo -e "           use root (NOT recommend!) to run 'hpcmgr'. Exit now."
  exit 1
fi
#CRITICAL: Environment Variable $NODE_NUM and $NODE_CORES MUST be written to /etc/profile IN ADVANCE!
source /etc/profile
if [ ! -z $APPS_INSTALL_SCRIPTS_URL ]; then
  URL_INSTSCRIPTS_ROOT=$APPS_INSTALL_SCRIPTS_URL
fi
rm -rf ~/.ssh/known_hosts
user_registry=/root/.cluster_secrets/user_secrets.txt
logfile='/var/log/hpcmgr.log'
time1=$(date)

echo -e "${time1}: HPC-NOW Cluster Manager task started." >> ${logfile}

function help_info() {
  echo -e "[ FATAL: ] The command parameter is invalid. Valid parameters are:"
  echo -e "           quick   - quick config"
  echo -e "           master  - refresh only master node"
  echo -e "           connect - check cluster connectivity"
  echo -e "           all     - refresh the whole cluster"
  echo -e "           clear   - clear the hostfile_dead_nodes list"
  echo -e "           users   - add, delete cluster users"
  echo -e "           install - install software"
  echo -e "[ FATAL: ] Please double check your input. Exit now."
}

function node_invalid_info() {
  echo -e "[ FATAL: ] It seems you are *NOT* working on the master node of a NOW Cluster."
  echo -e "           In this case, *ONLY* 'hpcmgr install' command is valid."
  echo -e "           Exit now."
}

if [ ! -n "$1" ]; then
  help_info
  exit 3
fi

if [ $1 != 'quick' ] && [ $1 != 'master' ] && [ $1 != 'all' ] && [ $1 != 'clear' ] && [ $1 != 'users' ] && [ $1 != 'connect' ] && [ $1 != 'install' ]; then
  help_info
  exit 3
fi

if [ $1 = 'clear' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 5
  fi
  rm -rf /root/hostfile_dead_nodes && touch /root/hostfile_dead_nodes
  echo -e "[ -INFO- ] The hostfile_dead_nodes list had been emptied. Exit now."
  echo -e "[ -INFO- ] The hostfile_dead_nodes list had been emptied." >> ${logfile}
  exit 0
fi

##### USER MANAGEMENT #####################
if [ $1 = 'users' ]; then
  if [ ! -f /root/hostfile ]; then
    node_invalid_info
    exit 5
  fi
  if [ ! -n "$2" ]; then
    echo -e "Usage: \n\thpcmgr users list\n\thpcmgr users add your_user_name your_password\n\thpcmgr users delete your_user_name\nPlease check your parameters. Exit now.\n"
    exit 3
  fi
  if [[ $2 != 'list' && $2 != 'add' && $2 != 'delete' ]]; then
    echo -e "Usage: \n\thpcmgr users list\n\thpcmgr users add your_user_name your_password\n\thpcmgr users delete your_user_name\nPlease check your parameters. Exit now.\n"
    exit 3
  fi
  if [ $2 = 'list' ]; then
    sacctmgr list user
    exit 0
  fi
  if [ $2 = 'add' ]; then
    if [ ! -n "$3" ]; then
      echo -e "Usage: \n\thpcmgr users add your_user_name your_password\nPlease check your parameters. Exit now.\n"
      exit 3
    else
      id $3
      if [ $? -eq 0 ]; then
        sacctmgr list user $3 >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
          echo -e "User $3 already exists in this cluster. Please specify another username. Exit now. \n"
          exit 11
        else
          echo -e "User $3 already exists in this OS but not in this cluster. Adding to the cluster now.\n"
          echo "y" | sacctmgr add user $3 account=hpc_users
          sed -i "/$3/,+0 s/DISABLED/ENABLED/g" $user_registry
          exit 0
        fi
      else
        useradd $3 -m
        if [ -n "$4" ]; then
          echo "$4" | passwd $3 --stdin > /dev/null 2>&1
          echo -e "username: $3 $4 ENABLED" >> $user_registry
        else
          echo -e "Generate random string for password." 
          if [ $CENTOS_V -eq 7 ]; then
            openssl rand 8 -base64 -out /root/.cluster_secrets/secret_$3.txt
          else
            openssl rand -base64 -out /root/.cluster_secrets/secret_$3.txt 8
          fi
          cat /root/.cluster_secrets/secret_$3.txt | passwd $3 --stdin > /dev/null 2>&1
          echo -n "username: $3 " >> $user_registry
          new_passwd=`cat /root/.cluster_secrets/secret_$3.txt`
          echo -e "$new_passwd ENABLED" >> $user_registry
          rm -rf /root/.cluster_secrets/secret_$3.txt
        fi
        mkdir -p /home/$3/.ssh
        rm -rf /home/$3/.ssh/*
        ssh-keygen -t rsa -N '' -f /home/$3/.ssh/id_rsa -q
        cat /home/$3/.ssh/id_rsa.pub >> /home/$3/.ssh/authorized_keys
        cat /etc/now-pubkey.txt >> /home/$3/.ssh/authorized_keys
        chown -R $3:$3 /home/$3/.ssh
        cp -r /home/user1/Desktop /home/$3/ && unlink /home/$3/Desktop/user1_data
        mkdir -p /hpc_data/$3_data && chmod -R 750 /hpc_data/$3_data && chown -R $3:$3 /hpc_data/$3_data
        ln -s /hpc_data/$3_data /home/$3/Desktop/
        ln -s /hpc_apps /home/$3/Desktop/ >> /dev/null 2>&1
        for i in $(seq 1 $NODE_NUM )
        do
          ping -c 2 -W 1 -q compute${i} >> ${logfile} 2>&1
          if [ $? -eq 0 ]; then
            ssh -n compute${i} "useradd $3 -m"
            #user_passwd=`cat $user_registry | grep $3 | awk '{print $3}'`
            #ssh -n compute${i} "echo '$user_passwd' | passwd $3 --stdin > /dev/null 2>&1"
            ssh -n compute${i} "rm -rf /home/$3/.ssh"
            #ssh -n compute${i} "mkdir -p /home/$3/Desktop && ln -s /hpc_apps /home/$3/Desktop/ && ln -s /hpc_data/$3_data /home/$3/Desktop/"
            scp -r -q /home/$3/.ssh root@compute${i}:/home/$3/
            #scp -q /home/$3/Desktop/*desktop root@compute${i}:/home/$3/Desktop/
            ssh -n compute${i} "chown -R $3:$3 /home/$3"
          fi
        done
        if [ -f /root/.cos.conf ]; then
          cp /root/.cos.conf /home/$3/ && chown -R $3:$3 /home/$3/.cos.conf
        fi
        if [ -f /root/.ossutilconfig ]; then
          cp /root/.ossutilconfig /home/$3/ && chown -R $3:$3 /home/$3/.ossutilconfig
        fi
        if [ -f /root/.s3cfg ]; then
          cp /root/.s3cfg /home/$3/ && chown -R $3:$3 /home/$3/.s3cfg
        fi
        echo "y" | sacctmgr add user $3 account=hpc_users
        exit 0
      fi
    fi
  fi
  if [ $2 = 'delete' ]; then
    if [ ! -n "$3" ]; then
      echo -e "Usage: \n\thpcmgr users delete your_user_name\n\thpcmgr users delete your_user_name os\nPlease check your parameters. Exit now."
      exit 3
    fi
    if [[ $3 = 'root' || $3 = 'user1' ]]; then
      echo -e "ROOT USER and User1 cannot be deleted! Exit now."
      exit 13
    fi
    sacctmgr list user $3 >> /dev/null 2>&1
    if [ ! -n "$4" ] || [ $4 != 'os' ]; then
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
    if [[ ! -n "$4" || $4 != "os" ]]; then
      echo -e "User $3 has been deleted from the cluster, but still in the OS."
      sed -i "/$3/,+0 s/ENABLED/DISABLED/g" $user_registry
      exit 0
    else
      echo -e "[ -WARN- ] User $3 will be erased from the Operating System permenantly!"
      userdel -f -r $3 && mv /hpc_data/${3}_data /hpc_data/${3}_data_deleted_user
      for i in $(seq 1 $NODE_NUM )
      do
        ping -c 2 -W 1 -q compute${i} >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
          ssh -n compute${i} "userdel -f -r $3"
        fi
      done
      sed -i "/$3/d" $user_registry
      echo -e "[ -DONE- ] User $3 Deleted permenantly."
      exit 0
    fi 
  fi
fi

# quick mode - quickly restart all the services in master and compute nodes
if [ $1 = 'quick' ]; then
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
  for i in $(seq 1 2 )
  do
    sleep 1
  done
  systemctl restart slurmctld >> $logfile 2>&1
  systemctl status slurmdbd >> ${logfile}
  systemctl status slurmctld >> ${logfile}
  echo -e "[ STEP 2 ] Restarting services on the compute node(s) ..."
  for i in $( seq 1 $NODE_NUM )
  do
    ssh compute${i} "mkdir -p /run/munge && chown -R slurm:slurm /run/munge && sudo -u slurm munged" >> $logfile 2>&1
    ssh compute${i} "systemctl restart slurmd" >> $logfile 2>&1
    scontrol update NodeName=compute${i} State=DOWN Reason=hung_completing
    scontrol update NodeName=compute${i} State=RESUME
    echo -e "\nSlurmd Status of Node ${i}:" >> ${logfile}
    ssh compute${i} "systemctl status slurmd" >> ${logfile}
    echo -e "\n" >> ${logfile}
  done
  echo -e "[ -DONE- ] HPC-NOW Cluster Status:\n"
  sinfo -N
  exit 0
fi

####### Check connectivities ###########################
if [ $1 = 'connect' ]; then
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
    ping -c 2 -W 1 -q $private_ip >> ${logfile} 2>&1
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
  if [ -f /etc/hosts-clean ]; then
    hostnamectl set-hostname master
    /bin/cp /etc/hosts-clean /etc/hosts
    cat /root/hostfile >> /etc/hosts
    /bin/cp /opt/slurm/etc/slurm.conf.128 /opt/slurm/etc/slurm.conf
    sed -i 's/NodeName=compute\[1-2\]/NodeName=compute\[1-'$NODE_NUM'\]/g' /opt/slurm/etc/slurm.conf
    sed -i 's/CPUs=128/CPUs='$NODE_CORES'/g' /opt/slurm/etc/slurm.conf
    sed -i 's/SCKTS/'$SOCKETS'/g' /opt/slurm/etc/slurm.conf
    sed -i 's/C_P_S/'$CORES_PER_SOCKET'/g' /opt/slurm/etc/slurm.conf
    sed -i 's/T_P_C/'$THREADS_PER_CORE'/g' /opt/slurm/etc/slurm.conf
    echo -e "[ STEP 4 ] The cluster configuration file has been updated.\n" >>  ${logfile}
  else
    echo -e "[ FATAL: ] PLEASE MAKE SURE the /etc/hosts-clean exists.\n Exit now."
    echo -e "[ FATAL: ] PLEASE MAKE SURE the /etc/hosts-clean exists.\n Exit now." >> ${logfile}
    exit 31
  fi
  echo -e "[ STEP 5 ] Setting up users of compute nodes ... "
  for i in $(seq 1 $NODE_NUM )
  do
    scp -r -q /etc/munge/munge.key root@compute${i}:/etc/munge
    while read hpc_user_row
    do
      username=`echo -e $hpc_user_row | awk '{print $2}'`
      #user_passwd=`echo -e $hpc_user_row | awk '{print $3}'`
      #ssh -n compute${i} "echo '$user_passwd' | passwd $username --stdin >> /dev/null 2>&1"
      scp -r -q /home/$username/.ssh root@compute${i}:/home/$username/
      ssh -n compute${i} "chown -R $username:$username /home/$username >> /dev/null 2>&1"
    done < $user_registry
  done
  echo -e "[ STEP 5 ] Users are ready."
  echo -e "[ -DONE- ] Connectivety check finished! \n[ -DONE- ] You need to run the command 'hpcmgr all' on the master node.\n[ -DONE- ] Exit now."
  exit 0
fi
# all mode: restart services on all nodes
if [[ $1 = 'master' || $1 = 'all' ]]; then
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
    mkdir -p /run/munge
    chown -R slurm:slurm /run/munge
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
    ssh compute${i} "mkdir -p /run/munge && chown -R slurm:slurm /run/munge && chown -R slurm:slurm /etc/munge"    
    ssh compute${i} "sudo -u slurm munged"
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] FAILED to start munge daemon on Compute${i} Node! Please check the installation of munge.\nExit now."
      echo -e "[ FATAL: ] FAILED to start munge daemon on Compute${i} Node! Please check the installation of munge.\nExit now." >> ${logfile}
      exit 33
    fi
  done
  echo -e "[ STEP 2 ] Authentication of Compute node(s) is OK."
  echo -e "[ STEP 3 ] Pulling the master node up ..."  
  systemctl restart slurmdbd
  systemctl status slurmdbd | grep "Active: active" >> ${logfile}
  if [ $? -eq 0 ]; then
    for i in $( seq 1 2)
    do
      sleep 1
    done
    systemctl restart slurmctld
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
      ssh compute${i} "hostnamectl set-hostname compute${i}"
      scp -q /root/hostfile root@compute${i}:/etc/
      scp -q /opt/slurm/etc/slurm.conf root@compute${i}:/opt/slurm/etc/
      ssh compute${i} "sed -i '/master/d' /etc/hosts && sed -i '/compute/d' /etc/hosts"
      ssh compute${i} "cat /etc/hostfile >> /etc/hosts && rm -rf /etc/hostfile && systemctl restart slurmd"
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
  sacctmgr list account hpc_users >> ${logfile} 2>&1
  if [ $? -ne 0 ]; then
    echo "y" | sacctmgr add account hpc_users >> ${logfile}
  fi
  for k in $(seq 1 $HPC_USER_NUM )
  do
    echo "y" | sacctmgr list user user${k} >> ${logfile} 2>&1
    if [ $? -ne 0 ]; then
      echo "y" | sacctmgr add user user${k} account=hpc_users >> ${logfile} 2>&1
    fi
  done
  echo -e "[ STEP 5 ] Cluster users are ready."
  echo -e "[ -DONE- ] HPC-NOW Cluster Status:\n"
  sinfo -N
fi

if [ $1 = 'install' ]; then
  if [ -z $URL_INSTSCRIPTS_ROOT ]; then
    echo -e "[ FATAL: ] Failed to connect to a valid appstore repo. Exit now."
    exit 35
  fi
  if [[ ! -n "$2" || $2 = 'list' ]]; then
    echo -e "[ -INFO- ] Usage: hpcmgr install software_to_be_installed"
    echo -e "[ -INFO- ] Please specify the software you'd like to build:\n"
    echo -e "1. Applications:"
    echo -e "\tof7       - OpenFOAM-v7"
    echo -e "\tof9       - OpenFOAM-v9"
    echo -e "\tof2112    - OpenFOAM-v2112"
    echo -e "\tlammps    - LAMMPS dev latest"
    echo -e "\tgromacs   - GROMACS"
    echo -e "\twrf       - WRF & WPS -4.4"
    echo -e "\tvasp5     - VASP-5.4.4 (BRING YOUR OWN LICENSE)"
    echo -e "\tvasp6.1   - VASP-6.1.0 (BRING YOUR OWN LICENSE)"
    echo -e "\tR         - R & RStudio (in development)"
    echo -e "\tpview     - ParaView-5"
    echo -e "\thdf5      - HDF5-1.10.9"
    echo -e "\tncdf4     - netCDF-C-4.9.0 & netCDF-fortran-4.5.3"
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
    echo -e "\tlapk311   - LAPACK-3.11.0"
    echo -e "\tzlib      - zlib-1.2.13"
    echo -e "\tslpack2   - ScaLAPACK-2.1.0"
    echo -e "\toblas     - OpenBLAS 0.3.15 *SINGLE THREAD*"
    echo -e "5. Other Tools:"
    echo -e "\tdesktop   - Desktop Environment" 
    echo -e "\tbaidu     - Baidu Netdisk"
    echo -e "\tnowdisk   - COSBrowser (RECOMMENDED)" 
    echo -e "\trar       - RAR for Linux (RECOMMENDED)" 
    echo -e "\tkswps     - WPS Office Suite for Linux (RECOMMENDED)" 
    echo -e "\tenvmod    - Environment Modules" 
    echo -e "\tvscode    - Visual Studio Code" 
    exit 0
  elif [[ -n $2 && $2 = 'of7' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_of7.sh | bash
  elif [[ -n $2 && $2 = 'of2112' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_of2112.sh | bash
  elif [[ -n $2 && $2 = 'mpich4' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_mpich4.sh | bash
  elif [[ -n $2 && $2 = 'mpich3' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_mpich3.sh | bash
  elif [[ -n $2 && $2 = 'gcc4' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_gcc4.sh | bash
  elif [[ -n $2 && $2 = 'gcc8' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_gcc8.sh | bash
  elif [[ -n $2 && $2 = 'gcc9' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_gcc9.sh | bash
  elif [[ -n $2 && $2 = 'gcc12' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_gcc12.sh | bash
  elif [[ -n $2 && $2 = 'nowdisk' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_cos.sh | bash
  elif [[ -n $2 && $2 = 'baidu' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_baidu.sh | bash
  elif [[ -n $2 && $2 = 'fftw3' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_fftw3.sh | bash
  elif [[ -n $2 && $2 = 'lammps' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_lammps.sh | bash
  elif [[ -n $2 && $2 = 'intel' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_intel.sh | bash
  elif [[ -n $2 && $2 = 'gromacs' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_gromacs.sh | bash
  elif [[ -n $2 && $2 = 'pview' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_paraview.sh | bash
  elif [[ -n $2 && $2 = 'of9' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_of9.sh | bash
  elif [[ -n $2 && $2 = 'ompi3' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_ompi3.sh | bash
  elif [[ -n $2 && $2 = 'ompi4' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_ompi4.sh | bash
  elif [[ -n $2 && $2 = 'lapk311' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_lapack311.sh | bash
  elif [[ -n $2 && $2 = 'R' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_R.sh | bash
  elif [[ -n $2 && $2 = 'slpack2' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_slpack2.sh | bash
  elif [[ -n $2 && $2 = 'vasp6.1' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_vasp6.1.sh | bash
  elif [[ -n $2 && $2 = 'vasp6.3' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_vasp6.3.sh | bash
  elif [[ -n $2 && $2 = 'vasp5' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_vasp5.sh | bash
  elif [[ -n $2 && $2 = 'hdf5' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_hdf5.sh | bash
  elif [[ -n $2 && $2 = 'zlib' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_zlib.sh | bash
  elif [[ -n $2 && $2 = 'ncdf4' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_netcdf4.sh | bash
  elif [[ -n $2 && $2 = 'desktop' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_desktop.sh | bash
  elif [[ -n $2 && $2 = 'envmod' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_module.sh | bash
  elif [[ -n $2 && $2 = 'rar' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_rar.sh | bash
  elif [[ -n $2 && $2 = 'kswps' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_wps.sh | bash
  elif [[ -n $2 && $2 = 'vscode' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_vscode.sh | bash
  elif [[ -n $2 && $2 = 'oblas' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_openblas.sh | bash
  elif [[ -n $2 && $2 = 'abinit' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_abinit.sh | bash
  elif [[ -n $2 && $2 = 'wrf' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_wrf.sh | bash
  elif [[ -n $2 && $2 = 'vasp6.3' ]]; then
    curl -s ${URL_INSTSCRIPTS_ROOT}install_vasp6.3.sh | bash
  else
    echo -e "[ FATAL: ] Unknown software. Please run 'hpcmgr install list' command to show all currently available software."
    echo -e "[ -INFO- ] If your software is not in the list, please describe your requirements and send email to info@hpc-now.com."
    echo -e "[ -INFO- ] Exit now."
  fi
fi  
time1=$(date)
echo -e  "End Time: ${time1}\n" >> ${logfile}