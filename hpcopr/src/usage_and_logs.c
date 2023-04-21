/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifdef _WIN32
#include "..\\include\\now_macros.h"
#include "..\\include\\now_global_vars.h"
#include "..\\include\\now_functions.h" 

#else
#include "../include/now_macros.h"
#include "../include/now_global_vars.h"
#include "../include/now_functions.h" 
#endif

int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option){
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char* usage_file=USAGE_LOG_FILE;
    char randstr[32]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cluster_id[32]="";
    char cloud_region[16]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_vendor[16]="";
    char unique_cluster_id[64]="";
    char current_date[32]="";
    char current_time[32]="";
    char prev_date[32]="";
    char prev_time[32]="";
    char master_config[16]="";
    char compute_config[16]="";
    char cpu_vendor[8]="";
    FILE* file_p=NULL;
    time_t current_time_long;
    struct tm* time_p=NULL;
    int vcpu=0;
    double running_hours=0;
    char running_hours_string[16]="";
    double cpu_hours=0;
    char cpu_hours_string[16]="";
    create_and_get_vaultdir(workdir,vaultdir);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,randstr);
    fclose(file_p);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\conf\\tf_prep.conf",workdir);
#else
    sprintf(filename_temp,"%s/conf/tf_prep.conf",workdir);
