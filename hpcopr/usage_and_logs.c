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
#include "usage_and_logs.h"

int view_system_logs(char* logfile, char* view_option){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(logfile)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the specified log. Either you haven't init your first\n");
        printf("|          cluster, or there are internal errors. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    sprintf(cmdline,"%s %s %s.tmp %s",COPY_FILE_CMD,logfile,logfile,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    if(strcmp(view_option,"more")==0){
        sprintf(cmdline,"more %s.tmp",logfile);
    }
    else{
        sprintf(cmdline,"%s %s.tmp",CAT_FILE_CMD,logfile);
    }
    system(cmdline);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " You can use '" HIGH_GREEN_BOLD ">" RESET_DISPLAY "' to store the latest logs to a file.\n");
    printf("|          Example: " HIGH_GREEN_BOLD "hpcopr history > hpcopr_hist.csv" RESET_DISPLAY " CSV is highly recommended.\n");
    printf("|          Example: " HIGH_GREEN_BOLD "hpcopr usage > hpcopr_usg.csv" RESET_DISPLAY " CSV is highly recommended.\n");
    printf("|          Example: " HIGH_GREEN_BOLD "hpcopr syserr > hpcopr_syserr.txt" RESET_DISPLAY "\n");
    return 0;
}

int write_operation_log(char* cluster_name, char* operation_logfile, char* operation, char* description, int runflag){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    FILE* file_p=fopen(operation_logfile,"a+");
    if(file_p==NULL){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to write operation log to the records. The cluster operation may\n");
        printf("|          not be affected, but will not be recorded to your system.\n" RESET_DISPLAY);
        return -1;
    }
    fprintf(file_p,"%d-%d-%d,%d:%d:%d,%s,%s,%s,%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec,cluster_name,operation,description,runflag);
    fclose(file_p);
    return 0;
}