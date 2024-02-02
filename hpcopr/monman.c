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
#include "monman.h"

/*
 * Return -1: Failed to get the workdir
 * Return -5: Cluster asleep and mon_data empty
 * Return -3: Cluster asleep but mon_data not empty
 * Return  1: Cluster not asleep, but seems remote copy failed
 * Return  0: Remote copy succeeded
 */
int get_cluster_mon_data(char* cluster_name, char* crypto_keyfile, char* sshkey_dir, char* mon_data_file){
    char workdir[DIR_LENGTH]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -7;
    }
    char mon_data_dir[DIR_LENGTH]="";
    snprintf(mon_data_dir,DIR_LENGTH-1,"%s%smon_data",HPC_NOW_ROOT_DIR,PATH_SLASH);
    if(folder_exist_or_not(mon_data_dir)!=0){
        if(mk_pdir(mon_data_dir)<0){
            return -1;
        }
    }
    sprintf(mon_data_file,"%s%smon_data_%s.csv",mon_data_dir,PATH_SLASH,cluster_name);
    if(cluster_asleep_or_not(workdir,crypto_keyfile)==0){
        if(file_empty_or_not(mon_data_file)<1){
            return -5;
        }
        else{
            return -3;
        }
    }
    remote_copy(workdir,crypto_keyfile,sshkey_dir,mon_data_file,"/hpc_data/cluster_data/mon_data.csv","root","get","",0);
    if(file_empty_or_not(mon_data_file)<1){
        strcpy(mon_data_file,"");
        return 1;
    }
    return 0;
}

int update_all_mon_data(char* cluster_registry, char* crypto_keyfile, char* sshkey_dir){
    char cluster_name_temp[64]="";
    char mon_data_file_temp[FILENAME_LENGTH]="";
    char registry_line[LINE_LENGTH_SHORT]="";
    int run_flag;
    int updated=0;
    FILE* file_p=fopen(cluster_registry,"r");
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fngetline(file_p,registry_line,LINE_LENGTH_SHORT);
        if(strlen(registry_line)==0){
            continue;
        }
        get_seq_nstring(cluster_name_temp,' ',4,cluster_name_temp,64);
        run_flag=get_cluster_mon_data(cluster_name_temp,crypto_keyfile,sshkey_dir,mon_data_file_temp);
        if(run_flag==0){
            updated++;
        }
    }
    return updated;
}

int valid_time_format_or_not(char* datetime_input, int extend_flag, char* date_string, char* time_string){
    char ymd[32]="";
    char year[8]="";
    char month[8]="";
    char mday[8]="";
    char hour_min[8]="";
    char hour[8]="";
    char min[8]="";
    int i=0;
    int year_num,month_num,day_num,hour_num,min_num;
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);

    int curr_year=time_p->tm_year+1900;
    int curr_month=time_p->tm_mon+1;
    int curr_mday=time_p->tm_mday;
    int curr_hour=time_p->tm_hour;
    int curr_min=time_p->tm_min;

    if(strlen(datetime_input)>16||strlen(datetime_input)<3){
        if(extend_flag==0){
            strcpy(date_string,"2000-1-1");
            strcpy(time_string,"0:0:0");
        }
        else{
            strcpy(date_string,"2049-12-31");
            strcpy(time_string,"23:59:59");
        }
        return -1;
    }
    get_seq_nstring(datetime_input,'@',1,ymd,32);
    get_seq_nstring(datetime_input,'@',2,hour_min,8);
    get_seq_nstring(ymd,'-',1,year,8);
    get_seq_nstring(ymd,'-',2,month,8);
    get_seq_nstring(ymd,'-',3,mday,8);
    get_seq_nstring(hour_min,':',1,hour,8);
    get_seq_nstring(hour_min,':',2,min,8);
    
    year_num=string_to_positive_num(year);
    month_num=string_to_positive_num(month);
    day_num=string_to_positive_num(mday);
    hour_num=string_to_positive_num(hour);
    min_num=string_to_positive_num(min);

    if(year_num<1){
        i=1;
        year_num=curr_year;
    }
    if(month_num<1||month_num>12){
        i=2;
        month_num=curr_month;
    }
    if(day_num<1||day_num>31){
        i=3;
        day_num=curr_mday;
    }
    if(day_num>28&&month_num==2){
        if(year_num%4!=0){
            i=4;
            day_num=curr_mday;
        }
        else if(year_num%400!=0){
            i=4;
            day_num=curr_mday;
        } 
    }
    if(day_num>30&&month_num!=1&&month_num!=3&&month_num!=5&&month_num!=7&&month_num!=8&&month_num!=10&&month_num!=12){
        i=4;
        day_num=curr_mday;
    }
    sprintf(date_string,"%d-%d-%d",year_num,month_num,day_num);
    if(hour_num<0||hour_num>23){
        i=5;
        hour_num=curr_hour;
    }
    if(min_num<0||min_num>59){
        i=6;
        min_num=curr_min;
    }
    sprintf(time_string,"%d:%d:0",hour_num,min_num);
    return i;
}

