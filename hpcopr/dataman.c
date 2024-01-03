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
#include "dataman.h"

void unset_bucket_envs(char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_C")==0){
#ifdef _WIN32
    system("set AWS_ACCESS_KEY_ID=");
    system("set AWS_SECRET_ACCESS_KEY=");
    system("set AWS_DEFAULT_REGION=");
#else 
    system("unset AWS_ACCESS_KEY_ID");
    system("unset AWS_SECRET_ACCESS_KEY");
    system("unset AWS_DEFAULT_REGION");
#endif
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
#ifdef _WIN32
    system("set AZCOPY_AUTO_LOGIN_TYPE=");
    system("set AZCOPY_SPA_APPLICATION_ID=");
    system("set AZCOPY_SPA_CLIENT_SECRET=");
    system("set AZCOPY_TENANT_ID=");
#else 
    system("unset AZCOPY_AUTO_LOGIN_TYPE");
    system("unset AZCOPY_SPA_APPLICATION_ID");
    system("unset AZCOPY_SPA_CLIENT_SECRET");
    system("unset AZCOPY_TENANT_ID");
#endif
    }

}

void bucket_path_check(char* path_string, char* hpc_user, char* real_path){
    if(*(path_string+0)!='/'){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/%s",path_string);
        }
        else{
            sprintf(real_path,"/%s/%s",hpc_user,path_string);
        }
    }
    else{
        if(strcmp(hpc_user,"root")==0){
            strcpy(real_path,path_string);
        }
        else{
            sprintf(real_path,"/%s%s",hpc_user,path_string);
        }
    }
}

void rf_flag_parser(const char* rflag, const char* fflag, char* real_rflag, char* real_fflag){
    if(strcmp(rflag,"recursive")!=0){
        strcpy(real_rflag,"");
    }
    else{
        strcpy(real_rflag,"--recursive");
    }
    if(strcmp(fflag,"force")!=0){
        strcpy(real_fflag,"");
    }
    else{
        strcpy(real_fflag,"--force");
    }
}

