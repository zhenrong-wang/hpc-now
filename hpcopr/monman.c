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
#include "monman.h"

int get_cluster_mon_data(char* cluster_name, char* sshkey_dir, char* mon_data_file){
    char workdir[DIR_LENGTH]="";
    get_workdir(workdir,cluster_name);
    char mon_data_dir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(mon_data_dir,"%s%smon_data",HPC_NOW_ROOT_DIR,PATH_SLASH);
    if(folder_exist_or_not(mon_data_dir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,mon_data_dir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    sprintf(mon_data_file,"%s%smon_data_%s.csv",mon_data_dir,PATH_SLASH,cluster_name);
    if(cluster_asleep_or_not(workdir)==0){
        if(file_empty_or_not(mon_data_file)<1){
            return -5;
        }
        else{
            return -3;
        }
    }
    remote_copy(workdir,sshkey_dir,mon_data_file,"/usr/hpc-now/mon_data.csv","root","get","",0);
    if(file_empty_or_not(mon_data_file)<1){
        strcpy(mon_data_file,"");
        return 1;
    }
    return 0;
}

int update_all_mon_data(char* cluster_registry, char* sshkey_dir){
    char cluster_name_temp[128]="";
    char mon_data_file_temp[FILENAME_LENGTH]="";
    char registry_line[256]="";
    int run_flag;
    int updated=0;
    FILE* file_p=fopen(cluster_registry,"r");
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fgetline(file_p,registry_line);
        if(strlen(registry_line)==0){
            continue;
        }
        get_seq_string(cluster_name_temp,' ',4,cluster_name_temp);
        run_flag=get_cluster_mon_data(cluster_name_temp,sshkey_dir,mon_data_file_temp);
        if(run_flag==0){
            updated++;
        }
    }
    return updated;
}

int valid_time_format_or_not(char* datetime_input, int extend_flag, char* date_string, char* time_string){
    char ymd[64]="";
    char year[32]="";
    char month[32]="";
    char mday[32]="";
    char hour_min[64]="";
    char hour[32]="";
    char min[32]="";
    int year_num,month_num,day_num,hour_num,min_num;
    if(strlen(datetime_input)>16){
        return 1;
    }
    get_seq_string(datetime_input,'~',1,ymd);
    get_seq_string(datetime_input,'~',2,hour_min);
    get_seq_string(ymd,'-',1,year);
    get_seq_string(ymd,'-',2,month);
    get_seq_string(ymd,'-',3,mday);
    get_seq_string(hour_min,':',1,hour);
    get_seq_string(hour_min,':',2,min);
    
    year_num=string_to_positive_num(year);
    month_num=string_to_positive_num(month);
    day_num=string_to_positive_num(mday);
    hour_num=string_to_positive_num(hour);
    min_num=string_to_positive_num(min);

    if(year_num==-1){
        goto illegal_string;
    }
    if(month_num<1||month_num>12){
        goto illegal_string;
    }
    if(day_num<1||day_num>31){
        goto illegal_string;
    }
    if(day_num>28&&month_num==2&&year_num%4!=0){
        goto illegal_string;
    }
    if(day_num>30&&month_num!=1&&month_num!=3&&month_num!=5&&month_num!=7&&month_num!=8&&month_num!=10&&month_num!=12){
        goto illegal_string;
    }
    if(hour_num<0||hour_num>23){
        goto illegal_string;
    }
    if(min_num<0||min_num>59){
        goto illegal_string;
    }
    sprintf(date_string,"%s-%s-%s",year,month,mday);
    sprintf(time_string,"%s:%s:0",hour,min);
    return 0;
illegal_string:
    if(extend_flag==0){
        strcpy(date_string,"1970-1-1");
        strcpy(time_string,"0:0:0");
    }
    else{
        strcpy(date_string,"2199-12-31");
        strcpy(time_string,"23:59:59");
    }
    return 1;
}

int show_cluster_mon_data(char* cluster_name, char* sshkey_dir, char* node_name, char* start_datetime, char* end_datetime, char* interval, char* view_option, char* export_dest){
    if(cluster_name_check(cluster_name)!=-127){
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
    char temp_time[32]="";
    char node_name_ext[32]="";
    char real_export_dest[DIR_LENGTH_EXT]="";
    char export_file[FILENAME_LENGTH]="";
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
    int run_flag=get_cluster_mon_data(cluster_name,sshkey_dir,cluster_mon_data_file);
    if(run_flag==-3){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The cluster %s is not running. The data is not updated.\n" RESET_DISPLAY,cluster_name);
    }
    else if(run_flag==1||run_flag==-5){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the monitor data of cluster " WARN_YELLO_BOLD "%s" RESET_DISPLAY " . Has it been inited?\n" RESET_DISPLAY, cluster_name);
        fclose(file_p_2);
        return -5;
    }
    sprintf(mon_data_file_temp,"%s%smon_data%smon_data_temp.csv",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH);
    file_p_2=fopen(mon_data_file_temp,"w+");
    if(file_p_2==NULL){
        return -1;
    }
    if(strlen(start_datetime)!=0){
        run_flag=valid_time_format_or_not(start_datetime,0,start_date,start_time);
        if(run_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The start date&time %s is invalid. Valid format: " HIGH_CYAN_BOLD "YYYY-MM-DD~HH:MM.\n",start_datetime);
            printf(WARN_YELLO_BOLD "|          Will start from the first timestamp.\n" RESET_DISPLAY);
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the specified start date&time " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",start_datetime);
        }
    }
    else{
        printf(WARN_YELLO_BOLD "[ -WARN- ] No start date&time specified. Will start from the first timestamp.\n" RESET_DISPLAY);
        strcpy(start_date,"1970-1-1");
        strcpy(start_time,"0:0:0");
    }
    datetime_to_num(start_date,start_time,&time_tm1);
    time1=mktime(&time_tm1);

    if(strlen(end_datetime)!=0){
        run_flag=valid_time_format_or_not(end_datetime,1,end_date,end_time);
        if(run_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The end date&time %s is invalid. Valid format: " HIGH_CYAN_BOLD "YYYY-MM-DD~HH:MM.\n",end_datetime);
            printf(WARN_YELLO_BOLD "|          Will end with the last timestamp.\n" RESET_DISPLAY);
        }
        else{
            datetime_to_num(end_date,end_time,&time_tm2);
            time2=mktime(&time_tm2);
            if(time1>time2){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified end date&time is earlier than the start date&time. Skipping it.\n" RESET_DISPLAY);
                strcpy(end_date,"2199-12-31");
                strcpy(end_time,"23:59:59");
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the specified end date&time " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",end_datetime);
            }
        }
    }
    else{
        printf(WARN_YELLO_BOLD "[ -WARN- ] No end date&time specified. Will end with the last timestamp." RESET_DISPLAY "\n");
        strcpy(end_date,"2199-12-31");
        strcpy(end_time,"23:59:59");
    }
    datetime_to_num(end_date,end_time,&time_tm2);
    time2=mktime(&time_tm2);
    
    if(strlen(node_name)==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No node specified. Will extract the data of master and all the compute nodes." RESET_DISPLAY "\n");
        node_filter_flag=0;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Extracting the data of specified node " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",node_name);
        sprintf(node_name_ext,",%s,",node_name);
        node_filter_flag=1;
    }

    interval_num=string_to_positive_num(interval);
    if(interval_num<0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No valid interval specified. Will use the default interval 5 mins." RESET_DISPLAY "\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the specified interval " HIGH_CYAN_BOLD "%d" RESET_DISPLAY " .\n",interval_num);
    }

    file_p=fopen(cluster_mon_data_file,"r");
    fgetline(file_p,mon_data_line);
    fgetline(file_p,mon_data_line);
    fprintf(file_p_2,"%s\n",mon_data_line);
    while(!feof(file_p)){
        fgetline(file_p,mon_data_line);
        get_seq_string(mon_data_line,',',1,temp_date);
        get_seq_string(mon_data_line,',',3,temp_time);
        datetime_to_num(temp_date,temp_time,&time_tm_tmp);
        time_tmp=mktime(&time_tm_tmp);
        if(time_tmp<time1){
            continue;
        }
        else if(time_tmp<time2||time_tmp==time2){
            if(node_filter_flag==0){
                if((time_tmp-time1)%(interval_num*60)==0){
                    fprintf(file_p_2,"%s\n",mon_data_line);
                }
                i++;
            }
            else{
                if(contain_or_not(mon_data_line,node_name_ext)==0){
                    if((time_tmp-time1)%(interval_num*60)==0){
                        fprintf(file_p_2,"%s\n",mon_data_line);
                    }
                    i++;
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
        sprintf(cmdline,"%s %s",CAT_FILE_CMD,mon_data_file_temp);
    }
    else{
        sprintf(cmdline,"%s %s | more",CAT_FILE_CMD,mon_data_file_temp);
    }
    system(cmdline);
    if(strlen(export_dest)==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] No export destination path specified. Will not export.\n" RESET_DISPLAY);
    }
    else{
        local_path_parser(export_dest,real_export_dest);
        if(folder_exist_or_not(real_export_dest)==0){
            sprintf(export_file,"%s%smon_data_cluster_%s.csv",export_dest,PATH_SLASH,cluster_name);
            sprintf(cmdline,"%s %s %s",COPY_FILE_CMD,mon_data_file_temp,export_file);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported to the specified folder " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",real_export_dest);
        }
        else if(file_creation_test(real_export_dest)==0){
            strcpy(export_file,real_export_dest);
            sprintf(cmdline,"%s %s %s",COPY_FILE_CMD,mon_data_file_temp,export_file);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported to the specified file " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",export_file);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified dest path %s doesn't work (either file already exists or folder doesn't exit).\n" RESET_DISPLAY,real_export_dest);
        }
    }
    return 0;
}