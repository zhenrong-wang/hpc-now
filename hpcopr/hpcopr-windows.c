/*
This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
It is distributed under the license: GNU Public License - v2.0
Bug report: info@hpc-now.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define CMDLINE_LENGTH 2048
#define CLUSTER_ID_LENGTH_MAX 24
#define CLUSTER_ID_LENGTH_MIN 8
#define DIR_LENGTH 256
#define FILENAME_LENGTH 512
#define LINE_LENGTH 4096 //It has to be very long, because tfstate file may contain very long line
#define AKSK_LENGTH 128
#define CONF_STRING_LENTH 64
#define URL_ALICLOUD_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-alicloud/"
#define URL_AWS_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-aws/"
#define URL_QCLOUD_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-qcloud/"
#define CRYPTO_KEY_FILE "C:\\programdata\\hpc-now\\now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-usage.log" //This is a global file!
#define OPERATION_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-operation.log"
#define SSHKEY_DIR "C:\\hpc-now\\.ssh"
#define NOW_CRYPTO_EXEC "c:\\programdata\\hpc-now\\bin\\now-crypto.exe"
#define TERRAFORM_EXEC "c:\\programdata\\hpc-now\\bin\\terraform.exe"
#define PASSWORD_LENGTH 19
#define PASSWORD_STRING_LENGTH 20
#define RANDSTR_LENGTH_PLUS 11
#define AWS_SLEEP_TIME_GLOBAL 180
#define AWS_SLEEP_TIME_CN 180
#define ALI_SLEEP_TIME 60
#define QCLOUD_SLEEP_TIME 20
#define GENERAL_SLEEP_TIME 15
#define ALI_TF_PLUGIN_VERSION "1.198.0"
#define QCLOUD_TF_PLUGIN_VERSION "1.79.10"
#define AWS_TF_PLUGIN_VERSION "4.55.0"
#define MAXIMUM_ADD_NODE_NUMBER 16 // You can modify this number to adding more than 16 nodes once


void print_empty_cluster_info(void){
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] It seems the cluster is empty. You can either:                           |\n");
    printf("|          a) Run 'hpcopr init' to generate a *default* cluster directly. OR        |\n");
    printf("|          b) Run 'hpcopr conf' to get and modify the configuration file and then   |\n");
    printf("|             Run 'hpcopr init' to generate a *customized* cluster.                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ FATAL: ] Exit now.                                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

void print_help(void){
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Usage: hpcopr command_name param1 param2                                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Commands:                                                                         |\n");
    printf("+ I  . Initialization --------------------------------------------------------------+\n");
    printf("|                                                                                   |\n");
    printf("|  new         : Create a new working directory or rotating a new keypair:          |\n");
    printf("|        workdir   - Create a new working directory to initialize a new cluster     |\n");
    printf("|                    using the 'init' command later.                                |\n");
    printf("|        keypair   - Rotating a new keypair for an existing cluster.                |\n");
    printf("|  init        : Initialize a new cluster. If the configuration file is absent,     |\n");
    printf("|                the command will generate a default configuration file. You can    |\n");
    printf("|                also add a param to this command to specify a cluster_id.          |\n");
    printf("|                    Example: hpcopr init hpcnow-demo                               |\n");
    printf("|  conf        : Get the default configuration file to edit and build a customized  |\n");
    printf("|                HPC cluster later (using the 'init' command).                      |\n");
    printf("|                                                                                   |\n");
    printf("+ II . Management     --------------------------------------------------------------+\n");
    printf("|                                                                                   |\n");
    printf("|  help        : Show this page and the information here.                           |\n");
    printf("|  usage       : Get the usage history of all your cluster(s).                      |\n");
    printf("|  syslog      : Get the detailed operation log of your cluster management.         |\n");
    printf("|  vault       : Check the sensitive information of your clusters.                  |\n");
    printf("|  graph       : Display the cluster map including all the nodes and status.        |\n");
    printf("|                                                                                   |\n");
    printf("+ III. Operation      --------------------------------------------------------------+\n");
    printf("|                                                                                   |\n");
    printf("|  delc        : Delete specified compute nodes:                                    |\n");
    printf("|        all       - Delete *ALL* the compute nodes, you can run 'hpcopr addc' to   |\n"); 
    printf("|                    add compute nodes later.                                       |\n");
    printf("|        NUM       - Delete the last NUM of the compute nodes. NUM should be less   |\n"); 
    printf("|                    than the current quantity of compute nodes.                    |\n");
    printf("|  addc  NUM   : Add compute nodes to current cluster. You can specify how many to  |\n");
    printf("|                be added.                                                          |\n");
    printf("|  shutdownc all|NUM                                                                |\n");
    printf("|              : Shutdown specified compute nodes. Similar to the command 'delc',   |\n"); 
    printf("|                you can specify to shut down all or part of the compute nodes by   |\n");
    printf("|                the parameter 'all' or 'NUM'.                                      |\n");
    printf("|  turnonc   all|NUM                                                                |\n");
    printf("|              : Turn on specified compute nodes. Similar to the command 'delc',    |\n"); 
    printf("|                you can specify to turn on all or part of the compute nodes by     |\n");
    printf("|                the parameter 'all' or 'NUM'.                                      |\n");
    printf("|  reconfc     : Reconfigure the compute nodes to a target instance type. i.e.      |\n");
    printf("|                  a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g    |\n");
    printf("|                  a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g                |\n");
    printf("|  reconfm     : Reconfigure the master node to a target instance type. i.e.        |\n");
    printf("|                  a64c128g | i64c128g | a96c192g | i96c192g | a32c64g | i32c64g    |\n");
    printf("|                  a16c32g  | i16c32g  |    ...   | a2c4g    | i2c4g                |\n");
    printf("|  sleep       : Turn off all the nodes (management and compute) of the cluster.    |\n"); 
    printf("|  wakeup    all|minimal                                                            |\n");
    printf("|              : minimal - Turn on the management nodes of the cluster.             |\n");
    printf("|              : all     - Turn on the management and compute nodes of the cluster. |\n");         
    printf("|  destroy     : *DESTROY* the whole cluster - including all the resources & data.  |\n");
    printf("|                                                                                   |\n"); 
    printf("+ IV. INSTALLATION    --------------------------------------------------------------+\n");
    printf("|                                                                                   |\n");
    printf("|  uninstall   : *REMOVE* the HPC-NOW services and data. Admin/root privilege is    |\n");
    printf("|                required for this operation.                                       |\n");
    printf("|                                                                                   |\n");     
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  HPC NOW, start now ... to infinity!            | H - igh         | N - o         |\n");
    printf("|                                                 | P - erformance  + O - perating  |\n");
    printf("|  https://www.hpc-now.com   |  info@hpc-now.com  | C - omputing    | W - orkload   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

void print_header(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    char string_temp[128]="";
    int i;
    int length_temp=19;
    printf("\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  Welcome to HPC_NOW Cluster Operator!                                             |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  HPC NOW, start now ... to infinity!            | H - igh         | N - o         |\n");
    printf("|                                                 | P - erformance  + O - perating  |\n");
    printf("|  https://www.hpc-now.com   |  info@hpc-now.com  | C - omputing    | W - orkload   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|   /HPC->  Welcome to HPC_NOW Cluster Operator!                                    |\n");
    if(time_p->tm_mon+1<10){
        length_temp--;
    }
    if(time_p->tm_mday<10){
        length_temp--;
    }
    if(time_p->tm_hour<10){
        length_temp--;
    }
    if(time_p->tm_min<10){
        length_temp--;
    }
    if(time_p->tm_sec<10){
        length_temp--;
    }
    for(i=0;i<72-length_temp;i++){
        *(string_temp+i)=' ';
    }
    printf("|\\\\/ ->NOW  %d-%d-%d %d:%d:%d%s|\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec,string_temp);
    printf("|    Version: 0.1.29   * This software is licensed under GPLv2, with NO WARRANTY! * |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    
}

void print_tail(void){
    printf("|  HPC NOW, start now ... to infinity!            | H - igh         | N - o         |\n");
    printf("|                                                 | P - erformance  + O - perating  |\n");
    printf("|  https://www.hpc-now.com   |  info@hpc-now.com  | C - omputing    | W - orkload   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

void print_not_in_a_workdir(char* current_dir){
    char temp_string[128]="";
    char temp_string2[128]="";
    int i;
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ FATAL: ] You are not in a working directory, *NO* critical operation is permitted.|\n");
    printf("|          A typical working directory: C:\\hpc-now\\now-cluster-# (# is a number).   |\n");
    sprintf(temp_string,"|          Current directory is %s.",current_dir);
    for(i=0;i<85-strlen(temp_string)-1;i++){
        *(temp_string2+i)=' ';
    }
    printf("%s%s|\n",temp_string,temp_string2);
    printf("|          Please use the 'cd' command to go to a working directory first.          |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ FATAL: ] Exit now.                                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

void reset_string(char* orig_string){
    int length=strlen(orig_string);
    int i;
    for(i=0;i<length;i++){
        *(orig_string+i)='\0';
    }
}

void datetime_to_num(char* date_string, char* time_string, struct tm* datetime_num){
    int i;
    int year=0,month=0,day=0;
    int hour=0,min=0,sec=0;
    int position1;
    int position2;
    for(i=0;i<strlen(date_string);i++){
        if(*(date_string+i)=='-'){
            position1=i;
            break;
        }
    }
    for(i=position1+1;i<strlen(date_string);i++){
        if(*(date_string+i)=='-'){
            position2=i;
            break;
        }
    }
    for(i=0;i<position1;i++){
        year+=(*(date_string+i)-'0')*pow(10,position1-1-i);
    }
    for(i=position1+1;i<position2;i++){
        month+=(*(date_string+i)-'0')*pow(10,position2-i-1);
    }
    for(i=position2+1;i<strlen(date_string);i++){
        day+=(*(date_string+i)-'0')*pow(10,strlen(date_string)-i-1);
    }

    for(i=0;i<strlen(date_string);i++){
        if(*(time_string+i)==':'){
            position1=i;
            break;
        }
    }
    for(i=position1+1;i<strlen(date_string);i++){
        if(*(time_string+i)==':'){
            position2=i;
            break;
        }
    }
//    printf("%s,%d\t%d\t%d\n",date_string,position1,position2,strlen(date_string));
    for(i=0;i<position1;i++){
        hour+=(*(time_string+i)-'0')*pow(10,position1-1-i);
    }
    for(i=position1+1;i<position2;i++){
        min+=(*(time_string+i)-'0')*pow(10,position2-i-1);
    }
    for(i=position2+1;i<strlen(time_string);i++){
        sec+=(*(time_string+i)-'0')*pow(10,strlen(time_string)-i-1);
    }
//    printf("%d,,,%d,,,%d,,,%d,,,%d,,,%d,,,\n",year,month,day,hour,min,sec);
    datetime_num->tm_year=year-1900;
    datetime_num->tm_mon=month-1;
    datetime_num->tm_mday=day;
    datetime_num->tm_hour=hour;
    datetime_num->tm_min=min;
    datetime_num->tm_sec=sec;
}

double calc_running_hours(char* prev_date, char* prev_time, char* current_date, char* current_time){
    time_t prev;
    time_t current;
    struct tm time_prev;
    struct tm time_current;
    datetime_to_num(prev_date,prev_time,&time_prev);
    datetime_to_num(current_date,current_time,&time_current);

    prev=mktime(&time_prev);
    current=mktime(&time_current);
//    printf("%ld,%ld\n",prev,current);
    return difftime(current,prev)/3600;
}

int get_crypto_key(char* crypto_key_filename, char* md5sum){
    char cmdline[CMDLINE_LENGTH]="";
    FILE* md5_tmp=NULL;
    char buffer[128]="";
    sprintf(cmdline,"certutil -hashfile %s md5 > md5.txt.tmp",crypto_key_filename);
    system(cmdline);
    md5_tmp=fopen("md5.txt.tmp","r");
    if(md5_tmp==NULL){
        return -1;
    }
    fgets(buffer,128,md5_tmp);
    fgets(md5sum,33,md5_tmp);
    fclose(md5_tmp);
    system("del /f /q md5.txt.tmp > nul 2>&1");
    return 0;
}

int fgetline(FILE* file_p, char* line_string){
    char ch;
    int i=0;
    if(file_p==NULL){
        return -1;
    }
    reset_string(line_string);
    do{
        ch=fgetc(file_p);
        if(ch!=EOF&&ch!='\n'){
            *(line_string+i)=ch;
            i++;
        }
        else if(ch=='\n'){
            return 0;
        }
        else if(ch==EOF){
            return 1;
        }
    }while(ch!=EOF&&ch!='\n');
    if(ch=='\n'){
        return 0;
    }
    else{
        return 1;
    }
}

int contain_or_not(const char* line, const char* findkey){
    int length_line=strlen(line);
    int length_findkey=strlen(findkey);
    int i,j;
    char* string_temp=(char *)malloc(sizeof(char)*(length_findkey+1));
    for(i=0;i<length_findkey;i++){
        *(string_temp+i)='\0';
    }
//    printf("%s,%s,%d-------------%dwerwerwrwrwr",line,findkey,length_line,length_findkey);
    if(length_line<length_findkey){
        free(string_temp);
        return 1;
    }
    for(i=0;i<length_line;i++){
        if(*(line+i)==*(findkey)){
            for(j=0;j<length_findkey;j++){
                *(string_temp+j)=*(line+i+j);
            }
            *(string_temp+length_findkey)='\0';
//            printf("%s,%s,%d--------\n",findkey,string_temp,length_findkey);
            if(strcmp(findkey,string_temp)==0){
                free(string_temp);
                return 0;
            }
        }
        else{
            continue;
        }
    }
    free(string_temp);
    return 1;
}

int global_replace(char* filename, char* orig_string, char* new_string){
    if(strcmp(orig_string,new_string)==0){
        return 1;
    }
    if(strlen(orig_string)==0){
        return -1;
    }
    FILE* file_p=fopen(filename, "r");
    FILE* file_p_tmp=NULL;
    if(file_p==NULL){      
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    char new_line[LINE_LENGTH]="";
    char head=*(orig_string);
    int length_orig=strlen(orig_string);
    char temp_string[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    int getline_flag;
    int i,j,k,line_length;

    sprintf(filename_temp,"%s.tmp",filename);
    file_p_tmp=fopen(filename_temp,"w+");
    do{
        getline_flag=fgetline(file_p,single_line);
        line_length=strlen(single_line);
        if(contain_or_not(single_line,orig_string)!=0||line_length<length_orig){
            fprintf(file_p_tmp,"%s\n",single_line);
            continue;
        }
        i=0;
        j=0;
        reset_string(new_line);
        reset_string(temp_string);
        do{
            if(*(single_line+i)!=head){
                *(new_line+j)=*(single_line+i);
                i++;
                j++;
                continue;
            }
            for(k=0;k<length_orig;k++){
                *(temp_string+k)=*(single_line+i+k);
            }
            if(strcmp(temp_string,orig_string)!=0){
                *(new_line+j)=*(single_line+i);
                i++;
                j++;
                continue;
            }
            else{
                for(k=0;k<strlen(new_string);k++){
                    *(new_line+j+k)=*(new_string+k);
                }
                j=j+k;
                i=i+length_orig;
                continue;
            }
        }while(i<line_length);
        if(getline_flag==1){
            fprintf(file_p_tmp,"%s",new_line);
        }
        else{
            fprintf(file_p_tmp,"%s\n",new_line);
        }
    }while(getline_flag==0);
    fclose(file_p);
    fclose(file_p_tmp);
    sprintf(cmdline,"move /y %s %s > nul 2>&1",filename_temp,filename);
    system(cmdline);
    return 0;
}

void create_and_get_stackdir(char* workdir, char* stackdir){
    char cmdline[CMDLINE_LENGTH]="";
    char string_temp[4]="";
    int i;
    int j=0;
    int cluster_num=0;
    for(i=strlen(workdir)-1;i>0;i--){
        if(*(workdir+i)=='-'){
            break;
        }
        else if(*(workdir+i)>'9'||*(workdir+i)<'0'){
            continue;
        }
        else{
            *(string_temp+j)=*(workdir+i);
            j++;
        }
    }
//    printf("%s,,,,%s,,,,\n",workdir,string_temp);
    for(j=0;j<strlen(string_temp);j++){
        cluster_num+=(*(string_temp+j)-'0')*pow(10,j);
    }
    sprintf(stackdir,"c:\\programdata\\hpc-now\\stack\\cluster-%d",cluster_num);
    sprintf(cmdline,"mkdir %s > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"attrib +h +s +r %s",stackdir);
    system(cmdline);
//    printf("%s,,,,,,\t\n",stackdir);
}


int remote_copy(char* workdir, char* sshkey_dir, char* option){
    if(strcmp(option,"hostfile")!=0){
        return -1;
    }
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    char hostfile[FILENAME_LENGTH]="";
    char currentstate[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    file_p=fopen(currentstate,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    sprintf(private_key,"%s\\now-cluster-login",sshkey_dir);
    sprintf(hostfile,"%s\\hostfile_latest",stackdir);
    sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/hostfile > nul 2>&1",private_key,hostfile,remote_address);
    system(cmdline);
    return 0;
}

void create_and_get_vaultdir(char* workdir, char* vaultdir){
    char cmdline[CMDLINE_LENGTH]="";
    char string_temp[4]="";
    int i;
    int j=0;
    int cluster_num=0;
    for(i=strlen(workdir)-1;i>0;i--){
        if(*(workdir+i)=='-'){
            break;
        }
        else if(*(workdir+i)>'9'||*(workdir+i)<'0'){
            continue;
        }
        else{
            *(string_temp+j)=*(workdir+i);
            j++;
        }
    }
    for(j=0;j<strlen(string_temp);j++){
        cluster_num+=(*(string_temp+j)-'0')*pow(10,j);
    }
    sprintf(vaultdir,"c:\\programdata\\hpc-now\\vault\\cluster-%d",cluster_num);
    sprintf(cmdline,"mkdir %s > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"attrib +h +s +r %s",vaultdir);
    system(cmdline);
}

int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string){
    int length=strlen(orig_line);
    int length_orig=strlen(orig_string);
    int length_new=strlen(new_string);
    char* temp_string=(char *)malloc(sizeof(char)*(length_orig+1));
    int i,j;
    int k=0,k2;
    for(i=0;i<length_orig+1;i++){
        *(temp_string+i)='\0';
    }
    if(length_orig==0){
        for(i=0;i<length;i++){
            *(new_line+i)=*(orig_line+i);
        }
        return length;
    }
    reset_string(new_line);
    i=0;
    do{
        if(*(orig_line+i)==*(orig_string)&&i+length_orig<length){
            for(j=0;j<length_orig;j++){
                *(temp_string+j)=*(orig_line+i+j);
            }
            if(strcmp(temp_string,orig_string)==0){
                for(j=0;j<length_new;j++){
                    *(new_line+k)=*(new_string+j);
                    k++;
                }
                *(new_line+k)='\0';
                i=i+length_orig;
                for(k2=0;k2<length_orig+1;k2++){
                    *(temp_string+k2)='\0';
                }
                continue;
            }
            else{
                *(new_line+k)=*(orig_line+i);
                i++;
                k++;
                for(k2=0;k2<length_orig+1;k2++){
                    *(temp_string+k2)='\0';
                }
                continue;
            }
        }
        else{
            *(new_line+k)=*(orig_line+i);
            i++;
            k++;
            continue;
        }
    }while(i<length);
    free(temp_string);
    return k;
} 

int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string){
    if(strcmp(orig_string,new_string)==0){
        return -1;
    }
    int replace_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){      
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    char new_single_line[LINE_LENGTH]="";
    sprintf(filename_temp,"%s.tmp",filename);
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0,flag6=0;
    FILE* file_temp_p=fopen(filename_temp,"w+");
    if(file_temp_p==NULL){
        fclose(file_p);
        return -1;
    }
    
    while(fgetline(file_p,single_line)!=1){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(strlen(findkey4)!=0){
            flag4=contain_or_not(single_line,findkey4);
        }
        if(strlen(findkey5)!=0){
            flag5=contain_or_not(single_line,findkey5);
        }
        if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0){
            fprintf(file_temp_p,"%s%c",single_line,'\n');
            flag1=0;
            flag2=0;
            flag3=0;
            flag4=0;
            flag5=0;
            continue;
        }
        else{
            flag6=contain_or_not(single_line,orig_string);
            if(flag6!=0){
                fprintf(file_temp_p,"%s%c",single_line,'\n');
            }
            else{
                replace_count++;
                if(strcmp(orig_string,new_string)==0){
                    fprintf(file_temp_p,"%s%c",single_line,'\n');
                    continue;
                }
//                printf("%s,%s\n",single_line,new_single_line);
                line_replace(single_line,new_single_line,orig_string,new_string);
//                printf("%s,%s\n",single_line,new_single_line);
                fprintf(file_temp_p,"%s%c",new_single_line,'\n');
                reset_string(new_single_line);
            }
            flag6=0;
        }
    }
    fprintf(file_temp_p,"%s",single_line);
    fclose(file_p);
    fclose(file_temp_p);
    sprintf(cmdline,"del /f /q %s > nul 2>&1 && move /y %s %s > nul 2>&1",filename,filename_temp,filename);
    system(cmdline);
    return replace_count;
}

int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5){
    if(strlen(findkey1)==0&&strlen(findkey2)==0&&strlen(findkey3)==0&&strlen(findkey4)==0&&strlen(findkey5)==0){
        return -1;
    }
    int find_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){      
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
    while(fgetline(file_p,single_line)!=1){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(strlen(findkey4)!=0){
            flag4=contain_or_not(single_line,findkey4);
        }
        if(strlen(findkey5)!=0){
            flag5=contain_or_not(single_line,findkey5);
        }
        if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0){
            flag1=0;
            flag2=0;
            flag3=0;
            flag4=0;
            flag5=0;
            continue;
        }
        else{
            find_count++;
        }
    }
    fclose(file_p);
    return find_count;
}

int calc_str_num(char* line, char split_ch){
    if(strlen(line)==0){
        return 0;
    }
    int i=0,j=0;
    int str_num=0;
    if(split_ch==' '){
        do{
            if(*(line+i)!=' '&&*(line+i)!='\t'){
                do{
                    j++;
                }while(*(line+i+j)!=' '&&*(line+i+j)!='\t'&&j<strlen(line)-i);
                if(j==(strlen(line)-i)){
                    str_num++;
                    return str_num;
                }
                if(*(line+i+j)==' '||*(line+i+j)=='\t'){
                    str_num++;
                    i=i+j;
                    j=0;
                }
            }
            else{
                i++;
            }
        }while(i<strlen(line));
        return str_num;
    }
    else{
        str_num=0;
        if(*(line)!=split_ch){
            str_num++;
        }
        do{
            if(*(line+i)==split_ch&&*(line+i+1)!=split_ch){
                str_num++;
            }
            i++;
        }while(i<strlen(line));
        return str_num;
    }
}

int get_seq_string(char* line, char split_ch, int string_seq, char* get_string){
    int total_string_num=calc_str_num(line,split_ch);
    int i=0,j=0;
    int string_seq_current;
    if(string_seq>total_string_num){
        strcpy(get_string,"");
        return -1;
    }
    if(split_ch==' '){
        string_seq_current=0;
        if(*(line)!=' '&&*(line)!='\t'){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==' '||*(line+i)=='\t'){
                if(*(line+i+1)!=' '&&*(line+i+1)!='\t'){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==' '||*(line+j)=='\t'){
                break;
            }
            else{
                *(get_string+j-i)=*(line+j);
            }
        }
        return 0;
    }
    else{
        string_seq_current=0;
        if(*(line)!=split_ch){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==split_ch){
                if(*(line+i+1)!=split_ch){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==split_ch){
                break;
            }
            else{
                *(get_string+j-i)=*(line+j);
            }
        }
        return 0;
    }
}

int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string){
    if(strlen(findkey_primary1)==0&&strlen(findkey_primary2)==0&&strlen(findkey_primary3)==0){
        return -1;
    }
    if(strlen(findkey1)==0&&strlen(findkey2)==0&&strlen(findkey3)==0){
        return -1;
    }
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){      
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    int flag_primary1=0,flag_primary2=0,flag_primary3=0;
    int flag_primary=1;
    int flag_eof_or_not=0;
    int flag1=0,flag2=0,flag3=0;
    int i;
    do{
        flag_eof_or_not=fgetline(file_p,single_line);
        if(strlen(findkey_primary1)!=0){
            flag_primary1=contain_or_not(single_line,findkey_primary1);
        }
        if(strlen(findkey_primary2)!=0){
            flag_primary2=contain_or_not(single_line,findkey_primary2);
        }
        if(strlen(findkey_primary3)!=0){
            flag_primary3=contain_or_not(single_line,findkey_primary3);
        }
        if(flag_primary1==0&&flag_primary2==0&&flag_primary3==0){
            flag_primary=0;
            break;
        }
        else{
            flag_primary=1;
            flag_primary1=0;
            flag_primary2=0;
            flag_primary3=0;
            continue;
        }
    }while(flag_primary!=0&&flag_eof_or_not==0);
    if(flag_eof_or_not==1){
        fclose(file_p);
        return 1;
    }
    i=0;
    while(flag_eof_or_not!=1&&i<plus_line_num){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(flag1!=0||flag2!=0||flag3!=0){
            flag1=0;
            flag2=0;
            flag3=0;
            i++;
            flag_eof_or_not=fgetline(file_p,single_line);
            continue;
        }
        else{
            fclose(file_p);
            return get_seq_string(single_line,split_ch,string_seq_num,get_string);
        }
    }
    strcpy(get_string,"");
    return 1;
}

int file_exist_or_not(char* filename){
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return 1;
    }
    else{
        fclose(file_p);
        return 0;
    }
}

int file_empty_or_not(char* filename){
    FILE* file_p=fopen(filename,"r");
    char temp_line[LINE_LENGTH]="";
    int line_num=0;
    if(file_p==NULL){
        return -1;
    }
    else{
        while(fgetline(file_p,temp_line)!=1){
            line_num++;
        }
        fclose(file_p);
        return line_num;
    }
}

int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag){
    if(file_exist_or_not(secret_file)!=0){
        return 1;
    }
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    if(file_exist_or_not(crypto_key_file)!=0){
        return 1;
    }
    char md5[33]="";
    char cmdline[CMDLINE_LENGTH]="";
    char decrypted_file_name[FILENAME_LENGTH]="";
    FILE* decrypted_file=NULL;
    if(get_crypto_key(crypto_key_file,md5)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to get the crypto key. Exit now.                                  |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    sprintf(cmdline,"%s decrypt %s %s.dat %s", now_crypto_exec, secret_file, secret_file, md5);
    system(cmdline);
    sprintf(decrypted_file_name,"%s.dat",secret_file);
    decrypted_file=fopen(decrypted_file_name,"r");
    if(decrypted_file==NULL){
        return -1;
    }
    fscanf(decrypted_file,"%s\n%s\n%s",ak,sk,cloud_flag);
    fclose(decrypted_file);
    reset_string(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1", decrypted_file_name);
    system(cmdline);
    return 0;
}

int folder_exist_or_not(char* foldername){
    char filename[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(filename,"%s\\testfile.txt",foldername);
    FILE* test_file=fopen(filename,"w+");
    if(test_file==NULL){
        return 1;
    }
    else{
        fclose(test_file);
        sprintf(cmdline,"del /q /f %s > nul 2>&1",filename);
        system(cmdline);
        return 0;
    }
}

int get_cpu_num(const char* vm_model){
    int length=strlen(vm_model);
    int i,c_index=0;
    int cpu_num=0;
    if(length<5||length>9){
        return -1;
    }
    if(*(vm_model)!='a'&&*(vm_model)!='i'&&*(vm_model)!='t'){
        return -1;
    }
    if(*(vm_model+length-1)!='g'){
        return -1;
    }
    if(*(vm_model+2)!='c'&&*(vm_model+3)!='c'&&*(vm_model+4)!='c'){
        return -1;
    }
    for(i=0;i<length;i++){
        if(*(vm_model+i)=='c'){
            c_index=i;
            break;
        }
    }
    for(i=1;i<c_index;i++){
        cpu_num+=(*(vm_model+i)-'0')*pow(10,c_index-i-1);
    }
    return cpu_num;
}

int generate_random_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
        mingw_gettimeofday(&current_time,NULL);
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%72;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
}

int generate_random_db_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[62]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
        mingw_gettimeofday(&current_time,NULL);
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%62;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
}


int generate_random_string(char* random_string){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[36]="abcdefghijklmnopqrstuvwxyz0123456789";
    unsigned int seed_num;
    mingw_gettimeofday(&current_time,NULL);
    seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
    srand(seed_num);
    rand_num=rand()%26;
    *(random_string+0)=*(ch_table+rand_num);
    usleep(5000);
    for(i=1;i<RANDSTR_LENGTH_PLUS-1;i++){
        mingw_gettimeofday(&current_time,NULL);
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%36;
        *(random_string+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    *(random_string+RANDSTR_LENGTH_PLUS-1)='\0';
    return 0;  
}

int check_pslock(char* workdir){
    system("tasklist | findstr hpcopr > c:\\programdata\\pscheck.txt 2>&1");
    if(find_multi_keys("c:\\programdata\\pscheck.txt","hpcopr","","","","")>1){
        system("del /f /q /s c:\\programdata\\pscheck.txt > nul 2>&1");
        return 1;
    }
    system("del /f /q /s c:\\programdata\\pscheck.txt > nul 2>&1");
    return 0;
}

int get_compute_node_num(char* currentstate_file, char* option){
    if(strcmp(option,"all")!=0&&strcmp(option,"on")!=0&&strcmp(option,"down")!=0){
        return -1;
    }
    FILE* file_p=fopen(currentstate_file,"r");
    char buffer[64]="";
    int i,node_num=0;
    int node_num_on=0;
    for(i=0;i<4;i++){
        if(fgetline(file_p,buffer)!=0){
            fclose(file_p);
            return 0;
        }
    }
    while(fgetline(file_p,buffer)==0&&strlen(buffer)!=0){
        fgetline(file_p,buffer);
        node_num++;
        if(strcmp(buffer,"running")==0||strcmp(buffer,"Running")==0||strcmp(buffer,"RUNNING")==0){
            node_num_on++;
        }
    }
    fclose(file_p);
    if(strcmp(option,"all")==0){
        return node_num;
    }
    else if(strcmp(option,"on")==0){
        return node_num_on;
    }
    else{
        return node_num-node_num_on;
    }
}

int decrypt_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\hpc_stack_base.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_base.tf.tmp %s\\hpc_stack_base.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\terraform.tfstate.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
        sprintf(cmdline,"%s decrypt %s\\terraform.tfstate.tmp %s\\terraform.tfstate %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\terraform.tfstate.backup.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
        sprintf(cmdline,"%s decrypt %s\\terraform.tfstate.backup.tmp %s\\terraform.tfstate.backup %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\hpc_stack_master.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_master.tf.tmp %s\\hpc_stack_master.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }


    sprintf(filename_temp,"%s\\hpc_stack_database.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_database.tf.tmp %s\\hpc_stack_database.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_natgw.tf.tmp %s\\hpc_stack_natgw.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\currentstate",stackdir);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf.tmp",stackdir,i);
        if(file_exist_or_not(filename_temp)==0){
            sprintf(cmdline,"%s decrypt %s\\hpc_stack_compute%d.tf.tmp %s\\hpc_stack_compute%d.tf %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
            system(cmdline);
        }
    }
    return 0;
}

int delete_decrypted_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    get_crypto_key(crypto_key_filename,md5sum);
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_base.tf %s\\hpc_stack_base.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\terraform.tfstate %s\\terraform.tfstate.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\terraform.tfstate.backup",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\terraform.tfstate.backup %s\\terraform.tfstate.backup.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_master.tf %s\\hpc_stack_master.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_database.tf %s\\hpc_stack_database.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_natgw.tf %s\\hpc_stack_natgw.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
        if(file_exist_or_not(filename_temp)==0){
            sprintf(cmdline,"%s encrypt %s\\hpc_stack_compute%d.tf %s\\hpc_stack_compute%d.tf.tmp %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
            system(cmdline);
            sprintf(cmdline,"del /f /q %s",filename_temp);
            system(cmdline);
        }
    }
    
    return 0;
}

int getstate(char* workdir, char* crypto_filename){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[8]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char filename_tfstate[FILENAME_LENGTH]="";
    char filename_currentstate[FILENAME_LENGTH]="";
    char filename_hostfile[FILENAME_LENGTH]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    int node_num_gs;
    int i;
    FILE* file_p_tfstate=NULL;
    FILE* file_p_currentstate=NULL;
    FILE* file_p_hostfile=NULL;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_tfstate,"%s\\terraform.tfstate",stackdir);
    sprintf(filename_currentstate,"%s\\currentstate",stackdir);
    file_p_tfstate=fopen(filename_tfstate,"r");
    if(file_p_tfstate==NULL){
        return -1;
    }
    sprintf(filename_currentstate,"%s\\currentstate",stackdir);
    if(file_exist_or_not(filename_currentstate)==0){
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_currentstate);
        system(cmdline);
    }
    sprintf(cmdline,"type nul > %s",filename_currentstate);
    system(cmdline);
    
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    sprintf(filename_hostfile,"%s\\hostfile_latest",stackdir);
    get_ak_sk(filename_temp,crypto_filename,buffer1,buffer2,cloud_flag);

    file_p_currentstate=fopen(filename_currentstate,"w+");
    file_p_hostfile=fopen(filename_hostfile,"w+");

    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        node_num_gs=find_multi_keys(filename_tfstate,"\"instance_name\": \"compute","","","","");
        find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"public_ip","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"private_ip","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        reset_string(string_temp);
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
            find_and_get(filename_tfstate,"\"instance_name\": \"database","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
        }
        else if(strcmp(cloud_flag,"CLOUD_A")==0){
            find_and_get(filename_tfstate,"\"instance_name\": \"master","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
            find_and_get(filename_tfstate,"\"instance_name\": \"database","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
        }
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"instance_name\": \"compute%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",50,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            reset_string(string_temp);
            if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_get(filename_tfstate,string_temp2,"","",30,"instance_status","","",'\"',4,string_temp);
                if(i!=node_num_gs-1){
                    fprintf(file_p_currentstate,"%s\n",string_temp);
                }
                else{
                    fprintf(file_p_currentstate,"%s",string_temp);
                }
                reset_string(string_temp);
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                find_and_get(filename_tfstate,string_temp2,"","",90,"\"status\":","","",'\"',4,string_temp);
                if(i!=node_num_gs-1){
                    fprintf(file_p_currentstate,"%s\n",string_temp);
                }
                else{
                    fprintf(file_p_currentstate,"%s",string_temp);
                }
                reset_string(string_temp);
            }
            reset_string(string_temp2);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        node_num_gs=find_multi_keys(filename_tfstate,"\"name\": \"compute","","","","");
        find_and_get(filename_tfstate,"\"name\": \"master","","",90,"\"public_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"master","","",90,"\"private_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"m_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"db_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"name\": \"compute%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",90,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            reset_string(string_temp);
            reset_string(string_temp2);
            sprintf(string_temp2,"\"name\": \"comp%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",30,"\"state\":","","",'\"',4,string_temp);
            if(i!=node_num_gs-1){
                fprintf(file_p_currentstate,"%s\n",string_temp);
            }
            else{
                fprintf(file_p_currentstate,"%s",string_temp);
            }
            reset_string(string_temp);
            reset_string(string_temp2);
        }
    }
    fclose(file_p_currentstate);
    fclose(file_p_hostfile);
    fclose(file_p_tfstate);
    return 0;
}

int generate_sshkey(char* sshkey_folder, char* pubkey){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    if(folder_exist_or_not(sshkey_folder)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1",sshkey_folder);
        if(system(cmdline)!=0){
            return -1;
        }
    }
    sprintf(cmdline,"attrib +h +s +r %s",sshkey_folder);
    system(cmdline);
    sprintf(filename_temp,"%s\\now-cluster-login",sshkey_folder);
    sprintf(filename_temp2,"%s\\now-cluster-login.pub",sshkey_folder);
    if(file_exist_or_not(filename_temp)==0&&file_exist_or_not(filename_temp2)==0){
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
    else{
        sprintf(cmdline,"del /f /q %s\\now-cluster-login* > nul 2>&1",sshkey_folder);
        system(cmdline);
        sprintf(cmdline,"ssh-keygen -t rsa -N \"\" -f %s\\now-cluster-login -q",sshkey_folder);
        system(cmdline);
        sprintf(filename_temp,"%s\\now-cluster-login.pub",sshkey_folder);
        file_p=fopen(filename_temp,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
}

int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes){
    if(strcmp(exec_type,"connect")!=0&&strcmp(exec_type,"all")!=0&&strcmp(exec_type,"clear")!=0){
        return -1;
    }
    if(delay_minutes<0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char remote_address[32]="";
    FILE* file_p=NULL;

    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    sprintf(private_key,"%s\\now-cluster-login",sshkey_folder);

    if(strcmp(exec_type,"clear")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr clear\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else if(strcmp(exec_type,"connect")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr connect\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else if(strcmp(exec_type,"all")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr all\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else{
        return -1;
    }
}

int graph(char* workdir, char* crypto_keyfile){
    if(getstate(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char master_address[32]="";
    char master_status[16]="";
    char db_status[16]="";
    char compute_address[32]="";
    char compute_status[16]="";
    char compute_config[16]="";
    char master_config[16]="";
    char line_buffer[32]="";
    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char master_tf[FILENAME_LENGTH];
    char stackdir[DIR_LENGTH]="";
    char head_string[128]="";
    char db_string[128]="";
    char compute_string[256]="";
    char ht_status[16]="";
    char string_temp[64]="";
    int i,node_num=0;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    FILE* file_p=fopen(currentstate,"r");
    if(file_p==NULL){
        return 1;
    }
    sprintf(compute_template,"%s\\compute_template",stackdir);
    FILE* file_p_2=fopen(compute_template,"r");
    if(file_p_2==NULL){
        fclose(file_p);
        return 1;
    }
    fgetline(file_p,master_address);
    fgetline(file_p,line_buffer);
    fgetline(file_p,master_status);
    fgetline(file_p,db_status);
    find_and_get(compute_template,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    if(find_multi_keys(compute_template,"cpu_threads_per_core = 1","","","","")!=0){
        strcpy(ht_status,"*HT-OFF*");
    }
    sprintf(master_tf,"%s\\hpc_stack_master.tf",stackdir);
    find_and_get(master_tf,"instance_type","","",1,"instance_type","","",'.',3,master_config);
    sprintf(head_string,"master(%s,%s,%s)<->|",master_address,master_status,master_config);
    for(i=0;i<strlen(head_string)-1;i++){
        *(string_temp+i)=' ';
    }
    sprintf(db_string,"%s|<->db(%s)",string_temp,db_status);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| HPC-NOW Cluster Graph and Status                                                  |\n");
    printf("+-----------------------------------------------------------------------------------+\n\n");
    printf("%s\n%s\n",db_string,head_string);
    while(fgetline(file_p,compute_address)==0){
        fgetline(file_p,compute_status);
        node_num++;
        if(strlen(ht_status)!=0){
            sprintf(compute_string,"%s|<->compute%d(%s,%s,%s,%s)",string_temp,node_num,compute_address,compute_status,compute_config,ht_status);
        }
        else{
            sprintf(compute_string,"%s|<->compute%d(%s,%s,%s)",string_temp,node_num,compute_address,compute_status,compute_config);
        }
        printf("%s\n",compute_string);
    }
    printf("\n+-----------------------------------------------------------------------------------+\n");
    printf("| HPC-NOW Cluster Graph and Status                                                  |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    fclose(file_p);
    fclose(file_p_2);
    return 0;
}


int update_cluster_summary(char* workdir, char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char md5sum[33]="";
    char master_address[32]="";
    char master_address_prev[16]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    FILE* file_p=NULL;
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    find_and_get(filename_temp,"Master","Node","IP:",1,"Master","Node","IP:",' ',4,master_address_prev);
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    file_p=fopen(filename_temp,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    if(strcmp(master_address,master_address_prev)!=0){
        sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
        global_replace(filename_temp,master_address_prev,master_address);
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt > nul 2>&1",vaultdir);
        system(cmdline);
        sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
        system(cmdline);
    }
    else{
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
        system(cmdline);
    }
    return 0;
}

int aws_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";

    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char logfile[FILENAME_LENGTH]="";
    char secret_file[FILENAME_LENGTH]="";
    char region_valid[FILENAME_LENGTH]="";

    char filename_temp[FILENAME_LENGTH]="";

    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;

    char* url_aws_root=URL_AWS_ROOT;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";

    char conf_line_buffer[256]="";
    char conf_param_buffer1[32]="";
    char conf_param_buffer2[32]="";
    char conf_param1[32]="";
    char conf_param2[32]="";
    char cluster_id_temp[16]="";
    char unique_cluster_id[96]="";
    char string_temp[128]="";

    char conf_print_string_temp1[256]="";
    char conf_print_string_temp2[256]="";
    char conf_print_string_temp3[512]="";

    char cluster_id[CONF_STRING_LENTH]="";
    char region_id[CONF_STRING_LENTH]="";
    char region_flag[16]="";
    char os_image[128]="";
    char db_os_image[128]="";
    char nat_os_image[128]="";
    char zone_id[CONF_STRING_LENTH]="";
    int node_num=0;
    int hpc_user_num=0;
    char master_init_param[CONF_STRING_LENTH]="";
    char master_passwd[CONF_STRING_LENTH]="";
    char compute_passwd[CONF_STRING_LENTH]="";
    char master_inst[CONF_STRING_LENTH]="";
    char compute_inst[CONF_STRING_LENTH]="";
    char os_image_raw[CONF_STRING_LENTH]="";
    char htflag[CONF_STRING_LENTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";

    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[LINE_LENGTH]="";
    char private_key_file[FILENAME_LENGTH]="";

    int number_of_vcpu=0;
    int cpu_core_num=0;
    int threads;
    FILE* file_p=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char md5sum[33]="";

    char bucket_id[12]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    char master_address[32]="";

    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";

    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i,j;
    
    if(folder_exist_or_not(workdir)==1){
        return -1;
    }
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);

    if(file_exist_or_not(currentstate)==0||file_exist_or_not(compute_template)==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] It seems the cluster is already in place. If you do want to rebuild the  |\n");
        printf("|          cluster, please run 'destroy' command and retry 'init' command.          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ START: ] Start initializing the cluster ...                                       |\n");
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
        reset_string(cmdline);
    }
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] IMPORTANT: No configure file found. Downloading the default configure    |\n");
        printf("|          file to initialize this cluster.                                         |\n");
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_aws_root,conf_file);
        system(cmdline);
        reset_string(cmdline);
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 1 ] Creating input files now...                                              |\n");
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    reset_string(cmdline);

    sprintf(cmdline,"curl %shpc_stack_aws.base -o %s\\hpc_stack.base -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_aws.master -o %s\\hpc_stack.master -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_aws.compute -o %s\\hpc_stack.compute -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_aws.database -o %s\\hpc_stack.database -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_aws.natgw -o %s\\hpc_stack.natgw -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %sregion_valid.tf -o %s\\region_valid.tf -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }

    sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_aws_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    sprintf(region_valid,"%s\\region_valid.tf",stackdir);
    sprintf(logfile,"%s\\now_cluster.log",logdir);
    global_replace(region_valid,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(region_valid,"BLANK_SECRET_KEY",secret_key);
    sprintf(cmdline,"cd %s && %s init >> %s\\tf_prep.log 2>%s && %s apply >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile,tf_exec,stackdir,logfile);
    system(cmdline);
    reset_string(cmdline);
    sprintf(cmdline,"del /f /q %s\\region_valid.tf > nul 2>&1",stackdir);
    system(cmdline);
    reset_string(cmdline);
    file_p=fopen(conf_file,"r");
    for(i=0;i<3;i++){
        fgets(conf_line_buffer,256,file_p);
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,cluster_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,region_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,zone_id);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        node_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    if(node_num>16){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value 16, reset to 16.|\n",node_num);
        node_num=16;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    if(hpc_user_num>8){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value 8, reset to 8.      |\n",hpc_user_num);
        hpc_user_num=8;
    }
    fscanf(file_p,"%s%s%s%s\n",conf_param_buffer1,conf_param_buffer2,conf_param1,conf_param2);
    sprintf(master_init_param,"%s %s",conf_param1,conf_param2);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image_raw);
    fscanf(file_p,"%s%s%s",conf_param_buffer1,conf_param_buffer2,htflag);
    fclose(file_p);
    number_of_vcpu=get_cpu_num(compute_inst);
    cpu_core_num=number_of_vcpu/2;
    if(strcmp(htflag,"OFF")==0){
        threads=1;
    }
    else if(strcmp(htflag,"ON")==0){
        threads=2;
    }
    if(strcmp(region_id,"cn-north-1")!=0&&strcmp(region_id,"cn-northwest-1")!=0&&strcmp(region_id,"us-east-1")!=0&&strcmp(region_id,"us-east-2")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently the NOW Cluster Service only support AWS Regions below:        |\n");
        printf("|          cn-northwest-1 | cn-north-1 | us-east-1 | us-east-2                      |\n");
        printf("|          If you'd like to use NOW Cluster in other AWS regions,                   |\n");
        printf("|          Please contact info@hpc-now.com\n                                        |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    if(contain_or_not(zone_id,region_id)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.   |\n");
        printf("+-----------------------------------------------------------------------------------+\n"); 
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    if(strcmp(region_id,"cn-northwest-1")==0){
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.0",os_image_raw);
        strcpy(db_os_image,"centos7cn.0");
        strcpy(nat_os_image,"centos7cn.0");
    }
    else if(strcmp(region_id,"cn-north-1")==0){
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.1",os_image_raw);
        strcpy(db_os_image,"centos7cn.1");
        strcpy(nat_os_image,"centos7cn.1");
    }
    else if(strcmp(region_id,"us-east-1")==0){
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.0",os_image_raw);
        strcpy(db_os_image,"centos7global.0");
        strcpy(nat_os_image,"centos7global.0");
    }
    else if(strcmp(region_id,"us-east-2")==0){
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.1",os_image_raw);
        strcpy(db_os_image,"centos7global.1");
        strcpy(nat_os_image,"centos7global.1");
    }
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    if(file_exist_or_not(filename_temp)!=0){
        reset_string(database_root_passwd);
        generate_random_db_passwd(database_root_passwd);
        usleep(10000);
        reset_string(database_acct_passwd);
        generate_random_db_passwd(database_acct_passwd);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fgetline(file_p,database_root_passwd);
        fgetline(file_p,database_acct_passwd);
        fclose(file_p);
    }
    if(strcmp(master_passwd,"*AUTOGEN*")==0||strlen(master_passwd)<8){
        reset_string(master_passwd);
        generate_random_passwd(master_passwd);
    }
    if(strcmp(compute_passwd,"*AUTOGEN*")==0||strlen(compute_passwd)<8){
        reset_string(compute_passwd);
        generate_random_passwd(compute_passwd);
    }
    if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MAX){
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id_temp+i)=*(cluster_id_input+i);
        }
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>12).\n",cluster_id_input);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] Using the CLUSTER_ID '%s' specified by the command.\n",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_input);
        reset_string(cluster_id);
        for(i=0;i<strlen(cluster_id_input);i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
    }
    else if(strlen(cluster_id_input)<CLUSTER_ID_LENGTH_MIN&&strlen(cluster_id_input)>0){
        sprintf(cluster_id_temp,"%s-hpcnow",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        strcpy(cluster_id,cluster_id_temp);
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.      |\n");
                printf("                  Extend to %s.\n", cluster_id);
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    if(file_exist_or_not(filename_temp)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Creating a Unique Cluster ID now...                                      |\n");
        reset_string(randstr);
        generate_random_string(randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s",randstr);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fscanf(file_p,"%s",randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        fclose(file_p);
    }
    reset_string(filename_temp);

    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Cluster Configuration:                                                   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(conf_print_string_temp1,"|          Cluster ID:            %s",cluster_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Region:                %s",region_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Avalability Zone:      %s",zone_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Number of Nodes:       %d",node_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);
    
    sprintf(conf_print_string_temp1,"|          Number of Users:       %d",hpc_user_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Master Node Instance:  %s",master_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Compute Node Instance: %s",compute_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          OS Image:              %s",os_image_raw);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Building you cluster now, this may take seconds ...                      |\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");

    generate_sshkey(sshkey_folder,pubkey);

    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
    sprintf(string_temp,"vpc-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_VPC_NAME",string_temp);
    sprintf(string_temp,"subnet-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_SUBNET_NAME",string_temp);
    sprintf(string_temp,"pubnet-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_replace(filename_temp,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(filename_temp,"BLANK_SECRET_KEY",secret_key);
    global_replace(filename_temp,"BUCKET_ACCESS_POLICY",randstr);
    global_replace(filename_temp,"BUCKET_USER_ID",randstr);
    global_replace(filename_temp,"BUCKET_ID",randstr);
    sprintf(string_temp,"%spublic",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_PUBLIC",string_temp);
    sprintf(string_temp,"%snatgw",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_NATGW",string_temp);
    sprintf(string_temp,"%sintra",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_INTRA",string_temp);
    sprintf(string_temp,"%smysql",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_MYSQL",string_temp);
    sprintf(string_temp,"%snag",randstr);
    global_replace(filename_temp,"NAS_ACCESS_GROUP",string_temp);
    global_replace(filename_temp,"DEFAULT_REGION_ID",region_id);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    sprintf(string_temp,"%d",node_num);
    global_replace(filename_temp,"DEFAULT_NODE_NUM",string_temp);
    sprintf(string_temp,"%d",hpc_user_num);
    global_replace(filename_temp,"DEFAULT_USER_NUM",string_temp);
    global_replace(filename_temp,"DEFAULT_MASTERINI",master_init_param);
    global_replace(filename_temp,"DEFAULT_MASTER_PASSWD",master_passwd);
    global_replace(filename_temp,"DEFAULT_COMPUTE_PASSWD",compute_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);

    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);
    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    sprintf(string_temp,"%d",cpu_core_num);
    global_replace(filename_temp,"CPU_CORE_NUM",string_temp);
    sprintf(string_temp,"%d",threads);
    global_replace(filename_temp,"THREADS_PER_CORE",string_temp);

    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"DB_OS_IMAGE",db_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);

    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"NAT_OS_IMAGE",nat_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    
    reset_string(filename_temp);
    
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        sprintf(string_temp,"comp%d",i+1);
        global_replace(filename_temp,"NUMBER",string_temp);
    }
    sprintf(cmdline,"move /y %s\\hpc_stack.base %s\\hpc_stack_base.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.database %s\\hpc_stack_database.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.master %s\\hpc_stack_master.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.natgw %s\\hpc_stack_natgw.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\hpc_stack.compute > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"cd %s && %s init >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"copy %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    get_crypto_key(crypto_keyfile,md5sum);
    getstate(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    find_and_get(filename_temp,"aws_iam_access_key","","",15,"\"id\":","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"aws_iam_access_key","","",15,"\"secret\":","","",'\"',4,bucket_sk);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Remote executing now ...                                                  \n");
    printf("[ -WAIT- ] >> ");
    if(strcmp(region_flag,"global_regions")==0){
        for(i=0;i<AWS_SLEEP_TIME_GLOBAL;i++){
            if(i%20==0){
                printf("%d >> ",AWS_SLEEP_TIME_GLOBAL-i);
            }
            usleep(1000000);
        }
        printf(" DONE \n");
    }
    else{
        for(i=0;i<AWS_SLEEP_TIME_CN;i++){
            if(i%20==0){
                printf("%d >> ",AWS_SLEEP_TIME_CN-i);
            }
            usleep(1000000);
        }
        printf(" DONE \n");
    }
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);

    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
    if(strcmp(region_flag,"cn_regions")==0){
        sprintf(cmdline,"curl %ss3cfg.txt -s -o %s\\s3cfg.txt",url_aws_root,stackdir);
        system(cmdline);
        sprintf(filename_temp,"%s\\s3cfg.txt",stackdir);
        sprintf(string_temp,"s3.%s.amazonaws.com.cn",region_id);
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
        global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg > nul 2>&1",private_key_file,filename_temp,master_address);        
        system(cmdline);
        sprintf(cmdline,"del /f /q %s > nul",filename_temp);
        system(cmdline);
    }
    else{
        sprintf(cmdline,"curl %ss3cfg.txt -s -o %s\\s3cfg.txt",url_aws_root,stackdir);
        system(cmdline);
        sprintf(filename_temp,"%s\\s3cfg.txt",stackdir);
        strcpy(string_temp,"s3.amazonaws.com");
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
        global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg > nul 2>&1",private_key_file,filename_temp,master_address);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
        system(cmdline);
    }

    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: s3:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the initialization:                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    graph(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The cluster is initializing now. This step may take at  |\n");
    printf("|          least 7 minutes. You can log into the master node now.                   |\n"); 
    printf("|          Please check the initialization progress in the /root/cluster_init.log.  |\n");
    printf("|          By default, NO HPC software will be built into the cluster.              |\n");
    printf("|          Please run 'hpcmgr install' command to install the software you need.    |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    natgw_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    
    if(*(master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,region_id);
    for(i=0;i<node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,region_id);
    }
    fclose(file_p);
    delete_decrypted_files(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_folder,"hostfile");
    remote_copy(workdir,sshkey_folder,"licfile");
    return 0;
}


int qcloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";

    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char logfile[FILENAME_LENGTH]="";
    char secret_file[FILENAME_LENGTH]="";

    char filename_temp[FILENAME_LENGTH]="";

    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;

    char* url_qcloud_root=URL_QCLOUD_ROOT;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";

    char conf_line_buffer[256]="";
    char conf_param_buffer1[32]="";
    char conf_param_buffer2[32]="";
    char conf_param1[32]="";
    char conf_param2[32]="";
    char cluster_id_temp[16]="";
    char unique_cluster_id[96]="";
    char string_temp[128]="";
    char conf_print_string_temp1[256]="";
    char conf_print_string_temp2[256]="";
    char conf_print_string_temp3[512]="";

    char cluster_id[CONF_STRING_LENTH]="";
    char region_id[CONF_STRING_LENTH]="";
    char os_image[32]="";
    char zone_id[CONF_STRING_LENTH]="";
    int node_num=0;
    int hpc_user_num=0;
    char master_init_param[CONF_STRING_LENTH]="";
    char master_passwd[CONF_STRING_LENTH]="";
    char compute_passwd[CONF_STRING_LENTH]="";
    char master_inst[CONF_STRING_LENTH]="";
    char compute_inst[CONF_STRING_LENTH]="";
    int master_bandwidth=0;
    char NAS_Zone[CONF_STRING_LENTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";

    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[LINE_LENGTH]="";
    char private_key_file[FILENAME_LENGTH]="";

    FILE* file_p=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char md5sum[33]="";

    char bucket_id[12]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    char master_address[32]="";

    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";

    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i,j;
    
    if(folder_exist_or_not(workdir)==1){
        return -1;
    }
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);

    if(file_exist_or_not(currentstate)==0||file_exist_or_not(compute_template)==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] It seems the cluster is already in place. Please destroy and retry.      |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ START: ] Start initializing the cluster ...                                       |\n");
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
        reset_string(cmdline);
    }
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] IMPORTANT: No configure file found. Downloading the default configure    |\n");
        printf("|          file to initialize this cluster.                                         |\n");
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_qcloud_root,conf_file);
        system(cmdline);
        reset_string(cmdline);
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 1 ] Creating input files now...                                              |\n");
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    reset_string(cmdline);

    sprintf(cmdline,"curl %shpc_stack_qcloud.base -o %s\\hpc_stack.base -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_qcloud.master -o %s\\hpc_stack.master -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_qcloud.compute -o %s\\hpc_stack.compute -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_qcloud.database -o %s\\hpc_stack.database -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stack_qcloud.natgw -o %s\\hpc_stack.natgw -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %sNAS_Zones_QCloud.txt -o %s\\NAS_Zones_QCloud.txt -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }

    sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_qcloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }

    reset_string(cmdline);
    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    
    sprintf(logfile,"%s\\now_cluster.log",logdir);
 
    file_p=fopen(conf_file,"r");
    for(i=0;i<3;i++){
        fgets(conf_line_buffer,256,file_p);
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,cluster_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,region_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,zone_id);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        node_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(node_num>16){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value 16, reset to 16.|\n",node_num);
        node_num=16;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>8){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value 8, reset to 8.      |\n",hpc_user_num);
        hpc_user_num=8;
    }
    fscanf(file_p,"%s%s%s%s\n",conf_param_buffer1,conf_param_buffer2,conf_param1,conf_param2);
    sprintf(master_init_param,"%s %s",conf_param1,conf_param2);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_inst);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        master_bandwidth+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(master_bandwidth>50){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value 50, reset to 50.|\n",node_num);
        node_num=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
    sprintf(filename_temp,"%s\\NAS_Zones_QCloud.txt",stackdir);
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.   |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    if(file_exist_or_not(filename_temp)!=0){
        reset_string(database_root_passwd);
        generate_random_db_passwd(database_root_passwd);
        usleep(10000);
        reset_string(database_acct_passwd);
        generate_random_db_passwd(database_acct_passwd);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fgetline(file_p,database_root_passwd);
        fgetline(file_p,database_acct_passwd);
        fclose(file_p);
    }
    if(strcmp(master_passwd,"*AUTOGEN*")==0||strlen(master_passwd)<8){
        reset_string(master_passwd);
        generate_random_passwd(master_passwd);
    }
    if(strcmp(compute_passwd,"*AUTOGEN*")==0||strlen(compute_passwd)<8){
        reset_string(compute_passwd);
        generate_random_passwd(compute_passwd);
    }
    if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MAX){
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id_temp+i)=*(cluster_id_input+i);
        }
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>12).\n",cluster_id_input);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] Using the CLUSTER_ID '%s' specified by the command.\n",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_input);
        reset_string(cluster_id);
        for(i=0;i<strlen(cluster_id_input);i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
    }
    else if(strlen(cluster_id_input)<CLUSTER_ID_LENGTH_MIN&&strlen(cluster_id_input)>0){
        sprintf(cluster_id_temp,"%s-hpcnow",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        strcpy(cluster_id,cluster_id_temp);
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.\n");
        printf("|          Extend to %s.\n", cluster_id);
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    if(file_exist_or_not(filename_temp)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Creating a Unique Cluster ID now...                                      |\n");
        reset_string(randstr);
        generate_random_string(randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s",randstr);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fscanf(file_p,"%s",randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        fclose(file_p);
    }
    reset_string(filename_temp);
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Cluster Configuration:                                                   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(conf_print_string_temp1,"|          Cluster ID:            %s",cluster_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Region:                %s",region_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Avalability Zone:      %s",zone_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Number of Nodes:       %d",node_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);
    
    sprintf(conf_print_string_temp1,"|          Number of Users:       %d",hpc_user_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Master Node Instance:  %s",master_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Compute Node Instance: %s",compute_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          OS Image:              %s",os_image);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Building you cluster now, this may take seconds ...                      |\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");

    generate_sshkey(sshkey_folder,pubkey);

    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
    sprintf(string_temp,"vpc-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_VPC_NAME",string_temp);
    sprintf(string_temp,"subnet-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_SUBNET_NAME",string_temp);
    sprintf(string_temp,"pubnet-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_replace(filename_temp,"CFSID",unique_cluster_id);
    global_replace(filename_temp,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(filename_temp,"BLANK_SECRET_KEY",secret_key);
    global_replace(filename_temp,"BUCKET_ACCESS_POLICY",randstr);
    global_replace(filename_temp,"BUCKET_USER_ID",randstr);
    global_replace(filename_temp,"BUCKET_ID",randstr);
    sprintf(string_temp,"%spublic",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_PUBLIC",string_temp);
    sprintf(string_temp,"%sintra",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_INTRA",string_temp);
    sprintf(string_temp,"%smysql",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_MYSQL",string_temp);
    sprintf(string_temp,"%snag",randstr);
    global_replace(filename_temp,"NAS_ACCESS_GROUP",string_temp);
    global_replace(filename_temp,"DEFAULT_REGION_ID",region_id);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"DEFAULT_NAS_ZONE",NAS_Zone);
    sprintf(string_temp,"%d",node_num);
    global_replace(filename_temp,"DEFAULT_NODE_NUM",string_temp);
    sprintf(string_temp,"%d",hpc_user_num);
    global_replace(filename_temp,"DEFAULT_USER_NUM",string_temp);
    global_replace(filename_temp,"DEFAULT_MASTERINI",master_init_param);
    global_replace(filename_temp,"DEFAULT_MASTER_PASSWD",master_passwd);
    global_replace(filename_temp,"DEFAULT_COMPUTE_PASSWD",compute_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);

    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);

    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"OS_IMAGE",os_image);

    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);

    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    reset_string(filename_temp);
    reset_string(string_temp);
    
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        global_replace(filename_temp,"RUNNING_FLAG","true");
    }

    sprintf(cmdline,"move /y %s\\hpc_stack.base %s\\hpc_stack_base.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.database %s\\hpc_stack_database.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.master %s\\hpc_stack_master.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.natgw %s\\hpc_stack_natgw.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\hpc_stack.compute > nul 2>&1 && del /f /q %s\\NAS_Zones_QCloud.txt > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"cd %s && %s init >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"copy %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    get_crypto_key(crypto_keyfile,md5sum);
    getstate(workdir,crypto_keyfile);

    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    find_and_get(filename_temp,"secret_id","","",1,"secret_id","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"secret_key","","",1,"secret_key","","",'\"',4,bucket_sk);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Remote executing now ...                                                  \n");
    printf("[ -WAIT- ] >> ");
    for(i=0;i<QCLOUD_SLEEP_TIME;i++){
        if(i%2==0){
            printf("%d >> ",QCLOUD_SLEEP_TIME-i);
        }
        usleep(1000000);
    }
    printf("DONE \n");
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);

    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
    
    sprintf(cmdline,"curl %scos.conf -s -o %s\\cos.conf",url_qcloud_root,stackdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\cos.conf",stackdir);
    global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
    global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
    global_replace(filename_temp,"DEFAULT_REGION",region_id);
    global_replace(filename_temp,"BLANK_BUCKET_NAME",bucket_id);
    sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.cos.conf > nul 2>&1",private_key_file,filename_temp,master_address);
    system(cmdline);
    sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.cos.conf\" > nul 2>&1",private_key_file,master_address);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);

    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    file_p=fopen(filename_temp,"w+");

    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: cos: %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the initialization:                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    graph(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The cluster is initializing now. This step may take at  |\n");
    printf("|          least 7 minutes. You can log into the master node now.                   |\n"); 
    printf("|          Please check the initialization progress in the /root/cluster_init.log.  |\n");
    printf("|          By default, NO HPC software will be built into the cluster.              |\n");
    printf("|          Please run 'hpcmgr install' command to install the software you need.    |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    natgw_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    
    if(*(master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,region_id);
    for(i=0;i<node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,region_id);
    }
    fclose(file_p);
    delete_decrypted_files(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_folder,"hostfile");
    remote_copy(workdir,sshkey_folder,"licfile");
    return 0;
}

int alicloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";

    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char logfile[FILENAME_LENGTH]="";
    char secret_file[FILENAME_LENGTH]="";

    char filename_temp[FILENAME_LENGTH]="";

    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;

    char* url_alicloud_root=URL_ALICLOUD_ROOT;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";

    char conf_line_buffer[256]="";
    char conf_param_buffer1[32]="";
    char conf_param_buffer2[32]="";
    char conf_param1[32]="";
    char conf_param2[32]="";
    char cluster_id_temp[16]="";
    char unique_cluster_id[96]="";
    char string_temp[128]="";

    char conf_print_string_temp1[256]="";
    char conf_print_string_temp2[256]="";
    char conf_print_string_temp3[512]="";

    char cluster_id[CONF_STRING_LENTH]="";
    char region_id[CONF_STRING_LENTH]="";
    char os_image[32]="";
    char zone_id[CONF_STRING_LENTH]="";
    int node_num=0;
    int hpc_user_num=0;
    char master_init_param[CONF_STRING_LENTH]="";
    char master_passwd[CONF_STRING_LENTH]="";
    char compute_passwd[CONF_STRING_LENTH]="";
    char master_inst[CONF_STRING_LENTH]="";
    char compute_inst[CONF_STRING_LENTH]="";
    int master_bandwidth=0;
    char NAS_Zone[CONF_STRING_LENTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";

    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[LINE_LENGTH]="";
    char private_key_file[FILENAME_LENGTH]="";

    FILE* file_p=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char md5sum[33]="";

    char bucket_id[12]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    char master_address[32]="";

    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";

    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i,j;
    
    if(folder_exist_or_not(workdir)==1){
        return -1;
    }
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);

    if(file_exist_or_not(currentstate)==0||file_exist_or_not(compute_template)==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] It seems the cluster is already in place.                                |\n");
        printf("|          Please empty your stack folder and retry.                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ START: ] Start initializing the cluster ...                                       |\n");
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
        reset_string(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
        reset_string(cmdline);
    }
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] IMPORTANT: No configure file found. Downloading the default configure    |\n");
        printf("|          file to initialize this cluster.                                         |\n");
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s",url_alicloud_root,conf_file);
        system(cmdline);
        reset_string(cmdline);
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 1 ] Creating input files now...                                              |\n");
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    reset_string(cmdline);

    sprintf(cmdline,"curl %shpc_stackv2.base -o %s\\hpc_stack.base -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stackv2.master -o %s\\hpc_stack.master -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stackv2.compute -o %s\\hpc_stack.compute -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stackv2.database -o %s\\hpc_stack.database -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %shpc_stackv2.natgw -o %s\\hpc_stack.natgw -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(cmdline,"curl %sNAS_Zones_ALI.txt -o %s\\NAS_Zones_ALI.txt -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_alicloud_root,stackdir);
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to download necessary file(s). Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }
    reset_string(cmdline);
    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    
    sprintf(logfile,"%s\\now_cluster.log",logdir);
 
    file_p=fopen(conf_file,"r");
    for(i=0;i<3;i++){
        fgets(conf_line_buffer,256,file_p);
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,cluster_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,region_id);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,zone_id);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        node_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(node_num>16){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value 16, reset to 16.\n",node_num);
        node_num=16;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>8){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value 8, reset to 8.\n",hpc_user_num);
        hpc_user_num=8;
    }
    fscanf(file_p,"%s%s%s%s\n",conf_param_buffer1,conf_param_buffer2,conf_param1,conf_param2);
    sprintf(master_init_param,"%s %s",conf_param1,conf_param2);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_passwd);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,master_inst);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        master_bandwidth+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(master_bandwidth>50){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value 50, reset to 50.\n",node_num);
        node_num=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
    sprintf(filename_temp,"%s\\NAS_Zones_ALI.txt",stackdir);
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.   |\n");
        printf("+-----------------------------------------------------------------------------------+\n"); 
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    if(file_exist_or_not(filename_temp)!=0){
        reset_string(database_root_passwd);
        generate_random_db_passwd(database_root_passwd);
        usleep(10000);
        reset_string(database_acct_passwd);
        generate_random_db_passwd(database_acct_passwd);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fgetline(file_p,database_root_passwd);
        fgetline(file_p,database_acct_passwd);
        fclose(file_p);
    }
    if(strcmp(master_passwd,"*AUTOGEN*")==0||strlen(master_passwd)<8){
        reset_string(master_passwd);
        generate_random_passwd(master_passwd);
    }
    if(strcmp(compute_passwd,"*AUTOGEN*")==0||strlen(compute_passwd)<8){
        reset_string(compute_passwd);
        generate_random_passwd(compute_passwd);
    }

    if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MAX){
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id_temp+i)=*(cluster_id_input+i);
        }
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>12).\n",cluster_id_input);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] Using the CLUSTER_ID '%s' specified by the command.\n",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_input);
        reset_string(cluster_id);
        for(i=0;i<strlen(cluster_id_input);i++){
            *(cluster_id+i)=*(cluster_id_input+i);
        }
    }
    else if(strlen(cluster_id_input)<CLUSTER_ID_LENGTH_MIN&&strlen(cluster_id_input)>0){
        sprintf(cluster_id_temp,"%s-hpcnow",cluster_id_input);
        global_replace(conf_file,cluster_id,cluster_id_temp);
        reset_string(cluster_id);
        strcpy(cluster_id,cluster_id_temp);
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.      |\n");
        printf("|          Extend to %s.\n", cluster_id);
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    if(file_exist_or_not(filename_temp)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Creating a Unique Cluster ID now...                                      |\n");
        reset_string(randstr);
        generate_random_string(randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"%s",randstr);
        fclose(file_p);
    }
    else{
        file_p=fopen(filename_temp,"r");
        fscanf(file_p,"%s",randstr);
        sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
        fclose(file_p);
    }
    reset_string(filename_temp);

    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Cluster Configuration:                                                   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(conf_print_string_temp1,"|          Cluster ID:            %s",cluster_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Region:                %s",region_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Avalability Zone:      %s",zone_id);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Number of Nodes:       %d",node_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);
    
    sprintf(conf_print_string_temp1,"|          Number of Users:       %d",hpc_user_num);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Master Node Instance:  %s",master_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          Compute Node Instance: %s",compute_inst);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    sprintf(conf_print_string_temp1,"|          OS Image:              %s",os_image);
    for(i=0;i<85-strlen(conf_print_string_temp1)-1;i++){
        *(conf_print_string_temp2+i)=' ';
    }
    sprintf(conf_print_string_temp3,"%s%s|",conf_print_string_temp1,conf_print_string_temp2);
    printf("%s\n",conf_print_string_temp3);
    reset_string(conf_print_string_temp1);
    reset_string(conf_print_string_temp2);
    reset_string(conf_print_string_temp3);

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Building you cluster now, this may take seconds ...                      |\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");

    generate_sshkey(sshkey_folder,pubkey);

    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
    sprintf(string_temp,"vpc-%s",unique_cluster_id);
    global_replace(filename_temp,"DEFAULT_VPC_NAME",string_temp);
    global_replace(filename_temp,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(filename_temp,"BLANK_SECRET_KEY",secret_key);
    global_replace(filename_temp,"BUCKET_ACCESS_POLICY",randstr);
    global_replace(filename_temp,"BUCKET_USER_ID",randstr);
    global_replace(filename_temp,"BUCKET_ID",randstr);
    sprintf(string_temp,"%spublic",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_PUBLIC",string_temp);
    sprintf(string_temp,"%sintra",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_INTRA",string_temp);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    sprintf(string_temp,"%smysql",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_MYSQL",string_temp);
    sprintf(string_temp,"%snag",randstr);
    global_replace(filename_temp,"NAS_ACCESS_GROUP",string_temp);
    global_replace(filename_temp,"DEFAULT_REGION_ID",region_id);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"DEFAULT_NAS_ZONE",NAS_Zone);
    sprintf(string_temp,"%d",node_num);
    global_replace(filename_temp,"DEFAULT_NODE_NUM",string_temp);
    sprintf(string_temp,"%d",hpc_user_num);
    global_replace(filename_temp,"DEFAULT_USER_NUM",string_temp);
    global_replace(filename_temp,"DEFAULT_MASTERINI",master_init_param);
    global_replace(filename_temp,"DEFAULT_MASTER_PASSWD",master_passwd);
    global_replace(filename_temp,"DEFAULT_COMPUTE_PASSWD",compute_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_replace(filename_temp,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);

    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);

    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);

    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);

    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    reset_string(filename_temp);
    reset_string(string_temp);
    
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
    }

    sprintf(cmdline,"move /y %s\\hpc_stack.base %s\\hpc_stack_base.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.database %s\\hpc_stack_database.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.master %s\\hpc_stack_master.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"move /y %s\\hpc_stack.natgw %s\\hpc_stack_natgw.tf > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\hpc_stack.compute > nul 2>&1 && del /f /q %s\\NAS_Zones_ALI.txt > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"cd %s && %s init >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s",stackdir,tf_exec,stackdir,logfile);
    system(cmdline);
    if(file_empty_or_not(logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Cluster initialization encountered problems.                             |\n");
        printf("|          Please check the logfile for details.                                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(cmdline,"copy %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    get_crypto_key(crypto_keyfile,md5sum);
    getstate(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ STEP 2 ] Remote executing now ...                                                  \n");
    printf("[ -WAIT- ] >> ");
    for(i=0;i<ALI_SLEEP_TIME;i++){
        if(i%10==0){
            printf("%d >> ",ALI_SLEEP_TIME-i);
        }
        usleep(1000000);
    }
    printf("DONE \n");
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    sprintf(filename_temp,"%s\\bucket_secrets.txt",stackdir);
    find_and_get(filename_temp,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk);
    sprintf(cmdline,"del /f /s /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);

    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
    
    sprintf(cmdline,"curl %s.ossutilconfig -s -o %s\\ossutilconfig",url_alicloud_root,stackdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\ossutilconfig",stackdir);
    global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
    global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
    global_replace(filename_temp,"DEFAULT_REGION",region_id);
    sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.ossutilconfig > nul 2>&1",private_key_file,filename_temp,master_address);
    system(cmdline);
    sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.ossutilconfig\" > nul 2>&1",private_key_file,master_address);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);

    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    file_p=fopen(filename_temp,"w+");

    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: oss:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the initialization:                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    graph(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The cluster is initializing now. This step may take at  |\n");
    printf("|          least 7 minutes. You can log into the master node now.                   |\n"); 
    printf("|          Please check the initialization progress in the /root/cluster_init.log.  |\n");
    printf("|          By default, NO HPC software will be built into the cluster.              |\n");
    printf("|          Please run 'hpcmgr install' command to install the software you need.    |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    natgw_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    
    if(*(master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,region_id);
    for(i=0;i<node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,region_id);
    }
    fclose(file_p);
    delete_decrypted_files(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_folder,"hostfile");
    remote_copy(workdir,sshkey_folder,"licfile");
    return 0;
}

int envcheck(char* pwd){
    char current_dir[DIR_LENGTH]="";
    char string_temp[DIR_LENGTH]="";
    int i;
    FILE* file_p=NULL;
    if(system("echo \%cd\% > .currentdir.tmp")!=0){
        strcpy(pwd,"");
        return 1;
    }
    file_p=fopen(".currentdir.tmp","r");
    if(file_p==NULL){
        strcpy(pwd,"");
        return 1;
    }
    fscanf(file_p,"%s",current_dir);
    fclose(file_p);
    system("del /f /q .currentdir.tmp");
    if(strlen(current_dir)>25){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    for(i=0;i<23;i++){
        *(string_temp+i)=*(current_dir+i);
    }
    if(strcmp(string_temp,"C:\\hpc-now\\now-cluster-")!=0&&strcmp(string_temp,"c:\\hpc-now\\now-cluster-")!=0){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(*(current_dir+23)>'9'||*(current_dir+23)<'1'){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(strlen(current_dir)==25){
        if(*(current_dir+24)>'9'||*(current_dir+24)<'0'){
            print_not_in_a_workdir(current_dir);
            strcpy(pwd,"");
            return 1;
        }
        strcpy(pwd,current_dir);
        return 0;
    }
    strcpy(pwd,current_dir);
    return 0;
}

int cluster_empty_or_not(char* workdir){
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    if(file_exist_or_not(filename_temp)!=0||file_empty_or_not(filename_temp)==0){
        return 0;
    }
    return 1;
}

int cluster_asleep_or_not(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char master_state[32]="";
    char buffer[32]="";
    int i;
    create_and_get_stackdir(workdir,stackdir);
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    for(i=0;i<2;i++){
        fgetline(file_p,buffer);
    }
    fgetline(file_p,master_state);
    fclose(file_p);
    if(strcmp(master_state,"running")!=0&&strcmp(master_state,"Running")!=0&&strcmp(master_state,"RUNNING")!=0){
        return 0;
    }
    else{
        return 1;
    }
}

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
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,randstr);
    fclose(file_p);
    sprintf(filename_temp,"%s\\conf\\tf_prep.conf",workdir);
    find_and_get(filename_temp,"CLUSTER_ID","","",1,"CLUSTER_ID","","",' ',3,cluster_id);
    sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
    find_and_get(filename_temp,"master_inst","","",1,"master_inst","","",' ',3,master_config);
    find_and_get(filename_temp,"REGION_ID","","",1,"REGION_ID","","",' ',3,cloud_region);
    sprintf(filename_temp,"%s\\compute_template",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
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

int cluster_destroy(char* workdir, char* crypto_keyfile){
    char doubleconfirm[LINE_LENGTH]="";
    char cloud_flag[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cmdline[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[LINE_LENGTH];
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_folder=SSHKEY_DIR;
    char md5sum[33]="";
    char master_address[32]="";
    char bucket_address[32]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    int i;
    int compute_node_num=0;
    printf("\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   YOU ARE DELETING THE WHOLE CLUSTER - INCLUDING ALL THE NODES AND *DATA*!      *|\n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.   |\n");
        printf("|          Nothing changed.                                                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Cluster operation started ...                                            |\n");
    }
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    reset_string(buffer1);
    reset_string(buffer2);
    get_crypto_key(crypto_keyfile,md5sum);

    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    find_and_get(filename_temp,"Master Node IP:","","",1,"Master Node IP:","","",' ',4,master_address);
    find_and_get(filename_temp,"NetDisk Address:","","",1,"NetDisk Address:","","",' ',4,bucket_address);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);

    if(strcmp(cloud_flag,"CLOUD_C")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/bin/s3cmd del -rf s3://%s\" > nul 2>&1",sshkey_folder,master_address,bucket_address);
        system(cmdline);
    }
    else if(strcmp(cloud_flag,"CLOUD_A")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/usr/bin/ossutil64 rm -rf oss://%s\" > nul 2>&1",sshkey_folder,master_address,bucket_address);
        system(cmdline);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/usr/local/bin/coscmd delete -rf /\" > nul 2>&1",sshkey_folder,master_address);
        system(cmdline);
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Destroying the resources, this step may take minutes ...                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    decrypt_files(workdir,crypto_keyfile);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(cmdline,"cd %s\\ && echo yes | %s destroy >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }

    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_A")==0){
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
    for(i=0;i<compute_node_num;i++){
        sprintf(string_temp,"compute%d",i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    delete_decrypted_files(workdir,crypto_keyfile);
    system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
    sprintf(cmdline,"move %s\\*.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\*.tmp c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\currentstate > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\compute_template > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\hostfile_latest c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\_CLUSTER_SUMMARY.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\UCID_LATEST.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\conf\\tf_prep.conf %s\\conf\\tf_prep.conf.destroyed > nul 2>&1",workdir,workdir);
    system(cmdline);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] The whole cluster has been destroyed.                                    |\n");
    printf("|          You can run 'init' command to rebuild it.                                |\n");
    printf("|          However, *ALL* the data has been erased permenantly.                     |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Thanks for using the NOW Cluster service!                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int delete_compute_node(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char string_temp2[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    int del_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing deleted. Exit now.         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return -1;
            }
            del_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(del_num==0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        if(del_num>compute_node_num){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] You specified a number larger than the quantity of compute nodes.        |\n");
            printf("           Do you mean deleting *ALL* the compute nodes?                            |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            reset_string(string_temp);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] You chose to deny this operation. Exit now.                              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You specified to delete %d from %d compute node(s).",del_num,compute_node_num);
            for(i=0;i<85-strlen(string_temp)-1;i++){
                *(string_temp2+i)=' ';
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("%s%s|\n",string_temp,string_temp2);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                system("del /f /s /q c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
                sprintf(cmdline,"move %s\\hpc_stack_compute%d.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1", stackdir,i);
                system(cmdline);
            }
            sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
            system(cmdline);
            sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
            if(file_empty_or_not(filename_temp)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
                printf("|          Exit now.                                                                |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] After the cluster operation:                                             |\n");
            graph(workdir,crypto_keyfile);
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.          |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            return 0;
        }
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(string_temp,"[ -INFO- ] You specified to delete *ALL* the %d compute node(s).",compute_node_num);
    for(i=0;i<85-strlen(string_temp)-1;i++){
        *(string_temp2+i)=' ';
    }
    printf("%s%s|\n",string_temp,string_temp2);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        system("del /f /s /q c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
        sprintf(cmdline,"move %s\\hpc_stack_compute%d.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1", stackdir,i);
        system(cmdline);
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.          |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int add_compute_node(char* workdir, char* crypto_keyfile, char* add_number_string){
    char string_temp[128]="";
    char string_temp2[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char* tf_exec=TERRAFORM_EXEC;
    int i;
    int add_number=0;
    int current_node_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    if(strlen(add_number_string)>2||strlen(add_number_string)<1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] The number of nodes to be added is invalid. A number (1-16) is needed.   |\n");
        printf("           Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    for(i=0;i<strlen(add_number_string);i++){
        if(*(add_number_string+i)<'0'||*(add_number_string+i)>'9'){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The number of nodes to be added is invalid. A number (1-16) is needed.   |\n");
            printf("           Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return -1;
        }
        else{
            add_number+=(*(add_number_string+i)-'0')*pow(10,strlen(add_number_string)-i-1);
        }
    }

    if(add_number>MAXIMUM_ADD_NODE_NUMBER||add_number<1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] The number of nodes to be added is out of range (1-16). Exit now.        |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(string_temp,"[ -INFO- ] You specified to add %d compute node(s).",add_number);
    for(i=0;i<85-strlen(string_temp)-1;i++){
        *(string_temp2+i)=' ';
    }
    printf("%s%s|\n",string_temp,string_temp2);
    decrypt_files(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] The cluster operation is in progress ...                                 |\n");
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    current_node_num=get_compute_node_num(filename_temp,"all");
    for(i=0;i<add_number;i++){
        sprintf(cmdline,"copy %s\\compute_template %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1+current_node_num);
        system(cmdline);
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i+1+current_node_num);
        sprintf(string_temp,"compute%d",i+1+current_node_num);
        global_replace(filename_temp,"compute1",string_temp);
        sprintf(string_temp,"comp%d",i+1+current_node_num);
        global_replace(filename_temp,"comp1",string_temp);
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=0;i<add_number;i++){
        sprintf(string_temp,"compute%d",current_node_num+i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been added.            |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int shudown_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char string_temp2[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    int down_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing to be shutdown. Exit now.  |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return -1;
            }
            down_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(down_num==0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        if(down_num>compute_node_num){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] You specified a number larger than the quantity of compute nodes.        |\n");
            printf("           Do you mean shutting down *ALL* the compute nodes?                       |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            reset_string(string_temp);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] You chose to deny this operation. Exit now.                              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You planned to shutdown %d from %d compute node(s).",down_num,compute_node_num);
            for(i=0;i<85-strlen(string_temp)-1;i++){
                *(string_temp2+i)=' ';
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("%s%s|\n",string_temp,string_temp2);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                if(strcmp(cloud_flag,"CLOUD_A")==0){
                    global_replace(filename_temp,"Running","Stopped");
                }
                else if(strcmp(cloud_flag,"CLOUD_B")==0){
                    find_and_replace(filename_temp,"running_flag","","","","","true","false");
                }
                else if(strcmp(cloud_flag,"CLOUD_C")==0){
                    global_replace(filename_temp,"running","stopped");
                }
            }
            sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
            system(cmdline);
            sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
            if(file_empty_or_not(filename_temp)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
                printf("|          Exit now.                                                                |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] After the cluster operation:                                             |\n");
            graph(workdir,crypto_keyfile);
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.          |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            return 0;
        }
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(string_temp,"[ -INFO- ] You planned to shutdown *ALL* the %d compute node(s).",compute_node_num);
    for(i=0;i<85-strlen(string_temp)-1;i++){
        *(string_temp2+i)=' ';
    }
    printf("%s%s|\n",string_temp,string_temp2);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Running","Stopped");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","true","false");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"running","stopped");
        }
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been shut down.        |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}


int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char string_temp2[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    int on_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    int compute_node_num_on=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_num_on=get_compute_node_num(filename_temp,"on");
    
    if(compute_node_num==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing to be turned on. Exit now. |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    if(compute_node_num==compute_node_num_on){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently, all the compute nodes are in the state of running.            |\n");
        printf("|          No compute node needs to be turned on. Exit now.                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return -1;
            }
            on_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(on_num==0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.              |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        if(on_num+compute_node_num_on>compute_node_num){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] You specified a number larger than the number of currently down nodes.   |\n");
            printf("           Do you mean turning on *ALL* the compute nodes?                          |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            reset_string(string_temp);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] You chose to deny this operation. Exit now.                              |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You planned to turn on %d compute node(s).",on_num);
            for(i=0;i<85-strlen(string_temp)-1;i++){
                *(string_temp2+i)=' ';
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("%s%s|\n",string_temp,string_temp2);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                if(strcmp(cloud_flag,"CLOUD_A")==0){
                    global_replace(filename_temp,"Stopped","Running");
                }
                else if(strcmp(cloud_flag,"CLOUD_B")==0){
                    find_and_replace(filename_temp,"running_flag","","","","","false","true");
                }
                else if(strcmp(cloud_flag,"CLOUD_C")==0){
                    global_replace(filename_temp,"stopped","running");
                }
            }
            sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
            system(cmdline);
            sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
            if(file_empty_or_not(filename_temp)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
                printf("|          Exit now.                                                                |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] After the cluster operation:                                             |\n");
            graph(workdir,crypto_keyfile);
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been turned on.        |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            return 0;
        }
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(string_temp,"[ -INFO- ] You planned to turn on *ALL* the %d compute node(s).",compute_node_num);
    for(i=0;i<85-strlen(string_temp)-1;i++){
        *(string_temp2+i)=' ';
    }
    printf("%s%s|\n",string_temp,string_temp2);
    decrypt_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Stopped","Running");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","false","true");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"stopped","running");
        }
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been turned on.        |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int check_reconfigure_list(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char single_line[64]="";
    char reconf_list[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(reconf_list,"%s\\reconf.list",stackdir);
    if((file_p=fopen(reconf_list,"r"))==NULL){
        return -1;
    }
    while(fgetline(file_p,single_line)==0){
        printf("%s\n",single_line);
    }
    printf("%s\n",single_line);
    return 0;
}

int reconfigure_compute_node(char* workdir, char* crypto_keyfile, char* new_config, char* htflag){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    char prev_config[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[16]="";
    int compute_node_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    char cmdline[CMDLINE_LENGTH]="";
    char node_name_temp[32]="";
    char* tf_exec=TERRAFORM_EXEC;
    int cpu_core_num=0;
    
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);

    sprintf(filename_temp,"%s\\currentstate",stackdir);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] Currently there is no compute nodes in your cluster. Exit now.           |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    decrypt_files(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
    if(find_multi_keys(filename_temp,new_config,"","","","")==0||find_multi_keys(filename_temp,new_config,"","","","")<0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Invalid compute configuration.  Exit now.                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s\\compute_template",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    
    if(strcmp(prev_config,new_config)==0){
        if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
            printf("|          Nothing changed. Exit now.                                               |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            if(strlen(htflag)==0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
                printf("|          Nothing changed. Exit now.                                               |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
            else if(strcmp(htflag,"hton")!=0&&strcmp(htflag,"htoff")!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
                printf("|          Nothing changed. Exit now.                                               |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
            else if(strcmp(htflag,"hton")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
                printf("|          Nothing changed. Exit now.                                               |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
            else if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
                printf("|          Nothing changed. Exit now.                                               |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                return 1;
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
                    sprintf(filename_temp2,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                    global_replace(filename_temp2,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
                }
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
                    sprintf(filename_temp2,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                    global_replace(filename_temp2,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
                }
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] The cluster operation is in progress ...                                 |\n");
            printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
            printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
            system(cmdline);
            sprintf(filename_temp2,"%s\\log\\now_cluster.log",workdir);
            if(file_empty_or_not(filename_temp2)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
                printf("|          Exit now.                                                                |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] After the cluster operation:                                             |\n");
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
            }
            sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
            system(cmdline);
            graph(workdir,crypto_keyfile);
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
            }
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -DONE- ] Congratulations! The compute nodes have been reconfigured.               |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            return 0;
        }
    }

    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
            global_replace(filename_temp,prev_config,new_config);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
            global_replace(filename_temp,prev_config,new_config);
            cpu_core_num=get_cpu_num(new_config)/2;
            find_and_get(filename_temp,"cpu_core_count =","","",1,"cpu_core_count =","","",' ',3,string_temp);
            sprintf(string_temp2,"%d",cpu_core_num);
            find_and_replace(filename_temp,"cpu_core_count =","","","","",string_temp,string_temp2);
        }
        if(strcmp(htflag,"hton")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
            for(i=1;i<compute_node_num+1;i++){
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                global_replace(filename_temp,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
            }
        }
        if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
            for(i=1;i<compute_node_num+1;i++){
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                global_replace(filename_temp,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
            }
        }
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] The cluster operation is in progress ...                                 |\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp2,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp2)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
    }
    sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
    system(cmdline);
    graph(workdir,crypto_keyfile);
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The compute nodes have been reconfigured.               |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int reconfigure_master_node(char* workdir, char* crypto_keyfile, char* new_config){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char prev_config[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[16]="";
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    char cmdline[CMDLINE_LENGTH]="";
    char* tf_exec=TERRAFORM_EXEC;
    
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        return -1;
    }

    decrypt_files(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
    if(find_multi_keys(filename_temp,new_config,"","","","")==0||find_multi_keys(filename_temp,new_config,"","","","")<0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Invalid master node configuration.  Exit now.                            |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    
    if(strcmp(prev_config,new_config)==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] The specified configuration is the same as previous configuration.       |\n");
        printf("|          Nothing changed. Exit now.                                               |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }

    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    global_replace(filename_temp,prev_config,new_config);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] The cluster operation is in progress ...                                 |\n");
    printf("[ -WARN- ] *DO NOT* TERMINATE THIS PROCESS MANNUALLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("[ -WARN- ] *OTHERWISE* THE CLUSTER WILL BE CORRUPTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    sprintf(filename_temp2,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp2)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -3;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    graph(workdir,crypto_keyfile);
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        usleep(1000000);
    }
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_cluster_summary(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The master node have been reconfigured.                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}


int cluster_sleep(char* workdir, char* crypto_keyfile){
    char string_temp[128]="";
    char string_temp2[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    if(find_multi_keys(filename_temp,"running","","","","")==0&&find_multi_keys(filename_temp,"Running","","","","")==0&&find_multi_keys(filename_temp,"RUNNING","","","","")==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Currently the cluster is in the state of hibernation. No node is running.|\n");
        printf("|          If you'd like to make it ready for running, please run 'wakeup' command. |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        print_tail();
        return 1;
    }

    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    printf("+-----------------------------------------------------------------------------------+\n");
    sprintf(string_temp,"[ -INFO- ] You planned to shutdown *ALL* the nodes of the current cluster.");
    for(i=0;i<85-strlen(string_temp)-1;i++){
        *(string_temp2+i)=' ';
    }
    printf("%s%s|\n",string_temp,string_temp2);
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }

    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }

    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }

    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Running","Stopped");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","true","false");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"running","stopped");
        }
    }
    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<GENERAL_SLEEP_TIME;i++){
            usleep(1000000);
        }
        sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        print_tail();
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");

    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }

    update_cluster_summary(workdir,crypto_keyfile);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! All the nodes of the current cluster have been shutdown.|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option){
    if(strcmp(option,"all")!=0&&strcmp(option,"minimal")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please specify either 'minimal' or 'all' as the second parameter.        |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s\\currentstate",stackdir);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    
    if(strcmp(option,"all")==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] ALL MODE: Turning on all the nodes of the current cluster.               |\n");
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] MINIMAL MODE: Turning on the management nodes of the current cluster.    |\n");
    }
    
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }

    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }

    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }

    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
            if(strcmp(cloud_flag,"CLOUD_A")==0){
                global_replace(filename_temp,"Stopped","Running");
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_replace(filename_temp,"running_flag","","","","","false","true");
            }
            else if(strcmp(cloud_flag,"CLOUD_C")==0){
                global_replace(filename_temp,"stopped","running");
            }
        }
    }

    sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
    system(cmdline);
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<10;i++){
            usleep(1000000);
        }
        sprintf(cmdline,"cd %s && echo yes | %s apply >> %s\\tf_prep.log 2>%s\\log\\now_cluster.log",stackdir,tf_exec,stackdir,workdir);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\log\\now_cluster.log",workdir);
    if(file_empty_or_not(filename_temp)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to modify the cluster. Please check the logfile for details.      |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] After the cluster operation:                                             |\n");
    graph(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    update_usage_summary(workdir,crypto_keyfile,"database","start");
    update_usage_summary(workdir,crypto_keyfile,"natgw","start");
    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(string_temp,"compute%d",i);
            update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
        }
    }
    update_cluster_summary(workdir,crypto_keyfile);

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Congratulations! The cluster is in the state of running.                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int check_current_user(void){
    char current_user_full[128]="";
    char current_user[128]="";
    int i,slash;
    if(system("whoami > c:\\programdata\\current_user.txt.tmp")!=0){
        return 1;
    }
    FILE* file_p_temp=fopen("c:\\programdata\\current_user.txt.tmp","r");
    fscanf(file_p_temp,"%s",current_user_full);
    fclose(file_p_temp);
    system("del /f /q c:\\programdata\\current_user.txt.tmp > nul 2>&1");
    for(i=0;i<strlen(current_user_full);i++){
        if(*(current_user_full+i)=='\\'){
            slash=i;
            break;
        }
    }
    for(i=slash+1;i<strlen(current_user_full);i++){
        *(current_user+i-slash-1)=*(current_user_full+i);
    }
    if(strcmp(current_user,"hpc-now")==0){
        return 0;
    }
    else{
        return 1;
    }
}

int create_new_workdir(char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    int current_cluster_num=0;
    char new_workdir[DIR_LENGTH]="";
    int new_cluster_num;
    char filename_temp[FILENAME_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char *now_crypto_exec=NOW_CRYPTO_EXEC;
    int ak_length,sk_length;

    if(folder_exist_or_not("c:\\programdata\\hpc-now")==0){
        strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
    }
    else{
        strcpy(filename_temp,"c:\\programdata\\secret.tmp.txt");
    }
    FILE* file_p=fopen(filename_temp,"w+");
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char md5sum[33]="";
    if(file_p==NULL){
        return -1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Please input your secrets key pair:                                      |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ] Access key ID :");
    scanf("%s",access_key);
    printf("[ INPUT: ] Access secrets:");
    scanf("%s",secret_key);
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);

    if(ak_length==24&&sk_length==30){
        fprintf(file_p,"%s\n%s\nCLOUD_A",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        fprintf(file_p,"%s\n%s\nCLOUD_B",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        fprintf(file_p,"%s\n%s\nCLOUD_C",access_key,secret_key);
        fclose(file_p);
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");  
        printf("[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.             |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        fclose(file_p);
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
        system(cmdline);
        return 1;
    }

    system("dir c:\\hpc-now\\ | findstr now-cluster- > .cluster_num.txt.tmp");
    current_cluster_num=file_empty_or_not(".cluster_num.txt.tmp");
    system("del /f /q .cluster_num.txt.tmp > nul 2>&1");
    new_cluster_num=current_cluster_num+1;
    sprintf(new_workdir,"c:\\hpc-now\\now-cluster-%d",new_cluster_num);
    sprintf(cmdline,"mkdir %s > nul 2>&1",new_workdir);
    system(cmdline);

    create_and_get_vaultdir(new_workdir,vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s\\.secrets.txt %s",now_crypto_exec,filename_temp,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] The secrets key pair has been encrypted and stored locally.              |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] The working directory of your new cluster: c:\\hpc-now\\now-cluster-%d.\n",new_cluster_num);
    printf("|          Please switch to it and run 'hpcopr init' to create a default cluster.   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Exit now.                                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int rotate_new_keypair(char* workdir, char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char *now_crypto_exec=NOW_CRYPTO_EXEC;
    int ak_length,sk_length;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[32]="";
    char doubleconfirm[32]="";

    char access_key_prev[AKSK_LENGTH]="";
    char secret_key_prev[AKSK_LENGTH]="";
    char cloud_flag_prev[32]="";
    char md5sum[33]="";

    if(folder_exist_or_not("c:\\programdata\\hpc-now")==0){
        strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
    }
    else{
        strcpy(filename_temp,"c:\\programdata\\secret.tmp.txt");
    }
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to create a temporary file in your system.                        |\n");
        printf("|          Please check the available disk space. Exit now.                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp2,"%s\\.secrets.txt",vaultdir);
    if(file_exist_or_not(filename_temp2)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Currently there is no secrets keypair. This working directory may be     |\n");
        printf("|          corrputed, which is very unusual. Please contact us via:                 |\n");
        printf("|          info@hpc-now.com for troubleshooting. Exit now.                          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }

    printf("\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   YOU ARE ROTATING THE CLOUD KEYPAIR, WHICH MAY DAMAGE THIS CLUSTER.            *|\n");
    printf("|*   BEFORE PROCEEDING, PLEASE MAKE SURE:                                          *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   1. Your new key pair comes from the *SAME* cloud vendor and account.          *|\n");
    printf("|*      This is * !!! EXTREMELY IMPORTANT !!! *                                    *|\n");
    printf("|*   2. Your new key pair is valid and able to manage cloud resources.             *|\n");
    printf("|*      This is * !!! VERY IMPORTANT !!! *                                         *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.   |\n");
        printf("|          Nothing changed.                                                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }

    get_ak_sk(filename_temp2,crypto_keyfile,access_key_prev,secret_key_prev,cloud_flag_prev);
    
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Please input your new secrets key pair:                                  |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ] Access key ID :");
    scanf("%s",access_key);
    printf("[ INPUT: ] Access secrets:");
    scanf("%s",secret_key);
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);

    if(ak_length==24&&sk_length==30){
        strcpy(cloud_flag,"CLOUD_A");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.             |\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.        |\n");
            printf("|          Current Vendor: AliCloud (HPC-NOW code: CLOUD_A).                        |\n");
            printf("|          Please rotate a keypair from an AliCloud account.                        |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_A",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_B");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.             |\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.        |\n");
            printf("|          Current Vendor: TencentCloud (HPC-NOW code: CLOUD_B).                    |\n");
            printf("|          Please rotate a keypair from a TencentCloud account.                     |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_B",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        strcpy(cloud_flag,"CLOUD_C");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.             |\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.        |\n");
            printf("|          Current Vendor: Amazon Web Services (HPC-NOW code: CLOUD_C).             |\n");
            printf("|          Please rotate a keypair from an Amazon Web Services account.             |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_C",access_key,secret_key);
        fclose(file_p);
    }
    else{
        printf("+-----------------------------------------------------------------------------------+\n");  
        printf("[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.             |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        fclose(file_p);
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
        system(cmdline);
        return 1;
    }

    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s\\.secrets.txt %s",now_crypto_exec,filename_temp,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);

    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s\\hpc_stack_base.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(filename_temp2,"%s\\hpc_stack_base.tf",stackdir);
        sprintf(cmdline,"%s decrypt %s %s %s",now_crypto_exec,filename_temp,filename_temp2,md5sum);
        system(cmdline);
        global_replace(filename_temp2,access_key_prev,access_key);
        global_replace(filename_temp2,secret_key_prev,secret_key);
        sprintf(cmdline,"%s encrypt %s %s %s",now_crypto_exec,filename_temp2,filename_temp,md5sum);
        system(cmdline);
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] The new secrets key pair has been encrypted and rotated locally.         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Exit now.                                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    return 0;
}

int get_default_conf(char* workdir, char* crypto_keyfile){
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[32]="";
    char* aws_url_root=URL_AWS_ROOT;
    char* qcloud_url_root=URL_QCLOUD_ROOT;
    char* ali_url_root=URL_ALICLOUD_ROOT;
    char confdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(confdir,"%s\\conf\\",workdir);
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1",confdir);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"move %s %s.prev > nul 2>&1",filename_temp,filename_temp);
        system(cmdline);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",ali_url_root,confdir);
        system(cmdline);
        return 0;
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",qcloud_url_root,confdir);
        system(cmdline);
        return 0;
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",aws_url_root,confdir);
        system(cmdline);
        return 0;
    }
    else{
        return 1;
    }
}

int check_internet(void){
    if(system("ping -n 2 www.baidu.com > nul 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service |\n");
        printf("|          or check your internet connectivity and retry later.                     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    return 0;
}

int check_and_install_prerequisitions(char* current_command){
    char cmdline[CMDLINE_LENGTH]="";
    char appdata_dir[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    char dirname_temp[DIR_LENGTH]="";
    char random_string[PASSWORD_STRING_LENGTH]="";
    reset_string(random_string);
    int flag=0;
    FILE* file_p=NULL;
    char* ali_plugin_version=ALI_TF_PLUGIN_VERSION;
    char* qcloud_plugin_version=QCLOUD_TF_PLUGIN_VERSION;
    char* aws_plugin_version=AWS_TF_PLUGIN_VERSION;
    char* usage_logfile=USAGE_LOG_FILE;
    char* operation_logfile=OPERATION_LOG_FILE;
    char* sshkey_dir=SSHKEY_DIR;
    
    printf("[ -INFO- ] Checking running environment for HPC-NOW services ...                    |\n");
    if(system("net user hpc-now > nul 2>&1")!=0){
        system("rd /s /q c:\\hpc-now > nul 2>&1");
        system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] There is no 'hpc-now' user in your OS currently.                         |\n");
        system("whoami /groups | find \"S-1-16-12288\" > c:\\programdata\\check.txt.tmp 2>&1");
        if(file_empty_or_not("c:\\programdata\\check.txt.tmp")==0){
            system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Please switch to administrator or users with administration privilege    |\n");
            printf("|          and run this command again to start using the HPC-NOW services.          |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return -1;    
        }
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        strcpy(cmdline,"net user hpc-now nowadmin2023~ /add /logonpasswordchg:yes > nul 2>&1");
        if(system(cmdline)!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.     |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return -1;
        }
        system("mkdir c:\\hpc-now > nul 2>&1");
        if(file_exist_or_not("C:\\hpc-now\\hpcopr.exe")!=0){
            sprintf(cmdline,"copy %s c:\\hpc-now\\hpcopr.exe > nul 2>&1",current_command);
            system(cmdline);
        }
        system("icacls c:\\hpc-now /grant hpc-now:(OI)(CI)F /t > nul 2>&1");
        system("icacls c:\\hpc-now /deny hpc-now:(DE) /t > nul 2>&1");
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] The user 'hpc-now' has been created with initial password as below:      |\n");
        printf("|          nowadmin2023~                                                            |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Please switch to the user 'hpc-now' and run this command to get started. |\n");
        printf("|          You will be required to change the password of 'hpc-now'.                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 127;
    }

    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.     |\n");
        printf("|          Please contact us via info@hpc-now.com for support. Exit now.            |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -127;
    }

    if(check_current_user()!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] You *MUST* create / switch to the user 'hpc-now' to operating a cluster. |\n");
        printf("|          Or you need to switch into a directory with permissions. Exit now.       |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 2;
    }

    if(folder_exist_or_not("c:\\programdata\\hpc-now")!=0){
        system("mkdir c:\\programdata\\hpc-now\\ > nul 2>&1");
    }

    system("attrib +h +s +r c:\\programdata\\hpc-now > nul 2>&1");
    if(folder_exist_or_not("c:\\programdata\\hpc-now\\.destroyed\\")!=0){
        system("mkdir c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1");
    }
    if(folder_exist_or_not("c:\\programdata\\hpc-now\\bin\\")!=0){
        system("mkdir c:\\programdata\\hpc-now\\bin\\ > nul 2>&1");
    }
    system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1"); 

    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\terraform.exe")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Downloading and installing necessary tools (1/5) ...                     |\n");
        flag=system("curl https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-win64/terraform.exe -o c:\\programdata\\hpc-now\\bin\\terraform.exe -s");
        if(flag!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Failed to download or install necessary tools. Please contact            |\n");
            printf("|          info@hpc-now.com for support. Exit now.                                  |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 3;
        }
    }

    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\now-crypto.exe")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Downloading and installing necessary tools (2/5) ...                     |\n");
        flag=system("curl https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto.exe -o c:\\programdata\\hpc-now\\bin\\now-crypto.exe -s");
        if(flag!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Failed to download or install necessary tools. Please contact            |\n");
            printf("|          info@hpc-now.com for support. Exit now.                                  |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            return 3;
        }
    }

    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");

    sprintf(cmdline,"del /f /s /q %s\\Microsoft\\Windows\\Recent\\* >nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(cmdline,"rd /q /s %s\\Microsoft\\Windows\\Recent\\ >nul 2>&1",appdata_dir);
    system(cmdline);

    sprintf(filename_temp,"%s\\.terraformrc",appdata_dir);
    if(file_exist_or_not(filename_temp)!=0){
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"%s\\.terraform.d/plugins\"\n",appdata_dir);
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }

    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\aliyun\\alicloud\\%s\\windows_amd64\\",appdata_dir,ali_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\terraform-provider-alicloud_v%s.exe",dirname_temp,ali_plugin_version);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-alicloud_%s_windows_amd64.zip",appdata_dir,ali_plugin_version);
        if(file_exist_or_not(filename_temp)!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Downloading and installing necessary tools (3/5) ...                     |\n");
            sprintf(cmdline,"curl https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-win64/terraform-provider-alicloud_%s_windows_amd64.zip -o %s -s",ali_plugin_version,filename_temp);
            system(cmdline);
        }
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp,dirname_temp);
        system(cmdline);
    }

    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\tencentcloudstack\\tencentcloud\\%s\\windows_amd64\\",appdata_dir,qcloud_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\terraform-provider-tencentcloud_v%s.exe",dirname_temp,qcloud_plugin_version);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-tencentcloud_%s_windows_amd64.zip",appdata_dir,qcloud_plugin_version);
        if(file_exist_or_not(filename_temp)!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Downloading and installing necessary tools (4/5) ...                     |\n");
            sprintf(cmdline,"curl https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-win64/terraform-provider-tencentcloud_%s_windows_amd64.zip -o %s -s",qcloud_plugin_version,filename_temp);
            system(cmdline);
        }
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp,dirname_temp);
        system(cmdline);   
    }

    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\hashicorp\\aws\\%s\\windows_amd64\\",appdata_dir,aws_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\terraform-provider-aws_v%s_x5.exe",dirname_temp,aws_plugin_version);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-aws_%s_windows_amd64.zip",appdata_dir,aws_plugin_version);
        if(file_exist_or_not(filename_temp)!=0){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Downloading and installing necessary tools (5/5) ...                     |\n");
            sprintf(cmdline,"curl https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-win64/terraform-provider-aws_%s_windows_amd64.zip -o %s -s",aws_plugin_version,filename_temp);
            system(cmdline);
        }
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp,dirname_temp);
        system(cmdline);   
    }
    
    if(file_exist_or_not("c:\\programdata\\hpc-now\\now_crypto_seed.lock")!=0){
        generate_random_passwd(random_string);
        file_p=fopen("c:\\programdata\\hpc-now\\now_crypto_seed.lock","w+");
        fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
        fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
        fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
        fprintf(file_p,"%s\n",random_string);
        fclose(file_p);
    }
    system("attrib +h +s +r c:\\programdata\\hpc-now\\now_crypto_seed.lock");
    if(folder_exist_or_not(SSHKEY_DIR)!=0){
        system("mkdir c:\\hpc-now\\.ssh\\ > nul 2>&1");
    }
    system("attrib +h +s +r c:\\hpc-now\\.ssh");
    if(file_exist_or_not(usage_logfile)!=0){
        file_p=fopen(usage_logfile,"w+");
        fprintf(file_p,"UCID,CLOUD_VENDOR,NODE_NAME,vCPU,START_DATE,START_TIME,STOP_DATE,STOP_TIME,RUNNING_HOURS,CPUxHOURS,CPU_MODEL,CLOUD_REGION\n");
        fclose(file_p);
    }
    
    sprintf(filename_temp,"%s\\known_hosts",sshkey_dir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"del /f /s /q %s > nul 2>&1",filename_temp);
        system(cmdline);
    }

// create the syslog file.
    if(file_exist_or_not(operation_logfile)!=0){
        sprintf(cmdline,"type nul > %s",operation_logfile);
        system(cmdline);
    }

    strcpy(cmdline,"setx PATH C:\\WINDOWS\\system32;C:\\hpc-now\\;C:\\WINDOWS;C:\\WINDOWS\\System32\\Wbem;C:\\WINDOWS\\System32\\WindowsPowerShell\\v1.0\\;C:\\WINDOWS\\System32\\OpenSSH\\ > nul 2>&1");
    system(cmdline);

    printf("[ -INFO- ] Running environment successfully checked. HPC-NOW services are ready.    |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    return 0;
}

int uninstall_services(void){
    char doubleconfirm[128]="";
    system("whoami /groups | find \"S-1-16-12288\" > c:\\programdata\\check.txt.tmp 2>&1");
    if(file_empty_or_not("c:\\programdata\\check.txt.tmp")==0){
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please switch to administrator or users with administration privilege    |\n");
        printf("|          and run this command again to uninstall the HPC-NOW services.            |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;    
    }

    printf("\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   YOU ARE UNINSTALLING THE HPC-NOW SERVICES, PLEASE CONFIRM THE ISSUES BELOW:   *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   1. You have *DESTROYED* all the clusters managed by this device.              *|\n");
    printf("|*      This is * !!! EXTREMELY IMPORTANT !!! *                                    *|\n");
    printf("|*   2. You have *CHECKED* your cloud service account and all the resources        *|\n");
    printf("|*      created by the HPC-NOW services on this device have been destructed.       *|\n");
    printf("|*   3. You have *EXPORTED* the usage log and systemlog to a permenant directory,  *|\n");
    printf("|*      You can run 'hpcopr syslog' and 'hpcopr usage' to get the logs and save    *|\n");
    printf("|*      them to a directory such as C:\\                                            *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.   |\n");
        printf("|          Nothing changed.                                                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] UNINSTALLING THE SERVICES AND REMOVING THE DATA NOW ...                  |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
    system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    system("rd /s /q c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
    system("net user hpc-now /delete > nul 2>&1");
    printf("[ -DONE- ] The HPC-NOW cluster services have been deleted from this OS and device.  |\n");
    printf("|          There are still remaining files for the specific user 'hpc-now'.         |\n");
    printf("|          Please mannually delete the folder C:\\Users\\hpc-now after reboot.        |\n");
    printf("|          Thanks a lot for using HPC-NOW services!                                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int get_usage(char* usage_logfile){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(usage_logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to get the usage record. Either you haven't initialize your first |\n");
        printf("|          cluster, or there are internal errors. Please contact us for technical   |\n");
        printf("|          supports via: info@hpc-now.com or other channels. Exit now.              |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    system("del /f /s /q c:\\hpc-now\\cluster_usage_temp.log > nul 2>&1");
    sprintf(cmdline,"copy %s c:\\hpc-now\\cluster_usage_temp.log > nul 2>&1",usage_logfile);
    system(cmdline);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] The latest usage summary has been printed to the file below:             |\n");
    printf("|          c:\\hpc-now\\cluster_usage_temp.log                                        |\n");
    printf("|          You can use either MS Office Excel (*strongly recommended*) or other     |\n");
    printf("|          plain text editors (for example, notepad) to view the usage details.     |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Thanks for using HPC-NOW Services!                                       |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
    return 0;
}

int get_syslog(char* operation_logfile){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(operation_logfile)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to get the operation log. There might be internal errors. Please  |\n");
        printf("|          contact us for technical supports via: info@hpc-now.com                  |\n");
        printf("|          or other channels. Exit now.                                             |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    system("del /f /s /q c:\\hpc-now\\cluster_syslog_temp.log > nul 2>&1");
    sprintf(cmdline,"copy %s c:\\hpc-now\\cluster_syslog_temp.log > nul 2>&1",operation_logfile);
    system(cmdline);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] The latet operation log has been printed to the file below:              |\n");
    printf("|          c:\\hpc-now\\cluster_syslog_temp.log                                       |\n");
    printf("|          You can use either MS Office Excel (*strongly recommended*) or other     |\n");
    printf("|          plain text editors (for example, notepad) to view the detailed log.      |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -DONE- ] Thanks for using HPC-NOW Services!                                       |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    print_tail();
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
    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",crypto_exec,vaultdir,vaultdir,md5sum);
    if(system(cmdline)!=0){
        return -1;
    }
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
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
    sprintf(cmdline,"del /f /s /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    return 0;
}

int system_cleanup(void){
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    char appdata_dir[DIR_LENGTH]="";
    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");
    
    sprintf(cmdline,"del /f /s /q %s\\Microsoft\\Windows\\Recent\\* >nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(cmdline,"rd /q /s %s\\Microsoft\\Windows\\Recent\\ >nul 2>&1",appdata_dir);
    system(cmdline);
    return 0;
}

int write_log(char* workdir, char* operation_logfile, char* operation, int runflag){
    //Write a systemlog to the file c:\hpc-now\cluster_operation.log
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    FILE* file_p=fopen(operation_logfile,"a+");
    if(file_p==NULL){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -WARN- ] Failed to write operation log to the records. The cluster operation may  |\n");
        printf("|          not be affected, but will not be recorded to your system.                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    fprintf(file_p,"%d-%d-%d,%d:%d:%d,%s,%s,%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec,workdir,operation,runflag);
    fclose(file_p);
    return 0;
}

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char buffer1[64];
    char cloud_flag[16];
    int run_flag=0;
    char pwd[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH];
    char filename_temp[FILENAME_LENGTH]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char string_temp[128]="";
    int check_flag=0;

    print_header();

    if(argc==2&&strcmp(argv[1],"uninstall")==0){
        run_flag=uninstall_services();
        if(run_flag!=0){
            print_tail();
        }
        return run_flag;
    }

    if(check_internet()!=0){
        system_cleanup();
        write_log("NULL",operation_log,"INTERNET_FAILED",-3);
        return -3;
    }
    check_flag=check_and_install_prerequisitions(argv[0]);
    if(check_flag!=0&&check_flag!=127&&check_flag!=-1){
        system_cleanup();
        write_log("NULL",operation_log,"PREREQ_FAILED",-3);
        return -3;
    }
    else if(check_flag==127||check_flag==-1){
        return 0;
    }

    if(check_current_user()!=0){
        system_cleanup();
        write_log("NULL",operation_log,"USER_CHECK_FAILED",-5);
        return -5;
    }

    if(argc==1){
        print_help();
        system_cleanup();
        write_log("NULL",operation_log,"NONE",0);
        return 0;
    }

    if(strcmp(argv[1],"new")!=0&&strcmp(argv[1],"init")!=0&&strcmp(argv[1],"graph")!=0&&strcmp(argv[1],"usage")!=0&&strcmp(argv[1],"delc")!=0&&strcmp(argv[1],"addc")!=0&&strcmp(argv[1],"shutdownc")!=0&&strcmp(argv[1],"turnonc")!=0&&strcmp(argv[1],"reconfc")!=0&&strcmp(argv[1],"sleep")!=0&&strcmp(argv[1],"wakeup")!=0&&strcmp(argv[1],"destroy")!=0&&strcmp(argv[1],"vault")!=0&&strcmp(argv[1],"help")!=0&&strcmp(argv[1],"syslog")!=0&&strcmp(argv[1],"conf")!=0&&strcmp(argv[1],"reconfm")!=0&&strcmp(argv[1],"uninstall")!=0){
        print_help();
        system_cleanup();
        write_log("NULL",operation_log,argv[1],1);
        return 1;
    }

    if(strcmp(argv[1],"help")==0){
        print_help();
        system_cleanup();
        write_log("NULL",operation_log,argv[1],0);
        return 0;
    }

    if(strcmp(argv[1],"new")==0){
            if(argc==2){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Please specify either 'workdir' or 'keypair' as the second parameter.    |\n");
            printf("|              workdir: creating a new working directory for a new cluster.         |\n");
            printf("|              keypair: Rotating a new keypair for an existing cluster.             |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            system_cleanup();
            return -1;
        }
        if(strcmp(argv[2],"workdir")==0){
            run_flag=create_new_workdir(crypto_keyfile);
            system_cleanup();
            write_log("NULL",operation_log,"new workdir",run_flag);
            return 0;
        }
        else if(strcmp(argv[2],"keypair")==0){
            if(envcheck(pwd)!=0){
                write_log(pwd,operation_log,"new keypair",3);
                system_cleanup();
                return -1;
            }
            run_flag=rotate_new_keypair(pwd,crypto_keyfile);
            if(run_flag!=0){
                write_log(pwd,operation_log,"new keypair",-1);
                print_tail();
                system_cleanup();
                return -1;
            }
            write_log(pwd,operation_log,"new keypair",0);
            print_tail();
            system_cleanup();
            return 0;
        }
        else{
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Please specify either 'workdir' or 'keypair' as the second parameter.    |\n");
            printf("|              workdir: creating a new working directory for a new cluster.         |\n");
            printf("|              keypair: Rotating a new keypair for an existing cluster.             |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            system_cleanup();
            return -1;
        }
    }

    if(strcmp(argv[1],"usage")==0){
        run_flag=get_usage(usage_log);
        system_cleanup();
        write_log("NULL",operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"syslog")==0){
        run_flag=get_syslog(operation_log);
        system_cleanup();
        write_log("NULL",operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(envcheck(pwd)!=0){
        system_cleanup();
        write_log(pwd,operation_log,"NONE",3);
        return 3;
    }
    create_and_get_vaultdir(pwd,vaultdir);
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    if(get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer1,cloud_flag)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to get the key file. HPC-NOW services can not be started.         |\n");
        printf("|          Please contact info@hpc-now.com for technical supports.                  |\n");
        printf("|          Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        print_tail();
        system_cleanup();
        write_log(pwd,operation_log,"KEY_CHECK_FAILED",5);
        return 5;
    }

    if(check_pslock(pwd)==1){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Another process is operating the clusters, please wait the termination   |\n");
        printf("|          of that process. Currently no extra operation is permitted. Exit now.    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        print_tail();
        system_cleanup();
        write_log(pwd,operation_log,"PROCESS_LOCKED",7);
        return 7;
    }

    if(strcmp(argv[1],"conf")==0){
        if(get_default_conf(pwd,crypto_keyfile)==-1){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The current cluster is not empty. In order to protect current cluster,   |\n");
            printf("|          downloading default configuration file is not permitted. If you do want  |\n");
            printf("|          to reconfigure the cluster from the default configuration, please run    |\n");
            printf("|          the 'destroy' command first and retry. Exit now.                         |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            system_cleanup();
            write_log(pwd,operation_log,"CLUSTER_NOT_EMPTY",23);
            return 23;
        }
        else if(get_default_conf(pwd,crypto_keyfile)==1){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.     |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            system_cleanup();
            write_log(pwd,operation_log,"INTERNAL_ERROR",31);
            return 31;
        }
        else{
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ -INFO- ] The default configuration file has been downloaded to the 'conf' folder. |\n");
            printf("|          You can edit it, and then run the 'init' command to build a customized   |\n");
            printf("|          HPC cluster. Exit now.                                                   |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            system_cleanup();
            write_log(pwd,operation_log,argv[1],0);
            return 0;
        }
    }

    if(strcmp(argv[1],"init")==0){
        if(argc==2){
            if(strcmp(cloud_flag,"CLOUD_C")==0){
                run_flag=aws_cluster_init("",pwd,crypto_keyfile);
                system_cleanup();
                write_log(pwd,operation_log,argv[1],run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                run_flag=qcloud_cluster_init("",pwd,crypto_keyfile);
                system_cleanup();
                write_log(pwd,operation_log,argv[1],run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                run_flag=alicloud_cluster_init("",pwd,crypto_keyfile);
                system_cleanup();
                write_log(pwd,operation_log,argv[1],run_flag);
                return run_flag;
            }
        }
        else if(argc==3){
            if(strcmp(cloud_flag,"CLOUD_C")==0){
                run_flag=aws_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                run_flag=qcloud_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                run_flag=alicloud_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
        }
        else{
            if(strcmp(cloud_flag,"CLOUD_C")==0){
                run_flag=aws_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                run_flag=qcloud_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                run_flag=alicloud_cluster_init(argv[2],pwd,crypto_keyfile);
                system_cleanup();
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                return run_flag;
            }
        }
        system_cleanup();
        return 0;
    }

    if(cluster_empty_or_not(pwd)==0){
        print_empty_cluster_info();
        print_tail();
        delete_decrypted_files(pwd,crypto_keyfile);
        system_cleanup();
        write_log(pwd,operation_log,"EMPTY_CLUSTER",11);
        return 11;
    }

    if(strcmp(argv[1],"graph")==0){
        decrypt_files(pwd,crypto_keyfile);
        run_flag=graph(pwd,crypto_keyfile);
        if(run_flag!=0){
            print_empty_cluster_info();
        }
        print_tail();
        delete_decrypted_files(pwd,crypto_keyfile);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"vault")==0){
        run_flag=get_vault_info(pwd,crypto_keyfile);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"sleep")==0){
        run_flag=cluster_sleep(pwd,crypto_keyfile);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"wakeup")==0){
        if(argc==2){
            run_flag=cluster_wakeup(pwd,crypto_keyfile,"minimal");
            system_cleanup();
            sprintf(string_temp,"%s default",argv[1]);
            write_log(pwd,operation_log,string_temp,run_flag);
            return run_flag;
        }
        else{
            run_flag=cluster_wakeup(pwd,crypto_keyfile,argv[2]);
            system_cleanup();
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            return run_flag;
        }
    }

    if(cluster_asleep_or_not(pwd)==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] The current cluster is in the state of hibernation. No modification is   |\n");
        printf("|          permitted in order to avoid unexpected results.                          |\n");
        printf("|          Please run 'wakeup' command first to modify the cluster. You             |\n");
        printf("|          can run 'wakeup minimal' option to turn the management nodes on, or      |\n");
        printf("|          run 'wakeup all' option to turn the whole cluster on. Exit now.          |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        write_log(pwd,operation_log,argv[1],13);
        return 13;
    }

    if(strcmp(argv[1],"destroy")==0&&cluster_empty_or_not(pwd)==1){
        run_flag=cluster_destroy(pwd,crypto_keyfile);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"delc")==0){
        if(argc==2){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] You need to specify a number or 'all' as the second parameter.           |\n");
            printf("|          Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        run_flag=delete_compute_node(pwd,crypto_keyfile,argv[2]);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"addc")==0){
        if(argc==2){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] You need to specify a number (range: 1-16) as the second parameter.      |\n");
            printf("|          Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        run_flag=add_compute_node(pwd,crypto_keyfile,argv[2]);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"shutdownc")==0){
        if(argc==2){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.    |\n");
            printf("|          Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        run_flag=shudown_compute_nodes(pwd,crypto_keyfile,argv[2]);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"turnonc")==0){
        if(argc==2){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.    |\n");
            printf("|          Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        run_flag=turn_on_compute_nodes(pwd,crypto_keyfile,argv[2]);
        system_cleanup();
        write_log(pwd,operation_log,argv[1],run_flag);
        return run_flag;
    }

    if(strcmp(argv[1],"reconfc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a configuration as the second parameter.             |\n");
            if(check_reconfigure_list(pwd)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Internal error. Please contact HPC-NOW via info@hpc-now.com              |\n");
                printf("|          for technical supports. Exit now.                                        |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                print_tail();
                system_cleanup();
                write_log(pwd,operation_log,argv[1],-1);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");            
            print_tail();
            system_cleanup();
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        else if(argc==3){
            run_flag=reconfigure_compute_node(pwd,crypto_keyfile,argv[2],"");
            system_cleanup();
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            return run_flag;
        }
        else{
            run_flag=reconfigure_compute_node(pwd,crypto_keyfile,argv[2],argv[3]);
            system_cleanup();
            sprintf(string_temp,"%s %s %s",argv[1],argv[2],argv[3]);
            write_log(pwd,operation_log,string_temp,run_flag);
            return run_flag;
        }
    }

    if(strcmp(argv[1],"reconfm")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a configuration as the second parameter.             |\n");
            if(check_reconfigure_list(pwd)!=0){
                printf("+-----------------------------------------------------------------------------------+\n");
                printf("[ FATAL: ] Internal error. Please contact HPC-NOW via info@hpc-now.com              |\n");
                printf("|          for technical supports. Exit now.                                        |\n");
                printf("+-----------------------------------------------------------------------------------+\n");
                print_tail();
                system_cleanup();
                write_log(pwd,operation_log,argv[1],-1);
                return -1;
            }
            printf("+-----------------------------------------------------------------------------------+\n");            
            print_tail();
            system_cleanup();
            write_log(pwd,operation_log,argv[1],17);
            return 17;
        }
        else{
            run_flag=reconfigure_master_node(pwd,crypto_keyfile,argv[2]);
            system_cleanup();
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            return run_flag;
        }
    }
    system_cleanup();
    return 0;
}