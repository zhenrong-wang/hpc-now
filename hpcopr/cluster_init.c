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
extern char url_initutils_root_var[LOCATION_LENGTH];
extern int code_loc_flag_var;

/*
 * 
 *
 */
int cluster_init_conf(char* cluster_name, int argc, char* argv[]){
//    char* region_id, char* zone_id, char* node_num, char* hpc_user_num, char* master_inst, char* compute_inst, char* os_image, char* ht_flag
    char cloud_flag[16]="";
    char workdir[DIR_LENGTH]="";
    get_workdir(workdir,cluster_name);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -5;
    }
    char tf_prep_conf[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(cmdline,"%s %s%sconf %s",MKDIR_CMD,workdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(tf_prep_conf,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    if(file_exist_or_not(tf_prep_conf)==0){
        return -3; // If the conf file already exists, exit.
    }
    FILE* file_p=fopen(tf_prep_conf,"w+");
    if(file_p==NULL){
        return -1;
    }
    int i;
    char header[16]="";
    char tail[64]="";
    char default_region[32]="";
    char real_region[32]="";
    char default_zone[36]="";
    char real_zone[36]="";
    int default_node_num=1;
    char real_node_num_string[16]="";
    int default_user_num=3;
    char real_user_num_string[16]="";
    char* default_master_inst="a8c16g";
    char real_master_inst[16]="";
    char* default_compute_inst="a4c8g";
    char real_compute_inst[16]="";
    char* default_os_image="centoss9";
    char real_os_image[16]="";
    char* default_ht_flag="ON";
    char real_ht_flag[16]="";

    for(i=2;i<argc;i++){
        get_seq_string(argv[i],'=',1,header);
        get_seq_string(argv[i],'=',2,tail);
        if(strcmp(header,"--r")==0){
            strcpy(real_region,tail);
        }
        else if(strcmp(header,"--az")==0){
            strcpy(real_zone,tail);
        }
        else if(strcmp(header,"--nn")==0){
            strcpy(real_node_num_string,tail);
        }
        else if(strcmp(header,"--un")==0){
            strcpy(real_user_num_string,tail);
        }
        else if(strcmp(header,"--mi")==0){
            strcpy(real_master_inst,tail);
        }
        else if(strcmp(header,"--ci")==0){
            strcpy(real_compute_inst,tail);
        }
        else if(strcmp(header,"--os")==0){
            strcpy(real_os_image,tail);
        }
        else if(strcmp(header,"--ht")==0){
            strcpy(real_ht_flag,tail);
        }
        else{
            continue;
        }
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        strcpy(default_region,"cn-hangzhou");
        strcpy(default_zone,"cn-hangzhou-j");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        strcpy(default_region,"ap-guangzhou");
        strcpy(default_zone,"ap-guangzhou-6");
    }
    else{
        strcpy(default_region,"cn-northwest-1");
        strcpy(default_zone,"cn-northwest-1a");
    }
    if(strlen(real_region)==0){
        strcpy(real_region,default_region);
    }
    if(strlen(real_zone)==0){
        strcpy(real_zone,default_zone);
    }
    if(strlen(real_node_num_string)==0){
        sprintf(real_node_num_string,"%d",default_node_num);
    }
    if(strlen(real_user_num_string)==0){
        sprintf(real_user_num_string,"%d",default_user_num);
    }
    if(strlen(real_master_inst)==0){
        strcpy(real_master_inst,default_master_inst);
    }
    if(strlen(real_compute_inst)==0){
        strcpy(real_compute_inst,default_compute_inst);
    }
    if(strlen(real_os_image)==0){
        strcpy(real_os_image,default_os_image);
    }
    if(strlen(real_ht_flag)==0){
        strcpy(real_ht_flag,default_ht_flag);
    }
    fprintf(file_p,"#IMPORTANT: THERE *MUST* BE 22 CHARACTERs (including spaces) BEFORE THE VALUE OF PARAMETERS.\n");
    fprintf(file_p,"#FOR EXAMPLE        : PARAMETER_VALUE (*WITHOUT* ANY SPACE OR OTHER CHARACTORS AFTER THE PARAMETER_VALUE!)\n");
    fprintf(file_p,"#                   : |<---THIS IS THE STANDARD START POINT! YOU NEED TO *ABSOLUTELY* ALIGN WITH THIS LINE.\n");
    fprintf(file_p,"CLUSTER_ID          : %s\n",cluster_name);
    fprintf(file_p,"REGION_ID           : %s\n",real_region);
    fprintf(file_p,"ZONE_ID             : %s\n",real_zone);
    fprintf(file_p,"NODE_NUM            : %s\n",real_node_num_string);
    fprintf(file_p,"HPC_USER_NUM        : %s\n",real_user_num_string);
    fprintf(file_p,"master_init_param   : db skip\n");
    fprintf(file_p,"master_passwd       : *AUTOGEN*\n");
    fprintf(file_p,"compute_passwd      : *AUTOGEN*\n");
    fprintf(file_p,"master_inst         : %s\n",real_master_inst);
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        fprintf(file_p,"master_bandwidth    : 50\n");
    }
    fprintf(file_p,"compute_inst        : %s\n",real_compute_inst);
    fprintf(file_p,"os_image            : %s\n",real_os_image);
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        fprintf(file_p,"hyperthreading      : %s\n",real_ht_flag);
    }
    fclose(file_p);
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
    char* error_log=OPERATION_ERROR_LOG;
    char secret_file[FILENAME_LENGTH]="";
    char region_valid[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
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
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
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
    sprintf(logdir,"%s%slog%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(confdir,"%s%sconf%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(compute_template,"%s%scompute_template",stackdir,PATH_SLASH);
    printf("[ START: ] Start initializing the cluster ...\n");
    if(folder_exist_or_not(stackdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,logdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,confdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(code_loc_flag_var==1){
        sprintf(url_aws_root,"%s%saws%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
    }
    else{
        sprintf(url_aws_root,"%saws/",url_code_root_var);
    }

    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sregion_valid.tf %s%sregion_valid.tf %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sregion_valid.tf -o %s%sregion_valid.tf -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    sprintf(cmdline,"%s %s%shpc_stack* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(secret_file,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
    sprintf(region_valid,"%s%sregion_valid.tf",stackdir,PATH_SLASH);
    global_replace(region_valid,"BLANK_ACCESS_KEY_ID",access_key);
    global_replace(region_valid,"BLANK_SECRET_KEY",secret_key);
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log,0)!=0){
        sprintf(cmdline,"%s %s%sregion_valid.tf %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        return 1;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,0)!=0){
        global_replace(region_valid,"cn-northwest-1","us-east-1");
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,0)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keypair may be invalid. Please use 'hpcopr new-keypair' to update with a\n");
            printf("|          valid keypair. Exit now.\n" RESET_DISPLAY);
            sprintf(cmdline,"%s %s%sregion_valid.tf %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 1;
        }
        region_valid_flag=1;
    }
    sprintf(cmdline,"%s %s%sregion_valid.tf %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(conf_file,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(conf_file)==1){
        printf(GENERAL_BOLD "[ -INFO- ] IMPORTANT: No configure file found. Use the default one.\n" RESET_DISPLAY);
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s %s", COPY_FILE_CMD,url_aws_root,PATH_SLASH,conf_file,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_aws_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 2;
        }
        if(region_valid_flag==1){
            global_replace(conf_file,"cn-northwest-1","us-east-1");
        }
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_aws.base %s%shpc_stack.base %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.base -o %s%shpc_stack.base -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_aws.master %s%shpc_stack.master %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.master -o %s%shpc_stack.master -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_aws.compute %s%shpc_stack.compute %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.compute -o %s%shpc_stack.compute -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_aws.database %s%shpc_stack.database %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.database -o %s%shpc_stack.database -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_aws.natgw %s%shpc_stack.natgw %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_aws.natgw -o %s%shpc_stack.natgw -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sreconf.list %s%sreconf.list %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s%sreconf.list -s",url_aws_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
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
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
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
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently the NOW Cluster Service only support AWS Regions below:\n");
        printf("|          cn-northwest-1 | cn-north-1 | us-east-1 | us-east-2\n");
        printf("|          If you'd like to use NOW Cluster in other AWS regions,\n");
        printf("|          Please contact info@hpc-now.com\n\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        return 3;
    }
    if(contain_or_not(zone_id,region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        return 3;
    }
    if(strcmp(region_id,"cn-northwest-1")==0){
        if(region_valid_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keypair is not valid to operate clusters in AWS China regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            return 4;
        }
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.0",os_image_raw);
        strcpy(db_os_image,"centos7cn.0");
        strcpy(nat_os_image,"centos7cn.0");
    }
    else if(strcmp(region_id,"cn-north-1")==0){
        if(region_valid_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keypair is not valid to operate clusters in AWS China regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            return 4;
        }
        strcpy(region_flag,"cn_regions");
        sprintf(os_image,"%scn.1",os_image_raw);
        strcpy(db_os_image,"centos7cn.1");
        strcpy(nat_os_image,"centos7cn.1");
    }
    else if(strcmp(region_id,"us-east-1")==0){
        if(region_valid_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keypair is not valid to operate clusters in AWS global regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            return 4;
        }
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.0",os_image_raw);
        strcpy(db_os_image,"centos7global.0");
        strcpy(nat_os_image,"centos7global.0");
    }
    else if(strcmp(region_id,"us-east-2")==0){
        if(region_valid_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keypair is not valid to operate clusters in AWS global regions.\n");
            printf("|          Please run 'hpcopr new-keypair' command to update with a valid keypair.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            return 4;
        }
        strcpy(region_flag,"global_regions");
        sprintf(os_image,"%sglobal.1",os_image_raw);
        strcpy(db_os_image,"centos7global.1");
        strcpy(nat_os_image,"centos7global.1");
    }
    sprintf(filename_temp,"%s%sdb_passwords.txt",vaultdir,PATH_SLASH);
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
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
    }
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
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
    sprintf(filename_temp,"%s%sroot_passwords.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf(HIGH_GREEN_BOLD "[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n" RESET_DISPLAY,os_image_raw);
    generate_sshkey(sshkey_folder,pubkey);
    sprintf(filename_temp,"%s%shpc_stack.base",stackdir,PATH_SLASH);
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

    file_p=fopen(filename_temp,"a");
    sprintf(user_passwords,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<hpc_user_num;i++){
        reset_string(user_passwd_temp);
        generate_random_passwd(user_passwd_temp);
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    sprintf(filename_temp,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);
    for(i=0;i<hpc_user_num;i++){
        sprintf(line_temp,"echo -e \"username: user%d ${var.user%d_passwd}\" >> /root/user_secrets.txt",i+1,i+1);
        insert_lines(filename_temp,"master_private_ip",line_temp);
    }
    sprintf(line_temp,"echo -e \"export HPCMGR_SCRIPT_URL=%shpcmgr.sh\\nexport APPS_INSTALL_SCRIPTS_URL=%sapps-install/\\nexport INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_shell_scripts_var,url_shell_scripts_var,url_initutils_root_var);
    insert_lines(filename_temp,"master_private_ip",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    sprintf(string_temp,"%d",cpu_core_num);
    global_replace(filename_temp,"CPU_CORE_NUM",string_temp);
    sprintf(string_temp,"%d",threads);
    global_replace(filename_temp,"THREADS_PER_CORE",string_temp);
    sprintf(line_temp,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_lines(filename_temp,"mount",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"DB_OS_IMAGE",db_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);

    sprintf(filename_temp,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"NAT_OS_IMAGE",nat_os_image);
    global_replace(filename_temp,"RG_NAME",unique_cluster_id);
    for(i=0;i<node_num;i++){
        sprintf(cmdline,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        sprintf(string_temp,"comp%d",i+1);
        global_replace(filename_temp,"NUMBER",string_temp);
    }
    sprintf(cmdline,"%s %s%shpc_stack.base %s%shpc_stack_base.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.database %s%shpc_stack_database.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.master %s%shpc_stack_master.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.natgw %s%shpc_stack_natgw.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.compute %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log,0)!=0){
        return 5;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%s*.tmp %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%sUCID_LATEST.txt %s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,confdir,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("|          Please run " HIGH_GREEN_BOLD "hpcopr viewlog err archive" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    sprintf(cmdline,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
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
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    }
    else{
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_CN);
        for(i=0;i<AWS_SLEEP_TIME_CN;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_CN-i);
            fflush(stdout);
            sleep(1);
        }
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    }
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    sprintf(private_key_file,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(strcmp(region_flag,"cn_regions")==0){
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%ss3cfg.txt %s%sbucket.conf %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s%sbucket.conf",url_aws_root,vaultdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n" RESET_DISPLAY);
        }
        else{
            sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
            sprintf(string_temp,"s3.%s.amazonaws.com.cn",region_id);
            global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
            global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
            global_replace(filename_temp,"DEFAULT_REGION",region_id);
            global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg %s",private_key_file,filename_temp,master_address,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo -e \"export BUCKET=s3://%s\" >> /etc/profile\" %s",private_key_file,master_address,bucket_id,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
    }
    else{
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%ss3cfg.txt %s%sbucket.conf %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %ss3cfg.txt -s -o %s%sbucket.conf",url_aws_root,vaultdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n" RESET_DISPLAY);
        }
        else{
            sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
            strcpy(string_temp,"s3.amazonaws.com");
            global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
            global_replace(filename_temp,"BLANK_SECRET_KEY_ID",bucket_sk);
            global_replace(filename_temp,"DEFAULT_REGION",region_id);
            global_replace(filename_temp,"DEFAULT_ENDPOINT",string_temp);
            sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.s3cfg %s",private_key_file,filename_temp,master_address,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo -e \"export BUCKET=s3://%s\" >> /etc/profile\" %s",private_key_file,master_address,bucket_id,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
    }
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: s3:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(cmdline,"%s %s%sdb_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(cmdline,"%s %s%sroot_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);  
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    sprintf(filename_temp,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s%scloud_flag.flg",cloud_flag,vaultdir,PATH_SLASH);
        system(cmdline);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
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
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/hostfile","root","put");
    print_cluster_init_done();
    delete_decrypted_files(workdir,crypto_keyfile);
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
    char user_passwords[FILENAME_LENGTH]="";
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
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
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
    sprintf(logdir,"%s%slog%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(confdir,"%s%sconf%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(compute_template,"%s%scompute_template",stackdir,PATH_SLASH);
    printf("[ START: ] Start initializing the cluster ...\n");
    if(folder_exist_or_not(stackdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,logdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,confdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(code_loc_flag_var==1){
        sprintf(url_qcloud_root,"%s%sqcloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
    }
    else{
        sprintf(url_qcloud_root,"%sqcloud/",url_code_root_var);
    }
    sprintf(conf_file,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(conf_file)==1){
        printf(GENERAL_BOLD "[ -INFO- ] IMPORTANT: No configure file found. Use the default one. \n" RESET_DISPLAY);
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,conf_file,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s", url_qcloud_root,conf_file);
        }
            if(system(cmdline)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 2;
        }
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    sprintf(cmdline,"%s %s%shpc_stack* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_qcloud.base %s%shpc_stack.base %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.base -o %s%shpc_stack.base -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_qcloud.master %s%shpc_stack.master %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.master -o %s%shpc_stack.master -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_qcloud.compute %s%shpc_stack.compute %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.compute -o %s%shpc_stack.compute -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_qcloud.database %s%shpc_stack.database %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.database -o %s%shpc_stack.database -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stack_qcloud.natgw %s%shpc_stack.natgw %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stack_qcloud.natgw -o %s%shpc_stack.natgw -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sNAS_Zones_QCloud.txt %s%sNAS_Zones_QCloud.txt %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_QCloud.txt -o %s%sNAS_Zones_QCloud.txt -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sreconf.list %s%sreconf.list %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s%sreconf.list -s",url_qcloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }

    sprintf(secret_file,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
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
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
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
        printf(WARN_YELLO_BOLD "[ -WARN- ] The master node bandwidth %d exceeds the maximum value 50, reset to 50.\n" RESET_DISPLAY,master_bandwidth);
        master_bandwidth=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
    sprintf(filename_temp,"%s%sNAS_Zones_QCloud.txt",stackdir,PATH_SLASH);
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        return 3;
    }
    sprintf(filename_temp,"%s%sdb_passwords.txt",vaultdir,PATH_SLASH);
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
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
    }
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
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
    sprintf(filename_temp,"%s%sroot_passwords.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf(HIGH_GREEN_BOLD "[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n" RESET_DISPLAY,os_image);
    generate_sshkey(sshkey_folder,pubkey);
    sprintf(filename_temp,"%s%shpc_stack.base",stackdir,PATH_SLASH);
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
    sprintf(string_temp,"%snatgw",randstr);
    global_replace(filename_temp,"SECURITY_GROUP_NATGW",string_temp);
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

    file_p=fopen(filename_temp,"a");
    sprintf(user_passwords,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<hpc_user_num;i++){
        reset_string(user_passwd_temp);
        generate_random_passwd(user_passwd_temp);
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    sprintf(filename_temp,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);
    for(i=0;i<hpc_user_num;i++){
        sprintf(line_temp,"echo -e \"username: user%d ${var.user%d_passwd}\" >> /root/user_secrets.txt",i+1,i+1);
        insert_lines(filename_temp,"master_private_ip",line_temp);
    }
    sprintf(line_temp,"echo -e \"export HPCMGR_SCRIPT_URL=%shpcmgr.sh\\nexport APPS_INSTALL_SCRIPTS_URL=%sapps-install/\\nexport INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_shell_scripts_var,url_shell_scripts_var,url_initutils_root_var);
    insert_lines(filename_temp,"master_private_ip",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    sprintf(line_temp,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_lines(filename_temp,"mount",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);

    sprintf(filename_temp,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RESOURCETAG",unique_cluster_id);

    for(i=0;i<node_num;i++){
        sprintf(cmdline,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
        global_replace(filename_temp,"RUNNING_FLAG","true");
    }
    sprintf(cmdline,"%s %s%shpc_stack.base %s%shpc_stack_base.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.database %s%shpc_stack_database.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.master %s%shpc_stack_master.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.natgw %s%shpc_stack_natgw.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.compute %s && %s %s%sNAS_Zones_QCloud.txt %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT,DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log,0)!=0){
        return 5;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%sUCID_LATEST.txt %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,confdir,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("|          Please run " HIGH_GREEN_BOLD "hpcopr viewlog err archive" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    sprintf(cmdline,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    find_and_get(filename_temp,"secret_id","","",1,"secret_id","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"secret_key","","",1,"secret_key","","",'\"',4,bucket_sk);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",QCLOUD_SLEEP_TIME);
    for(i=0;i<QCLOUD_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",QCLOUD_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    sprintf(private_key_file,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%scos.conf %s%sbucket.conf %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %scos.conf -s -o %s%sbucket.conf",url_qcloud_root,vaultdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n" RESET_DISPLAY);
    }
    else{
        sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
        global_replace(filename_temp,"BLANK_BUCKET_NAME",bucket_id);
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.cos.conf %s",private_key_file,filename_temp,master_address,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.cos.conf\" %s",private_key_file,master_address,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo -e \"export BUCKET=cos://%s\" >> /etc/profile\" %s",private_key_file,master_address,bucket_id,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: cos: %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(cmdline,"%s %s%sdb_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(cmdline,"%s %s%sroot_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    sprintf(filename_temp,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s%scloud_flag.flg",cloud_flag,vaultdir,PATH_SLASH);
        system(cmdline);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
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
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/hostfile","root","put");
    print_cluster_init_done();
    delete_decrypted_files(workdir,crypto_keyfile);
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
    char user_passwords[FILENAME_LENGTH]="";
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
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
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
    sprintf(logdir,"%s%slog%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(confdir,"%s%sconf%s",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(compute_template,"%s%scompute_template",stackdir,PATH_SLASH);
    printf("[ START: ] Start initializing the cluster ...\n");
    if(folder_exist_or_not(stackdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(vaultdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(logdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,logdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,confdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(code_loc_flag_var==1){
        sprintf(url_alicloud_root,"%s%salicloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
    }
    else{
        sprintf(url_alicloud_root,"%salicloud/",url_code_root_var);
    }
    sprintf(conf_file,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(conf_file)==1){
        printf(GENERAL_BOLD "[ -INFO- ] IMPORTANT: No configure file found. Use the default one. \n" RESET_DISPLAY);
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,conf_file,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s",url_alicloud_root,conf_file);
        }
        if(system(cmdline)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 2;
        }
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    sprintf(cmdline,"%s %s%shpc_stack* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stackv2.base %s%shpc_stack.base %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.base -o %s%shpc_stack.base -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stackv2.master %s%shpc_stack.master %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.master -o %s%shpc_stack.master -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stackv2.compute %s%shpc_stack.compute %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.compute -o %s%shpc_stack.compute -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stackv2.database %s%shpc_stack.database %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.database -o %s%shpc_stack.database -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%shpc_stackv2.natgw %s%shpc_stack.natgw %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %shpc_stackv2.natgw -o %s%shpc_stack.natgw -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sNAS_Zones_ALI.txt %s%sNAS_Zones_ALI.txt %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sNAS_Zones_ALI.txt -o %s%sNAS_Zones_ALI.txt -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%sreconf.list %s%sreconf.list %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %sreconf.list -o %s%sreconf.list -s",url_alicloud_root,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s). Exit now.\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 2;
    }
    sprintf(secret_file,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    get_ak_sk(secret_file,crypto_keyfile,access_key,secret_key,cloud_flag);
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
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    if(node_num<MINUMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        node_num=MINUMUM_ADD_NODE_NUMBER;
    }
    fgetline(file_p,conf_line_buffer);
    i=strlen(conf_line_buffer)-22;
    for(j=i;j>0;j--){
        hpc_user_num+=(conf_line_buffer[22+i-j]-'0')*pow(10,j-1);
    }
    reset_string(conf_line_buffer);
    if(hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n" RESET_DISPLAY,hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
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
        printf(WARN_YELLO_BOLD "[ -WARN- ] The master node bandwidth %d exceeds the maximum value 50, reset to 50.\n" RESET_DISPLAY,master_bandwidth);
        master_bandwidth=50;
    }
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,compute_inst);
    fscanf(file_p,"%s%s%s\n",conf_param_buffer1,conf_param_buffer2,os_image);
    fclose(file_p);
    sprintf(filename_temp,"%s%sNAS_Zones_ALI.txt",stackdir,PATH_SLASH);
    if(find_multi_keys(filename_temp,zone_id,"","","","")>0){
        strcpy(NAS_Zone,zone_id);
    }
    else{
        find_and_get(filename_temp,region_id,"","",1,region_id,"","",' ',1,NAS_Zone);
    }
 
    if(contain_or_not(zone_id,region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Avalability Zone ID doesn't match with Region ID, please double check.\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        return 3;
    }
    sprintf(filename_temp,"%s%sdb_passwords.txt",vaultdir,PATH_SLASH);
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
    }
    else if(strlen(cluster_id_input)>CLUSTER_ID_LENGTH_MIN||strlen(cluster_id_input)==CLUSTER_ID_LENGTH_MIN){
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
    }
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
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
    sprintf(filename_temp,"%s%sroot_passwords.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
    fclose(file_p);
    printf(HIGH_GREEN_BOLD "[ STEP 2 ] Cluster Configuration:\n");
    printf("|          Cluster ID:            %s\n",cluster_id);
    printf("|          Region:                %s\n",region_id);
    printf("|          Avalability Zone:      %s\n",zone_id);
    printf("|          Number of Nodes:       %d\n",node_num);
    printf("|          Number of Users:       %d\n",hpc_user_num);
    printf("|          Master Node Instance:  %s\n",master_inst);
    printf("|          Compute Node Instance: %s\n",compute_inst);
    printf("|          OS Image:              %s\n" RESET_DISPLAY,os_image);
    generate_sshkey(sshkey_folder,pubkey);
    sprintf(filename_temp,"%s%shpc_stack.base",stackdir,PATH_SLASH);
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
    file_p=fopen(filename_temp,"a");
    sprintf(user_passwords,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<hpc_user_num;i++){
        reset_string(user_passwd_temp);
        generate_random_passwd(user_passwd_temp);
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    sprintf(filename_temp,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"MASTER_INST",master_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    sprintf(string_temp,"%d",master_bandwidth);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    global_replace(filename_temp,"PUBLIC_KEY",pubkey);
    for(i=0;i<hpc_user_num;i++){
        sprintf(line_temp,"echo -e \"username: user%d ${var.user%d_passwd}\" >> /root/user_secrets.txt",i+1,i+1);
        insert_lines(filename_temp,"master_private_ip",line_temp);
    }
    sprintf(line_temp,"echo -e \"export HPCMGR_SCRIPT_URL=%shpcmgr.sh\\nexport APPS_INSTALL_SCRIPTS_URL=%sapps-install/\\nexport INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_shell_scripts_var,url_shell_scripts_var,url_initutils_root_var);
    insert_lines(filename_temp,"master_private_ip",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    global_replace(filename_temp,"COMPUTE_INST",compute_inst);
    global_replace(filename_temp,"CLOUD_FLAG",cloud_flag);
    global_replace(filename_temp,"OS_IMAGE",os_image);
    sprintf(line_temp,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_lines(filename_temp,"mount",line_temp);

    sprintf(filename_temp,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);

    sprintf(filename_temp,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_replace(filename_temp,"DEFAULT_ZONE_ID",zone_id);
    global_replace(filename_temp,"MASTER_BANDWIDTH",string_temp);
    global_replace(filename_temp,"RG_DISPLAY_NAME",unique_cluster_id);
    reset_string(filename_temp);
    reset_string(string_temp);

    for(i=0;i<node_num;i++){
        sprintf(cmdline,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        sprintf(string_temp,"compute%d",i+1);
        global_replace(filename_temp,"COMPUTE_NODE_N",string_temp);
    }
    sprintf(cmdline,"%s %s%shpc_stack.base %s%shpc_stack_base.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.database %s%shpc_stack_database.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.master %s%shpc_stack_master.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.natgw %s%shpc_stack_natgw.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack.compute %s && %s %s%sNAS_Zones_ALI.txt %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT,DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(terraform_execution(tf_exec,"init",workdir,crypto_keyfile,error_log,0)!=0){
        return 5;
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%sUCID_LATEST.txt %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,confdir,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("|          Please run " HIGH_GREEN_BOLD "hpcopr viewlog err archive" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    sprintf(cmdline,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",ALI_SLEEP_TIME);
    for(i=0;i<ALI_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",ALI_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    sprintf(filename_temp,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id);
    sprintf(filename_temp,"%s%sbucket_secrets.txt",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak);
    find_and_get(filename_temp,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    sprintf(private_key_file,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(code_loc_flag_var==1){
        sprintf(cmdline,"%s %s%s.ossutilconfig %s%sbucket.conf %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"curl %s.ossutilconfig -s -o %s%sbucket.conf",url_alicloud_root,vaultdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the bucket configuration file. The bucket may not work.\n" RESET_DISPLAY);
    }
    else{
        sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
        global_replace(filename_temp,"BLANK_ACCESS_KEY",bucket_ak);
        global_replace(filename_temp,"BLANK_SECRET_KEY",bucket_sk);
        global_replace(filename_temp,"DEFAULT_REGION",region_id);
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/.ossutilconfig %s",private_key_file,filename_temp,master_address,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"chmod 644 /root/.ossutilconfig\" %s",private_key_file,master_address,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo -e \"export BUCKET=oss://%s\" >> /etc/profile\" %s",private_key_file,master_address,bucket_id,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    fprintf(file_p,"HPC-NOW CLUSTER SUMMARY\nMaster Node IP: %s\nMaster Node Root Password: %s\n\nNetDisk Address: oss:// %s\nNetDisk Region: %s\nNetDisk AccessKey ID: %s\nNetDisk Secret Key: %s\n",master_address,master_passwd,bucket_id,region_id,bucket_ak,bucket_sk);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",database_root_passwd,database_acct_passwd);
    sprintf(cmdline,"%s %s%sdb_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    fprintf(file_p,"+----------------------------------------------------------------+\n");
    fprintf(file_p,"%s\n%s\n",master_passwd,compute_passwd);
	fclose(file_p);
    sprintf(cmdline,"%s %s%sroot_passwords.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    sprintf(filename_temp,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"echo %s > %s%scloud_flag.flg",cloud_flag,vaultdir,PATH_SLASH);
        system(cmdline);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(master_inst);
    compute_vcpu=get_cpu_num(compute_inst);
    sprintf(filename_temp,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    reset_string(string_temp);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,string_temp);
    database_vcpu=get_cpu_num(string_temp);
    reset_string(string_temp);
    sprintf(filename_temp,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
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
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/hostfile","root","put");
    print_cluster_init_done();
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}