#endif
    find_and_get(filename_temp,"CLUSTER_ID","","",1,"CLUSTER_ID","","",' ',3,cluster_id);
    sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
    find_and_get(filename_temp,"master_inst","","",1,"master_inst","","",' ',3,master_config);
    find_and_get(filename_temp,"REGION_ID","","",1,"REGION_ID","","",' ',3,cloud_region);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\compute_template",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/compute_template",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif

    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_vendor);
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);

    if(strcmp(option,"start")==0){
        file_p=fopen(usage_file,"a+");
        if(contain_or_not(node_name,"compute")==0){
            vcpu=get_cpu_num(compute_config);
            if(*(compute_config+0)!='a'){
                strcpy(cpu_vendor,"intel64");
            }
            else{
                strcpy(cpu_vendor,"amd64");
            }
            fprintf(file_p,"%s,%s,%s,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,node_name,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        if(strcmp(node_name,"master")==0){
            vcpu=get_cpu_num(master_config);
            if(*(master_config+0)!='a'){
                strcpy(cpu_vendor,"intel64");
            }
            else{
                strcpy(cpu_vendor,"amd64");
            }
            fprintf(file_p,"%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        if(strcmp(node_name,"natgw")==0||strcmp(node_name,"database")==0){
            vcpu=2;
            strcpy(cpu_vendor,"intel64");
            fprintf(file_p,"%s,%s,%s,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,node_name,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        fclose(file_p);
        return -1;
    }
    else if(strcmp(option,"stop")==0){
        find_and_get(usage_file,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',5,prev_date);
        find_and_get(usage_file,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',6,prev_time);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","RUNNING_DATE",current_date);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","RUNNING_TIME",current_time);
        running_hours=calc_running_hours(prev_date,prev_time,current_date,current_time);
        sprintf(running_hours_string,"%.4lf",running_hours);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL1",running_hours_string);
        if(contain_or_not(node_name,"compute")==0){
            vcpu=get_cpu_num(compute_config);
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"master")==0){
            vcpu=get_cpu_num(master_config);
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"database")==0||strcmp(node_name,"natgw")==0){
            vcpu=2;
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        return -1;
    }
    return -1;
}

int get_usage(char* usage_logfile){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(usage_logfile)!=0){
        printf("[ FATAL: ] Failed to get the usage record. Either you haven't initialize your first\n");
        printf("|          cluster, or there are internal errors. Exit now.\n");
        return 1;
    }
#ifdef _WIN32
    system("del /f /s /q c:\\hpc-now\\cluster_usage_temp.log > nul 2>&1");
    sprintf(cmdline,"copy /y %s c:\\hpc-now\\cluster_usage_temp.log > nul 2>&1",usage_logfile);
#elif __APPLE__
    system("rm -rf /Users/hpc-now/now-cluster-usage-latest.log >> /dev/null 2>&1");
    sprintf(cmdline,"/bin/cp %s /Users/hpc-now/cluster_usage_temp.log >> /dev/null 2>&1",usage_logfile);
#elif __linux__
    system("rm -rf /home/hpc-now/now-cluster-usage-latest.log >> /dev/null 2>&1");
    sprintf(cmdline,"/bin/cp %s /home/hpc-now/cluster_usage_temp.log >> /dev/null 2>&1",usage_logfile);
#endif
    system(cmdline);
#ifdef _WIN32
    system("more c:\\hpc-now\\cluster_usage_temp.log");
#elif __APPLE__
    system("more /Users/hpc-now/cluster_usage_temp.log");
#elif __linux__
    system("more /home/hpc-now/cluster_usage_temp.log");
#endif
    printf("[ -DONE- ] The latest usage summary has been printed to the file below:\n");
#ifdef _WIN32
    printf("|          c:\\hpc-now\\cluster_usage_temp.log\n");
    printf("|          You can use either MS Office Excel (*strongly recommended*) or other\n");
#elif __APPLE__
    printf("|          /Users/hpc-now/cluster_usage_temp.log\n");
    printf("|          You can use either any CSV file processing tools (i.e. LibreOffice) or\n");
#elif __linux__
    printf("|          /home/hpc-now/cluster_usage_temp.log\n");
    printf("|          You can use either any CSV file processing tools (i.e. LibreOffice) or\n");
#endif
    printf("|          plain text editors (for example, notepad) to view the usage details.\n");
    return 0;
}

int get_syslog(char* operation_logfile){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(operation_logfile)!=0){
        printf("[ FATAL: ] Failed to get the operation log. Exit now.\n");      
        return 1;
    }
#ifdef _WIN32
    system("del /f /s /q c:\\hpc-now\\cluster_syslog_temp.log > nul 2>&1");
    sprintf(cmdline,"copy /y %s c:\\hpc-now\\cluster_syslog_temp.log > nul 2>&1",operation_logfile);
#elif __APPLE__
    system("rm -rf /Users/hpc-now/now-cluster-operation-latest.log >> /dev/null 2>&1");
    sprintf(cmdline,"/bin/cp %s /Users/hpc-now/cluster_syslog_temp.log >> /dev/null 2>&1",operation_logfile);
#elif __linux__
    system("rm -rf /home/hpc-now/now-cluster-operation-latest.log >> /dev/null 2>&1");
    sprintf(cmdline,"/bin/cp %s /home/hpc-now/cluster_syslog_temp.log >> /dev/null 2>&1",operation_logfile);
#endif
    system(cmdline);
#ifdef _WIN32
    system("more c:\\hpc-now\\cluster_syslog_temp.log");
#elif __APPLE__
    system("more /Users/hpc-now/cluster_syslog_temp.log");
#elif __linux__
    system("more /home/hpc-now/cluster_syslog_temp.log");
#endif
    printf("[ -DONE- ] The latet operation log has been printed to the file below:\n");
#ifdef _WIN32
    printf("|          c:\\hpc-now\\cluster_syslog_temp.log\n");
    printf("|          You can use either MS Office Excel (*strongly recommended*) or other\n");
#elif __APPLE__
    printf("|          /Users/hpc-now/cluster_syslog_temp.log\n");
    printf("|          You can use either any CSV file processing tools (i.e. LibreOffice) or\n");
#elif __linux__
    printf("|          /home/hpc-now/cluster_syslog_temp.log\n");
    printf("|          You can use either any CSV file processing tools (i.e. LibreOffice) or\n");
#endif
    printf("|          plain text editors (for example, notepad) to view the detailed log.\n");
    return 0;
}

int system_cleanup(void){
#ifdef _WIN32
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    char appdata_dir[DIR_LENGTH]="";
    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");
    
    sprintf(cmdline,"del /f /s /q %s\\Microsoft\\Windows\\Recent\\* > nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(cmdline,"rd /q /s %s\\Microsoft\\Windows\\Recent\\ > nul 2>&1",appdata_dir);
    system(cmdline);
    return 0;
#else
    //Keep it here for further use. 
    return 0;
#endif
}

int write_log(char* workdir, char* operation_logfile, char* operation, int runflag){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    FILE* file_p=fopen(operation_logfile,"a+");
    if(file_p==NULL){
        printf("[ -WARN- ] Failed to write operation log to the records. The cluster operation may\n");
        printf("|          not be affected, but will not be recorded to your system.\n");
        return -1;
    }
    fprintf(file_p,"%d-%d-%d,%d:%d:%d,%s,%s,%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec,workdir,operation,runflag);
    fclose(file_p);
    return 0;
}

int get_vault_info(char* workdir, char* crypto_keyfile){
    char md5sum[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    char* crypto_exec=NOW_CRYPTO_EXEC;
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    if(cluster_empty_or_not(workdir)==0){
        return -1;
    }
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",crypto_exec,vaultdir,vaultdir,md5sum);
#else
    sprintf(cmdline,"%s decrypt %s/_CLUSTER_SUMMARY.txt %s/_CLUSTER_SUMMARY.txt.tmp %s",crypto_exec,vaultdir,vaultdir,md5sum);
#endif
    if(system(cmdline)!=0){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#else
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#endif
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    printf("+------------ HPC-NOW CLUSTER SENSITIVE INFORMATION: ------------+\n");
    while(fgetline(file_p,single_line)==0){
        if(strlen(single_line)!=0){
            printf("%s\n",single_line);
        }
    }
    printf("+---------- DO NOT DISCLOSE THE INFORMATION TO OTHERS -----------+\n\n");
    fclose(file_p);
#ifdef _WIN32
    sprintf(cmdline,"del /f /s /q %s > nul 2>&1",filename_temp);
#else
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    return 0;
}
