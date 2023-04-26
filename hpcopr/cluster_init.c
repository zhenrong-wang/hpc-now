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
#include "cluster_general_funcs.h"
#include "general_funcs.h"
#include "general_print_info.h"
#include "cluster_init.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern char url_shell_scripts_var[LOCATION_LENGTH];
extern int code_loc_flag_var;

int aws_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char* error_log=OPERATION_ERROR_LOG;
    char secret_file[FILENAME_LENGTH]="";
    char region_valid[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;
    char url_aws_root[LOCATION_LENGTH_EXTENDED]="";
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
    char cluster_id[CONF_STRING_LENTH]="";
    char region_id[CONF_STRING_LENTH]="";
    char region_flag[16]="";
    char os_image[64]="";
    char db_os_image[64]="";
    char nat_os_image[64]="";
    char zone_id[CONF_STRING_LENTH]="";
    int node_num=0;
    int hpc_user_num=0;
    char master_init_param[CONF_STRING_LENTH]="";
    char master_passwd[CONF_STRING_LENTH]="";
    char compute_passwd[CONF_STRING_LENTH]="";
    char master_inst[CONF_STRING_LENTH]="";
    char compute_inst[CONF_STRING_LENTH]="";
    char os_image_raw[32]="";
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
    int region_valid_flag=0;
    int i,j;
    if(folder_exist_or_not(workdir)==1){
        return -1;
    }
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);
#else
    sprintf(logdir,"%s/log/",workdir);
    sprintf(confdir,"%s/conf/",workdir);
    sprintf(currentstate,"%s/currentstate",stackdir);
    sprintf(compute_template,"%s/compute_template",stackdir);
#endif
    printf("[ START: ] Start initializing the cluster ...\n");
#ifdef _WIN32
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
    }
#else
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir -p %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir -p %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir -p %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir -p %s",confdir);
        system(cmdline);
    }
