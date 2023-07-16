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
#include "jobman.h"

int get_job_info(int argc, char** argv, char* workdir, char* user_name, char* sshkey_dir, char* crypto_keyfile, jobinfo* job_info){
    char string_temp[128]="";
    char pub_app_reg[FILENAME_LENGTH]="";
    char priv_app_reg[FILENAME_LENGTH]="";
    char compute_node_num_string[4]="";
    char compute_cores_string[4]="";
    int compute_node_num;
    int compute_cores;
    if(strcmp(user_name,"root")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user cannot submit jobs, please specify another user." RESET_DISPLAY "\n");
        hpc_user_list(workdir,crypto_keyfile,0);
        return -3;
    }
    if(cmd_keyword_check(argc,argv,"--app",string_temp)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please specify an app for this job.\n");
//        app_list(workdir,"installed",user_name,sshkey_dir);
        printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
        fflush(stdin);
        scanf("%s",string_temp);
        getchar();
    }
    
    if(cmd_keyword_check(argc,argv,"--nn",string_temp)!=0){
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify the node num for this job:");
        graph(workdir,crypto_keyfile,1);
        fflush(stdin);
        scanf("%s",string_temp);
        getchar();
    }
    if(string_to_positive_num(string_temp)<1){
        printf("[ FATAL: ] The specified node num %s is invalid. Exit now.\n",string_temp);
        return -5;
    }
    return 0;
}

int job_submit(char* workdir, char* user_name, char* sshkey_dir, jobinfo* job_info){
    char remote_commands[CMDLINE_LENGTH]="";
    sprintf(remote_commands,"hpcmgr job_submit %s %d %d %s %d %s %s",job_info->app_name,job_info->node_num,job_info->tasks_per_node,job_info->job_name,job_info->duration_hours,job_info->job_exec,job_info->job_data);
    return remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,2,"","");
}

int job_list(char* workdir, char* user_name, char* sshkey_dir){
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char dirname_temp[DIR_LENGTH]="";
    char job_list_cache[FILENAME_LENGTH]="";
    char string_temp[256]="";
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    get_cluster_name(cluster_name,workdir);
    sprintf(dirname_temp,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    sprintf(job_list_cache,"%s%sjob_list_%s.txt",dirname_temp,PATH_SLASH,cluster_name);
    remote_exec_general(workdir,sshkey_dir,user_name,"sacct","",0,3,job_list_cache,NULL_STREAM);
    if(file_exist_or_not(job_list_cache)!=0){
        return -1;
    }
    printf("\n");
    file_p=fopen(job_list_cache,"r");
    while(!feof(file_p)){
        fgetline(file_p,string_temp);
        if(strlen(string_temp)==0){
            continue;
        }
        printf("%s\n",string_temp);
    }
    fclose(file_p);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,job_list_cache,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int job_cancel(char* workdir, char* user_name, char* sshkey_dir, char* job_id){
    char remote_commands[CMDLINE_LENGTH]="";
    sprintf(remote_commands,"scancel --verbose %s",job_id);
    return remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,3,"","");
}