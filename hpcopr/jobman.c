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
#include "userman.h"
#include "jobman.h"

int get_job_info(int argc, char** argv, char* workdir, char* user_name, char* sshkey_dir, char* crypto_keyfile, jobinfo* job_info, int batch_flag_local){
    char string_temp[128]="";
    char node_num_string[128]="";
    char node_cores_string[128]="";
    char duration_hours_string[128]="";
    char cluster_node_num_string[4]="";
    char cluster_node_cores_string[4]="";
    char filename_temp[FILENAME_LENGTH]="";
    char app_name[128]="";
    char exec_name[128]="";
    char job_data[256]="";
    char job_data_final[256]="";
    char job_name[128]="";
    int cluster_node_num=0;
    int cluster_node_cores=0;
    int specified_node_num=0;
    int specified_node_cores=0;
    int duration_hours=0;
    int run_flag;
    int num_temp=0;
    int i;

    get_state_value(workdir,"total_compute_nodes:",cluster_node_num_string);
    get_state_value(workdir,"compute_node_cores:",cluster_node_cores_string);
    cluster_node_num=string_to_positive_num(cluster_node_num_string);
    cluster_node_cores=string_to_positive_num(cluster_node_cores_string);

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Current cluster cores status:\n");
    remote_exec_general(workdir,sshkey_dir,user_name,"tail -n 1 /hpc_data/cluster_data/mon_cores.dat","",0,2,"","");

    if(strcmp(user_name,"root")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user cannot submit jobs, please specify another user." RESET_DISPLAY "\n");
        hpc_user_list(workdir,crypto_keyfile,0);
        return -3;
    }
    if(cmd_keyword_check(argc,argv,"--app",app_name)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] App name not specified. Use --app APP_NAME ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please specify an app for this job.\n");
        app_list(workdir,"installed",user_name,"",sshkey_dir,"","");
        printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
        fflush(stdin);
        scanf("%127s",app_name);
        getchar();
    }
    snprintf(filename_temp,511,"%s%s.tmp%sapp_check.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH);
    app_list(workdir,"check",user_name,app_name,sshkey_dir,filename_temp,"");
    run_flag=find_multi_keys(filename_temp,"not available","","","","");
    if(run_flag>0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified app " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is invalid. Exit now." RESET_DISPLAY "\n",app_name);
        return -5;
    }
    if(cmd_keyword_check(argc,argv,"--nn",node_num_string)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Node num not specified. Use --nn NODE_NUM ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify compute node num (<=%d) for this job: ",cluster_node_num);
        fflush(stdin);
        scanf("%127s",node_num_string);
        getchar();
    }
    specified_node_num=string_to_positive_num(node_num_string);
    if(specified_node_num<1||specified_node_num>cluster_node_num){
        printf("[ FATAL: ] The specified node num %s is invalid. Exit now.\n",node_num_string);
        return -5;
    }

    if(cmd_keyword_check(argc,argv,"--tn",node_cores_string)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Threads-per-node not specified. Use --tn THREADS_PER_NODE ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify threads per node (<=%d) for this job: ",cluster_node_cores);
        fflush(stdin);
        scanf("%127s",node_cores_string);
        getchar();
    }
    specified_node_cores=string_to_positive_num(node_cores_string);
    if(specified_node_cores<1||specified_node_num>cluster_node_cores){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified threads " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " per node is invalid. Exit now." RESET_DISPLAY "\n",node_cores_string);
        return -5;
    }

    if(cmd_keyword_check(argc,argv,"--jname",job_name)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Job name not specified. Use --jname JOB_NAME ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(WARN_YELLO_BOLD "[ -WARN- ] No job name specified. Input y or yes to use the default (%s-job)." RESET_DISPLAY "\n",user_name);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Or, input a string as the job name:" RESET_DISPLAY" ");
        fflush(stdin);
        scanf("%127s",string_temp);
        getchar();
        if(strcmp(string_temp,"y")==0||strcmp(string_temp,"yes")==0||strcmp(string_temp,"Y")==0||strcmp(string_temp,"YES")==0||strcmp(string_temp,"Yes")==0){
            snprintf(job_name,127,"%s-job",user_name);
        }
        else{
            strcpy(job_name,string_temp);
        }
        reset_nstring(string_temp,128);
    }

    if(cmd_keyword_check(argc,argv,"--jtime",duration_hours_string)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Duration hours not specified. Use --jtime JOB_TIME ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(WARN_YELLO_BOLD "[ -WARN- ] No duration hours specified. Input y or yes to use the default (INFINITE)." RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Or, input a positive number: ");
        fflush(stdin);
        scanf("%127s",string_temp);
        getchar();
        if(strcmp(string_temp,"y")==0||strcmp(string_temp,"yes")==0||strcmp(string_temp,"Y")==0||strcmp(string_temp,"YES")==0||strcmp(string_temp,"Yes")==0){
            duration_hours=400000000;
        }
        else{
            num_temp=string_to_positive_num(string_temp);
            if(num_temp<1){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified duration hours %s is invalid. Using the default." RESET_DISPLAY "\n",string_temp);
                duration_hours=400000000;
            }
            else{
                duration_hours=num_temp;
            }
        }
        reset_nstring(string_temp,128);
    }
    else{
        if(strcmp(duration_hours_string,"y")==0||strcmp(duration_hours_string,"yes")==0||strcmp(duration_hours_string,"Y")==0||strcmp(duration_hours_string,"YES")==0||strcmp(duration_hours_string,"Yes")==0){
            duration_hours=400000000;
        }
        else{
            num_temp=string_to_positive_num(duration_hours_string);
            if(num_temp<1){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified duration hours %s is incorrect. Using the default." RESET_DISPLAY "\n",duration_hours_string);
                duration_hours=400000000;
            }
            else{
                duration_hours=num_temp;
            }
        }
    }
    
    if(cmd_keyword_check(argc,argv,"--jexec",exec_name)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Job execution not specified. Use --jexec JOB_EXEC ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please specify an executable: ");
        fflush(stdin);
        scanf("%127s",exec_name);
        getchar();
    }

    if(cmd_keyword_ncheck(argc,argv,"--jdata",job_data,256)!=0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Job data directory not specified. Use --jdata JOB_DIR." RESET_DISPLAY "\n");
            return 17;
        }
        printf(WARN_YELLO_BOLD "[ -WARN- ] No data directory specified. Please specify a remote path." RESET_DISPLAY "\n");
        printf("|          *MUST* use " HIGH_CYAN_BOLD "@d/" RESET_DISPLAY " (user data) or " HIGH_CYAN_BOLD "@p/" RESET_DISPLAY " (public data) as the prefix.\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        fflush(stdin);
        scanf("%255s",job_data);
        getchar();
    }

    reset_nstring(string_temp,128);
    for(i=0;i<3;i++){
        *(string_temp+i)=*(job_data+i);
    }
    if(strcmp(string_temp,"@d/")!=0&&strcmp(string_temp,"@p/")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified data directory " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is invalid. Exit now." RESET_DISPLAY "\n",job_data);
        return -5;
    }
    else{
        direct_path_check(job_data,user_name,job_data_final);
    }
   
    strcpy(job_info->app_name,app_name);
    job_info->node_num=specified_node_num;
    job_info->tasks_per_node=specified_node_cores;
    strcpy(job_info->job_name,job_name);
    job_info->duration_hours=duration_hours;
    strcpy(job_info->job_exec,exec_name);
    strcpy(job_info->job_data,job_data_final);
    if(cmd_flag_check(argc,argv,"--echo")==0){
        strcpy(job_info->echo_flag,"true");
    }
    else{
        strcpy(job_info->echo_flag,"false");
    }
    
    printf("\n");
    printf(GENERAL_BOLD "[ -INFO- ] Job Information Summary:" RESET_DISPLAY "\n");
    printf("|          App Name       : %s\n",job_info->app_name);
    printf("|          Job Nodes      : %d\n",job_info->node_num);
    printf("|          Cores Per Node : %d\n",job_info->tasks_per_node);
    printf("|          Job Name       : %s\n",job_info->job_name);
    printf("|          Duration Hours : %d\n",job_info->duration_hours);
    printf("|          Job Executable : %s\n",job_info->job_exec);
    printf("|          Data Directory : %s\n",job_info->job_data);
    printf("|          Console Output : %s\n",job_info->echo_flag);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The job will be sent to the cluster.\n\n");
    return 0;
}

