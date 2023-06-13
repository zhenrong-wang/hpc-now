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
#include "dataman.h"

void unset_aws_bucket_envs(void){
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

int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[64]="";
    char line_buffer[128]="";
    char header[16]="";
    char tail[64]="";
    sprintf(filename_temp,"%s%svault%sbucket_info.txt.tmp",workdir,PATH_SLASH,PATH_SLASH);
    get_crypto_key(crypto_keyfile,md5sum);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%svault%sbucket_info.txt",workdir,PATH_SLASH,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    int i=0;
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fgetline(file_p,line_buffer);
        get_seq_string(line_buffer,' ',1,header);
        get_seq_string(line_buffer,' ',2,tail);
        if(strcmp(header,"BUCKET:")==0){
            strcpy(bucket_address,tail);
            i++;
        }
        else if(strcmp(header,"REGION:")==0){
            strcpy(region_id,tail);
            i++;
        }
        else if(strcmp(header,"BUCKET_AK:")==0){
            strcpy(bucket_ak,tail);
            i++;
        }
        else if(strcmp(header,"BUCKET_SK:")==0){
            strcpy(bucket_sk,tail);
            i++;
        }
        else{
            continue;
        }
    }
    fclose(file_p);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(i!=4){
        strcpy(bucket_address,"");
        strcpy(region_id,"");
        strcpy(bucket_ak,"");
        strcpy(bucket_sk,"");
        return 1;
    }
    else{
        return 0;
    }
}

int bucket_path_check(char* path_string, char* real_path, char* hpc_user){
    char header[16]="";
    char tail[DIR_LENGTH]="";
    get_seq_string(path_string,':',1,header);
    get_seq_string(path_string,':',2,tail);
    if(strcmp(header,"--b")!=0){
        strcpy(real_path,path_string);
        return 1;
    }
    else{
        if(strcmp(hpc_user,"root")==0){
            if(*(tail+0)!='/'){
                sprintf(real_path,"/%s",tail);
            }
            else{
                sprintf(real_path,"%s",tail);
            }
        }
        else{
            if(*(tail+0)!='/'){
                sprintf(real_path,"/%s/%s",hpc_user,tail);
            }
            else{
                sprintf(real_path,"/%s%s",hpc_user,tail);
            }
        }
        return 0;
    }
}

int rf_flag_parser(const char* rf_flag, const char* cloud_flag, char* real_rflag, char* real_fflag){
    if(strcmp(rf_flag,"-r")!=0&&strcmp(rf_flag,"-rf")!=0&&strcmp(rf_flag,"-f")!=0){
        strcpy(real_rflag,"");
        strcpy(real_fflag,"");
        return 1;
    }
    else if(strcmp(rf_flag,"-r")==0){
        strcpy(real_rflag,"--recursive");
        strcpy(real_fflag,"");
    }
    else if(strcmp(rf_flag,"-rf")==0){
        strcpy(real_rflag,"--recursive");
        strcpy(real_fflag,"--force");
    }
    else{
        strcpy(real_rflag,"");
        strcpy(real_fflag,"--force");
    }
    return 0;
}

