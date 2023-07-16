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
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "general_funcs.h"
#include "time_process.h"
#include "cluster_general_funcs.h"
#include "general_print_info.h"
#include "appman.h"

int app_list(char* workdir, char* option, char* user_name, char* sshkey_dir){
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char dirname_temp[DIR_LENGTH]="";
    char pub_apps_cache[FILENAME_LENGTH]="";
    char priv_apps_cache[FILENAME_LENGTH]="";
    char string_temp[128]="";
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    get_cluster_name(cluster_name,workdir);
    if(strcmp(option,"installed")==0){
        sprintf(dirname_temp,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
        sprintf(pub_apps_cache,"%s%spub_apps.reg",dirname_temp,PATH_SLASH);
        sprintf(priv_apps_cache,"%s%spriv_apps.reg",dirname_temp,PATH_SLASH);
        remote_copy(workdir,sshkey_dir,pub_apps_cache,"/usr/hpc-now/.public_apps.reg","root","get","",0);
        remote_copy(workdir,sshkey_dir,priv_apps_cache,"/usr/hpc-now/.private_apps.reg","root","get","",0);
        if(file_exist_or_not(pub_apps_cache)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the app registry of this cluster." RESET_DISPLAY "\n");
            return -1;
        }
        file_p=fopen(pub_apps_cache,"r");
        printf("\n|       +- Installed Apps ~ Public:\n");
        while(!feof(file_p)){
            fgetline(file_p,string_temp);
            if(strlen(string_temp)==0){
                continue;
            }
            printf("|          +- %s\n",string_temp);
        }
        fclose(file_p);
        if(file_exist_or_not(priv_apps_cache)==0){
            file_p=fopen(priv_apps_cache,"r");
            printf("|       +- Installed Apps ~ Private:\n");
            while(!feof(file_p)){
                fgetline(file_p,string_temp);
                if(strlen(string_temp)==0){
                    continue;
                }
                if(strcmp(user_name,"root")==0){
                    printf("|          +- %s\n",string_temp);
                }
                else{
                    if(contain_or_not(string_temp,user_name)==0){
                        printf("|          +- %s\n",string_temp);
                    }
                }
            }
            fclose(file_p);
        }
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,pub_apps_cache,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,priv_apps_cache,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
    }
    else{
        remote_exec_general(workdir,sshkey_dir,"root","hpcmgr install list","-t",0,1);
    }
    return 0;
}

int app_operation(char* workdir, char* user_name, char* option, char* app_name, char* sshkey_dir){
    if(strcmp(option,"build")!=0&&strcmp(option,"install")!=0&&strcmp(option,"remove")!=0){
        return -3;
    }
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag=0;
    sprintf(remote_commands,"nohup hpcmgr %s %s > /tmp/app_operation_%s.log 2>&1 &",option,app_name,user_name);
    run_flag=remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,2);
    if(run_flag!=0){
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ] App operation is in progress. Detailed info as below.\n");
    printf("|          You can press 'ctrl C' to stop viewing the log.\n" RESET_DISPLAY "\n");
    sprintf(remote_commands,"tail -f /tmp/app_operation_%s.log",user_name);
    remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"-t",0,1);
    return 0;
}