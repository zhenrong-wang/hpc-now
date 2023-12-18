/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
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
    printf("|          a) Run 'hpcopr init' to create a *default* cluster directly .\n");
    printf("|          b) Run 'hpcopr init' with init options. e.g. --rg region_id .\n");
    printf("|          c) Run 'hpcopr edit-conf' -> 'hpcopr init' (not recommended).\n");
}

void print_cluster_init_done(void){
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The cluster is initializing now. " WARN_YELLO_BOLD "This step needs\n");
    printf("|          at least *7* minutes. *DO NOT* operate the cluster during \n"); 
    printf("|          this period." RESET_DISPLAY " Use 'hpcopr ssh -u USERNAME' to log in by SSH.\n");
    printf("|          " HIGH_CYAN_BOLD "The desktop will be ready after the init process." RESET_DISPLAY "\n");
}

void print_help(char* cmd_name){
    printf(GENERAL_BOLD "[ -INFO- ] Usage: hpcopr " RESET_DISPLAY GREY_LIGHT "-b" RESET_DISPLAY HIGH_GREEN_BOLD " Command " RESET_DISPLAY GENERAL_BOLD "CMD_FLAG ..." RESET_DISPLAY " [ " HIGH_CYAN_BOLD "KEY_WORD1" RESET_DISPLAY " KEY_STRING1 ] ...\n");
    printf("|          A Global and special CMD_FLAG : " GENERAL_BOLD "-b" RESET_DISPLAY " Enter the batch execution mode\n");
    printf("|          Global KEY_WORD and KEY_STRING: " GENERAL_BOLD "-c CLUSTER_NAME\n" RESET_DISPLAY "\n");
    printf("|        * Advanced KEY_WORD & KEY_STRING: " GENERAL_BOLD "--dbg-level TF_DEBUG_LEVEL (Default: info)\n" RESET_DISPLAY "\n");
    printf("|                                          " GENERAL_BOLD "--max-time  TF_MAXIMUM_WAIT_TIME (600~1200)\n" RESET_DISPLAY "\n");
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "| Command Instructions\n" RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "+ 0    . Get-Started:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"envcheck")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "envcheck" RESET_DISPLAY "    :~ Quickly check the running environment.\n");
        printf("|   --gcp         ~ Check the connectivity to Google Cloud Platform.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ I    . Multi-Cluster Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"new-cluster")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "new-cluster" RESET_DISPLAY " :~ Create a new cluster to initialize.\n");
        printf("|   --cname  CLUSTER_NAME           ~ A-Z | a-z | 0-9 | - , %d<=length<=%d\n",CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
        printf("|   --ak     ACCESS_KEY             ~ Cloud access key id\n");
        printf("|   --sk     SECRET_KEY             ~ cloud access secret id\n");
        printf("|            GCP_KEY_FILE_PATH      " HIGH_CYAN_BOLD "~ Only for GCP:" RESET_DISPLAY ", specify the path of a JSON-Format key file.\n");
        printf("|   --az-sid AZURE_SUBSCRIPTION_ID  " HIGH_CYAN_BOLD "~ Only for Azure:" RESET_DISPLAY " Subscription ID\n");
        printf("|   --az-tid AZURE_TENANT_ID        " HIGH_CYAN_BOLD "~ Only for Azure:" RESET_DISPLAY " Tenant ID\n");
        printf("|   --echo                          ~ Specify 'echo' to echo the ak/sk.\n");
        printf("|   --gcp                           ~ Specify Google Cloud Platform cluster.\n");
    }
    if(strcmp(cmd_name,"ls-clusters")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "ls-clusters" RESET_DISPLAY " :~ List all the current clusters.\n");
    }
    if(strcmp(cmd_name,"switch")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "switch" RESET_DISPLAY "      :~ Switch to a cluster in the registry to operate.\n");
        printf("|   --list                ~ List out all the cluster names.\n");
        printf("|   -c   TARGET_CLUSTER   ~ Specify a target cluster.\n");
    }
    if(strcmp(cmd_name,"glance")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "glance" RESET_DISPLAY "      :~ View all the clusters or a target cluster.\n");
        printf("|   --all                 ~ Glance all the clusters.\n");
        printf("|   -c   TARGET_CLUSTER   ~ Specify a target cluster.\n");
    }
    if(strcmp(cmd_name,"refresh")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "refresh" RESET_DISPLAY "     :~ Refresh a cluster without changing the resources.\n");
        printf("|   -c   TARGET_CLUSTER   ~ Specify a target cluster.\n");
        printf("|   --force               ~ do force-refresh " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
    }
    if(strcmp(cmd_name,"export")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "export" RESET_DISPLAY "      :~ Export a cluster to another hpcopr client. Optional params:\n");
        printf("|   --ul     USER_LIST    ~ Specify a list of users to be exported (split by ':').\n");
        printf("|    -p      PASSWORD     ~ Specify a password string to encrypt the files.\n");
        printf("|    -d      DEST_PATH    ~ Specify a destination path to export to.\n");
        printf("|   --admin               ~ Export with cluster admin privilege.\n");
        printf("|                         ~ ONLY valid when user1 is in the user list.\n");
    }
    if(strcmp(cmd_name,"import")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "import" RESET_DISPLAY "      :~ Import a cluster to the current hpcopr client.\n");
        printf("|    -s      SOURCE_PATH  ~ Specify the path of the source file.\n");
        printf("|    -p      PASSWORD     ~ Input the password string to decrypt and import.\n");
    }
    if(strcmp(cmd_name,"remove")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "remove" RESET_DISPLAY "      :~ Completely remove a cluster from the OS and registry.\n");
        printf("|   -c   TARGET_CLUSTER   ~ Specify a target cluster.\n");
        printf("|   --force               ~ Do force-remove " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
    }
    if(strcmp(cmd_name,"exit-current")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "exit-current" RESET_DISPLAY ":~ Exit the current cluster.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ II   . Global Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"help")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "help" RESET_DISPLAY "        :~ Show this page and the information here.\n");
        printf("|   --cmd    CMD_NAME     ~ Search help info by command name.\n");
    }
    if(strcmp(cmd_name,"usage")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "usage" RESET_DISPLAY "       :~ View and/or export the usage history.\n");
        printf("|   --read                ~ Use system util 'more' to read the usage.\n");
        printf("|   --print(Default)      ~ Print out the usage data.\n");
        printf("|    -d       DEST_PATH   ~ Export the usage data to a destination file.\n");
    }
    if(strcmp(cmd_name,"monman")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "monman" RESET_DISPLAY "      :~ Get, filter, and extract cluster monitoring data.\n");
        printf("|    -n     NODE_LIST        ~ Specify node names connected by :, e.g. " HIGH_CYAN_BOLD "compute1:compute2:master" RESET_DISPLAY "\n");
        printf("|    -s     START_TIMESTAMP  ~ Specify a " HIGH_CYAN_BOLD "strictly-formatted" RESET_DISPLAY " start timestamp. e.g. " HIGH_CYAN_BOLD "2023-1-1@12:10" RESET_DISPLAY " \n");
        printf("|                            ~ " WARN_YELLO_BOLD "*MUST* use a "HIGH_CYAN_BOLD "@" RESET_DISPLAY " to split the date and time!" RESET_DISPLAY "\n");
        printf("|    -e     END_TIMESTAMP    ~ Specify a " HIGH_CYAN_BOLD "strictly-formatted" RESET_DISPLAY " end timestamp. e.g. " HIGH_CYAN_BOLD "2023-1-1@12:10" RESET_DISPLAY "\n");
        printf("|   --level INTERVAL_MINUTES ~ Time interval by minutes.\n");
        printf("|    -d     DEST_PATH        ~ Export the data to a destination folder or file.\n");
    }
    if(strcmp(cmd_name,"history")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "history" RESET_DISPLAY "     :~ View and/or export the operation log.\n");
        printf("|   --read                ~ Use system util 'more' to read the usage.\n");
        printf("|   --print(Default)      ~ Print out the usage data.\n");
        printf("|    -d       DEST_PATH   ~ Export the usage data to a destination file.\n");
    }
    if(strcmp(cmd_name,"syserr")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "syserr" RESET_DISPLAY "      :~ View and/or export the system cmd errors.\n");
        printf("|   --read                ~ Use system util 'more' to read the usage.\n");
        printf("|   --print(Default)      ~ Print out the usage data.\n");
        printf("|    -d      DEST_PATH    ~ Export the usage data to a destination file.\n");
    }
    if(strcmp(cmd_name,"ssh")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "ssh" RESET_DISPLAY "         :~ SSH to the master node of a cluster.\n");
        printf("|   -u       USER_NAME    ~ SSH to the cluster as a valid user.\n");
    }
    if(strcmp(cmd_name,"rdp")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "rdp" RESET_DISPLAY "         :~ Connect to the desktop of the cluster with RDP.\n");
        printf("|   -u       USER_NAME    ~ Connect as a valid user.\n");
        printf("|   --copypass            ~ Copy the user's password to the system clipboard. " WARN_YELLO_BOLD "HIGH RISK!" RESET_DISPLAY "\n");
        printf("|                         ~ " WARN_YELLO_BOLD "VERY RISKY! Please DO empty your system clipboard after pasting it" RESET_DISPLAY "\n");
        printf("|                         ~ " WARN_YELLO_BOLD "to the RDP Client! Otherwise the password will probably be leaked!" RESET_DISPLAY "\n");
        printf("|                         ~ " WARN_YELLO_BOLD "You need to copy other contents to overwrite the system clipboard." RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf("+  Advanced - For developers:\n");
    }
    if(strcmp(cmd_name,"set-tf")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "set-tf" RESET_DISPLAY "   :~ Configure the tf execution configurations.\n");
        printf("|   --tf-run    EXECUTION_NAME  ~ terraform or tofu\n");
        printf("|   --dbg-level DEBUG_LOG_LEVEL ~ debug log output level, default: warn\n");
        printf("|   --max-time  MAX_WAIT_TIME   ~ maximum waiting time (600~1200), default 600\n");
    }
    if(strcmp(cmd_name,"configloc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "configloc" RESET_DISPLAY "   :~ Configure the locations for the terraform binaries, providers\n");
        printf("|              :~ IaC templates and shell scripts.\n");
    }
    if(strcmp(cmd_name,"decrypt")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "decrypt" RESET_DISPLAY "   :~ " WARN_YELLO_BOLD "VERY RISKY!" RESET_DISPLAY " Decrypt files related to a cluster list.\n");
        printf("|   -c CLUSTER_LIST ~ A list in format of " HIGH_CYAN_BOLD "cluster1:cluster2:..." RESET_DISPLAY "\n");
        printf("|   --all           ~ All the clusters in current registry, " GENERAL_BOLD "this option has higher priority." RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"encrypt")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "encrypt" RESET_DISPLAY "   :~ Encrypt files related to a cluster list.\n");
        printf("|   -c CLUSTER_LIST ~ A list in format of " HIGH_CYAN_BOLD "cluster1:cluster2:..." RESET_DISPLAY "\n");
        printf("|   --all           ~ All the clusters in current registry, " GENERAL_BOLD "this option has higher priority." RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"showloc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "showloc" RESET_DISPLAY "     :~ Show the current configured locations.\n");
    }
    if(strcmp(cmd_name,"showmd5")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "showmd5" RESET_DISPLAY "     :~ Show the md5sum values of core components.\n");
    }
    if(strcmp(cmd_name,"resetloc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "resetloc" RESET_DISPLAY "    :~ Reset to the default locations.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ III  . Cluster Initialization: " RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"cloud-info")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "cloud-info" RESET_DISPLAY " :~ Display the cloud information of a specified cluster\n");
    }
    if(strcmp(cmd_name,"rotate-key")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "rotate-key" RESET_DISPLAY " :~ *Rotate* a new keypair for an existing cluster. The new keypair\n");
        printf("|              :~  should be valid and comes from the same cloud vendor.\n");
        printf("|   --ak     ACCESS_KEY   ~ Cloud access key id\n");
        printf("|   --sk     SECRET_KEY   ~ cloud access secret id\n");
        printf("|   --echo                ~ Specify 'echo' to echo the ak/sk.\n");
    }
    if(strcmp(cmd_name,"get-conf")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "get-conf" RESET_DISPLAY "    :~ Get the default configuration file to edit and build a customized\n");
        printf("|                HPC cluster later (using the 'init' command).\n");
    }
    if(strcmp(cmd_name,"edit-conf")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "edit-conf" RESET_DISPLAY "   :~ Edit and save the default configuration file *before* init.\n");
    }
    if(strcmp(cmd_name,"rm-conf")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "rm-conf" RESET_DISPLAY "     :~ Remove the configuration file *before* init.\n");
    }
    if(strcmp(cmd_name,"init")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "init" RESET_DISPLAY "        :~ Initialize a new cluster. If the configuration file is absent,\n");
        printf("|              :~ the command will generate a default configuration file.\n");
        printf("|   --force           ~ Remove the existed conf file(comes from a previously-failed init process)\n");
        printf("|   --rg REGION_ID    ~ Cloud Region ID\n");
        printf("|   --az AZ_ID        ~ Cloud Availability Zone ID\n");
        printf("|   --nn NODE_NUM     ~ Initial compute number\n");
        printf("|   --un USER_NUM     ~ Initial cluster user number\n");
        printf("|   --mi MASTER_INST  ~ Master node instance type\n");
        printf("|   --ci COMPUTE_INST ~ Compute node instance type\n");
        printf("|   --ht ON | OFF     ~ Hyperthreading option for AWS\n");
        printf("|   --vol VOLUME_GB   ~ Shared Volume in GB (" HIGH_CYAN_BOLD "Only for Huaweicloud, Microsoft Azure and GCP" RESET_DISPLAY ")\n");
        printf("|                     ~ This volume cannot be reduced! You can increase it by the 'hpcopr nfsup' command.\n");
        printf("|                     ~ Therefore, please specify a reasonable volume.\n");
        printf("|   --os OS_NAME      ~ Valid names (see " HIGH_CYAN_BOLD "[c]" RESET_DISPLAY " below): centos7(CentOS 7.9), centoss9(CentOS Stream 9); OR\n");
        printf("|        OS_ID        ~ The cloud image ID string, e.g. ami-xxxxxxxxx, id-xxxxxxxx; OR\n");
        printf("|        OS_SELFLINK  ~ Only for Google Cloud Platform, e.g: projects/xxxx/xxxx\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "[a] This option is *NOT* valid for Microsoft Azure Cloud." RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "[b] If this option is absent, the HPC-NOW will use a default image. See the list below:" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    +------------------------------------------+" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | AliCloud     | CLOUD_A | CentOS Stream 9 |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | TencentCloud | CLOUD_B | CentOS Stream 9 |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | AWS          | CLOUD_C | CentOS Stream 9 |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | Huaweicloud  | CLOUD_D | Rocky Linux 9.x |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | BaiduBCE     | CLOUD_E | CentOS Stream 9 |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | Azure Cloud  | CLOUD_F | Oracle Linux 9  |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    | GCP          | CLOUD_G | CentOS Stream 9 |" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    +------------------------------------------+" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "[c] For Huaweicloud, the valid names are: rocky9(Rocky Linux 9), euleros(EulerOS)." RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "[d] We *STRONGLY* recommend you to use the default images, unless that your image" RESET_DISPLAY "\n");
        printf("|                     ~ " HIGH_CYAN_BOLD "    has been tested and validated." RESET_DISPLAY "\n");
        printf("|   --inst URL        ~ You can specify an URL for the scripts of app management.\n");
        printf("|   --repo URL        ~ You can specify an URL for the HPC package repository.\n");
        printf("|                     ~ If not specified, the default URLs will be used automatically.\n");
        printf("|                     ~ You can run 'hpcopr showloc' to check them.\n");
    }
    if(strcmp(cmd_name,"rebuild")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "rebuild" RESET_DISPLAY "     :~ Rebuild the nodes *without* destroying the cluster's storage.\n");
        printf("|   --mc              ~ Only rebuild the master and all the compute node(s).\n");
        printf("|   --mcdb            ~ All the nodes above + database node.\n");
        printf("|   --all             ~ All the nodes above + nat node.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ IV   . Cluster Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"vault")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "vault" RESET_DISPLAY "       :~ Check the sensitive information of the current cluster.\n");
        printf("|   -u     USER_NAME   ~ A Valid cluster user.\n");
        printf("|   --bkey             ~ Display the cloud bucket AccessKey and SecretKey.\n");
        printf("|   --rkey             ~ " WARN_YELLO_BOLD "(RISKY!)" RESET_DISPLAY " Display with root password. By default, it is hidden.\n");
    }
    if(strcmp(cmd_name,"graph")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "graph" RESET_DISPLAY "       :~ Display the cluster map including all the nodes and status.\n");
        printf("|   --level DISP_LEVEL ~ You can choose to graph with 'csv','txt', or 'topo'\n");
    }
    if(strcmp(cmd_name,"viewlog")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "viewlog" RESET_DISPLAY "     :~ View the operation log of the current cluster.\n");
        printf("|   --log STREAM_TYPE  ~ Choose standard output (std), standart error (err), or TF debug stream (dbg).\n");
        printf("|   --this | --hist    ~ Choose the log of this run or historical runs.\n");
        printf("|   --print            ~ Print out (not stream out) the contents.\n");
        printf("|    -d   EXPORT_DEST  ~ Export the log to a specified folder or file.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ V  . Cluster Operation:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"delc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "delc" RESET_DISPLAY "        :~ Delete specified compute nodes.\n");
        printf("|   --nn NODE_NUM ~ Delete the last NUM of the compute nodes. '--nn all'\n");
        printf("|                 ~ means all nodes.\n");
    }
    if(strcmp(cmd_name,"addc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "addc" RESET_DISPLAY "        :~ Add compute nodes to current cluster. You must specify how many\n");
        printf("|              :~ to be added.\n");
        printf("|   --nn NODE_NUM ~ Add NUM new compute nodes.\n");
    }
    if(strcmp(cmd_name,"shutdownc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "shutdownc" RESET_DISPLAY "   :~ Shutdown specified compute nodes. Similar to 'delc',\n");
        printf("|              :~ you can specify to shut down all or part of the compute nodes by\n");
        printf("|              :~ the param --nn NODE_NUM.\n");
        printf("|   --nn NODE_NUM ~ Nodes to be shutdown, '--nn all' means all nodes.\n");
    }
    if(strcmp(cmd_name,"turnonc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "turnonc" RESET_DISPLAY "     :~ Turn on specified compute nodes. Similar to 'delc',\n");
        printf("|              :~ you can specify to turn on all or part of the compute nodes by\n");
        printf("|              :~ the param --nn NODE_NUM.\n");
        printf("|   --nn NODE_NUM ~ Nodes to be turned on, '--nn all' means all nodes.\n");
    }
    if(strcmp(cmd_name,"reconfc")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "reconfc" RESET_DISPLAY "     :~ Reconfigure all the compute nodes.\n");
        printf("|   --list             ~ List out all the available configurations\n");
        printf("|   --conf NEW_CONFIG  ~ Specify the new_configuration.\n");
        printf("|   --ht ON | OFF      ~ Turn on or off hyperthreading for AWS clusters.\n");
    }
    if(strcmp(cmd_name,"reconfm")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "reconfm" RESET_DISPLAY "     :~ Reconfigure the master node.\n");
        printf("|   --list             ~ List out all the available configurations\n");
        printf("|   --conf NEW_CONFIG  ~ Specify the new_configuration.\n");
    }
    if(strcmp(cmd_name,"sleep")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "sleep" RESET_DISPLAY "       :~ Turn off all the nodes (management and compute) of the cluster.\n"); 
    }
    if(strcmp(cmd_name,"wakeup")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "wakeup" RESET_DISPLAY "      :~ Wake up the cluster nodes.\n");
        printf("|   --minimal      ~ Turn on the management nodes of the cluster.\n");
        printf("|   --all          ~ Turn on the management and compute nodes of the cluster.\n");
    }
    if(strcmp(cmd_name,"nfsup")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "nfsup" RESET_DISPLAY "      :~ Increase the NFS volume. " HIGH_CYAN_BOLD "Only for Huaweicloud, Microsoft Azure and GCP." RESET_DISPLAY "\n");
        printf("|   --vol NEW_VOLUME  ~ Specify a positive number as the new volume.\n");
    }
    if(strcmp(cmd_name,"destroy")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "destroy" RESET_DISPLAY "     :~ *DESTROY* the whole cluster - including all the resources & data.\n");
        printf("|   --force        ~ Do force-destroy " WARN_YELLO_BOLD "( DANGER! )" RESET_DISPLAY ".\n");
    }
    if(strcmp(cmd_name,"payment")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "payment" RESET_DISPLAY "     :~ Switch the payment method between on-demand and monthly.\n");
        printf("|   --od          ~ Switch to On-Demand method.\n");
        printf("|   --month       ~ Switch to Monthly-pay method.\n");
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ VI   . Cluster User Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"userman")==0||strcmp(cmd_name,"all")==0){
        print_usrmgr_info();
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ VII  . Cluster Data Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"dataman")==0||strcmp(cmd_name,"all")==0){
        print_datamgr_info();
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ VIII . Cluster App Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"appman")==0||strcmp(cmd_name,"all")==0){
        print_appmgr_info();
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ IX   . Cluster Job Management:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"jobman")==0||strcmp(cmd_name,"all")==0){
        print_jobmgr_info();
    }
    if(strcmp(cmd_name,"all")==0){
        printf(GENERAL_BOLD "\n+ X    . Others:" RESET_DISPLAY "\n");
    }
    if(strcmp(cmd_name,"about")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "about" RESET_DISPLAY "       :~ About this software and HPC-NOW project.\n");
    }
    if(strcmp(cmd_name,"version")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "version" RESET_DISPLAY "     :~ Display the version info.\n");
    }
    if(strcmp(cmd_name,"license")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "license" RESET_DISPLAY "     :~ Read the terms of the MIT License.\n");
        printf("|   --print           ~ Print out the license terms.\n");
        printf("|   --read            ~ Read the license terms.\n");
    }
    if(strcmp(cmd_name,"repair")==0||strcmp(cmd_name,"all")==0){
        printf("|  " HIGH_GREEN_BOLD "repair" RESET_DISPLAY "      :~ Try to repair the hpcopr core components.\n");
    }
    printf("\n");
    printf(GENERAL_BOLD "<> visit:" RESET_DISPLAY " https://www.hpc-now.com " GENERAL_BOLD "<> mailto:" RESET_DISPLAY " info@hpc-now.com\n");
}