int bucket_cp(char* workdir, char* hpc_user, char* source_path, char* target_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    char bucket_address[64]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char real_rflag[16]="";
    char real_fflag[16]="";
    int path_flag1,path_flag2;
    char real_source_path[DIR_LENGTH]="";
    char real_target_path[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
        return -1;
    }
    rf_flag_parser(rf_flag,cloud_flag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    path_flag1=bucket_path_check(source_path,real_source_path,hpc_user);
    path_flag2=bucket_path_check(target_path,real_target_path,hpc_user);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(path_flag1==path_flag2){
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(path_flag1==1&&path_flag2==0){
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(path_flag1==path_flag2){
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(path_flag1==1&&path_flag2==0){
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else{
        if(path_flag1==path_flag2){
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s && %s AWS_SECRET_ACCESS_KEY=%s && %s AWS_DEFAULT_REGION=%s && %s s3 cp %s%s %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(path_flag1==1&&path_flag2==0){
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s && %s AWS_SECRET_ACCESS_KEY=%s && %s AWS_DEFAULT_REGION=%s && %s s3 cp %s %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s && %s AWS_SECRET_ACCESS_KEY=%s && %s AWS_DEFAULT_REGION=%s && %s s3 cp %s%s %s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    if(system(cmdline)!=0){
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 1;
    }
    else{
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 0;
    }
}

int bucket_rm(char* workdir, char* hpc_user, char* remote_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    char bucket_address[64]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char real_rflag[16]="";
    char real_fflag[16]="";
    char real_remote_path[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
        return -1;
    }
    rf_flag_parser(rf_flag,cloud_flag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    bucket_path_check(remote_path,real_remote_path,hpc_user);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s rm %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag,real_fflag);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s rm %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag,real_fflag);
    }
    else{
        sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s && %s AWS_SECRET_ACCESS_KEY=%s && %s AWS_DEFAULT_REGION=%s && %s s3 rm %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_remote_path,real_rflag,real_fflag);
    }
    if(system(cmdline)!=0){
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 1;
    }
    else{
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 0;
    }
}

int bucket_mv(char* workdir, char* hpc_user, char* prev_path, char* new_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    char real_prev_path[DIR_LENGTH]="";
    char real_new_path[DIR_LENGTH]="";
    bucket_path_check(prev_path,real_prev_path,hpc_user);
    bucket_path_check(new_path,real_new_path,hpc_user);
    if(bucket_cp(workdir,hpc_user,real_prev_path,real_new_path,rf_flag,crypto_keyfile,cloud_flag)!=0){
        return 1;
    }
    if(bucket_rm(workdir,hpc_user,real_prev_path,"-rf",crypto_keyfile,cloud_flag)!=0){
        return 3;
    }
    return 0;
}

int bucket_ls(char* workdir, char* hpc_user, char* remote_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    char bucket_address[64]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char real_rflag[16]="";
    char real_fflag[16]="";
    char real_remote_path[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
        return -1;
    }
    rf_flag_parser(rf_flag,cloud_flag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    bucket_path_check(remote_path,real_remote_path,hpc_user);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s ls %s%s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s ls %s%s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag);
    }
    else{
        sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s && %s AWS_SECRET_ACCESS_KEY=%s && %s AWS_DEFAULT_REGION=%s && %s s3 ls %s%s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_remote_path,real_rflag);
    }
    if(system(cmdline)!=0){
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 1;
    }
    else{
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            unset_aws_bucket_envs();
        }
        return 0;
    }
    return 0;
}

int direct_path_check(char* path_string, char* real_path, char* hpc_user){
    char header[32]="";
    char tail[DIR_LENGTH]="";
    get_seq_string(path_string,':',1,header);
    get_seq_string(path_string,':',2,tail);
    if(strcmp(header,"--home")==0){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/root/%s",tail);
        }
        else{
            sprintf(real_path,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"--data")==0){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/hpc_data/%s",tail);
        }
        else{
            sprintf(real_path,"/hpc_data/%s_data/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"--apps")==0){
        if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
            sprintf(real_path,"/hpc_apps/%s",tail);
        }
        else{
            sprintf(real_path,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"--/")==0){
        if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
            sprintf(real_path,"/%s",tail);
        }
        else{
            sprintf(real_path,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else{
        strcpy(real_path,path_string);
        return 1;
    }
}

int direct_cp_mv(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* target_path, char* rf_flag, char* cmd_type){
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
        if(strcmp(rf_flag,"-f")==0){
            strcpy(real_rf_flag,"-f");
        }
        else{
            strcpy(real_rf_flag,"");
        }
    }
    else{
        if(strcmp(rf_flag,"-r")==0||strcmp(rf_flag,"-f")||strcmp(rf_flag,"-rf")==0){
            strcpy(real_rf_flag,rf_flag);
        }
        else{
            strcpy(real_rf_flag,"");
        }
    }
    path_flag1=direct_path_check(source_path,real_source_path,hpc_user);
    path_flag2=direct_path_check(target_path,real_target_path,hpc_user);
    if(strcmp(cmd_type,"mv")==0){
        if(path_flag1==path_flag2){
            if(strcmp(hpc_user,"user1")==0||strcmp(hpc_user,"root")==0){
                sprintf(remote_commands,"sudo mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            else{
                sprintf(remote_commands,"mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,0,1);
        }
        else{
            return 3;
        }
    }
    else{
        if(path_flag1==path_flag2){
            if(strcmp(hpc_user,"user1")==0||strcmp(hpc_user,"root")==0){
                sprintf(remote_commands,"sudo /bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            else{
                sprintf(remote_commands,"/bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,0,1);
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

int direct_rm_ls_mkdir(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* rf_flag, char* cmd_type){
    if(strcmp(cmd_type,"rm")!=0&&strcmp(cmd_type,"ls")!=0&&strcmp(cmd_type,"mkdir")!=0){
        return -1;
    }
    int run_flag=0;
    char real_remote_path[DIR_LENGTH]="";
    char real_rf_flag[4]="";
    char remote_commands[CMDLINE_LENGTH]="";
    direct_path_check(remote_path,real_remote_path,hpc_user);
    if(strcmp(rf_flag,"-r")!=0&&strcmp(rf_flag,"-f")!=0&&strcmp(rf_flag,"-rf")!=0){
        strcpy(real_rf_flag,"");
    }
    else{
        strcpy(real_rf_flag,rf_flag);
    }
    if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
        if(strcmp(cmd_type,"rm")==0){
            sprintf(remote_commands,"sudo rm %s %s",real_rf_flag,real_remote_path);
        }
        else if(strcmp(cmd_type,"ls")==0){
            sprintf(remote_commands,"sudo ls -la %s",real_remote_path);
        }
        else{
            sprintf(remote_commands,"sudo mkdir -p %s",real_remote_path);
        }
    }
    else{
        if(strcmp(cmd_type,"rm")==0){
            sprintf(remote_commands,"rm %s %s",real_rf_flag,real_remote_path);
        }
        else if(strcmp(cmd_type,"ls")==0){
            sprintf(remote_commands,"ls -la %s",real_remote_path);
        }
        else{
            sprintf(remote_commands,"mkdir -p %s",real_remote_path);
        }
    }
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,0,1);
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
    direct_path_check(remote_path,real_remote_path,hpc_user);
    if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
        if(strcmp(cmd_type,"tail")!=0){
            sprintf(remote_commands,"sudo %s %s",cmd_type,real_remote_path);
        }
        else{
            sprintf(remote_commands,"sudo %s -f %s",cmd_type,real_remote_path);
        }
    }
    else{
        if(strcmp(cmd_type,"tail")!=0){
            sprintf(remote_commands,"%s %s",cmd_type,real_remote_path);
        }
        else{
            sprintf(remote_commands,"%s -f %s",cmd_type,real_remote_path);
        }
    }
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,0,1);
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int remote_bucket_cp(char* workdir, char* hpc_user, char* sshkey_dir, char* bucket_path, char* remote_path, char* rf_flag, char* cloud_flag, char* cmd_type, char* crypto_keyfile){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"rdownload")!=0&&strcmp(cmd_type,"rupload")!=0){
        return -5;
    }
    int run_flag=0;
    char real_rflag[16]="";
    char real_fflag[16]="";
    char real_bucket_path[DIR_LENGTH]="";
    char real_remote_path[DIR_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    char real_rf_flag[4]="";
    char bucket_address[32]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
//        printf("\n%d\n",get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk));
        return -1;
    }
    if(bucket_path_check(bucket_path,real_bucket_path,hpc_user)==1||direct_path_check(remote_path,real_remote_path,hpc_user)==1){
        return -7;
    }
    if(rf_flag_parser(rf_flag,cloud_flag,real_rflag,real_fflag)==1){
        strcpy(real_rf_flag,"");
    }
    else{
        strcpy(real_rf_flag,rf_flag);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(strcmp(cmd_type,"rdownload")==0){
            sprintf(remote_commands,"ossutil cp %s%s %s %s %s",bucket_address,real_bucket_path,real_remote_path,real_rflag,real_fflag);
        }
        else{
            sprintf(remote_commands,"ossutil cp %s %s%s %s %s",real_remote_path,bucket_address,real_bucket_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"rdownload")==0){
            sprintf(remote_commands,"coscmd download %s %s %s",real_rf_flag,real_bucket_path,real_remote_path);
        }
        else{
            sprintf(remote_commands,"coscmd upload %s %s %s",real_rf_flag,real_remote_path,real_bucket_path);
        }
    }
    else{
        if(strcmp(cmd_type,"rdownload")==0){
            sprintf(remote_commands,"s3cmd get %s%s %s %s %s",bucket_address,real_bucket_path,real_remote_path,real_rflag,real_fflag);
        }
        else{
            sprintf(remote_commands,"s3cmd put %s %s%s %s %s",real_remote_path,bucket_address,real_bucket_path,real_rflag,real_fflag);
        }
    }
//    printf("%s ---\n",remote_commands);
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,0,1);
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}