#endif
    if(code_loc_flag_var==1){
#ifdef _WIN32
        sprintf(url_aws_root,"%s\\aws\\",url_code_root_var);
#else
        sprintf(url_aws_root,"%s/aws/",url_code_root_var);
#endif
    }
    else{
        sprintf(url_aws_root,"%saws/",url_code_root_var);
    }

    if(code_loc_flag_var==1){
#ifdef _WIN32
        sprintf(cmdline,"copy /y %s\\region_valid.tf %s\\region_valid.tf > nul 2>&1",url_aws_root,stackdir);
#else
        sprintf(cmdline,"/bin/cp %s/region_valid.tf %s/region_valid.tf >> /dev/null 2>&1",url_aws_root,stackdir);
#endif
    }
    else{
#ifdef _WIN32
        sprintf(cmdline,"curl %sregion_valid.tf -o %s\\region_valid.tf -s",url_aws_root,stackdir);
#else
        sprintf(cmdline,"curl %sregion_valid.tf -o %s/region_valid.tf -s",url_aws_root,stackdir);
#endif
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    sprintf(region_valid,"%s\\region_valid.tf",stackdir);
#else
    sprintf(cmdline,"rm -rf %s/hpc_stack* >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(secret_file,"%s/.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    sprintf(region_valid,"%s/region_valid.tf",stackdir);
#endif
    global_replace(region_valid,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(region_valid,"BLANK_SECRET_KEY",secret_key);
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        global_replace(region_valid,"cn-northwest-1","us-east-1");
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
            printf("[ FATAL: ] The keypair may be invalid. Please use 'hpcopr new-keypair' to update with a\n");
            printf("|          valid keypair. Exit now.\n");
#ifdef _WIN32
            sprintf(cmdline,"del /f /q %s\\region_valid.tf > nul 2>&1",stackdir);
#else
            sprintf(cmdline,"rm -rf %s/region_valid.tf >> /dev/null 2>&1",stackdir);
#endif
            system(cmdline);
            return -1;
        }
        region_valid_flag=1;
    }
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\region_valid.tf > nul 2>&1",stackdir);
#else
    sprintf(cmdline,"rm -rf %s/region_valid.tf >> /dev/null 2>&1",stackdir);
#endif
    system(cmdline);
    
#ifdef _WIN32
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("[ -INFO- ] IMPORTANT: No configure file found. Downloading the default configure\n");
        printf("|          file to initialize this cluster.\n");
        if(code_loc_flag_var==1){
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s > nul 2>&1", url_aws_root,conf_file);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_aws_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
            return 2;
        }
        if(region_valid_flag==1){
            global_replace(conf_file,"cn-northwest-1","us-east-1");
        }
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy %s\\hpc_stack_aws.base %s\\hpc_stack.base > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.base -o %s\\hpc_stack.base -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_aws.master %s\\hpc_stack.master > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.master -o %s\\hpc_stack.master -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_aws.compute %s\\hpc_stack.compute > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.compute -o %s\\hpc_stack.compute -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_aws.database %s\\hpc_stack.database > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.database -o %s\\hpc_stack.database -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_aws.natgw %s\\hpc_stack.natgw > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.natgw -o %s\\hpc_stack.natgw -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\reconf.list %s\\reconf.list > nul 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
#else
    sprintf(conf_file,"%s/tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("[ -INFO- ] IMPORTANT: No configure file found. Use the default one.\n");
        if(code_loc_flag_var==1){
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s >> /dev/null 2>&1",url_aws_root,conf_file);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_aws_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
            return 2;
        }
        if(region_valid_flag==1){
            global_replace(conf_file,"cn-northwest-1","us-east-1");
        }
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_aws.base %s/hpc_stack.base >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.base -o %s/hpc_stack.base -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_aws.master %s/hpc_stack.master >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.master -o %s/hpc_stack.master -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_aws.compute %s/hpc_stack.compute >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.compute -o %s/hpc_stack.compute -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_aws.database %s/hpc_stack.database >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.database -o %s/hpc_stack.database -s",url_aws_root,stackdir);
    }
    
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_aws.natgw %s/hpc_stack.natgw >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.natgw -o %s/hpc_stack.natgw -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/reconf.list %s/reconf.list >> /dev/null 2>&1",url_aws_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s/reconf.list -s",url_aws_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
#endif
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
    if(node_num>MAXIMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n",hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf("[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n",hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
        hpc_user_num=MINIMUM_ADD_USER_NUNMBER;
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
        printf("[ FATAL: ] Currently the NOW Cluster Service only support AWS Regions below:\n");
        printf("|          cn-northwest-1 | cn-north-1 | us-east-1 | us-east-2\n");
        printf("|          If you'd like to use NOW Cluster in other AWS regions,\n");
        printf("|          Please contact info@hpc-now.com\n\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
    if(contain_or_not(zone_id,region_id)!=0){
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
    if(strcmp(region_id,"cn-northwest-1")==0){
        if(region_valid_flag==1){
            printf("[ FATAL: ] The keypair is not valid to operate clusters in AWS China regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n");
            return -1;
        }
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.0",os_image_raw);
        strcpy(db_os_image,"centos7cn.0");
        strcpy(nat_os_image,"centos7cn.0");
    }
    else if(strcmp(region_id,"cn-north-1")==0){
        if(region_valid_flag==1){
            printf("[ FATAL: ] The keypair is not valid to operate clusters in AWS China regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n");
            return -1;
        }
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.1",os_image_raw);
        strcpy(db_os_image,"centos7cn.1");
        strcpy(nat_os_image,"centos7cn.1");
    }
    else if(strcmp(region_id,"us-east-1")==0){
        if(region_valid_flag==0){
            printf("[ FATAL: ] The keypair is not valid to operate clusters in AWS global regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n");
            return -1;
        }
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.0",os_image_raw);
        strcpy(db_os_image,"centos7global.0");
        strcpy(nat_os_image,"centos7global.0");
    }
    else if(strcmp(region_id,"us-east-2")==0){
        if(region_valid_flag==0){
            printf("[ FATAL: ] The keypair is not valid to operate clusters in AWS global regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n");
            return -1;
        }
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.1",os_image_raw);
        strcpy(db_os_image,"centos7global.1");
        strcpy(nat_os_image,"centos7global.1");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/db_passwords.txt",vaultdir);
#endif
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
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>%d).\n",cluster_id_input,CLUSTER_ID_LENGTH_MAX);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.\n");
        printf("|          Extend to %s.\n", cluster_id);
    }
    else{
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    if(file_exist_or_not(filename_temp)==1){
        printf("[ -INFO- ] Creating a Unique Cluster ID now...\n");
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
#ifdef _WIN32
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/root_passwords.txt",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n",os_image_raw);
    generate_sshkey(sshkey_folder,pubkey);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.base",stackdir);
#endif
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
    global_replace(filename_temp,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.master",stackdir);
#endif
    
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.compute",stackdir);
#endif
    
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    sprintf(string_temp,"%d",cpu_core_num);
    global_replace(filename_temp,"CPU_CORE_NUM",string_temp);
    sprintf(string_temp,"%d",threads);
    global_replace(filename_temp,"THREADS_PER_CORE",string_temp);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.database",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"DB_OS_IMAGE",db_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.natgw",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"NAT_OS_IMAGE",nat_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
#ifdef _WIN32
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy /y %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
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
#else
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"/bin/cp %s/hpc_stack.compute %s/hpc_stack_compute%d.tf >> /dev/null 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        sprintf(string_temp,"comp%d",i+1);
        global_replace(filename_temp,"NUMBER",string_temp);
    }
    sprintf(cmdline,"mv  %s/hpc_stack.base %s/hpc_stack_base.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.database %s/hpc_stack_database.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.master %s/hpc_stack_master.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.natgw %s/hpc_stack_natgw.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/hpc_stack.compute >> /dev/null 2>&1",stackdir);
    system(cmdline);
#endif
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        printf("[ -INFO- ] Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
#ifdef _WIN32
            system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
            sprintf(cmdline,"move %s\\*.tmp c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\UCID_LATEST.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\conf\\tf_prep.conf %s\\conf\\tf_prep.conf.destroyed > nul 2>&1",workdir,workdir);
            system(cmdline);
#elif __APPLE__
            system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#elif __linux__
            system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#endif
            printf("[ -INFO- ] Successfully rolled back. Please check the errolog for details.\n");
            return -1;
        }
        printf("[ -INFO- ] Failed to roll back. Please try 'hpcopr destroy' later.\n");
        return -1;
    }
#ifdef _WIN32
    sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
#else
    sprintf(cmdline,"/bin/cp %s/hpc_stack_compute1.tf %s/compute_template >> /dev/null 2>&1",stackdir,stackdir);
#endif
    system(cmdline);
    getstate(workdir,crypto_keyfile);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate",stackdir);
#endif
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    find_and_get(filename_temp,"aws_iam_access_key","","",15,"\"id\":","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"aws_iam_access_key","","",15,"\"secret\":","","",'\"',4,bucket_sk);
    if(strcmp(region_flag,"global_regions")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_GLOBAL);
        for(i=0;i<AWS_SLEEP_TIME_GLOBAL;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_GLOBAL-i);
            fflush(stdout);
            sleep(1);
        }
        printf("[ -DONE- ] Remote execution commands sent.\n");
    }
    else{
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_CN);
        for(i=0;i<AWS_SLEEP_TIME_CN;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_CN-i);
            fflush(stdout);
            sleep(1);
        }
        printf("[ -DONE- ] Remote execution commands sent.\n");
    }
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
#ifdef _WIN32
    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
