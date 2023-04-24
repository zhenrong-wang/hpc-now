/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "now_macros.h"
#include "general_funcs.h"
#include "components.h"
#include "cluster_general_funcs.h"
#include "prereq_check.h"

extern char URL_CODE_ROOT[LOCATION_LENGTH];
extern char URL_TF_ROOT[LOCATION_LENGTH];
extern char URL_SHELL_SCRIPTS[LOCATION_LENGTH];
extern char URL_NOW_CRYPTO[LOCATION_LENGTH];
extern int TF_LOC_FLAG;
extern int CODE_LOC_FLAG;
extern int NOW_CRYPTO_LOC_FLAG;

int check_internet(void){
#ifdef _WIN32
    if(system("ping -n 2 www.baidu.com > nul 2>&1")!=0){
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service\n");
        printf("|          or check your internet connectivity and retry later.\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }
#else
    if(system("ping -c 2 www.baidu.com >> /dev/null 2>&1")!=0){
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service\n");
        printf("|          or check your internet connectivity and retry later.\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }
#endif
    return 0;
}

int file_validity_check(char* filename, int repair_flag, char* target_md5){
    char md5sum[64]="";
    if(file_exist_or_not(filename)!=0){
        return 1;
    }
    else{
        if(repair_flag==1){
            get_crypto_key(filename,md5sum);
            if(strcmp(md5sum,target_md5)!=0){
                return 1;
            }
            else{
                return 0;
            }
        }
        return 0;
    }
}

int check_current_user(void){
#ifdef _WIN32
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
#else
    if(system("whoami | grep -w hpc-now >> /dev/null 2>&1")!=0){
        return 1;
    }
    else{
        return 0;
    }
#endif
}

int check_and_install_prerequisitions(int repair_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp_zip[FILENAME_LENGTH]="";
    char dirname_temp[DIR_LENGTH]="";
    int flag=0;
    int file_check_flag=0;
    int force_repair_flag;
    FILE* file_p=NULL;
    char* ali_plugin_version=ALI_TF_PLUGIN_VERSION;
    char* qcloud_plugin_version=QCLOUD_TF_PLUGIN_VERSION;
    char* aws_plugin_version=AWS_TF_PLUGIN_VERSION;
    char* usage_logfile=USAGE_LOG_FILE;
    char* operation_logfile=OPERATION_LOG_FILE;
    char* sshkey_dir=SSHKEY_DIR;
    char doubleconfirm[64]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* crypto_exec=NOW_CRYPTO_EXEC;
#ifdef _WIN32
    char appdata_dir[128]="";
    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");
#endif
    if(file_exist_or_not(usage_logfile)!=0){
        force_repair_flag=1;
    }
    else{
        force_repair_flag=repair_flag;
    }
    if(repair_flag==1){
        printf("[ -INFO- ] Start checking and repairing the HPC-NOW services now ... \n");
        printf("|        . Checking and repairing the registry now ...\n");
    }
    else{
        printf("[ -INFO- ] Checking the environment for HPC-NOW services ...\n");
    }
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        printf("[ -INFO- ] No registry file found. Creating a blank cluster registry now ...\n");
        file_p=fopen(ALL_CLUSTER_REGISTRY,"w+");
        if(file_p==NULL){
            printf("[ FATAL: ] Failed to open/write to the cluster registry. Exit now.\n");
            return -1;
        }
        fclose(file_p);
    }
    if(repair_flag==1){
        printf("|        v The registry has been repaired.\n");
        printf("|        . Checking and repairing the location configuration file now ...\n");
        if(reset_locations()!=0){
            printf("[ FATAL: ] Failed to reset the locations for binaries and templates. Exit now.\n");
            return -1;
        }
        printf("|        v All the locations has been reset to the default ones.\n");
    }
    flag=get_locations();
    if(flag!=0){
        if(flag==-1){
            printf("[ -INFO- ] Location configuration file not found. For developers, we recommend\n");
        }
        else{
            printf("[ -INFO- ] Location configuration format incorrect. For developers, we recommend\n");
        }
        printf("|          to exit and run the command 'hpcopr configloc' to provide your locations\n");
        printf("|          For general users without self-defined locations, we recommend to use\n");
        printf("|          the default ones.\n");
        printf("|          For more information, please check the documentations.\n");
        printf("[ -INFO- ] Would you like to use the default locations? Only 'y-e-s' is accepted as\n");
        printf("|          a confirmation. \n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")==0){
            if(reset_locations()!=0){
                printf("[ FATAL: ] Failed to reset the locations for binaries and templates. Exit now.\n");
                return 2;
            }
            get_locations();
        }
        else{
            printf("[ -INFO- ] You chose to deny this operation. Would you like to specify the locations\n");
            printf("|          immediately? Only 'y-e-s' is accepted as a confirmation. You can also run\n");
            printf("|          the command 'hpcopr configloc' later to update the locations.\n");
            printf("[ INPUT: ] ");
            fflush(stdin);
            scanf("%s",doubleconfirm);
            if(strcmp(doubleconfirm,"y-e-s")!=0){
                printf("[ -INFO- ] You chose to deny this operation. Exit now.\n");
                return 2;
            }
            else{
                if(configure_locations()!=0){
                    printf("[ FATAL: ] Failed to configure the locations. Exit now.\n");
                    return 2;
                }
            }
        }
    }
    if(repair_flag==1){
        printf("|        v Location configuration has been repaired.\n");
        printf("|        . Checking and repairing the key directories and files ...\n");
    }
#ifdef _WIN32
    system("mkdir c:\\programdata\\hpc-now\\ > nul 2>&1");
    system("attrib +h +s +r c:\\programdata\\hpc-now > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\bin\\ > nul 2>&1");
    system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
    sprintf(cmdline,"mkdir %s\\terraform.d\\ > nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(filename_temp_zip,"%s\\terraform.d\\terraform_%s_windows_amd64.zip",appdata_dir,TERRAFORM_VERSION);
#elif __linux__
    system("rm -rf /home/hpc-now/.ssh/known_hosts >> /dev/null 2>&1");
    system("mkdir -p /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1");
    system("mkdir -p /usr/.hpc-now/.bin/ >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    sprintf(cmdline,"mkdir -p /home/hpc-now/.terraform.d/ >> /dev/null 2>&1");
    system(cmdline);
    sprintf(filename_temp_zip,"/home/hpc-now/.terraform.d/terraform_%s_linux_amd64.zip",TERRAFORM_VERSION);
#elif __APPLE__
    system("rm -rf /Users/hpc-now/.ssh/known_hosts >> /dev/null 2>&1");
    system("mkdir -p /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1");
    system("mkdir -p /Applications/.hpc-now/.bin/ >> /dev/null 2>&1");
    system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    sprintf(cmdline,"mkdir -p /Users/hpc-now/.terraform.d/ >> /dev/null 2>&1");
    system(cmdline);
    sprintf(filename_temp_zip,"/Users/hpc-now/.terraform.d/terraform_%s_darwin_amd64.zip",TERRAFORM_VERSION);
#endif
    file_check_flag=file_validity_check(tf_exec,force_repair_flag,MD5_TF_EXEC);
    if(file_check_flag==1){
        file_check_flag=file_validity_check(filename_temp_zip,repair_flag,MD5_TF_ZIP);
        if(file_check_flag==1){
            printf("[ -INFO- ] Downloading/Copying the Terraform binary ...\n");
            printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode.\n\n");
            if(TF_LOC_FLAG==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform_%s_windows_amd64.zip %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform_%s_linux_amd64.zip %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform_%s_darwin_amd64.zip %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform_%s_windows_amd64.zip -o %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform_%s_linux_amd64.zip -o %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform_%s_darwin_amd64.zip -o %s",URL_TF_ROOT,TERRAFORM_VERSION,filename_temp_zip);
#endif
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
//        printf("%s,,,,,\"\n",cmdline);
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C c:\\programdata\\hpc-now\\bin\\ > nul 2>&1",filename_temp_zip);
#elif __linux__
        sprintf(cmdline,"unzip -o -q %s -d /usr/.hpc-now/.bin/ >> /dev/null 2>&1",filename_temp_zip);
#elif __APPLE__
        sprintf(cmdline,"unzip -o -q %s -d /Applications/.hpc-now/.bin/ >> /dev/null 2>&1",filename_temp_zip);
#endif
//        printf("%s,,,,,\"\n",cmdline);
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to unzip the terraform binary file. Exit now.\n");
            return 3;
        }        
    }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s",tf_exec);
        system(cmdline);
#endif
    if(repair_flag==1){
        printf("|        v The Terraform executable has been repaired.\n");
    }

    if(repair_flag!=0){
        file_check_flag=file_validity_check(crypto_exec,repair_flag,MD5_NOW_CRYPTO);
    }
    else{
        file_check_flag=file_validity_check(crypto_exec,0,MD5_NOW_CRYPTO);
    }
    if(file_check_flag==1){
        printf("[ -INFO- ] Downloading/Copying the now-crypto.exe ...\n");
        printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode.\n\n");
        if(NOW_CRYPTO_LOC_FLAG==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s %s",URL_NOW_CRYPTO,crypto_exec);
#else
            sprintf(cmdline,"/bin/cp %s %s",URL_NOW_CRYPTO,crypto_exec);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_NOW_CRYPTO,crypto_exec);
#else
            sprintf(cmdline,"curl %s -o %s",URL_NOW_CRYPTO,crypto_exec);
#endif
        }
//        printf("%s,,,,,\"\n",cmdline);
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s",crypto_exec);
        system(cmdline);
#endif
    if(repair_flag==1){
        printf("|        v The now-crypto executable has been repaired.\n");
    }

#ifdef _WIN32
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
#elif __linux__
    if(file_exist_or_not("/home/hpc-now/.terraformrc")!=0){
        file_p=fopen("/home/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"/home/hpc-now/.terraform.d/plugins\"\n");
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }
#elif __APPLE__
    if(file_exist_or_not("/Users/hpc-now/.terraformrc")!=0){
        file_p=fopen("/Users/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"/Users/hpc-now/.terraform.d/plugins\"\n");
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }
#endif
    if(repair_flag==1){
        printf("|        v The terraformrc file has been repaired.\n");
        printf("|        . Checking and repairing the Terraform Providers ... \n");
    }

#ifdef _WIN32
    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\aliyun\\alicloud\\%s\\windows_amd64\\",appdata_dir,ali_plugin_version);
    sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s\\terraform-provider-alicloud_v%s.exe",dirname_temp,ali_plugin_version);
    sprintf(filename_temp_zip,"%s\\terraform.d\\terraform-provider-alicloud_%s_windows_amd64.zip",appdata_dir,ali_plugin_version);
#elif __linux__
    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/aliyun/alicloud/%s/linux_amd64/",ali_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-alicloud_v%s",dirname_temp,ali_plugin_version);
    sprintf(filename_temp_zip,"/home/hpc-now/.terraform.d/terraform-provider-alicloud_%s_linux_amd64.zip",ali_plugin_version);
#elif __APPLE__
    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/aliyun/alicloud/%s/darwin_amd64/",ali_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-alicloud_v%s",dirname_temp,ali_plugin_version);
    sprintf(filename_temp_zip,"/Users/hpc-now/.terraform.d/terraform-provider-alicloud_%s_darwin_amd64.zip",ali_plugin_version);
#endif
    file_check_flag=file_validity_check(filename_temp,force_repair_flag,MD5_ALI_TF);
    if(file_check_flag==1){
        printf("[ -INFO- ] Downloading/Copying the cloud Terraform providers (1/3) ...\n");
        printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode.\n\n");
        file_check_flag=file_validity_check(filename_temp_zip,force_repair_flag,MD5_ALI_TF_ZIP);
        if(file_check_flag==1){
            if(TF_LOC_FLAG==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform-provider-alicloud_%s_windows_amd64.zip %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform-provider-alicloud_%s_linux_amd64.zip %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform-provider-alicloud_%s_darwin_amd64.zip %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform-provider-alicloud_%s_windows_amd64.zip -o %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform-provider-alicloud_%s_linux_amd64.zip -o %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform-provider-alicloud_%s_darwin_amd64.zip -o %s",URL_TF_ROOT,ali_plugin_version,filename_temp_zip);
#endif
            }
//            printf("%s,,,,,\"\n",cmdline);
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp_zip,dirname_temp);
#else
        sprintf(cmdline,"unzip -o -q %s -d %s >> /dev/null 2>&1",filename_temp_zip,dirname_temp);
#endif
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to unzip the provider file. Exit now.\n");
            return 3;
        }
    }

#ifdef _WIN32
    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\tencentcloudstack\\tencentcloud\\%s\\windows_amd64\\",appdata_dir,qcloud_plugin_version);
    sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s\\terraform-provider-tencentcloud_v%s.exe",dirname_temp,qcloud_plugin_version);
    sprintf(filename_temp_zip,"%s\\terraform.d\\terraform-provider-tencentcloud_%s_windows_amd64.zip",appdata_dir,qcloud_plugin_version);
#elif __linux__
    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/tencentcloudstack/tencentcloud/%s/linux_amd64/",qcloud_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-tencentcloud_v%s",dirname_temp,qcloud_plugin_version);
    sprintf(filename_temp_zip,"/home/hpc-now/.terraform.d/terraform-provider-tencentcloud_%s_linux_amd64.zip",qcloud_plugin_version);
#elif __APPLE__
    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/tencentcloudstack/tencentcloud/%s/darwin_amd64/",qcloud_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-tencentcloud_v%s",dirname_temp,qcloud_plugin_version);
    sprintf(filename_temp_zip,"/Users/hpc-now/.terraform.d/terraform-provider-tencentcloud_%s_darwin_amd64.zip",qcloud_plugin_version);
#endif
    file_check_flag=file_validity_check(filename_temp,force_repair_flag,MD5_QCLOUD_TF);
    if(file_check_flag==1){
        printf("[ -INFO- ] Downloading/Copying the cloud Terraform providers (2/3) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr or repair mode.\n\n");
        file_check_flag=file_validity_check(filename_temp_zip,force_repair_flag,MD5_QCLOUD_TF_ZIP);
        if(file_check_flag==1){
            if(TF_LOC_FLAG==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform-provider-tencentcloud_%s_windows_amd64.zip %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform-provider-tencentcloud_%s_linux_amd64.zip %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform-provider-tencentcloud_%s_darwin_amd64.zip %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform-provider-tencentcloud_%s_windows_amd64.zip -o %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform-provider-tencentcloud_%s_linux_amd64.zip -o %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform-provider-tencentcloud_%s_darwin_amd64.zip -o %s",URL_TF_ROOT,qcloud_plugin_version,filename_temp_zip);
#endif
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp_zip,dirname_temp);
#else
        sprintf(cmdline,"unzip -o -q %s -d %s >> /dev/null 2>&1",filename_temp_zip,dirname_temp);
#endif
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to unzip the provider file. Exit now.\n");
            return 3;
        }
    }

#ifdef _WIN32
    sprintf(dirname_temp,"%s\\terraform.d\\plugins\\registry.terraform.io\\hashicorp\\aws\\%s\\windows_amd64\\",appdata_dir,aws_plugin_version);
    sprintf(cmdline,"mkdir %s > nul 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s\\terraform-provider-aws_v%s_x5.exe",dirname_temp,aws_plugin_version);
    sprintf(filename_temp_zip,"%s\\terraform.d\\terraform-provider-aws_%s_windows_amd64.zip",appdata_dir,aws_plugin_version);
#elif __linux__
    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/hashicorp/aws/%s/linux_amd64/",aws_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-aws_v%s_x5",dirname_temp,aws_plugin_version);
    sprintf(filename_temp_zip,"/home/hpc-now/.terraform.d/terraform-provider-aws_%s_linux_amd64.zip",aws_plugin_version);