void print_header(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    printf(GENERAL_BOLD "|   /HPC->  Welcome to HPC-NOW Cluster Operator! Version: %s\n",CORE_VERSION_CODE);
    printf("|\\\\/ ->NOW  %d-%d-%d %d:%d:%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd  LICENSE: MIT\n" RESET_DISPLAY "\n");
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
    printf("| Therefore, we also made this software public under the MIT License.\n");
    printf("| Please check the source code here: https://gitee.com/zhenrong-wang/hpc-now/\n");
    printf("| If you encounter any issues about this software, please feel free to contact us\n");
    printf("| via info@hpc-now.com or other channels.\n");
    printf("| Let's build this open source cloud HPC platform together!\n");
    print_tail();
}

int read_license(char* option){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%sMIT.LICENSE",NOW_LIC_DIR,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"notepad %s",filename_temp);
        system(cmdline);
        return 0;
#else
        if(strcmp(option,"print")==0){
            sprintf(cmdline,"%s %s",CAT_FILE_CMD,filename_temp);
        }
        else{
            sprintf(cmdline,"more %s",filename_temp);
        }
        system(cmdline);
        return 0;
#endif
    }
#ifdef _WIN32
    sprintf(cmdline,"curl -s %s",URL_LICENSE);
    system(cmdline);
    return 0;