#else
    sprintf(private_key_file,"%s/now-cluster-login",sshkey_folder);
#endif
    if(strcmp(region_flag,"cn_regions")==0){
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\s3cfg.txt %s\\s3cfg.txt > nul 2>&1",url_aws_root,stackdir);
#else
            sprintf(cmdline,"/bin/cp %s/s3cfg.txt %s/s3cfg.txt >> /dev/null 2>&1",url_aws_root,stackdir);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s\\s3cfg.txt",url_aws_root,stackdir);
#else
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s/s3cfg.txt",url_aws_root,stackdir);
#endif
        }
        if(system(cmdline)!=0){
            printf("[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n");
        }
        else{
#ifdef _WIN32
            sprintf(filename_temp,"%s\\s3cfg.txt",stackdir);
#else
            sprintf(filename_temp,"%s/s3cfg.txt",stackdir);
#endif
            sprintf(string_temp,"s3.%s.amazonaws.com.cn",region_id);
            global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
            global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
            global_replace(filename_temp,"DEFAULT_REGION",region_id);
            global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
#ifdef _WIN32
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg > nul 2>&1",private_key_file,filename_temp,master_address);
            system(cmdline);
            sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg >> /dev/null 2>&1",private_key_file,filename_temp,master_address);
            system(cmdline);
            sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
            system(cmdline);
        }
    }
    else{
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\s3cfg.txt %s\\s3cfg.txt > nul 2>&1",url_aws_root,stackdir);
#else
            sprintf(cmdline,"/bin/cp %s/s3cfg.txt %s/s3cfg.txt >> /dev/null 2>&1",url_aws_root,stackdir);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s\\s3cfg.txt",url_aws_root,stackdir);
#else
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s/s3cfg.txt",url_aws_root,stackdir);
#endif
        }
        if(system(cmdline)!=0){
            printf("[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n");
        }
        else{
#ifdef _WIN32
            sprintf(filename_temp,"%s\\s3cfg.txt",stackdir);
#else
            sprintf(filename_temp,"%s/s3cfg.txt",stackdir);
#endif
            strcpy(string_temp,"s3.amazonaws.com");
            global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
            global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
            global_replace(filename_temp,"DEFAULT_REGION",region_id);
            global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
#ifdef _WIN32
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg > nul 2>&1",private_key_file,filename_temp,master_address);
            system(cmdline);
            sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg >> /dev/null 2>&1",private_key_file,filename_temp,master_address);
            system(cmdline);
            sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
            system(cmdline);
        }
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#else
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: s3:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\db_passwds.txt > nul 2>&1",vaultdir);
#else
    sprintf(cmdline,"rm -rf %s/db_passwds.txt >> /dev/null 2>&1",vaultdir);