int show_cluster_mon_data(char* cluster_name, char* crypto_keyfile, char* sshkey_dir, char* node_name_list, char* start_datetime, char* end_datetime, char* interval, char* view_option, char* export_dest){
    if(cluster_name_check(cluster_name)!=-7){
        return -3;
    }
    char cluster_mon_data_file[FILENAME_LENGTH]="";
    char mon_data_file_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char mon_data_line[LINE_LENGTH_SHORT]="";
    char start_date[32]="";
    char start_time[32]="";
    char end_date[32]="";
    char end_time[32]="";
    char temp_date[32]="";
    char temp_time_final[40]="";
    char temp_time[30]="";
    char node_name_list_converted[256][16]={""};
    char real_export_dest[DIR_LENGTH_EXT]="";
    char export_file[FILENAME_LENGTH]="";
    char node_name_temp[32]="";
    int node_filter_flag;
    int interval_num;
    int i;
    time_t time1;
    time_t time2;
    time_t time_tmp;
    struct tm time_tm1;
    struct tm time_tm2;
    struct tm time_tm_tmp;
    
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    int run_flag=get_cluster_mon_data(cluster_name,crypto_keyfile,sshkey_dir,cluster_mon_data_file);
    if(run_flag==-3){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The cluster %s is not running. The data is not updated.\n" RESET_DISPLAY,cluster_name);
    }
    else if(run_flag==1||run_flag==-5){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the monitor data of cluster " WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD" ." RESET_DISPLAY "\n", cluster_name);
        return -5;
    }
    snprintf(mon_data_file_temp,FILENAME_LENGTH-1,"%s%smon_data%smon_data_temp.csv",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH);
    file_p_2=fopen(mon_data_file_temp,"w+");
    if(file_p_2==NULL){
        return -1;
    }
    run_flag=valid_time_format_or_not(start_datetime,0,start_date,start_time);
    if(run_flag==-1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No start date&time specified. Will start from the first timestamp." RESET_DISPLAY "\n");
    }
    else if(run_flag==1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Start date&time: Using the current year.\n");
    }
    else if(run_flag==2){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Start date&time: Using the current month.\n");
    }
    else if(run_flag==3){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Start date&time: Using the current mday.\n");
    }
    else if(run_flag==4){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Start date&time: Using the current hour.\n");
    }
    else if(run_flag==5){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Start date&time: Using the current minute.\n");
    }
    else{
        printf(GENERAL_BOLD "[ INFO- ]" RESET_DISPLAY " Start date&time: %s~%s.\n",start_date,start_time);
    }
    datetime_to_num(start_date,start_time,&time_tm1);
    time1=mktime(&time_tm1);
    run_flag=valid_time_format_or_not(end_datetime,1,end_date,end_time);
    if(run_flag==-1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No end date&time specified. Will end with the last timestamp." RESET_DISPLAY "\n");
    }
    else if(run_flag==1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] End date&time: Using the current year.\n");
    }
    else if(run_flag==2){
        printf(WARN_YELLO_BOLD "[ -WARN- ] End date&time: Using the current month.\n");
    }
    else if(run_flag==3){
        printf(WARN_YELLO_BOLD "[ -WARN- ] End date&time: Using the current mday.\n");
    }
    else if(run_flag==4){
        printf(WARN_YELLO_BOLD "[ -WARN- ] End date&time: Using the current hour.\n");
    }
    else if(run_flag==5){
        printf(WARN_YELLO_BOLD "[ -WARN- ] End date&time: Using the current minute.\n");
    }
    else{
        datetime_to_num(end_date,end_time,&time_tm2);
        time2=mktime(&time_tm2);
        if(time1>time2){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified end date&time is earlier than the start date&time. Skipping it." RESET_DISPLAY "\n");
            valid_time_format_or_not("",1,end_date,end_time);
        }
        else{
            printf(GENERAL_BOLD "[ INFO- ]" RESET_DISPLAY " End date&time: %s~%s.\n",end_date,end_time);
        }
    }
    datetime_to_num(end_date,end_time,&time_tm2);
    time2=mktime(&time_tm2);

    if(strlen(node_name_list)==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No node specified. Will extract the data of master and all the compute nodes." RESET_DISPLAY "\n");
        node_filter_flag=-1;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Extracting the data of specified node(s):\n");
        printf("[  ****  ] " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",node_name_list);
        node_filter_flag=calc_str_num(node_name_list,':');
        for(i=0;i<node_filter_flag;i++){
            get_seq_nstring(node_name_list,':',i+1,node_name_list_converted[i],16);
        }
    }

    interval_num=string_to_positive_num(interval);
    if(interval_num<1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No valid interval specified. Will use the default interval 5 mins." RESET_DISPLAY "\n");
        interval_num=5;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the specified interval " HIGH_CYAN_BOLD "%d" RESET_DISPLAY " mins.\n",interval_num);
    }

    file_p=fopen(cluster_mon_data_file,"r");
    fngetline(file_p,mon_data_line,LINE_LENGTH_SHORT);
    fngetline(file_p,mon_data_line,LINE_LENGTH_SHORT);
    fprintf(file_p_2,"%s\n",mon_data_line);
    while(!feof(file_p)){
        fngetline(file_p,mon_data_line,LINE_LENGTH_SHORT);
        get_seq_nstring(mon_data_line,',',1,temp_date,32);
        get_seq_nstring(mon_data_line,',',2,temp_time,30);
        snprintf(temp_time_final,39,"%s:0",temp_time);
        datetime_to_num(temp_date,temp_time_final,&time_tm_tmp);
        time_tmp=mktime(&time_tm_tmp);
//        printf("*******%ld**%ld***%ld,,,,,%d,,,,,%ld\n",time1,time2,time_tmp,interval_num,(time_tmp-time1)%(interval_num*60));
        if(time_tmp<time1){
            continue;
        }
        else if(time_tmp<time2||time_tmp==time2){
            if((time_tmp-time1)%(interval_num*60)!=0){
                continue;
            }
            if(node_filter_flag==-1){
                fprintf(file_p_2,"%s\n",mon_data_line);
            }
            else{
                get_seq_nstring(mon_data_line,',',4,node_name_temp,32);
                for(i=0;i<node_filter_flag;i++){
                    if(strcmp(node_name_temp,node_name_list_converted[i])==0){
                        fprintf(file_p_2,"%s\n",mon_data_line);
                    }
                }
            }
        }
        else{
            break;
        }
    }
    fclose(file_p);
    fclose(file_p_2);
    if(strcmp(view_option,"print")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",CAT_FILE_CMD,mon_data_file_temp);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s | more",CAT_FILE_CMD,mon_data_file_temp);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the monitor data file." RESET_DISPLAY "\n");
        return -7;
    }
    if(strlen(export_dest)==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No export destination path specified. Will not export." RESET_DISPLAY "\n");
    }
    else{
        local_path_nparser(export_dest,real_export_dest,DIR_LENGTH_EXT);
        if(folder_exist_or_not(real_export_dest)==0){
            snprintf(export_file,FILENAME_LENGTH-1,"%s%smon_data_cluster_%s.csv",export_dest,PATH_SLASH,cluster_name);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",COPY_FILE_CMD,mon_data_file_temp,export_file);
            if(system(cmdline)==0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported to the specified folder " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",real_export_dest);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to export to the specified folder " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",real_export_dest);
            }
        }
        else if(file_creation_test(real_export_dest)==0){
            strcpy(export_file,real_export_dest);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",COPY_FILE_CMD,mon_data_file_temp,export_file);
            if(system(cmdline)==0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported to the specified file " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",export_file);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to export to the specified file " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",export_file);
            }
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified dest path %s doesn't work.\n" RESET_DISPLAY,real_export_dest);
        }
    }
    return 0;
}