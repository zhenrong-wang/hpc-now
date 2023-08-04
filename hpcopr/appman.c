/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
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

int app_list(char* workdir, char* option, char* user_name, char* app_name, char* sshkey_dir, char* std_redirect){
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag=0;
    if(strcmp(option,"installed")==0){
        run_flag=remote_exec_general(workdir,sshkey_dir,user_name,"hpcmgr applist avail","-t",0,3,std_redirect,NULL_STREAM);
    }
    else if(strcmp(option,"check")==0){
        sprintf(remote_commands,"hpcmgr applist check %s",app_name);
        run_flag=remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"-t",0,3,std_redirect,NULL_STREAM);
    }
    else{
        run_flag=remote_exec_general(workdir,sshkey_dir,"root","hpcmgr applist","-t",0,3,std_redirect,NULL_STREAM);
    }
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
    /*char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char dirname_temp[DIR_LENGTH]="";
    char pub_apps_cache[FILENAME_LENGTH]="";
    char priv_apps_cache[FILENAME_LENGTH]="";
    char string_temp[128]="";
    char app_name_ext[128]="";
    char user_name_ext[128]="";
    char user_name_temp[128]="";
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    int check_flag=0;
    int run_flag=0;
    get_cluster_name(cluster_name,workdir);
    if(strcmp(option,"installed")==0||strcmp(option,"check")==0){
        sprintf(dirname_temp,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
        sprintf(pub_apps_cache,"%s%spub_apps_%s.reg",dirname_temp,PATH_SLASH,cluster_name);
        sprintf(priv_apps_cache,"%s%spriv_apps_%s.reg",dirname_temp,PATH_SLASH,cluster_name);
        remote_copy(workdir,sshkey_dir,pub_apps_cache,"/usr/hpc-now/.public_apps.reg","root","get","",0);
        remote_copy(workdir,sshkey_dir,priv_apps_cache,"/usr/hpc-now/.private_apps.reg","root","get","",0);
        if(file_exist_or_not(pub_apps_cache)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the app registry of this cluster." RESET_DISPLAY "\n");
            return -1;
        }
        sprintf(app_name_ext,"< %s >",app_name);
        sprintf(user_name_ext,"< %s >",user_name);
        file_p=fopen(pub_apps_cache,"r");
        if(strcmp(option,"installed")==0){
            printf("\n|       +- Installed Apps ~ Public:\n");
        }
        while(!feof(file_p)){
            fgetline(file_p,string_temp);
            if(strlen(string_temp)==0){
                continue;
            }
            if(strcmp(option,"installed")==0){
                printf("|          +- %s\n",string_temp);
            }
            else{
                if(contain_or_not(string_temp,app_name_ext)==0){
                    check_flag=2;
                    break;
                }
            }
        }
        fclose(file_p);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,pub_apps_cache,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
        if(strcmp(option,"check")==0&&check_flag==2){
            if(check_silent_flag==1){
                printf("[ -INFO- ] The app %s is available for all users.\n",app_name);
            }
            return 4;
        }
        if(file_exist_or_not(priv_apps_cache)==0){
            if(strcmp(user_name,"root")==0){
                check_flag=4;
            }
            else{
                check_flag=6;
            }
            file_p=fopen(priv_apps_cache,"r");
            if(strcmp(option,"installed")==0){
                printf("|       +- Installed Apps ~ Private:\n");
            }
            while(!feof(file_p)){
                fgetline(file_p,string_temp);
                if(strlen(string_temp)==0){
                    continue;
                }
                if(strcmp(user_name,"root")==0){
                    if(strcmp(option,"installed")==0){
                        printf("|          +- %s\n",string_temp);
                    }
                    else{
                        if(contain_or_not(string_temp,app_name_ext)==0){
                            get_seq_string(string_temp,' ',5,user_name_temp);
                            check_flag=5;
                        }
                    }
                }
                else{
                    if(contain_or_not(string_temp,user_name_ext)==0){
                        if(strcmp(option,"installed")==0){
                            printf("|          +- %s\n",string_temp);
                        }
                        if(contain_or_not(string_temp,app_name_ext)==0){
                            check_flag=7;
                        }
                    }
                }
            }
            fclose(file_p);
            sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,priv_apps_cache,SYSTEM_CMD_REDIRECT_NULL);
            system(cmdline);
            if(strcmp(option,"check")==0&&check_silent_flag==1){
                if(check_flag==4){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The app " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is *NOT* available for any user." RESET_DISPLAY "\n",app_name);
                }
                else if(check_flag==6){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The app " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is *NOT* available for " WARN_YELLO_BOLD "%s" RESET_DISPLAY ".\n",app_name,user_name);
                }
                else if(check_flag==5){
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The app " HIGH_GREEN_BOLD "%s" RESET_DISPLAY " is available for " HIGH_GREEN_BOLD "%s" RESET_DISPLAY ".\n",app_name,user_name_temp);
                }
                else{
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The app " HIGH_GREEN_BOLD "%s" RESET_DISPLAY " is available for " HIGH_GREEN_BOLD "%s" RESET_DISPLAY ".\n",app_name,user_name);
                }
            }
        }
        if(strcmp(option,"check")==0){
            if(check_flag==5){
                return 5;
            }
            else if(check_flag==7){
                return 7;
            }
            else{
                return 6;
            }
        }
        else{
            return 0;
        }
    }
    else{
        run_flag=remote_exec_general(workdir,sshkey_dir,"root","hpcmgr applist","-t",0,1,"","");
        if(run_flag!=0){
            return 1;
        }
        else{
            return 0;
        }
    }*/
}

int app_operation(char* workdir, char* user_name, char* option, char* app_name, char* sshkey_dir){
    if(strcmp(option,"build")!=0&&strcmp(option,"install")!=0&&strcmp(option,"remove")!=0){
        return -3;
    }
    char remote_commands[CMDLINE_LENGTH]="";
    int run_flag=0;
    sprintf(remote_commands,"nohup hpcmgr %s %s > /hpc_apps/%s_apps/appman_%s.log 2>&1 &",option,app_name,user_name,app_name);
    run_flag=remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,2,"","");
    if(run_flag!=0){
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ] App operation is in progress. Detailed info as below.\n");
    printf("|          You can press 'ctrl C' to stop viewing the log.\n" RESET_DISPLAY "\n");
    sprintf(remote_commands,"tail -f /hpc_apps/%s_apps/appman_%s.log",user_name,app_name);
    run_flag=remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"-t",0,1,"","");
    if(run_flag!=0){
        return 3;
    }
    return 0;
}