#else
    if(strcmp(option,"print")==0){
        sprintf(cmdline,"curl -s %s",URL_LICENSE);
    }
    else{
        sprintf(cmdline,"curl -s %s | more",URL_LICENSE);
    }
    system(cmdline);
    return 0;
#endif
}

void print_usrmgr_info(void){
    printf("| Usage:~ hpcopr " HIGH_GREEN_BOLD "userman" RESET_DISPLAY " --ucmd USER_CMD [ KEY_WORD1 KEY_STRING1 ] ...\n");
    printf("| * The cluster must be in running state (minimal or all). *\n");
    printf("|   --ucmd list    ~ List all the current cluster users.\n");
    printf("|   --ucmd add     ~ Add a user to the cluster. By default, added users are enabled.\n");
    printf("|     -u     USERNAME   ~ Username string (A-Z | a-z | - , Length %d-%d)\n",USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
    printf("|     -p     PASSWORD   ~ password string.\n");
    printf("|   --ucmd delete  ~ Delete a user from the cluster.\n");
    printf("|     -u     USERNAME   ~ Username to be deleted.\n");
    printf("|   --ucmd enable  ~ Enable a *disabled* user. Enabled users can run HPC workloads.\n");
    printf("|     -u     USERNAME   ~ Username to be enabled.\n");
    printf("|   --ucmd disable ~ Disable a user. Disabled users still can access the cluster.\n");
    printf("|     -u     USERNAME   ~ Username to be disabled.\n");
    printf("|   --ucmd passwd  ~ Change user's password.\n");
    printf("|     -u     USERNAME   ~ An existed username.\n");
    printf("|     -p     PASSWORD   ~ New password string.\n");
}

void print_datamgr_info(void){
    printf("| Usage:~ hpcopr " HIGH_GREEN_BOLD "dataman" RESET_DISPLAY " CMD_FLAG... [ KEY_WORD1 KEY_STRING1 ] ...\n");
    printf("| General Flags    :~ -r, -rf, --recursive, --force, -f.\n");
    printf("|    -s SOURCE_PATH  ~ Source path of the binary operations. e.g. cp\n");
    printf("|    -d DEST_PATH    ~ Destination path of binary operations. e.g. cp\n");
    printf("|    -t TARGET_PATH  ~ Target path of unary operations. e.g. ls\n");
    printf("| Bucket Operations:~ Transfer and manage data with the bucket.\n");
    printf("|   --dcmd put       ~ Upload a local file or folder to the bucket path.\n");
    printf("|   --dcmd get       ~ Download a bucket object(file or folder) to the local path.\n");
    printf("|   --dcmd copy      ~ Copy a bucket object to another folder/path.\n");
    printf("|   --dcmd list      ~ Show the object list of a specified folder/path.\n");
    printf("|   --dcmd delete    ~ Delete an object (file or folder) of the bucket.\n");
    printf("|   --dcmd move      ~ Move an existed object (file or folder) in the bucket.\n");
    printf("|    Example: hpcopr dataman --dcmd put -s ./foo -d /foo -u user1\n");
    printf("| Direct Operations:~ Transfer and manage data in the cluster storage.\n");
    printf("| * The cluster must be in running state (minimal or all). *\n");
    printf("|   --dcmd cp        ~ Remote copy between local and the cluster storage.\n");
    printf("|   --dcmd mv        ~ Move the remote files/folders in the cluster storage.\n");
    printf("|   --dcmd ls        ~ List the files/folders in the cluster storage.\n");
    printf("|   --dcmd rm        ~ Remove the files/folders in the cluster storage.\n");
    printf("|   --dcmd mkdir     ~ Make a directory in the cluster storage.\n");
    printf("|   --dcmd cat       ~ Print out a remote plain text file.\n");
    printf("|   --dcmd more      ~ Read a remote file.\n");
    printf("|   --dcmd less      ~ Read a remote file.\n");
    printf("|   --dcmd tail      ~ Streaming out a remote file dynamically.\n");
    printf("|   --dcmd rput      ~ Upload a *remote* file or folder to the bucket path.\n");
    printf("|   --dcmd rget      ~ Download a bucket object(file or folder) to the *remote* path.\n");
    printf("|     @h/ to specify the $HOME prefix of the cluster.\n");
    printf("|     @d/ to specify the /hpc_data/user_data prefix.\n");
    printf("|     @a/ to specify the /hpc_apps/ prefix, only for root or user1.\n");
    printf("|     @p/ to specify the public folder prefix " WARN_YELLO_BOLD "( INSECURE !)" RESET_DISPLAY ".\n");
    printf("|     @R/ to specify the / prefix, only for root or user1.\n");
    printf("|     @t/ to specify the /tmp prefix.\n");
    printf("|    Example: hpcopr dataman --dcmd cp -s ~/foo/ -d @h/foo -r -u user1\n");
}

void print_appmgr_info(void){
    printf("| Usage:~ hpcopr " HIGH_GREEN_BOLD "appman" RESET_DISPLAY " --acmd APP_CMD CMD_FLAG [ KEY_WORD1 KEY_STRING1 ] ...\n");
    printf("| * The cluster must be in running state (minimal or all). *\n");
    printf("| * -u USERNAME    ~ A valid user name. Use 'root' for all users.\n");
    printf("| *                ~ " WARN_YELLO_BOLD "Admin or Operator role is required for root." RESET_DISPLAY "\n");
    printf("| * Optional: --inst LOCATION   ~ A location for the scripts of app management.\n");
    printf("| * Optional: --repo LOCATION   ~ A location for the HPC package repository.\n");
    printf("|   --acmd store   ~ List out the apps in store.\n");
    printf("|   --acmd avail   ~ List out all the installed apps.\n");
    printf("|   --acmd check   ~ Check whether an app is available.\n");
    printf("|     --app  APP_NAME   ~ The app name to be installed.\n");
    printf("|   --acmd install ~ Install an app to all users or a specified user.\n");
    printf("|     --app  APP_NAME   ~ The app name to be installed.\n");
    printf("|   --acmd build   ~ Compile and build an app to all users or a specified user.\n");
    printf("|     --app  APP_NAME   ~ The app name to be compiled and built.\n");
    printf("|   --acmd remove  ~ Remove an app from the cluster.\n");
    printf("|     --app  APP_NAME   ~ The app name to be removed.\n");
    printf("|   --acmd update-conf  ~ Update the locations of app scripts and package repository.\n");
    printf("|                       ~ You need to specify new location(s).\n");
    printf("|   --acmd check-conf   ~ Display the locations of app scripts and package repository.\n");
}

void print_jobmgr_info(void){
    printf("| Usage:~ hpcopr " HIGH_GREEN_BOLD "jobman" RESET_DISPLAY " --jcmd APP_CMD [ KEY_WORD1 KEY_STRING1 ] ...\n");
    printf("| * The cluster must be in running state (minimal or all). *\n");
    printf("| * -u USERNAME    ~ A valid user name. " WARN_YELLO_BOLD "The root user CANNOT submit jobs." RESET_DISPLAY "\n");
    printf("|   --jcmd submit  ~ Submit a job to the cluster.\n");
    printf("|     --app   APP_NAME         ~ The app name for this job.\n");
    printf("|     --nn    NODE_NUM         ~ The number of compute nodes for this job.\n");
    printf("|     --tn    THREADS_PER_NODE ~ Threads per node for this job.\n");
    printf("|     --jname JOB_NAME         ~ Job name. 'y' for the default name.\n");
    printf("|     --jtime DURATION_HOURS   ~ Duration hours. 'y' for INFINITE.\n");
    printf("|     --jexec EXECUTABLE_NAME  ~ Executable name for this job.\n");
    printf("|     --jdata DATA_DIRECTORY   ~ The data directory for this job.\n");
    printf("|                              ~ MUST use @d/ or @p/ as the prefix.\n");
    printf("|     --echo                   ~ View the job console output.\n");
    printf("|   --jcmd list    ~ List out all the jobs.\n");
    printf("|   --jcmd cancel  ~ Cancel a job with specified ID\n");
    printf("|     --jid   JOB_ID           ~ A valid job ID.\n");
}

void list_all_commands(void){
    printf(GENERAL_BOLD " 0.  GET-STARTED:" RESET_DISPLAY HIGH_GREEN_BOLD " envcheck" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 1.  Multi-Cluster Management: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD "     new-cluster  ls-clusters \n");
    printf("     switch  glance  refresh \n");
    printf("     export  import  remove  exit-current " RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 2.  Global Management: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD "     help  usage  monman  history  syserr \n");
    printf("     ssh  rdp \n");
    printf("     set-tf configloc  showloc  showmd5  resetloc \n");
    printf("     encrypt decrypt " RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 3.  Cluster Initialization: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD "     cloud-info  rotate-key  get-conf  edit-conf \n");
    printf("     rm-conf  init  rebuild " RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 4.  Cluster Management: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD "     vault  graph  viewlog " RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 5.  Cluster Operation: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD "     delc     addc     shutdownc  turnonc \n");
    printf("     reconfc  reconfm  nfsup \n");
    printf("     sleep    wakeup   destroy\n");
    printf("     payment" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 6.  User Mgmt: " RESET_DISPLAY HIGH_GREEN_BOLD "userman" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 7.  Data Mgmt: " RESET_DISPLAY HIGH_GREEN_BOLD "dataman" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 8.  App Mgmt : " RESET_DISPLAY HIGH_GREEN_BOLD "appman" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 9.  Job Mgmt : " RESET_DISPLAY HIGH_GREEN_BOLD"jobman" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD " 10. Others: " RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD"     about  version  license  repair " RESET_DISPLAY "\n");
}