#endif
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    get_crypto_key(crypto_keyfile,md5sum);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\root_passwords.txt > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
#else
    sprintf(cmdline,"rm -rf %s/root_passwords.txt >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s/_CLUSTER_SUMMARY.txt.tmp %s/_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt.tmp >> /dev/null 2>&1",vaultdir);
    system(cmdline);
#endif   
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
#else
    sprintf(filename_temp,"%s/.cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s/.cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
    }
#endif
    printf("[ -INFO- ] After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
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
    print_cluster_init_done();
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
    char* error_log=OPERATION_ERROR_LOG;
    char secret_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;
    char url_qcloud_root[LOCATION_LENGTH_EXTENDED];
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

#ifdef _WIN32
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);
#else
    sprintf(logdir,"%s/log/",workdir);
    sprintf(confdir,"%s/conf/",workdir);
    sprintf(currentstate,"%s/currentstate",stackdir);
    sprintf(compute_template,"%s/compute_template",stackdir);
#endif
    printf("[ START: ] Start initializing the cluster ...\n");
#ifdef _WIN32
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
    }
#else
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir -p %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir -p %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir -p %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir -p %s",confdir);
        system(cmdline);
    }
#endif
    if(code_loc_flag_var==1){
#ifdef _WIN32
        sprintf(url_qcloud_root,"%s\\qcloud\\",url_code_root_var);
#else
        sprintf(url_qcloud_root,"%s/qcloud/",url_code_root_var);
#endif
    }
    else{
        sprintf(url_qcloud_root,"%sqcloud/",url_code_root_var);
    }

#ifdef _WIN32
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
#else
    sprintf(conf_file,"%s/tf_prep.conf",confdir);
#endif
    if(file_exist_or_not(conf_file)==1){
        printf("[ -INFO- ] IMPORTANT: No configure file found. Downloading the default configure\n");
        printf("|          file to initialize this cluster.\n");
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s > nul 2>&1", url_qcloud_root,conf_file);
#else
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s >> /dev/null 2>&1", url_qcloud_root,conf_file);
#endif
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_qcloud_root,conf_file);
        }
            if(system(cmdline)!=0){
            printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
            return 2;
        }
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_qcloud.base %s\\hpc_stack.base > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.base -o %s\\hpc_stack.base -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_qcloud.master %s\\hpc_stack.master > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.master -o %s\\hpc_stack.master -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_qcloud.compute %s\\hpc_stack.compute > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.compute -o %s\\hpc_stack.compute -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stack_qcloud.database %s\\hpc_stack.database > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.database -o %s\\hpc_stack.database -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy %s\\hpc_stack_qcloud.natgw %s\\hpc_stack.natgw > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.natgw -o %s\\hpc_stack.natgw -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\NAS_Zones_QCloud.txt %s\\NAS_Zones_QCloud.txt > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_QCloud.txt -o %s\\NAS_Zones_QCloud.txt -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\reconf.list %s\\reconf.list > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }

    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
