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
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " It seems the cluster is empty. You can either:\n");
    printf("|          a) Run 'hpcopr init' to create a *default* cluster directly. OR\n");
    printf("|          b) Run 'hpcopr get-conf' -> 'hpcopr edit-conf' -> 'hpcopr init' \n");
    printf("|          Exit now.\n");
}

void print_cluster_init_done(void){
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congratulations! The cluster is initializing now. This step may take at\n");
    printf("|          least 7 minutes. Please do not operate the cluster during this period. \n"); 
    printf("|          You can now log on the master node by 'hpcopr ssh USERNAME'.\n");
    printf("|          " HIGH_CYAN_BOLD "The desktop will be ready after the init process.\n" RESET_DISPLAY);
}

void print_help(char* plain_flag){
    if(strcmp(plain_flag,"plain")==0){
        printf("[ -INFO- ] Usage: hpcopr Command PARAM1 PARAM2 ... (Optional)-c=CLUSTER_NAME\n");
        printf("| Commands:\n");
        printf("+ 0  . Get-Started:\n");
        printf("|  envcheck    : Quickly check the running environment.\n");
        printf("+ I  . Multi-Cluster Management:\n");
        printf("|  new-cluster : Create a new cluster to initialize.\n");
        printf("|      (Optional)CLUSTER_NAME (Optional)AK (Optional)SK (Optional)ECHO_FLAG\n");
        printf("|        CLUSTER_NAME - A-Z | a-z | 0-9 | - , %d<=length<=%d\n",CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
        printf("|        AK           - cloud access key id\n");
        printf("|        SK           - cloud access secret id\n");
        printf("|        ECHO_FLAG    - Specify 'echo' to echo the ak/sk.\n");
        printf("|  ls-clusters : List all the current clusters.\n");
        printf("|  switch      : Switch to a cluster in the registry to operate.\n");
        printf("|      (Optional)TARGET_CLUSTER_NAME - Keep it blank to view the cluster list.\n");
        printf("|  glance      : View all the clusters or a target cluster.\n");
        printf("|      (Optional)all - Default to glance the current cluster.\n");
        printf("|  refresh     : Refresh a cluster without changing the resources.\n");
        printf("|      (Optional)force \n");
        printf("|        Specify 'force' as the 2nd param to do force-refresh ( DANGER! ).\n");
        printf("|  exit-current: Exit the current cluster.\n");
        printf("|  remove      : Completely remove a cluster from the OS and registry.\n");
        printf("|      (Optional)force\n");
        printf("|        Specify 'force' as the 2nd param to do force-remove ( DANGER! ).\n");
        printf("+ II. Global Management:\n");
        printf("|  help        : Show this page and the information here.\n");
        printf("|      (Optional)plain - Show this page in plain text (for redirecting to a file).\n");
        printf("|  usage       : View and/or export the usage history.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|        print - Print all the information to standard output.\n");
        printf("|        read  - Read the information with 'more'.\n");
        printf("|        EXPORT_DEST  - Export the plain-text-format to a file..\n");
        printf("|  history     : View and/or export the operation log.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|  syserr      : View and/or export the system cmd errors.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|  ssh         : SSH to the master node of a cluster.\n");
        printf("|      USER_NAME\n"); 
        printf("|        USER_NAME    - Must specify to log in as which user. For example: user1.\n");
        printf("+  Advanced - For developers:\n");
        printf("|  configloc   : Configure the locations for the terraform binaries, providers, IaC\n");
        printf("|                templates and shell scripts.\n");
        printf("|  showloc     : Show the current configured locations.\n");
        printf("|  showmd5     : Show the md5sum values of core components.\n");
        printf("|  resetloc    : Reset to the default locations.\n");
        printf("+ III. Cluster Initialization: \n");
        printf("|  new-keypair : *Rotate* a new keypair for an existing cluster. The new keypair\n");
        printf("|                should be valid and comes from the same cloud vendor.\n");
        printf("|      (Optional)AK (Optional)SK (Optional)ECHO_FLAG\n");
        printf("|        AK           - cloud access key id\n");
        printf("|        SK           - cloud access secret id\n");
        printf("|        ECHO_FLAG    - Specify 'echo' to echo the ak/sk.\n");
        printf("|  get-conf    : Get the default configuration file to edit and build a customized\n");
        printf("|                HPC cluster later (using the 'init' command).\n");
        printf("|  edit-conf   : Edit and save the default configuration file *before* init.\n");
        printf("|  rm-conf     : Remove the configuration file *before* init.\n");
        printf("|  init        : Initialize a new cluster. If the configuration file is absent,\n");
        printf("|                the command will generate a default configuration file.\n");
        printf("|      (Optional)--advanced_param=\n");
        printf("|        --r=REGION_ID | --az=AZ_ID | --nn=COMPUTE_NODE_NUM | --un=HPC_USER_NUM\n");
        printf("|        --mi=MASTER_CONFIG | --ci=COMPUTE_CONFIG | --os=OS_IMAGE\n");
        printf("|        --ht=ON/OFF (Hyperthread option, only valid for AWS)\n");
        printf("|  rebuild     : Rebuild the nodes *without* destroying the cluster's storage.\n");
        printf("|        REBUILD_OPTION\n");
        printf("|          mc   - Only rebuild the master and all the compute node(s).\n");
        printf("|          mcdb - All the nodes above + database node.\n");
        printf("|          all  - All the nodes above + nat node.\n");
        printf("+ IV . Cluster Management:\n");
        printf("|  vault       : Check the sensitive information of the current cluster.\n");
        printf("|        (Optional)ROOT_FLAG\n");
        printf("|          root  - Display with root password. By default, the root password is hidden.\n");
        printf("|  graph       : Display the cluster map including all the nodes and status.\n");
        printf("|  viewlog     : View the operation log of the current cluster.\n");
        printf("|      (Optional)STREAM_NAME (Optional)LOG_TYPE (Optional)VIEW_OPTION\n");
        printf("|        STREAM_NAME - std(default) | err . std: normal output. err: error output.\n");
        printf("|        LOG_TYPE    - realtime(default) | archive .\n");
        printf("|                      realtime: view the output of the current on-going operation.\n");
        printf("|                      archive : view the output for the historical operations.\n");
        printf("|        VIEW_OPTION - stream(default) | print.\n");
        printf("|                      stream: refresh the latest lines.\n");
        printf("|                      print : print out all the lines.\n");
        printf("+ V  . Cluster Operation:\n");
        printf("|  delc        : Delete specified compute nodes:\n");
        printf("|        all     - Delete *ALL* the compute nodes, you can run 'hpcopr addc' to\n");
        printf("|                  add compute nodes later.\n");
        printf("|        NUM     - Delete the last NUM of the compute nodes. NUM should be less\n");
        printf("|                  than the current quantity of compute nodes.\n");
        printf("|  addc        : Add compute nodes to current cluster. You must specify how many\n");
        printf("|                to be added.\n");
        printf("|        NUM     - Add NUM new compute nodes.\n");
        printf("|  shutdownc   : Shutdown specified compute nodes. Similar to 'delc',\n");
        printf("|                you can specify to shut down all or part of the compute nodes by\n");
        printf("|                the param 'all' or 'NUM'.\n");
        printf("|        all | NUM .\n");
        printf("|  turnonc     : Turn on specified compute nodes. Similar to 'delc',\n");
        printf("|                you can specify to turn on all or part of the compute nodes by\n");
        printf("|                the parameter 'all' or 'NUM'.\n");
        printf("|        all | NUM .\n");
        printf("|  reconfc     : Reconfigure all the compute nodes.\n");
        printf("|      (Optional)NEW_CONFIG (Optional)HT_FLAG\n");
        printf("|        NEW_CONFIG  - target configuration. i.e.\n");
        printf("|                a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g\n");
        printf("|                a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g\n");
        printf("|                Keep it blank to view all the available options.\n");
        printf("|        HT_FLAG     - Hyper-Thread flag (hton|htoff). Only valid for AWS.\n");
        printf("|  reconfm     : Reconfigure the master node.\n");
        printf("|      (Optional)NEW_CONFIG\n");
        printf("|        NEW_CONFIG  - target configuration. Keep it blank to view all options.\n");
        printf("|  sleep       : Turn off all the nodes (management and compute) of the cluster.\n"); 
        printf("|  wakeup      : Wake up the cluster nodes.\n");
        printf("|      (Optional)minimal(default) | all\n");
        printf("|        minimal - Turn on the management nodes of the cluster.\n");
        printf("|        all     - Turn on the management and compute nodes of the cluster.\n");
        printf("|  destroy     : *DESTROY* the whole cluster - including all the resources & data.\n");
        printf("|      (Optional)force\n");
        printf("|           Specify 'force' as the 2nd param to do force-destroy ( DANGER! ).\n");
        printf("+ VI . Cluster User Management:\n");
        print_usrmgr_info("plain");
        printf("+ VII. Others:\n");
        printf("|  about       : About this software and HPC-NOW project.\n");
        printf("|  version     : Display the version info.\n");
        printf("|  license     : Read the terms and conditions of the GNU Public License - 2.0\n");
        printf("|  repair      : Try to repair the hpcopr core components.\n");
        printf("\n");
        printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ] Usage: hpcopr " RESET_DISPLAY HIGH_GREEN_BOLD "Command" RESET_DISPLAY " PARAM1 PARAM2 ... (Optional)-c=CLUSTER_NAME\n");
        printf(GENERAL_BOLD "| " RESET_DISPLAY HIGH_GREEN_BOLD "Commands:\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "+ 0  . Get-Started:\n");
        printf("|  " HIGH_GREEN_BOLD "envcheck" RESET_DISPLAY "    : Quickly check the running environment.\n");
        printf(GENERAL_BOLD "+ I  . Multi-Cluster Management:\n");
        printf("|  " HIGH_GREEN_BOLD "new-cluster" RESET_DISPLAY " : Create a new cluster to initialize.\n");
        printf("|      (Optional)CLUSTER_NAME (Optional)AK (Optional)SK (Optional)ECHO_FLAG\n");
        printf("|        CLUSTER_NAME - A-Z | a-z | 0-9 | - , %d<=length<=%d\n",CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
        printf("|        AK           - cloud access key id\n");
        printf("|        SK           - cloud access secret id\n");
        printf("|        ECHO_FLAG    - Specify 'echo' to echo the ak/sk.\n");
        printf("|  " HIGH_GREEN_BOLD "ls-clusters" RESET_DISPLAY " : List all the current clusters.\n");
        printf("|  " HIGH_GREEN_BOLD "switch" RESET_DISPLAY "      : Switch to a cluster in the registry to operate.\n");
        printf("|      (Optional)TARGET_CLUSTER_NAME - Keep it blank to view the cluster list.\n");
        printf("|  " HIGH_GREEN_BOLD "glance" RESET_DISPLAY "      : View all the clusters or a target cluster.\n");
        printf("|      (Optional)all - Default to glance the current cluster.\n");
        printf("|  " HIGH_GREEN_BOLD "refresh" RESET_DISPLAY "     : Refresh the target cluster without changing the resources.\n");
        printf("|      (Optional)force\n");
        printf("|        Specify 'force' as the 2nd param to do force-refresh " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
        printf("|  " HIGH_GREEN_BOLD "exit-current" RESET_DISPLAY ": Exit the current cluster.\n");
        printf("|  " HIGH_GREEN_BOLD "remove" RESET_DISPLAY "      : Completely remove a cluster from the OS and registry.\n");
        printf("|      (Optional)force\n");
        printf("|        Specify 'force' as the 2nd param to do force-remove " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
        printf(GENERAL_BOLD "+ II. Global Management:\n");
        printf("|  " HIGH_GREEN_BOLD "help" RESET_DISPLAY "        : Show this page and the information here.\n");
        printf("|      (Optional)plain - Show this page in plain text (for redirecting to a file).\n");
        printf("|  " HIGH_GREEN_BOLD "usage" RESET_DISPLAY "       : View and/or export the usage history.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|        print - Print all the information to standard output.\n");
        printf("|        read  - Read the information with 'more'.\n");
        printf("|        EXPORT_DEST  - Export the plain-text-format to a file..\n");
        printf("|  " HIGH_GREEN_BOLD "history" RESET_DISPLAY "     : View and/or export the operation log.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|  " HIGH_GREEN_BOLD "syserr" RESET_DISPLAY "      : View and/or export the system cmd errors.\n");
        printf("|      (Optional)print(default)|read (Optional)EXPORT_DEST\n");
        printf("|  " HIGH_GREEN_BOLD "ssh" RESET_DISPLAY "         : SSH to the master node of a cluster.\n");
        printf("|      USER_NAME\n"); 
        printf("|        USER_NAME    - Must specify to log in as which user. For example: user1.\n");
        printf(WARN_YELLO_BOLD "+  Advanced - For developers:\n" RESET_DISPLAY);
        printf("|  " HIGH_GREEN_BOLD "configloc" RESET_DISPLAY "   : Configure the locations for the terraform binaries, providers, IaC\n");
        printf("|                templates and shell scripts.\n");
        printf("|  " HIGH_GREEN_BOLD "showloc" RESET_DISPLAY "     : Show the current configured locations.\n");
        printf("|  " HIGH_GREEN_BOLD "showmd5" RESET_DISPLAY "     : Show the md5sum values of core components.\n");
        printf("|  " HIGH_GREEN_BOLD "resetloc" RESET_DISPLAY "    : Reset to the default locations.\n");
        printf(GENERAL_BOLD "+ III. Cluster Initialization: \n");
        printf("|  " HIGH_GREEN_BOLD "new-keypair" RESET_DISPLAY " : *Rotate* a new keypair for an existing cluster. The new keypair\n");
        printf("|                should be valid and comes from the same cloud vendor.\n");
        printf("|      (Optional)AK (Optional)SK (Optional)ECHO_FLAG\n");
        printf("|        AK           - cloud access key id\n");
        printf("|        SK           - cloud access secret id\n");
        printf("|        ECHO_FLAG    - Specify 'echo' to echo the ak/sk.\n");
        printf("|  " HIGH_GREEN_BOLD "get-conf" RESET_DISPLAY "    : Get the default configuration file to edit and build a customized\n");
        printf("|                HPC cluster later (using the 'init' command).\n");
        printf("|  " HIGH_GREEN_BOLD "edit-conf" RESET_DISPLAY "   : Edit and save the default configuration file *before* init.\n");
        printf("|  " HIGH_GREEN_BOLD "rm-conf" RESET_DISPLAY "     : Remove the configuration file *before* init.\n");
        printf("|  " HIGH_GREEN_BOLD "init" RESET_DISPLAY "        : Initialize a new cluster. If the configuration file is absent,\n");
        printf("|                the command will generate a default configuration file.\n");
        printf("|      (Optional)--advanced_param=\n");
        printf("|        --r=REGION_ID | --az=AZ_ID | --nn=COMPUTE_NODE_NUM | --un=HPC_USER_NUM\n");
        printf("|        --mi=MASTER_CONFIG | --ci=COMPUTE_CONFIG | --os=OS_IMAGE\n");
        printf("|        --ht=ON/OFF (Hyperthread option, only valid for AWS)\n");
        printf("|  " HIGH_GREEN_BOLD "rebuild" RESET_DISPLAY "     : Rebuild the nodes *without* destroying the cluster's storage.\n");
        printf("|      REBUILD_OPTION\n");
        printf("|        mc   - Only rebuild the master and all the compute node(s).\n");
        printf("|        mcdb - All the nodes above + database node.\n");
        printf("|        all  - All the nodes above + nat node.\n");
        printf(GENERAL_BOLD "+ IV . Cluster Management:\n");
        printf("|  " HIGH_GREEN_BOLD "vault" RESET_DISPLAY "       : Check the sensitive information of the current cluster.\n");
        printf("|      (Optional)ROOT_FLAG\n");
        printf("|        root  - Display with root password. By default, the root password is hidden.\n");
        printf("|  " HIGH_GREEN_BOLD "graph" RESET_DISPLAY "       : Display the cluster map including all the nodes and status.\n");
        printf("|  " HIGH_GREEN_BOLD "viewlog" RESET_DISPLAY "     : View the operation log of the current cluster.\n");
        printf("|      (Optional)STREAM_NAME (Optional)LOG_TYPE (Optional)VIEW_OPTION\n");
        printf("|        STREAM_NAME - std(default) | err . std: normal output. err: error output.\n");
        printf("|        LOG_TYPE    - realtime(default) | archive .\n");
        printf("|                      realtime: view the output of the current on-going operation.\n");
        printf("|                      archive : view the output for the historical operations.\n");
        printf("|        VIEW_OPTION - stream(default) | print.\n");
        printf("|                      stream: refresh the latest lines.\n");
        printf("|                      print : print out all the lines.\n");
        printf(GENERAL_BOLD "+ V  . Cluster Operation:\n");
        printf("|  " HIGH_GREEN_BOLD "delc" RESET_DISPLAY "        : Delete specified compute nodes:\n");
        printf("|        all     - Delete *ALL* the compute nodes, you can run 'hpcopr addc' to\n");
        printf("|                  add compute nodes later.\n");
        printf("|        NUM     - Delete the last NUM of the compute nodes. NUM should be less\n");
        printf("|                  than the current quantity of compute nodes.\n");
        printf("|  " HIGH_GREEN_BOLD "addc" RESET_DISPLAY "        : Add compute nodes to current cluster. You must specify how many\n");
        printf("|                to be added.\n");
        printf("|        NUM    - Add NUM new compute nodes.\n");
        printf("|  " HIGH_GREEN_BOLD "shutdownc" RESET_DISPLAY "   : Shutdown specified compute nodes. Similar to 'delc',\n");
        printf("|                you can specify to shut down all or part of the compute nodes by\n");
        printf("|                the param 'all' or 'NUM'.\n");
        printf("|        all | NUM .\n");
        printf("|  " HIGH_GREEN_BOLD "turnonc" RESET_DISPLAY "     : Turn on specified compute nodes. Similar to 'delc',\n");
        printf("|                you can specify to turn on all or part of the compute nodes by\n");
        printf("|                the parameter 'all' or 'NUM'.\n");
        printf("|        all | NUM .\n");
        printf("|  " HIGH_GREEN_BOLD "reconfc" RESET_DISPLAY "     : Reconfigure all the compute nodes.\n");
        printf("|      (Optional)NEW_CONFIG (Optional)HT_FLAG\n");
        printf("|        NEW_CONFIG  - target configuration. i.e.\n");
        printf("|                  a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g\n");
        printf("|                  a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g\n");
        printf("|          Keep it blank to view all the available options.\n");
        printf("|        HT_FLAG     - Hyper-Thread flag (hton|htoff). Only valid for AWS.\n");
        printf("|  " HIGH_GREEN_BOLD "reconfm" RESET_DISPLAY "     : Reconfigure the master node.\n");
        printf("|      (Optional)NEW_CONFIG\n");
        printf("|          NEW_CONFIG  - target configuration. Keep it blank to view all options.\n");
        printf("|  " HIGH_GREEN_BOLD "sleep" RESET_DISPLAY "       : Turn off all the nodes (management and compute) of the cluster.\n"); 
        printf("|  " HIGH_GREEN_BOLD "wakeup" RESET_DISPLAY "      : Wake up the cluster nodes.\n");
        printf("|      (Optional)minimal(default) | all\n");
        printf("|        minimal - Turn on the management nodes of the cluster.\n");
        printf("|        all     - Turn on the management and compute nodes of the cluster.\n");
        printf("|  " HIGH_GREEN_BOLD "destroy" RESET_DISPLAY "     : *DESTROY* the whole cluster - including all the resources & data.\n");
        printf("|      (Optional)force\n");
        printf("|        Specify 'force' as the 2nd param to do force-destroy " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
        printf(GENERAL_BOLD "+ VI . Cluster User Management:\n");
        print_usrmgr_info("");
        printf(GENERAL_BOLD "+ VII. Others:\n" RESET_DISPLAY);
        printf("|  " HIGH_GREEN_BOLD "about" RESET_DISPLAY "       : About this software and HPC-NOW project.\n");
        printf("|  " HIGH_GREEN_BOLD "version" RESET_DISPLAY "     : Display the version info.\n");
        printf("|  " HIGH_GREEN_BOLD "license" RESET_DISPLAY "     : Read the terms and conditions of the GNU Public License - 2.0\n");
        printf("|  " HIGH_GREEN_BOLD "repair" RESET_DISPLAY "      : Try to repair the hpcopr core components.\n");
        printf("\n");
        printf(GENERAL_BOLD "<> visit:" RESET_DISPLAY " https://www.hpc-now.com" GENERAL_BOLD " <> mailto:" RESET_DISPLAY " info@hpc-now.com\n");
    }
}

void print_header(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    printf(GENERAL_BOLD "|   /HPC->  Welcome to HPC-NOW Cluster Operator! Version: %s\n",CORE_VERSION_CODE);
    printf("|\\\\/ ->NOW  %d-%d-%d %d:%d:%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd LICENSE: GPL-2.0\n\n" RESET_DISPLAY);
}

void print_tail(void){
    printf("\n");
    printf(GENERAL_BOLD "<> visit:" RESET_DISPLAY " https://www.hpc-now.com" GENERAL_BOLD " <> mailto:" RESET_DISPLAY " info@hpc-now.com\n");
}

void print_version(void){
    printf("| This is free software; see the source for copying conditions.  There is NO\n");
    printf("| warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
    printf(HIGH_GREEN_BOLD "| Version: %s\n" RESET_DISPLAY,CORE_VERSION_CODE);
    print_tail();
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
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%sGPL-2",NOW_LIC_DIR,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"more %s",filename_temp);
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

void print_usrmgr_info(char* plain_flag){
    if(strcmp(plain_flag,"plain")==0){
        printf("| Usage: hpcopr userman Option (Optional)USERMAN_PARAM1 (Optional)USERMAN_PARAM2\n");
        printf("| * The cluster must be in running state (minimal or all). *\n");
        printf("|        list    : List all the current cluster users.\n");
        printf("|        add     : Add a user to the cluster. By default, added users are enabled.\n");
        printf("|          USERMAN_PARAM1   - Username string (A-Z | a-z | - , Length %d-%d)\n",USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
        printf("|          USERMAN_PARAM2   - password string\n");
        printf("|        delete  : Delete a user from the cluster.\n");
        printf("|          USERMAN_PARAM1   - Username to be deleted\n");
        printf("|        enable  : Enable a *disabled* user. Enabled users can run HPC workloads.\n");
        printf("|          USERMAN_PARAM1   - Username to be enabled\n");
        printf("|        disable : Disable a user. Disabled users still can access the cluster.\n");
        printf("|          USERMAN_PARAM1   - Username to be enabled\n");
        printf("|        passwd  : Change user's password.\n");
        printf("|          USERMAN_PARAM1   - An existed username\n");
        printf("|          USERMAN_PARAM2   - new password string\n");
    }
    else{
        printf(GENERAL_BOLD "| Usage:" RESET_DISPLAY HIGH_GREEN_BOLD " hpcopr userman" RESET_DISPLAY HIGH_CYAN_BOLD " Option" RESET_DISPLAY GENERAL_BOLD " (Optional)" RESET_DISPLAY "USERMAN_PARAM1 " GENERAL_BOLD "(Optional)" RESET_DISPLAY "USERMAN_PARAM2\n");
        printf(GENERAL_BOLD "| * The cluster must be in running state (minimal or all). *\n" RESET_DISPLAY);
        printf("|        " HIGH_CYAN_BOLD "list" RESET_DISPLAY "    : List all the current cluster users.\n");
        printf("|        " HIGH_CYAN_BOLD "add" RESET_DISPLAY "     : Add a user to the cluster. By default, added users are enabled.\n");
        printf("|          USERMAN_PARAM1   - Username string " WARN_YELLO_BOLD "(A-Z | a-z | - , Length %d-%d)" RESET_DISPLAY "\n",USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
        printf("|          USERMAN_PARAM2   - password string\n");
        printf("|        " HIGH_CYAN_BOLD "delete" RESET_DISPLAY "  : Delete a user from the cluster.\n");
        printf("|          USERMAN_PARAM1   - Username to be deleted\n");
        printf("|        " HIGH_CYAN_BOLD "enable" RESET_DISPLAY "  : Enable a *disabled* user. Enabled users can run HPC workloads.\n");
        printf("|          USERMAN_PARAM1   - Username to be enabled\n");
        printf("|        " HIGH_CYAN_BOLD "disable" RESET_DISPLAY " : Disable a user. Disabled users still can access the cluster.\n");
        printf("|          USERMAN_PARAM1   - Username to be enabled\n");
        printf("|        " HIGH_CYAN_BOLD "passwd" RESET_DISPLAY "  : Change user's password.\n");
        printf("|          USERMAN_PARAM1   - An existed username\n");
        printf("|          USERMAN_PARAM2   - new password string\n");
    }
}