#elif __APPLE__
    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/hashicorp/aws/%s/darwin_amd64/",aws_plugin_version);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
    system(cmdline);
    sprintf(filename_temp,"%s/terraform-provider-aws_v%s_x5",dirname_temp,aws_plugin_version);
    sprintf(filename_temp_zip,"/Users/hpc-now/.terraform.d/terraform-provider-aws_%s_x5_darwin_amd64.zip",aws_plugin_version);
#endif
    file_check_flag=file_validity_check(filename_temp,force_repair_flag,MD5_AWS_TF);
    if(file_check_flag==1){
        printf("[ -INFO- ] Downloading/Copying the cloud Terraform providers (3/3) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr or repair mode.\n\n");
        file_check_flag=file_validity_check(filename_temp_zip,force_repair_flag,MD5_AWS_TF_ZIP);
        if(file_check_flag==1){
            if(TF_LOC_FLAG==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform-provider-aws_%s_windows_amd64.zip %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform-provider-aws_%s_linux_amd64.zip %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform-provider-aws_%s_darwin_amd64.zip %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform-provider-aws_%s_windows_amd64.zip -o %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform-provider-aws_%s_linux_amd64.zip -o %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform-provider-aws_%s_darwin_amd64.zip -o %s",URL_TF_ROOT,aws_plugin_version,filename_temp_zip);
#endif
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s > nul 2>&1",filename_temp_zip,dirname_temp);
#else
        sprintf(cmdline,"unzip -o -q %s -d %s >> /dev/null 2>&1",filename_temp_zip,dirname_temp);
#endif
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to unzip the provider file. Exit now.\n");
            return 3;
        }
    }

    if(repair_flag==1){
        printf("|        v The Terraform Providers have been repaired.\n");
        printf("|        . Checking and repairing the key folders and environment variables ... \n");
    }

#ifdef _WIN32
    sprintf(cmdline,"mkdir %s > nul 2>&1",sshkey_dir);
    system(cmdline);
    sprintf(cmdline,"attrib +h +s +r %s",sshkey_dir);
    system(cmdline);
#else
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",sshkey_dir);
    system(cmdline);
#endif
    
    if(file_exist_or_not(usage_logfile)!=0){
        file_p=fopen(usage_logfile,"w+");
        fprintf(file_p,"UCID,CLOUD_VENDOR,NODE_NAME,vCPU,START_DATE,START_TIME,STOP_DATE,STOP_TIME,RUNNING_HOURS,CPUxHOURS,CPU_MODEL,CLOUD_REGION\n");
        fclose(file_p);
    }
    if(file_exist_or_not(operation_logfile)!=0){
        file_p=fopen(operation_logfile,"w+");
        fclose(file_p);
    }
    
#ifdef _WIN32
    strcpy(cmdline,"setx PATH C:\\WINDOWS\\system32;C:\\hpc-now\\;C:\\WINDOWS;C:\\WINDOWS\\System32\\Wbem;C:\\WINDOWS\\System32\\WindowsPowerShell\\v1.0\\;C:\\WINDOWS\\System32\\OpenSSH\\ > nul 2>&1");
    system(cmdline);
    sprintf(cmdline,"del /f /q %%homepath%%\\.ssh\\known_hosts > nul 2>&1");
#elif __linux__
    if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/home/hpc-now/.bin/:$PATH >> /home/hpc-now/.bashrc");
        system(cmdline);
    }
    sprintf(cmdline,"rm -rf %s/known_hosts >> /dev/null 2>&1",sshkey_dir);
#elif __APPLE__
    if(system("cat /Users/hpc-now/.bashrc | grep PATH=/Users/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/Users/hpc-now/.bin/:$PATH >> /Users/hpc-now/.bashrc");
        system(cmdline);
    }
    sprintf(cmdline,"rm -rf %s/known_hosts >> /dev/null 2>&1",sshkey_dir);
#endif
    system(cmdline);
    if(repair_flag==1){
        printf("|        v Environment variables have been repaired.\n");
        printf("|        v SSH files have been repaired. \n");
        printf("[ -INFO- ] Running environment successfully check and repaired.\n");
    }
    else{
        printf("[ -INFO- ] Running environment successfully checked.\n");
    }
    return 0;
}