#else
    sprintf(conf_file,"%s/tf_prep.conf",confdir);
    if(file_exist_or_not(conf_file)==1){
        printf("[ -INFO- ] IMPORTANT: No configure file found. Use the default one.\n");
        if(code_loc_flag_var==1){
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s >> /dev/null 2>&1", url_qcloud_root,conf_file);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_qcloud_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
            return 2;
        }
    }
    sprintf(cmdline,"rm -rf %s/hpc_stack* >> /dev/null 2>&1",stackdir);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_qcloud.base %s/hpc_stack.base >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.base -o %s/hpc_stack.base -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_qcloud.master %s/hpc_stack.master >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.master -o %s/hpc_stack.master -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_qcloud.compute %s/hpc_stack.compute >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.compute -o %s/hpc_stack.compute -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_qcloud.database %s/hpc_stack.database >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.database -o %s/hpc_stack.database -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stack_qcloud.natgw %s/hpc_stack.natgw >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.natgw -o %s/hpc_stack.natgw -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/NAS_Zones_QCloud.txt %s/NAS_Zones_QCloud.txt >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_QCloud.txt -o %s/NAS_Zones_QCloud.txt -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/reconf.list %s/reconf.list >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s/reconf.list -s",url_qcloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    sprintf(secret_file,"%s/.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
#endif
    file_p=fopen(conf_file,"r");
    for(i=0;i<3;i++){
        fgetline(file_p,conf_line_buffer);
    }
    fgetline(file_p,conf_line_buffer);
    get_seq_string(conf_line_buffer,' ',3,cluster_id);
    fgetline(file_p,conf_line_buffer);
    get_seq_string(conf_line_buffer,' ',3,region_id);
    fgetline(file_p,conf_line_buffer);
    get_seq_string(conf_line_buffer,' ',3,zone_id);
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        node_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(node_num>MAXIMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n",hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf("[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n",hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
        hpc_user_num=MINIMUM_ADD_USER_NUNMBER;
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
        printf("[ -WARN- ] The master node bandwidth %d exceeds the maximum value 50, reset to 50.\n",node_num);
        master_bandwidth=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\NAS_Zones_QCloud.txt",stackdir);
#else
    sprintf(filename_temp,"%s/NAS_Zones_QCloud.txt",stackdir);
#endif
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/db_passwords.txt",vaultdir);
#endif
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
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>%d).\n",cluster_id_input,CLUSTER_ID_LENGTH_MAX);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.\n");
        printf("|          Extend to %s.\n", cluster_id);
    }
    else{
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    if(file_exist_or_not(filename_temp)==1){
        printf("[ -INFO- ] Creating a Unique Cluster ID now...\n");
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
#ifdef _WIN32
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/root_passwords.txt",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n",os_image);
    generate_sshkey(sshkey_folder,pubkey);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.base",stackdir);
#endif
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
    global_replace(filename_temp,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.master",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.compute",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"OS_IMAGE",os_image);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.database",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.natgw",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);

#ifdef _WIN32
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy /y %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
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
#else
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"/bin/cp %s/hpc_stack.compute %s/hpc_stack_compute%d.tf >> /dev/null 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        global_replace(filename_temp,"RUNNING_FLAG","true");
    }
    sprintf(cmdline,"mv %s/hpc_stack.base %s/hpc_stack_base.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.database %s/hpc_stack_database.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.master %s/hpc_stack_master.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.natgw %s/hpc_stack_natgw.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/hpc_stack.compute >> /dev/null && rm -rf %s/NAS_Zones_QCloud.txt >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
#endif
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        printf("[ -INFO- ] Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
#ifdef _WIN32
            system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
            sprintf(cmdline,"move %s\\*.tmp c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\UCID_LATEST.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\conf\\tf_prep.conf %s\\conf\\tf_prep.conf.destroyed > nul 2>&1",workdir,workdir);
            system(cmdline);
#elif __APPLE__
            system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#elif __linux__
            system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#endif
            printf("[ -INFO- ] Successfully rolled back. Please check the errolog for details.\n");
            return -1;
        }
        printf("[ -INFO- ] Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
#ifdef _WIN32
    sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
#else
    sprintf(cmdline,"/bin/cp %s/hpc_stack_compute1.tf %s/compute_template >> /dev/null 2>&1",stackdir,stackdir);
#endif
    system(cmdline);
    getstate(workdir,crypto_keyfile);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate",stackdir);
#endif
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    find_and_get(filename_temp,"secret_id","","",1,"secret_id","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"secret_key","","",1,"secret_key","","",'\"',4,bucket_sk);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",QCLOUD_SLEEP_TIME);
    for(i=0;i<QCLOUD_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",QCLOUD_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf("[ -DONE- ] Remote execution commands sent.\n");
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
#ifdef _WIN32
    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\cos.conf %s\\cos.conf > nul 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %scos.conf -s -o %s\\cos.conf",url_qcloud_root,stackdir);
    }
#else
    sprintf(private_key_file,"%s/now-cluster-login",sshkey_folder);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/cos.conf %s/cos.conf >> /dev/null 2>&1",url_qcloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %scos.conf -s -o %s/cos.conf",url_qcloud_root,stackdir);
    }
