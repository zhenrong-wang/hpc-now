/*
* This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
* The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
* It is distributed under the license: GNU Public License - v2.0
* Bug report: info@hpc-now.com
*/

#ifdef _WIN32
#include "..\\include\\now_macros.h"
#include "..\\include\\now_global_vars.h"
#include "..\\include\\now_functions.h" 

#else
#include "../include/now_macros.h"
#include "../include/now_global_vars.h"
#include "../include/now_functions.h" 
#endif

void print_empty_cluster_info(void){
    printf("[ -INFO- ] It seems the cluster is empty. You can either:\n");
    printf("|          a) Run 'hpcopr init' to generate a *default* cluster directly. OR\n");
    printf("|          b) Run 'hpcopr conf' to get and modify the configuration file and then\n");
    printf("|             Run 'hpcopr init' to generate a *customized* cluster.\n");
    printf("[ FATAL: ] Exit now.\n");
}

void print_help(void){
    printf("[ -INFO- ] Usage: hpcopr command_name param1 param2\n");
    printf("| Commands:\n");
    printf("+ I  . Initialization:\n");
    printf("|  new         : Create a new working directory or rotating a new keypair:\n");
    printf("|        workdir   - Create a new working directory to initialize a new cluster\n");
    printf("|                    using the 'init' command later.\n");
    printf("|        keypair   - Rotating a new keypair for an existing cluster.\n");
    printf("|  init        : Initialize a new cluster. If the configuration file is absent,\n");
    printf("|                the command will generate a default configuration file. You can\n");
    printf("|                also add a param to this command to specify a cluster_id.\n");
    printf("|                    Example: hpcopr init hpcnow-demo\n");
    printf("|  conf        : Get the default configuration file to edit and build a customized\n");
    printf("|                HPC cluster later (using the 'init' command).\n");
    printf("+ II . Management:\n");
    printf("|  help        : Show this page and the information here.\n");
    printf("|  usage       : Get the usage history of all your cluster(s).\n");
    printf("|  syslog      : Get the detailed operation log of your cluster management.\n");
    printf("|  vault       : Check the sensitive information of your clusters.\n");
    printf("|  graph       : Display the cluster map including all the nodes and status.\n");
    printf("+ III. Operation:\n");
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
    printf("+ IV . Advanced - For developers:\n");
    printf("|  configloc   : Configure the locations for the terraform binaries, providers, IaC\n");
    printf("|                templates and shell scripts.\n");
    printf("|  showloc     : Show the current configured locations.\n");
    printf("|  resetloc    : Reset to the default locations.\n");
    printf("+ V  . Other:\n");
    printf("|  about       : Display the version and other info.\n");
    printf("|  license     : Read the terms and conditions of the GNU Public License - 2.0\n");
    printf("\n");
    printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

void print_header(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    printf("|   /HPC->  Welcome to HPC_NOW Cluster Operator! Version: %s\n",VERSION_CODE);
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

void print_not_in_a_workdir(char* current_dir){
    char temp_string[128]="";
    printf("[ FATAL: ] You are not in a working directory, *NO* critical operation is permitted.\n");
#ifdef _WIN32
    printf("|          A typical working directory: C:\\hpc-now\\now-cluster-# (# is a number).\n");
#else
    printf("|          A typical working directory: /Users/hpc-now/now-cluster-# (# is a number).\n");
#endif
    sprintf(temp_string,"|          Current directory is %s.",current_dir);
    printf("%s\n",temp_string);
    printf("|          Please use the 'cd' command to go to a working directory first.\n");
    printf("[ FATAL: ] Exit now.\n");
}

void read_license(void){
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(cmdline,"curl -s %s | more",URL_LICENSE);
    if(system(cmdline)!=0){
        sprintf(cmdline,"curl -s %s | more",URL_LICENSE_FSF);
        if(system(cmdline)!=0){
#ifdef __APPLE__
            sprintf(cmdline,"more /Users/hpc-now/LICENSES/GPL-2");
#elif _WIN32
            sprintf(cmdline,"more c:\\hpc-now\\LICENSES\\GPL-2");
#elif __linux__
            sprintf(cmdline,"more /home/hpc-now/LICENSES/GPL-2");
#endif
            system(cmdline);
        }
    }
}