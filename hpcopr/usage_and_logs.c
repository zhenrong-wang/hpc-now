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
#include "usage_and_logs.h"

int view_system_logs(char* logfile, char* view_option, char* export_dest){
    char cmdline[CMDLINE_LENGTH]="";
    char logfile_temp[FILENAME_LENGTH]="";
    int run_flag;
    if(file_exist_or_not(logfile)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the specified log. Either you haven't init your first\n");
        printf("|          cluster, or there are internal errors. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(logfile_temp,"%s.temp",logfile);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s %s",COPY_FILE_CMD,logfile,logfile_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    if(strcmp(view_option,"read")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"more %s",logfile_temp);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",CAT_FILE_CMD,logfile_temp);
    }
    system(cmdline);
    if(strlen(export_dest)==0){
        printf(GENERAL_BOLD "\n[ -DONE- ]" RESET_DISPLAY " You can also export the latest logs to a file.\n");
        printf("|          Example: " HIGH_GREEN_BOLD "hpcopr history -d history.csv" RESET_DISPLAY " .\n");
        printf("|          Example: " HIGH_GREEN_BOLD "hpcopr usage -d usage.csv" RESET_DISPLAY " .\n");
        printf("|          Example: " HIGH_GREEN_BOLD "hpcopr syserr -d syserr.txt" RESET_DISPLAY " .\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,logfile_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 0;
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s %s",MOVE_FILE_CMD,logfile_temp,export_dest,SYSTEM_CMD_REDIRECT);
        run_flag=system(cmdline);
        if(run_flag!=0){
            printf(FATAL_RED_BOLD "\n[ FATAL: ] Failed to export the log to " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " .\n",export_dest);
            printf("|          Please check the path. Exit now." RESET_DISPLAY "\n");
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,logfile_temp,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 1;
        }
        else{
            printf(GENERAL_BOLD "\n[ -DONE- ] The latest log has been successfully exported to " HIGH_GREEN_BOLD "%s"  RESET_DISPLAY" .\n",export_dest);
            return 0;
        }
    }
}

int write_operation_log(char* cluster_name, char* operation_logfile, int argc, char** argv, char* description, int runflag){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    char cmdline[CMDLINE_LENGTH]="";
    int i,j,k=0;
    for(i=0;i<argc;i++){
        for(j=0;j<strlen(argv[i]);j++){
            *(cmdline+k)=*(argv[i]+j);
            k++;
        }
        *(cmdline+k)=' ';
        k++;
    }
    FILE* file_p=fopen(operation_logfile,"a+");
    if(file_p==NULL){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to write operation log to the records. The cluster operation may\n");
        printf("|          not be affected, but will not be recorded to your system." RESET_DISPLAY "\n");
        return -1;
    }
    fprintf(file_p,"%d-%d-%d,%d:%d:%d,%s,%s,%s,%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec,cluster_name,cmdline,description,runflag);
    fclose(file_p);
    return 0;
}