#endif
    if(system(cmdline)!=0){
        printf("[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n");
    }
    else{
#ifdef _WIN32
        sprintf(filename_temp,"%s\\cos.conf",stackdir);
#else
        sprintf(filename_temp,"%s/cos.conf",stackdir);
#endif
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
        global_replace(filename_temp,"BLANK_BUCKET_NAME",bucket_id);
#ifdef _WIN32
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.cos.conf > nul 2>&1",private_key_file,filename_temp,master_address);
        system(cmdline);
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.cos.conf\" > nul 2>&1",private_key_file,master_address);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.cos.conf >> /dev/null 2>&1",private_key_file,filename_temp,master_address);
        system(cmdline);
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.cos.conf\" >> /dev/null 2>&1",private_key_file,master_address);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#else
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: cos: %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\db_passwords.txt > nul 2>&1",vaultdir);
#else
    sprintf(cmdline,"rm -rf %s/db_passwords.txt >> /dev/null 2>&1",vaultdir);
#endif
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    get_crypto_key(crypto_keyfile,md5sum);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\root_passwords.txt > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
#else
    sprintf(cmdline,"rm -rf %s/root_passwords.txt >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s/_CLUSTER_SUMMARY.txt.tmp %s/_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt.tmp >> /dev/null 2>&1",vaultdir);
    system(cmdline);
#endif
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
#else
    sprintf(filename_temp,"%s/.cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s/.cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
    }
#endif
    printf("[ -INFO- ] After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
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
    print_cluster_init_done();
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
    char* error_log=OPERATION_ERROR_LOG;
    char secret_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;
    char url_alicloud_root[LOCATION_LENGTH_EXTENDED]="";
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
#ifdef _WIN32
    sprintf(logdir,"%s\\log\\",workdir);
    sprintf(confdir,"%s\\conf\\",workdir);
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);
#else
    sprintf(logdir,"%s/log/",workdir);
    sprintf(confdir,"%s/conf/",workdir);
    sprintf(currentstate,"%s/currentstate",stackdir);
    sprintf(compute_template,"%s/compute_template",stackdir);
#endif
    printf("[ START: ] Start initializing the cluster ...\n");
#ifdef _WIN32
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir %s",confdir);
        system(cmdline);
    }


#else
    if(folder_exist_or_not(stackdir)==1){
        sprintf(cmdline,"mkdir -p %s",stackdir);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)==1){
        sprintf(cmdline,"mkdir -p %s",vaultdir);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)==1){
        sprintf(cmdline,"mkdir -p %s",logdir);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)==1){
        sprintf(cmdline,"mkdir -p %s",confdir);
        system(cmdline);
    }
#endif
    if(code_loc_flag_var==1){
#ifdef _WIN32
        sprintf(url_alicloud_root,"%s\\alicloud\\",url_code_root_var);
#else
        sprintf(url_alicloud_root,"%s/alicloud/",url_code_root_var);
#endif
    }
    else{
        sprintf(url_alicloud_root,"%salicloud/",url_code_root_var);
    }

#ifdef _WIN32
    sprintf(conf_file,"%s\\tf_prep.conf",confdir);
#else
    sprintf(conf_file,"%s/tf_prep.conf",confdir);
