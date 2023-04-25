/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "general_funcs.h"
#include "general_print_info.h"

void print_empty_cluster_info(void){
    printf("[ -INFO- ] It seems the cluster is empty. You can either:\n");
    printf("|          a) Run 'hpcopr init' to generate a *default* cluster directly. OR\n");
    printf("|          b) Run 'hpcopr get-conf' to get and modify the configuration file and then\n");
    printf("|             Run 'hpcopr init' to generate a *customized* cluster.\n");
    printf("[ FATAL: ] Exit now.\n");
}

void print_cluster_init_done(void){
    printf("[ -DONE- ] Congratulations! The cluster is initializing now. This step may take at\n");
    printf("|          least 7 minutes. You can log into the master node now.\n"); 
    printf("|          Please check the initialization progress in the /root/cluster_init.log.\n");
    printf("|          By default, NO HPC software will be built into the cluster.\n");
    printf("|          Please run 'hpcmgr install' command to install the software you need.\n");
}

void print_help(void){
    printf("[ -INFO- ] Usage: hpcopr command_name PARAM1 PARAM2 ...\n");
    printf("| Commands:\n");
    printf("+ I  . Multi-Cluster Management:\n");
    printf("| * You DO NOT need to switch to a cluster first.\n");
    printf("|  envcheck    : Quickly check the running environment.\n");
    printf("|  new-cluster : Create a new cluster, you can specify the cluster name, extra\n");
    printf("|                *optional* parameters are accepted:\n");
    printf("|                  PARAM1 - cluster name (A-Z | a-z | 0-9 | - , %d<=length<=%d\n",CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
    printf("|                  PARAM2 - cloud access key id\n");
    printf("|                  PARAM3 - cloud access secret id\n");
    printf("|  ls-clusters : List all the current clusters.\n");
    printf("|  switch      : TARGET_CLUSTER_NAME\n");
    printf("|                Switch to a cluster in the registry to operate.\n");
    printf("|  glance      : all | TARGET_CLUSTER_NAME\n");
    printf("|                Quickly view all the clusters or a specified target cluster.\n");
    printf("|  exit-current: Exit the current cluster.\n");
    printf("|  remove      : TARGET_CLUSTER_NAME\n");
    printf("|                Completely remove a cluster from the OS and registry.\n");
    printf("+ II. Global Management:\n");
    printf("| * You DO NOT need to switch to a cluster first.\n");
    printf("|  help        : Show this page and the information here.\n");
    printf("|  usage       : Get the usage history of all your cluster(s).\n");
    printf("|  syslog      : Get the detailed operation log of your cluster management.\n");
    printf("+  Advanced - For developers:\n");
    printf("|  configloc   : Configure the locations for the terraform binaries, providers, IaC\n");
    printf("|                templates and shell scripts.\n");
    printf("|  showloc     : Show the current configured locations.\n");
    printf("|  resetloc    : Reset to the default locations.\n");
    printf("+ III. Cluster Initialization: \n");
    printf("| * You need to switch to a cluster first. *\n");
    printf("|  new-keypair : *Rotate* a new keypair for an existing cluster. The new keypair\n");
    printf("|                should be valid and comes from the same cloud vendor.\n");
    printf("|  get-conf    : Get the default configuration file to edit and build a customized\n");
    printf("|                HPC cluster later (using the 'init' command).\n");
    printf("|  edit-conf   : Edit and save the default configuration file *before* init.\n");
    printf("|  init        : Initialize a new cluster. If the configuration file is absent,\n");
    printf("|                the command will generate a default configuration file.\n");
    printf("+ IV . Cluster Management:\n");
    printf("| * You need to switch to a cluster first.\n");
    printf("|  vault       : Check the sensitive information of the current cluster.\n");
    printf("|  graph       : Display the cluster map including all the nodes and status.\n");
    printf("+ V  . Cluster Operation:\n");
    printf("| * You need to switch to a specific cluster first *.\n");
    printf("|  delc        : Delete specified compute nodes:\n");
    printf("|        all       - Delete *ALL* the compute nodes, you can run 'hpcopr addc' to\n");
    printf("|                    add compute nodes later.\n");
    printf("|        NUM       - Delete the last NUM of the compute nodes. NUM should be less\n");
    printf("|                    than the current quantity of compute nodes.\n");
    printf("|  addc  NUM   : Add compute nodes to current cluster. You can specify how many to\n");
    printf("|                be added.\n");
    printf("|  shutdownc all|NUM\n");
    printf("|              : Shutdown specified compute nodes. Similar to the command 'delc',\n");
    printf("|                you can specify to shut down all or part of the compute nodes by\n");
    printf("|                the parameter 'all' or 'NUM'.\n");
    printf("|  turnonc   all|NUM\n");
    printf("|              : Turn on specified compute nodes. Similar to the command 'delc',\n");
    printf("|                you can specify to turn on all or part of the compute nodes by\n");
    printf("|                the parameter 'all' or 'NUM'.\n");
    printf("|  reconfc     : Reconfigure the compute nodes to a target instance type. i.e.\n");
    printf("|                  a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g\n");
    printf("|                  a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g\n");
    printf("|  reconfm     : Reconfigure the master node to a target instance type. i.e.\n");
    printf("|                  a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g\n");
    printf("|                  a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g\n");
    printf("|  sleep       : Turn off all the nodes (management and compute) of the cluster.\n"); 
    printf("|  wakeup    all|minimal\n");
    printf("|              : minimal - Turn on the management nodes of the cluster.\n");
    printf("|              : all     - Turn on the management and compute nodes of the cluster.\n");
    printf("|  destroy     : *DESTROY* the whole cluster - including all the resources & data.\n");
    printf("+ VI . Others:\n");
    printf("|  about       : Display the version and other info.\n");
    printf("|  license     : Read the terms and conditions of the GNU Public License - 2.0\n");
    printf("|  repair      : Try to repair the hpcopr core components.\n");
    printf("\n");
    printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

void print_header(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    printf("|   /HPC->  Welcome to HPC_NOW Cluster Operator! Version: %s\n",CORE_VERSION_CODE);
    printf("|\\\\/ ->NOW  %d-%d-%d %d:%d:%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd LICENSE: GPL-2.0\n\n");
}

void print_tail(void){
    printf("\n");
    printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

void print_about(void){
    printf("| This is free software; see the source for copying conditions.  There is NO\n");
    printf("| warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
    printf("| This project is powered by many excellent free and open-source projects:\n");
    printf("|   1. GNU/Linux: maybe the most widely used software on this planet.\n");
    printf("|   2. Terraform: a powerful platform for cloud resource orchestration.\n");
    printf("|   3. GNOME    : a popular and user-friendly desktop environment for GNU/Linux.\n");
    printf("|   4. XRDP     : an open source Remote Desktop Program.\n");
    printf("|   5. SLURM    : an open source cluster management and job scheduling system.\n");
    printf("|   6. MUNGE    : an authentication service for creating and validating credentials.\n");
    printf("|      ......\n");
    printf("| Therefore, we also made this software public under the GPL-2.0 license.\n");
    printf("| Please check the source code here: https://gitee.com/zhenrong-wang/hpc-now/\n");
    printf("| If you encounter any issues about this software, please feel free to contact us\n");
    printf("| via info@hpc-now.com or other channels.\n");
    printf("| Let's build this open source cloud HPC platform together!\n");
    print_tail();
}

int read_license(void){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not("c:\\hpc-now\\LICENSES\\GPL-2")==0){
#ifdef __APPLE__
        sprintf(cmdline,"more /Users/hpc-now/LICENSES/GPL-2");
#elif _WIN32
        sprintf(cmdline,"more c:\\hpc-now\\LICENSES\\GPL-2");
#elif __linux__
        sprintf(cmdline,"more /home/hpc-now/LICENSES/GPL-2");
#endif
        system(cmdline);
        return 0;
    }
    sprintf(cmdline,"curl -s %s | more",URL_LICENSE);
    if(system(cmdline)!=0){
        sprintf(cmdline,"curl -s %s | more",URL_LICENSE_FSF);
        if(system(cmdline)!=0){
            return 1;
        }
    }
    return 0;
}