int bucket_cp(char* workdir, char* hpc_user, char* source_path, char* target_path, char* rflag, char* fflag, char* crypto_keyfile, char* cloud_flag, char* cmd_type){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"put")!=0&&strcmp(cmd_type,"get")!=0&&strcmp(cmd_type,"copy")!=0){
        return -5;
    }
    /*char bucket_address[128]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";*/
    bucket_info bucketinfo;
    char az_subscription_id[128]="";
    char az_tenant_id[128]="";
    char real_rflag[32]="";
    char real_fflag[32]="";
    char real_source_path[DIR_LENGTH]="";
    char real_target_path[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_ninfo(workdir,crypto_keyfile,LINE_LENGTH_SHORT,&bucketinfo)!=0){
        return -1;
    }
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strlen(real_rflag)!=0){
            strcpy(real_rflag,"-r");
        }
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"-f");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--yes");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--force-if-read-only");
        }
    }
    if(strcmp(cmd_type,"put")==0){
        local_path_parser(source_path,real_source_path);
        bucket_path_check(target_path,hpc_user,real_target_path);
    }
    else if(strcmp(cmd_type,"get")==0){
        bucket_path_check(source_path,hpc_user,real_source_path);
        local_path_parser(target_path,real_target_path);
    }
    else{
        bucket_path_check(source_path,hpc_user,real_source_path);
        bucket_path_check(target_path,hpc_user,real_target_path);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s%s %s %s",OSSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s %s%s %s %s",OSSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s %s %s",OSSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s%s %s %s",COSCLI_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s %s%s %s %s",COSCLI_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s %s %s",COSCLI_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s%s %s%s %s %s",SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,bucketinfo.region_id,S3CLI_EXEC,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s %s%s %s %s",SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,bucketinfo.region_id,S3CLI_EXEC,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s%s %s %s %s",SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,bucketinfo.region_id,S3CLI_EXEC,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s cp -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s%s %s%s %s %s",OBSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s cp -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s %s%s %s %s",OBSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s cp -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s%s %s %s %s",OBSUTIL_EXEC,bucketinfo.region_id,bucketinfo.bucket_ak,bucketinfo.bucket_sk,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(decrypt_bcecredentials(workdir)!=0){
            return -1;
        }
        create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s bos cp %s%s %s%s %s %s --conf-path %s",BCECMD_EXEC,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag,vaultdir);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s bos cp %s %s%s %s %s --conf-path %s",BCECMD_EXEC,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag,vaultdir);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s bos cp %s%s %s %s %s --conf-path %s",BCECMD_EXEC,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag,vaultdir);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        get_azure_ninfo(workdir,LINE_LENGTH_SHORT,az_subscription_id,az_tenant_id,128);
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AZCOPY_AUTO_LOGIN_TYPE=SPN&&%s AZCOPY_SPA_APPLICATION_ID=%s&&%s AZCOPY_SPA_CLIENT_SECRET=%s&&%s AZCOPY_TENANT_ID=%s&&%s cp %s%s %s%s %s %s --log-level=ERROR",SET_ENV_CMD,SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,az_tenant_id,AZCOPY_EXEC,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AZCOPY_AUTO_LOGIN_TYPE=SPN&&%s AZCOPY_SPA_APPLICATION_ID=%s&&%s AZCOPY_SPA_CLIENT_SECRET=%s&&%s AZCOPY_TENANT_ID=%s&&%s cp %s %s%s %s %s --log-level=ERROR",SET_ENV_CMD,SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,az_tenant_id,AZCOPY_EXEC,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AZCOPY_AUTO_LOGIN_TYPE=SPN&&%s AZCOPY_SPA_APPLICATION_ID=%s&&%s AZCOPY_SPA_CLIENT_SECRET=%s&&%s AZCOPY_TENANT_ID=%s&&%s cp %s%s %s %s %s --log-level=ERROR",SET_ENV_CMD,SET_ENV_CMD,bucketinfo.bucket_ak,SET_ENV_CMD,bucketinfo.bucket_sk,SET_ENV_CMD,az_tenant_id,AZCOPY_EXEC,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else{
        create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
        gcp_credential_convert(workdir,"decrypt",1);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s auth activate-service-account --key-file=%s%s.bucket_key.json %s",GCLOUD_CLI,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        if(system(cmdline)!=0){
            gcp_credential_convert(workdir,"delete",1);
            return 1;
        }
        if(strcmp(cmd_type,"copy")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s storage cp %s%s %s%s %s",GCLOUD_CLI,bucketinfo.bucket_address,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s storage cp %s %s%s %s",GCLOUD_CLI,real_source_path,bucketinfo.bucket_address,real_target_path,real_rflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s storage cp %s%s %s %s",GCLOUD_CLI,bucketinfo.bucket_address,real_source_path,real_target_path,real_rflag);
        }
    }
    if(system(cmdline)!=0){
        unset_bucket_envs(cloud_flag);
        if(strcmp(cloud_flag,"CLOUD_E")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scredentials %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            gcp_credential_convert(workdir,"delete",1);
        }
        return 1;
    }
    else{
        unset_bucket_envs(cloud_flag);
        if(strcmp(cloud_flag,"CLOUD_E")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scredentials %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            gcp_credential_convert(workdir,"delete",1);
        }
        return 0;
    }
}