#endif
    if(file_exist_or_not(conf_file)==1){
        printf("[ -INFO- ] IMPORTANT: No configure file found. Use the default one. \n");
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s > nul 2>&1",url_alicloud_root,conf_file);         
#else
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s >> /dev/null 2>&1",url_alicloud_root,conf_file);
#endif
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s",url_alicloud_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
            return 2;
        }
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s\\hpc_stack* > nul 2>&1",stackdir);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stackv2.base %s\\hpc_stack.base > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.base -o %s\\hpc_stack.base -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stackv2.master %s\\hpc_stack.master > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.master -o %s\\hpc_stack.master -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stackv2.compute %s\\hpc_stack.compute > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.compute -o %s\\hpc_stack.compute -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stackv2.database %s\\hpc_stack.database > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.database -o %s\\hpc_stack.database -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\hpc_stackv2.natgw %s\\hpc_stack.natgw > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.natgw -o %s\\hpc_stack.natgw -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\NAS_Zones_ALI.txt %s\\NAS_Zones_ALI.txt > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_ALI.txt -o %s\\NAS_Zones_ALI.txt -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\reconf.list %s\\reconf.list > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s\\reconf.list -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    sprintf(secret_file,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
#else
    sprintf(cmdline,"rm -rf %s/hpc_stack* >> /dev/null 2>&1",stackdir);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stackv2.base %s/hpc_stack.base >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.base -o %s/hpc_stack.base -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stackv2.master %s/hpc_stack.master >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.master -o %s/hpc_stack.master -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stackv2.compute %s/hpc_stack.compute >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.compute -o %s/hpc_stack.compute -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stackv2.database %s/hpc_stack.database >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.database -o %s/hpc_stack.database -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/hpc_stackv2.natgw %s/hpc_stack.natgw >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.natgw -o %s/hpc_stack.natgw -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/NAS_Zones_ALI.txt %s/NAS_Zones_ALI.txt >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_ALI.txt -o %s/NAS_Zones_ALI.txt -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/reconf.list %s/reconf.list >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s/reconf.list -s",url_alicloud_root,stackdir);
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n");
        return 2;
    }
    sprintf(secret_file,"%s/.secrets.txt",vaultdir);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