int job_submit(char* workdir, char* user_name, char* sshkey_dir, jobinfo* job_info){
    char remote_commands[CMDLINE_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char dirname_temp[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char remote_filename_temp[FILENAME_LENGTH]="";
    int i,run_flag=0;
    snprintf(dirname_temp,383,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    snprintf(cmdline,2047,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(filename_temp,511,"%s%sjob_submit_info.tmp",dirname_temp,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"App Name       ::%s\n",job_info->app_name);
    fprintf(file_p,"Job Nodes      ::%d\n",job_info->node_num);
    fprintf(file_p,"Cores Per Node ::%d\n",job_info->tasks_per_node);
    fprintf(file_p,"Total Cores    ::%d\n",job_info->tasks_per_node*job_info->node_num);
    fprintf(file_p,"Job Name       ::%s\n",job_info->job_name);
    fprintf(file_p,"Duration Hours ::%d\n",job_info->duration_hours);
    fprintf(file_p,"Job Executable ::%s\n",job_info->job_exec);
    fprintf(file_p,"Data Directory ::%s",job_info->job_data);
    fclose(file_p);
    snprintf(remote_filename_temp,511,"/tmp/job_submit_info_%s.tmp",user_name);
    remote_copy(workdir,sshkey_dir,filename_temp,remote_filename_temp,user_name,"put","",0);
    snprintf(remote_commands,2047,"hpcmgr submit %s",remote_filename_temp);
    run_flag=remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,2,"","");
    if(run_flag!=0){
        return 1;
    }
    snprintf(cmdline,2047,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    if(strcmp(job_info->echo_flag,"true")==0){
        printf("\n");
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You can press " WARN_YELLO_BOLD "Ctrl C" RESET_DISPLAY " to stop displaying the job output.\n");
        for(i=0;i<5;i++){
            printf(GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " Will display the job output in %d seconds ... \r",5-i);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
        snprintf(filename_temp,511,"%s/%s_run.log",job_info->job_data,job_info->job_name);
        snprintf(remote_commands,2047,"tail -f %s",filename_temp);
        remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,2,"","");
    }
    else{
        printf("[ -DONE- ] You can now log into the cluster and view the job output.\n");
    }
    return 0;
}

int job_list(char* workdir, char* user_name, char* sshkey_dir){
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char dirname_temp[DIR_LENGTH]="";
    char job_list_cache[FILENAME_LENGTH]="";
    char string_temp[LINE_LENGTH_SHORT]="";
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    get_cluster_name(cluster_name,workdir);
    snprintf(dirname_temp,383,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    snprintf(cmdline,2047,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(job_list_cache,511,"%s%sjob_list_%s.txt",dirname_temp,PATH_SLASH,cluster_name);
    remote_exec_general(workdir,sshkey_dir,user_name,"sacct","",0,3,job_list_cache,NULL_STREAM);
    if(file_exist_or_not(job_list_cache)!=0){
        return -1;
    }
    printf("\n");
    file_p=fopen(job_list_cache,"r");
    while(!feof(file_p)){
        fngetline(file_p,string_temp,LINE_LENGTH_SHORT);
        if(strlen(string_temp)==0){
            continue;
        }
        printf("%s\n",string_temp);
    }
    fclose(file_p);
    snprintf(cmdline,2047,"%s %s %s",DELETE_FILE_CMD,job_list_cache,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int job_cancel(char* workdir, char* user_name, char* sshkey_dir, char* job_id, int batch_flag_local){
    char remote_commands[CMDLINE_LENGTH]="";
    char string_temp[256]="";
    if(strlen(job_id)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Job ID not specified. Use --jid JOB_ID ." RESET_DISPLAY "\n");
            return 17;
        }
        job_list(workdir,user_name,SSHKEY_DIR);
        prompt_to_input("Please input a jobID from the list above.",string_temp,batch_flag_local);
        snprintf(remote_commands,2047,"scancel --verbose %s",job_id);
    }
    else{
        snprintf(remote_commands,2047,"scancel --verbose %s",job_id);
    }
    return remote_exec_general(workdir,sshkey_dir,user_name,remote_commands,"",0,3,"","");
}