int bucket_rm_ls(char* workdir, char* hpc_user, char* remote_path, char* rflag, char* fflag, char* crypto_keyfile, char* cloud_flag, char* cmd_type){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"delete")!=0&&strcmp(cmd_type,"list")!=0){
        return -5;
    }
    /*char bucket_address[128]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";*/
    bucket_info binfo;
    char az_subscription_id[128]="";
    char az_tenant_id[128]="";
    char real_rflag[32]="";
    char real_fflag[32]="";
    char real_remote_path[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_ninfo(workdir,crypto_keyfile,LINE_LENGTH_SHORT,&binfo)!=0){
        return -1;
    }
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strlen(real_rflag)!=0){
            strcpy(real_rflag,"-r");
        }
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"-f");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--yes");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--force-if-read-only");
        }
    }
    bucket_path_check(remote_path,hpc_user,real_remote_path);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e oss-%s.aliyuncs.com -i %s -k %s rm %s%s %s %s",OSSUTIL_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e oss-%s.aliyuncs.com -i %s -k %s ls %s%s",OSSUTIL_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e cos.%s.myqcloud.com -i %s -k %s rm %s%s %s %s",COSCLI_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s -e cos.%s.myqcloud.com -i %s -k %s ls %s%s %s",COSCLI_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path,real_rflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 rm %s%s %s %s",SET_ENV_CMD,binfo.bucket_ak,SET_ENV_CMD,binfo.bucket_sk,SET_ENV_CMD,binfo.region_id,S3CLI_EXEC,binfo.bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 ls %s%s %s",SET_ENV_CMD,binfo.bucket_ak,SET_ENV_CMD,binfo.bucket_sk,SET_ENV_CMD,binfo.region_id,S3CLI_EXEC,binfo.bucket_address,real_remote_path,real_rflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s rm -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s%s %s %s",OBSUTIL_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s ls -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s%s",OBSUTIL_EXEC,binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_remote_path);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(decrypt_bcecredentials(workdir)!=0){
            return -1;
        }
        create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s bos rm %s%s %s %s --conf-path %s",BCECMD_EXEC,binfo.bucket_address,real_remote_path,real_rflag,real_fflag,vaultdir);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s bos ls %s%s %s --summerize --conf-path %s",BCECMD_EXEC,binfo.bucket_address,real_remote_path,real_rflag,vaultdir);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(get_azure_ninfo(workdir,LINE_LENGTH_SHORT,az_subscription_id,az_tenant_id,128)!=0){
            return -1;
        }
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AZCOPY_AUTO_LOGIN_TYPE=SPN&&%s AZCOPY_SPA_APPLICATION_ID=%s&&%s AZCOPY_SPA_CLIENT_SECRET=%s&&%s AZCOPY_TENANT_ID=%s&&%s remove %s%s %s %s --log-level=ERROR",SET_ENV_CMD,SET_ENV_CMD,binfo.bucket_ak,SET_ENV_CMD,binfo.bucket_sk,SET_ENV_CMD,az_tenant_id,AZCOPY_EXEC,binfo.bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s AZCOPY_AUTO_LOGIN_TYPE=SPN&&%s AZCOPY_SPA_APPLICATION_ID=%s&&%s AZCOPY_SPA_CLIENT_SECRET=%s&&%s AZCOPY_TENANT_ID=%s&&%s list %s%s --log-level=ERROR",SET_ENV_CMD,SET_ENV_CMD,binfo.bucket_ak,SET_ENV_CMD,binfo.bucket_sk,SET_ENV_CMD,az_tenant_id,AZCOPY_EXEC,binfo.bucket_address,real_remote_path);
        }
    }
    else{
        create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
        gcp_credential_convert(workdir,"decrypt",1);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s auth activate-service-account --key-file=%s%s.bucket_key.json %s",GCLOUD_CLI,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        if(system(cmdline)!=0){
            gcp_credential_convert(workdir,"delete",1);
            return 1;
        }
        if(strcmp(cmd_type,"delete")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s storage rm %s%s %s --all-versions",GCLOUD_CLI,binfo.bucket_address,real_remote_path,real_rflag);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s storage ls %s%s %s --readable-sizes --long",GCLOUD_CLI,binfo.bucket_address,real_remote_path,real_rflag);
        }
    }
    if(system(cmdline)!=0){
        unset_bucket_envs(cloud_flag);
        if(strcmp(cloud_flag,"CLOUD_E")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scredentials %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            gcp_credential_convert(workdir,"delete",1);
        }
        return 1;
    }
    else{
        unset_bucket_envs(cloud_flag);
        if(strcmp(cloud_flag,"CLOUD_E")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scredentials %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            gcp_credential_convert(workdir,"delete",1);
        }
        return 0;
    }
}

