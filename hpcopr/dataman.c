/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
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
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"put")!=0&&strcmp(cmd_type,"get")!=0&&strcmp(cmd_type,"copy")!=0){
        return -5;
    }
    char bucket_address[64]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char real_rflag[16]="";
    char real_fflag[16]="";
    char real_source_path[DIR_LENGTH]="";
    char real_target_path[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
        return -1;
    }
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
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
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"copy")==0){
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
        }
    }
    else{
        if(strcmp(cmd_type,"copy")==0){
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s%s %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else if(strcmp(cmd_type,"put")==0){
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,real_source_path,bucket_address,real_target_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 cp %s%s %s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_source_path,real_target_path,real_rflag,real_fflag);
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

int bucket_rm_ls(char* workdir, char* hpc_user, char* remote_path, char* rflag, char* fflag, char* crypto_keyfile, char* cloud_flag, char* cmd_type){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"delete")!=0&&strcmp(cmd_type,"list")!=0){
        return -5;
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
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
    }
    bucket_path_check(remote_path,hpc_user,real_remote_path);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(strcmp(cmd_type,"delete")==0){
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s rm %s%s %s %s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e oss-%s.aliyuncs.com -i %s -k %s ls %s%s",OSSUTIL_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"delete")==0){
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s rm %s%s %s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s -e cos.%s.myqcloud.com -i %s -k %s ls %s%s %s",COSCLI_EXEC,region_id,bucket_ak,bucket_sk,bucket_address,real_remote_path,real_rflag);
        }
    }
    else{
        if(strcmp(cmd_type,"delete")==0){
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 rm %s%s %s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_remote_path,real_rflag,real_fflag);
        }
        else{
            sprintf(cmdline,"%s AWS_ACCESS_KEY_ID=%s&&%s AWS_SECRET_ACCESS_KEY=%s&&%s AWS_DEFAULT_REGION=%s&&%s s3 ls %s%s %s",SET_ENV_CMD,bucket_ak,SET_ENV_CMD,bucket_sk,SET_ENV_CMD,region_id,S3CLI_EXEC,bucket_address,real_remote_path,real_rflag);
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
                    sprintf(remote_commands,"sudo mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else{
                    sprintf(remote_commands,"mv %s %s %s &",real_source_path,real_target_path,real_rf_flag);
                }
            }
            else{
                if(strcmp(hpc_user,"user1")==0){
                    sprintf(remote_commands,"sudo mv /home/user1/%s /home/user1/%s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else if(strcmp(hpc_user,"root")==0){
                    sprintf(remote_commands,"mv /root/%s /root/%s %s &",real_source_path,real_target_path,real_rf_flag);
                }
                else{
                    sprintf(remote_commands,"mv /home/%s/%s /home/%s/%s %s &",hpc_user,real_source_path,hpc_user,real_target_path,real_rf_flag);
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
                sprintf(remote_commands,"sudo /bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
            }
            else{
                sprintf(remote_commands,"/bin/cp %s %s %s &",real_source_path,real_target_path,real_rf_flag);
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
            sprintf(real_remote_path,"/root/%s",remote_path);
        }
        else{
            sprintf(real_remote_path,"/home/%s/%s",hpc_user,remote_path);
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
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int remote_bucket_cp(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* dest_path, char* rflag, char* fflag, char* cloud_flag, char* crypto_keyfile, char* cmd_type){
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    if(strcmp(cmd_type,"rput")!=0&&strcmp(cmd_type,"rget")!=0){
        return -5;
    }
    int run_flag=0;
    char real_rflag[16]="";
    char real_fflag[16]="";
    char real_source_path[DIR_LENGTH]="";
    char real_dest_path[DIR_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    char bucket_address[32]="";
    char region_id[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
//        printf("\n%d\n",get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk));
        return -1;
    }
    rf_flag_parser(rflag,fflag,real_rflag,real_fflag);
    if(strcmp(cloud_flag,"CLOUD_B")==0||strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(real_fflag,"");
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
            sprintf(remote_commands,"ossutil64 -e oss-%s.aliyuncs.com -i %s -k %s cp %s%s %s %s %s",region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            sprintf(remote_commands,"ossutil64 -e oss-%s.aliyuncs.com -i %s -k %s cp %s %s%s %s %s",region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(strcmp(cmd_type,"rget")==0){
            sprintf(remote_commands,"coscli -e cos.%s.myqcloud.com -i %s -k %s cp %s%s %s %s %s",region_id,bucket_ak,bucket_sk,bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            sprintf(remote_commands,"coscli -e cos.%s.myqcloud.com -i %s -k %s cp %s %s%s %s %s",region_id,bucket_ak,bucket_sk,real_source_path,bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
    else{
        if(strcmp(cmd_type,"rget")==0){
            sprintf(remote_commands,"export AWS_ACCESS_KEY_ID=%s&&export AWS_SECRET_ACCESS_KEY=%s&&export AWS_DEFAULT_REGION=%s&&aws s3 cp %s%s %s %s %s",bucket_ak,bucket_sk,region_id,bucket_address,real_source_path,real_dest_path,real_rflag,real_fflag);
        }
        else{
            sprintf(remote_commands,"export AWS_ACCESS_KEY_ID=%s&&export AWS_SECRET_ACCESS_KEY=%s&&export AWS_DEFAULT_REGION=%s&&aws s3 cp %s %s%s %s %s",bucket_ak,bucket_sk,region_id,real_source_path,bucket_address,real_dest_path,real_rflag,real_fflag);
        }
    }
//    printf("%s ---\n",remote_commands);
    run_flag=remote_exec_general(workdir,sshkey_dir,hpc_user,remote_commands,"-n",0,1,"","");
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}