#endif 
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
    if(node_num>MAXIMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf("[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n",node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf("[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n",hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf("[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n",hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
        hpc_user_num=MINIMUM_ADD_USER_NUNMBER;
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
        printf("[ -WARN- ] The master node bandwidth %d exceeds the maximum value 50, reset to 50.\n",node_num);
        master_bandwidth=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\NAS_Zones_ALI.txt",stackdir);
#else
    sprintf(filename_temp,"%s/NAS_Zones_ALI.txt",stackdir);
#endif
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf("[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/db_passwords.txt",vaultdir);
#endif
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
        printf("[ -WARN- ] The CLUSTER_ID '%s' specified by the command is too long (length>%d).\n",cluster_id_input,CLUSTER_ID_LENGTH_MAX);
        printf("|          Cut it to %s\n",cluster_id);
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
        printf("[ -WARN- ] The CLUSTER_ID specified by the command and conf file is too short.\n");
        printf("|          Extend to %s.\n", cluster_id);
    }
    else{
        printf("[ -INFO- ] Using the CLUSTER_ID '%s' speficied in the conf file.\n",cluster_id);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    if(file_exist_or_not(filename_temp)==1){
        printf("[ -INFO- ] Creating a Unique Cluster ID now...\n");
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
#ifdef _WIN32
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/root_passwords.txt",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf("[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n",os_image);
    generate_sshkey(sshkey_folder,pubkey);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.base",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.base",stackdir);
#endif
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
    global_replace(filename_temp,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.master",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.master",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.compute",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.compute",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.database",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.database",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack.natgw",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack.natgw",stackdir);
#endif
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    reset_string(filename_temp);
    reset_string(string_temp);

#ifdef _WIN32
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"copy /y %s\\hpc_stack.compute %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1);
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
#else
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"/bin/cp %s/hpc_stack.compute %s/hpc_stack_compute%d.tf >> /dev/null 2>&1",stackdir,stackdir,i+1);
        system(cmdline);
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
    }

    sprintf(cmdline,"mv %s/hpc_stack.base %s/hpc_stack_base.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.database %s/hpc_stack_database.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.master %s/hpc_stack_master.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hpc_stack.natgw %s/hpc_stack_natgw.tf >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/hpc_stack.compute >> /dev/null && rm -rf %s/NAS_Zones_ALI.txt >> /dev/null 2>&1",stackdir,stackdir);
    system(cmdline);
#endif
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        printf("[ -INFO- ] Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
#ifdef _WIN32
            system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
            sprintf(cmdline,"move %s\\*.tmp c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\UCID_LATEST.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"move %s\\conf\\tf_prep.conf %s\\conf\\tf_prep.conf.destroyed > nul 2>&1",workdir,workdir);
            system(cmdline);
#elif __APPLE__
            system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#elif __linux__
            system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
            sprintf(cmdline,"mv %s/*.tmp /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/UCID_LATEST.txt /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
            system(cmdline);
            sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
            system(cmdline);
#endif
            printf("[ -INFO- ] Successfully rolled back. Please check the errolog for details.\n");
            return -1;
        }
        printf("[ -INFO- ] Failed to roll back. Please try 'hpcopr destroy' later.\n");
        return -1;
    }
#ifdef _WIN32
    sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s\\compute_template > nul 2>&1",stackdir,stackdir);
#else
    sprintf(cmdline,"/bin/cp %s/hpc_stack_compute1.tf %s/compute_template >> /dev/null 2>&1",stackdir,stackdir);
#endif
    system(cmdline);
    get_crypto_key(crypto_keyfile,md5sum);
    getstate(workdir,crypto_keyfile);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",ALI_SLEEP_TIME);
    for(i=0;i<ALI_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",ALI_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf("[ -DONE- ] Remote execution commands sent.\n");
#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    sprintf(filename_temp,"%s\\bucket_secrets.txt",stackdir);
    find_and_get(filename_temp,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk);
    sprintf(cmdline,"del /f /s /q %s > nul 2>&1",filename_temp);
#else
    sprintf(filename_temp,"%s/terraform.tfstate",stackdir);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    sprintf(filename_temp,"%s/bucket_secrets.txt",stackdir);
    find_and_get(filename_temp,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
#ifdef _WIN32
    sprintf(private_key_file,"%s\\now-cluster-login",sshkey_folder);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"copy /y %s\\.ossutilconfig %s\\ossutilconfig > nul 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %s.ossutilconfig -s -o %s\\ossutilconfig",url_alicloud_root,stackdir);
    }
#else
    sprintf(private_key_file,"%s/now-cluster-login",sshkey_folder);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"/bin/cp %s/.ossutilconfig %s/ossutilconfig >> /dev/null 2>&1",url_alicloud_root,stackdir);
    }
    else{
        sprintf(cmdline,"curl %s.ossutilconfig -s -o %s/ossutilconfig",url_alicloud_root,stackdir);
    }
#endif
    if(system(cmdline)!=0){
        printf("[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n");
    }
    else{
#ifdef _WIN32
        sprintf(filename_temp,"%s\\ossutilconfig",stackdir);
#else
        sprintf(filename_temp,"%s/ossutilconfig",stackdir);
#endif
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
#ifdef _WIN32
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.ossutilconfig > nul 2>&1",private_key_file,filename_temp,master_address);
        system(cmdline);
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.ossutilconfig\" > nul 2>&1",private_key_file,master_address);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.ossutilconfig >> /dev/null 2>&1",private_key_file,filename_temp,master_address);
        system(cmdline);
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.ossutilconfig\" >> /dev/null 2>&1",private_key_file,master_address);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
        system(cmdline);
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#else
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#endif
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: oss:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\db_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
    sprintf(filename_temp,"%s/db_passwords.txt",vaultdir);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\root_passwords.txt",vaultdir);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
    system(cmdline);
#else
    sprintf(filename_temp,"%s/root_passwords.txt",vaultdir);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
    system(cmdline);
    sprintf(cmdline,"%s encrypt %s/_CLUSTER_SUMMARY.txt.tmp %s/_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt.tmp >> /dev/null 2>&1",vaultdir);
    system(cmdline);
#endif
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s\\cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
        sprintf(cmdline,"attrib +h +s +r %s\\cloud_flag.flg",confdir);
        system(cmdline);
    }
#else
    sprintf(filename_temp,"%s/.cloud_flag.flg",confdir);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s/.cloud_flag.flg",cloud_flag,confdir);
        system(cmdline);
    }
#endif
    printf("[ -INFO- ] After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
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
    print_cluster_init_done();
    return 0;
}