int direct_cp_mv(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* target_path, char* recursive_flag, char* force_flag, char* cmd_type){
    if(strcmp(cmd_type,"mv")!=0&&strcmp(cmd_type,"cp")!=0){
        return -1;
    }
    int path_flag1,path_flag2;
    int run_flag=0;
    char real_source_path[DIR_LENGTH]="";
    char real_target_path[DIR_LENGTH]="";
    char real_rf_flag[4]="";
    char remote_commands[CMDLINE_LENGTH]="";

    if(strcmp(cmd_type,"mv")==0){
        if(strcmp(force_flag,"force")==0){
            strcpy(real_rf_flag,"-f");
        }
        else{
            strcpy(real_rf_flag,"");
        }
    }
    else{
        if(strcmp(force_flag,"force")==0&&strcmp(recursive_flag,"recursive")==0){
            strcpy(real_rf_flag,"-rf");
        }
        else if(strcmp(force_flag,"force")!=0&&strcmp(recursive_flag,"recursive")!=0){
            strcpy(real_rf_flag,"");
        }
        else if(strcmp(force_flag,"force")==0&&strcmp(recursive_flag,"recursive")!=0){
            strcpy(real_rf_flag,"-f");
        }
        else{
            strcpy(real_rf_flag,"-r");
        }
    }
    path_flag1=direct_path_check(source_path,hpc_user,real_source_path);
    path_flag2=direct_path_check(target_path,hpc_user,real_target_path);
    if(strcmp(cmd_type,"mv")==0){
        if(path_flag1==path_flag2){
            if(path_flag1==0){
                if(strcmp(hpc_user,"user1")==0||strcmp(hpc_user,"root")==0){
                    snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else{
                    snprintf(remote_commands,CMDLINE_LENGTH-1,"mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
                }
            }
            else{
                if(strcmp(hpc_user,"user1")==0){
                    snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo mv /home/user1/%s /home/user1/%s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else if(strcmp(hpc_user,"root")==0){
                    snprintf(remote_commands,CMDLINE_LENGTH-1,"mv /root/%s /root/%s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else{
                    snprintf(remote_commands,CMDLINE_LENGTH-1,"mv /home/%s/%s /home/%s/%s %s &",hpc_user,real_source_path,hpc_user,real_target_path,real_rf_flag);
                }
            }
            run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
        }
        else{
            return 3;
        }
    }
    else{
        if(path_flag1==path_flag2){
            if(strcmp(hpc_user,"user1")==0||strcmp(hpc_user,"root")==0){
                snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo /bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            else{
                snprintf(remote_commands,CMDLINE_LENGTH-1,"/bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
        }
        else if(path_flag1==1&&path_flag2==0){
            run_flag=remote_copy(workdir,sshkey_dir,real_source_path,real_target_path,hpc_user,"put",real_rf_flag,1);
        }
        else{
            run_flag=remote_copy(workdir,sshkey_dir,real_target_path,real_source_path,hpc_user,"get",real_rf_flag,1);
        }
    }
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int direct_rm_ls_mkdir(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* force_flag, char* recursive_flag, char* cmd_type){
    if(strcmp(cmd_type,"rm")!=0&&strcmp(cmd_type,"ls")!=0&&strcmp(cmd_type,"mkdir")!=0){
        return -1;
    }
    int run_flag=0;
    char real_remote_path[DIR_LENGTH]="";
    char real_rf_flag[4]="";
    char remote_commands[CMDLINE_LENGTH]="";
    if(direct_path_check(remote_path,hpc_user,real_remote_path)!=0){
        if(strcmp(hpc_user,"root")==0){
            snprintf(real_remote_path,DIR_LENGTH-1,"/root/%s",remote_path);
        }
        else{
            snprintf(real_remote_path,DIR_LENGTH-1,"/home/%s/%s",hpc_user,remote_path);
        }
    }
    if(strcmp(force_flag,"force")==0&&strcmp(recursive_flag,"recursive")==0){
        strcpy(real_rf_flag,"-rf");
    }
    else if(strcmp(force_flag,"force")!=0&&strcmp(recursive_flag,"recursive")!=0){
        strcpy(real_rf_flag,"");
    }
    else if(strcmp(force_flag,"force")==0&&strcmp(recursive_flag,"recursive")!=0){
        strcpy(real_rf_flag,"-f");
    }
    else{
        strcpy(real_rf_flag,"-r");
    }
    if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
        if(strcmp(cmd_type,"rm")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo rm %s %s",real_rf_flag,real_remote_path);
        }
        else if(strcmp(cmd_type,"ls")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo ls -la %s",real_remote_path);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo mkdir -p %s",real_remote_path);
        }
    }
    else{
        if(strcmp(cmd_type,"rm")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"rm %s %s",real_rf_flag,real_remote_path);
        }
        else if(strcmp(cmd_type,"ls")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"ls -la %s",real_remote_path);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"mkdir -p %s",real_remote_path);
        }
    }
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int direct_file_operations(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* cmd_type){
    if(strcmp(cmd_type,"cat")!=0&&strcmp(cmd_type,"more")!=0&&strcmp(cmd_type,"less")!=0&&strcmp(cmd_type,"tail")!=0&&strcmp(cmd_type,"vim")!=0){
        return -3;
    }
    int run_flag=0;
    char real_remote_path[DIR_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    direct_path_check(remote_path,hpc_user,real_remote_path);
    if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
        if(strcmp(cmd_type,"tail")!=0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo %s %s",cmd_type,real_remote_path);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"sudo %s -f %s",cmd_type,real_remote_path);
        }
    }
    else{
        if(strcmp(cmd_type,"tail")!=0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"%s %s",cmd_type,real_remote_path);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"%s -f %s",cmd_type,real_remote_path);
        }
    }
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int remote_bucket_cp(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* dest_path, char* rflag, char* fflag, char* cloud_flag, char* crypto_keyfile, char* cmd_type){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"rput")!=0&&strcmp(cmd_type,"rget")!=0){
        return -5;
    }
    int run_flag=0;
    char real_rflag[32]="";
    char real_fflag[32]="";
    char real_source_path[DIR_LENGTH]="";
    char real_dest_path[DIR_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    /*char bucket_address[128]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";*/
    bucket_info binfo;
    char az_subscription_id[128]="";
    char az_tenant_id[128]="";
    if(get_bucket_ninfo(workdir,crypto_keyfile,LINE_LENGTH_SHORT,&binfo)!=0){
        return -1;
    }
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strlen(real_rflag)!=0){
            strcpy(real_rflag,"-r");
        }
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"-f");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--yes");
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(strlen(real_fflag)!=0){
            strcpy(real_fflag,"--force-if-read-only");
        }
    }
    if(strcmp(cmd_type,"rput")==0){
        direct_path_check(source_path,hpc_user,real_source_path);
        bucket_path_check(dest_path,hpc_user,real_dest_path);
    }
    else{
        bucket_path_check(source_path,hpc_user,real_source_path);
        direct_path_check(dest_path,hpc_user,real_dest_path);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"ossutil64 -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"ossutil64 -e oss-%s.aliyuncs.com -i %s -k %s cp %s %s%s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"coscli -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"coscli -e cos.%s.myqcloud.com -i %s -k %s cp %s %s%s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"export AWS_ACCESS_KEY_ID=%s&&export AWS_SECRET_ACCESS_KEY=%s&&export AWS_DEFAULT_REGION=%s&&aws s3 cp %s%s %s %s %s",binfo.bucket_ak,binfo.bucket_sk,binfo.region_id,binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"export AWS_ACCESS_KEY_ID=%s&&export AWS_SECRET_ACCESS_KEY=%s&&export AWS_DEFAULT_REGION=%s&&aws s3 cp %s %s%s %s %s",binfo.bucket_ak,binfo.bucket_sk,binfo.region_id,real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"obscli cp -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s%s %s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"obscli cp -e=obs.%s.myhuaweicloud.com -i=%s -k=%s %s %s%s %s %s",binfo.region_id,binfo.bucket_ak,binfo.bucket_sk,real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"bcecmd bos cp %s%s %s %s %s --conf-path /hpc_data/cluster_data/.bucket_creds/",binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"bcecmd bos cp %s %s%s %s %s --conf-path /hpc_data/cluster_data/.bucket_creds/",real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(get_azure_ninfo(workdir,LINE_LENGTH_SHORT,az_subscription_id,az_tenant_id,128)!=0){
            return -1;
        }
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"export AZCOPY_AUTO_LOGIN_TYPE=SPN&&export AZCOPY_SPA_APPLICATION_ID=%s&&export AZCOPY_SPA_CLIENT_SECRET=%s&&export AZCOPY_TENANT_ID=%s&&azcopy cp %s%s %s %s %s --log-level=ERROR",binfo.bucket_ak,binfo.bucket_sk,az_tenant_id,binfo.bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"export AZCOPY_AUTO_LOGIN_TYPE=SPN&&export AZCOPY_SPA_APPLICATION_ID=%s&&export AZCOPY_SPA_CLIENT_SECRET=%s&&export AZCOPY_TENANT_ID=%s&&azcopy cp %s %s%s %s %s --log-level=ERROR",binfo.bucket_ak,binfo.bucket_sk,az_tenant_id,real_source_path,binfo.bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else{
        remote_exec_general(workdir,sshkey_dir,hpc_user,"gcloud auth activate-service-account --key-file=/hpc_data/cluster_data/.bucket_key.json >> /dev/null 2>&1","-n",0,1,"","");
        if(strcmp(cmd_type,"rget")==0){
            snprintf(remote_commands,CMDLINE_LENGTH-1,"gcloud storage cp %s%s %s %s",binfo.bucket_address,real_source_path,real_dest_path,real_rflag);
        }
        else{
            snprintf(remote_commands,CMDLINE_LENGTH-1,"gcloud storage cp %s %s%s %s",real_source_path,binfo.bucket_address,real_dest_path,real_rflag);
        }
    }
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}