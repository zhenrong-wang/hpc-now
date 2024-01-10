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

int appman_update_conf(char* workdir, char* crypto_keyfile, const char* new_inst_loc, const char* new_repo_loc, char* sshkey_dir, char* std_redirect){
    char cluster_role[16]="";
    char cluster_role_ext[16]="";
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag;
    cluster_role_detect(workdir,cluster_role,cluster_role_ext,16);
    if(strcmp(cluster_role,"opr")!=0&&strcmp(cluster_role,"admin")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Only cluster operator or administrator is able to configure the app manager." RESET_DISPLAY "\n");
        return 1;
    }
    if(strlen(new_inst_loc)==0&&strlen(new_repo_loc)==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " No new locations specified. Nothing changed.\n");
        return 3;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updating the location(s) now ...\n");
        snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr appman-conf-update --inst=%s --repo=%s",new_inst_loc,new_repo_loc);
        run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,"root",remote_commands,"-t",0,3,std_redirect,NULL_STREAM);
    }
    if(run_flag==0){
        return 0;
    }
    else{
        return 5;
    }
}

int appman_check_conf(char* workdir, char* crypto_keyfile, char* user_name, char* sshkey_dir){
    int run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,user_name,"hpcmgr appman-conf-show","-t",0,3,"",NULL_STREAM);
    if(run_flag==0){
        return 0;
    }
    else{
        return 1;
    }
}

int app_list(char* workdir, char* crypto_keyfile, char* option, char* user_name, char* app_name, char* sshkey_dir, char* std_redirect, char* inst_loc){
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag=0;
    if(strcmp(option,"installed")==0){
        run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,user_name,"hpcmgr applist avail","-t",0,3,std_redirect,NULL_STREAM);
    }
    else if(strcmp(option,"check")==0){
        snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr applist check --app=%s",app_name);
        run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,user_name,remote_commands,"-t",0,3,std_redirect,NULL_STREAM);
    }
    else{
        snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr applist --inst=%s",inst_loc);
        run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,"root",remote_commands,"-t",0,3,std_redirect,NULL_STREAM);
    }
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int app_operation(char* workdir, char* crypto_keyfile, char* user_name, char* option, char* app_name, char* sshkey_dir, char* inst_loc, char* repo_loc){
    if(strcmp(option,"build")!=0&&strcmp(option,"install")!=0&&strcmp(option,"remove")!=0){
        return -3;
    }
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag=0;
    if(strcmp(option,"build")==0||strcmp(option,"install")==0){
        snprintf(remote_commands,CMDLINE_LENGTH-1,"nohup hpcmgr %s --app=%s --inst=%s --repo=%s > /hpc_apps/%s_apps/appman_%s.log 2>&1 &",option,app_name,inst_loc,repo_loc,user_name,app_name);
    }
    else{
        snprintf(remote_commands,CMDLINE_LENGTH-1,"nohup hpcmgr remove --app=%s --inst=%s > /hpc_apps/%s_apps/appman_%s.log 2>&1 &",app_name,inst_loc,user_name,app_name);
    }
    
    run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,user_name,remote_commands,"",0,2,"","");
    if(run_flag!=0){
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ] App operation is in progress. Detailed info as below.\n");
    printf("[  ****  ] You can press 'ctrl C' to stop viewing the log.\n" RESET_DISPLAY "\n");
    snprintf(remote_commands,CMDLINE_LENGTH-1,"tail -f /hpc_apps/%s_apps/appman_%s.log",user_name,app_name);
    run_flag=remote_exec_general(workdir,crypto_keyfile,sshkey_dir,user_name,remote_commands,"-t",0,1,"","");
    if(run_flag!=0){
        return 3;
    }
    return 0;
}