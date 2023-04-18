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

int envcheck(char* pwd){
    char current_dir[DIR_LENGTH]="";
    char string_temp[DIR_LENGTH]="";
    int i;
    FILE* file_p=NULL;
#ifdef _WIN32
    if(system("echo \%cd\% > .currentdir.tmp")!=0){
#else
    if(system("pwd > /tmp/.currentdir.tmp")!=0){
#endif
        strcpy(pwd,"");
        return 1;
    }

#ifdef _WIN32
    file_p=fopen(".currentdir.tmp","r");
#else
    file_p=fopen("/tmp/.currentdir.tmp","r");
#endif
    if(file_p==NULL){
        strcpy(pwd,"");
        return 1;
    }
    fscanf(file_p,"%s",current_dir);
    fclose(file_p);
#ifdef _WIN32
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
#elif __APPLE__
    system("rm -rf /tmp/.currentdir.tmp");
    if(strlen(current_dir)>29){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    for(i=0;i<27;i++){
        *(string_temp+i)=*(current_dir+i);
    }
    if(strcmp(string_temp,"/Users/hpc-now/now-cluster-")!=0){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(*(current_dir+27)>'9'||*(current_dir+27)<'1'){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(strlen(current_dir)==29){
        if(*(current_dir+28)>'9'||*(current_dir+28)<'0'){
            print_not_in_a_workdir(current_dir);
            strcpy(pwd,"");
            return 1;
        }
        strcpy(pwd,current_dir);
        return 0;
    }
#elif __linux__
    system("rm -rf /tmp/.currentdir.tmp");
    if(strlen(current_dir)>28){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    for(i=0;i<26;i++){
        *(string_temp+i)=*(current_dir+i);
    }
    if(strcmp(string_temp,"/home/hpc-now/now-cluster-")!=0){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(*(current_dir+26)>'9'||*(current_dir+26)<'1'){
        print_not_in_a_workdir(current_dir);
        strcpy(pwd,"");
        return 1;
    }
    if(strlen(current_dir)==28){
        if(*(current_dir+27)>'9'||*(current_dir+27)<'0'){
            print_not_in_a_workdir(current_dir);
            strcpy(pwd,"");
            return 1;
        }
        strcpy(pwd,current_dir);
        return 0;
    }
#endif
    strcpy(pwd,current_dir);
    return 0;
}

int check_internet(void){
#ifdef _WIN32
    if(system("ping -n 2 www.baidu.com > nul 2>&1")!=0){
#else
    if(system("ping -c 2 www.baidu.com >> /dev/null 2>&1")!=0){
#endif
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service\n");
        printf("|          or check your internet connectivity and retry later.\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }
    return 0;
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

int check_and_install_prerequisitions(void){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char dirname_temp[DIR_LENGTH]="";
    char md5sum[64]="";
    int flag=0;
    FILE* file_p=NULL;
    char* ali_plugin_version=ALI_TF_PLUGIN_VERSION;
    char* qcloud_plugin_version=QCLOUD_TF_PLUGIN_VERSION;
    char* aws_plugin_version=AWS_TF_PLUGIN_VERSION;
    char* usage_logfile=USAGE_LOG_FILE;
    char* operation_logfile=OPERATION_LOG_FILE;
    char* sshkey_dir=SSHKEY_DIR;
    char doubleconfirm[64]="";
#ifdef _WIN32
    char random_string[PASSWORD_STRING_LENGTH]="";
    char appdata_dir[128]="";
#endif

    printf("[ -INFO- ] Checking running environment for HPC-NOW services ...\n");
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        printf("[ -INFO- ] No registry file found. Create a blank cluster registry now.\n");
#ifdef _WIN32
        system("mkdir -p c:\\programdata\\hpc-now\\etc\\ > nul 2>&1");
#elif __APPLE__
        system("mkdir -p /Applications/.hpc-now/.etc/ >> /dev/null 2>&1");
#elif __linux__
        system("mkdir -p /usr/.hpc-now/.etc/ >> /dev/null 2>&1");
#endif
        file_p=fopen(ALL_CLUSTER_REGISTRY,"w+");
        if(file_p==NULL){
            printf("[ FATAL: ] Failed to open/write to the cluster registry. Exit now.\n");
            return -1;
        }
        fclose(file_p);
    }
    if(get_locations()==-1){
        printf("[ -INFO- ] Location configuration not found. If you are a developer, we recommend you\n");
        printf("|          to exit and run the command 'hpcopr configloc' to provide your locations\n");
        printf("|          If you are a user without self-defined locations, you can continue with\n");
        printf("|          the default ones. For more information, please check the documentations.\n");
        printf("[ -INFO- ] Would you like to use the default locations? Only 'y-e-s' is accepted as\n");
        printf("|          a confirmation. \n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
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
                    printf("[ FATAL: ] Failed to set the locations. Exit now.\n");
                    return 2;
                }
            }
        }
        if(reset_locations()!=0){
            printf("[ FATAL: ] Failed to set the locations for binaries and templates. Exit now.\n");
            return 2;
        }
    }
    else if(get_locations()==1){
        printf("[ -INFO- ] Location configuration format not correct. Reset to the default locations.\n");
        if(reset_locations()!=0){
            printf("[ FATAL: ] Failed to set the locations for binaries and templates. Exit now.\n");
            return 2;
        }
    }
    
#ifdef _WIN32
    system("mkdir c:\\programdata\\hpc-now\\ > nul 2>&1");
    system("attrib +h +s +r c:\\programdata\\hpc-now > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\bin\\ > nul 2>&1");
    system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1"); 
    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\terraform.exe")==0){
        get_crypto_key("c:\\programdata\\hpc-now\\bin\\terraform.exe",md5sum);
    }
    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\terraform.exe")!=0||strcmp(md5sum,MD5_TF_EXEC)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (1/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(REPO_LOC_FLAG==1){
            sprintf(cmdline,"copy /y %s\\terraform-win64\\terraform.exe c:\\programdata\\hpc-now\\bin\\terraform.exe",URL_REPO_ROOT);
        }
        else{
            sprintf(cmdline,"curl %sterraform-win64/terraform.exe -o c:\\programdata\\hpc-now\\bin\\terraform.exe",URL_REPO_ROOT);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }

    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\now-crypto.exe")==0){
        get_crypto_key("c:\\programdata\\hpc-now\\bin\\now-crypto.exe",md5sum);
    }
    if(file_exist_or_not("c:\\programdata\\hpc-now\\bin\\now-crypto.exe")!=0||strcmp(md5sum,MD5_NOW_CRYPTO)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (2/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(NOW_CRYPTO_LOC_FLAG==1){
            sprintf(cmdline,"copy /y %s c:\\programdata\\hpc-now\\bin\\now-crypto.exe",URL_NOW_CRYPTO);
        }
        else{
            sprintf(cmdline,"curl %s -o c:\\programdata\\hpc-now\\bin\\now-crypto.exe",URL_NOW_CRYPTO);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }

    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");

    sprintf(cmdline,"del /f /s /q %s\\Microsoft\\Windows\\Recent\\* > nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(cmdline,"rd /q /s %s\\Microsoft\\Windows\\Recent\\ > nul 2>&1",appdata_dir);
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
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (3/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-alicloud_%s_windows_amd64.zip",appdata_dir,ali_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"copy /y %s\\terraform-win64\\terraform-provider-alicloud_%s_windows_amd64.zip %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-win64/terraform-provider-alicloud_%s_windows_amd64.zip -o %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
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
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (4/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-tencentcloud_%s_windows_amd64.zip",appdata_dir,qcloud_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"copy /y %s\\terraform-win64\\terraform-provider-tencentcloud_%s_windows_amd64.zip %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-win64/terraform-provider-tencentcloud_%s_windows_amd64.zip -o %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
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
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (5/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"%s\\terraform.d\\terraform-provider-aws_%s_windows_amd64.zip",appdata_dir,aws_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"copy /y %s\\terraform-win64\\terraform-provider-aws_%s_windows_amd64.zip %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-win64/terraform-provider-aws_%s_windows_amd64.zip -o %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
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
    system("attrib +h +s +r c:\\programdata\\hpc-now\\now_crypto_seed.lock > nul 2>&1");
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

    if(file_exist_or_not(operation_logfile)!=0){
        sprintf(cmdline,"type nul > %s",operation_logfile);
        system(cmdline);
    }
    strcpy(cmdline,"setx PATH C:\\WINDOWS\\system32;C:\\hpc-now\\;C:\\WINDOWS;C:\\WINDOWS\\System32\\Wbem;C:\\WINDOWS\\System32\\WindowsPowerShell\\v1.0\\;C:\\WINDOWS\\System32\\OpenSSH\\ > nul 2>&1");
    system(cmdline);

#elif __linux__
    system("rm -rf /home/hpc-now/.ssh/known_hosts >> /dev/null 2>&1");
    system("mkdir -p /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1");
    system("mkdir -p /usr/.hpc-now/.bin/ >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    if(file_exist_or_not("/usr/.hpc-now/.bin/terraform.exe")==0){
        get_crypto_key("/usr/.hpc-now/.bin/terraform.exe",md5sum);
    }
    if(file_exist_or_not("/usr/.hpc-now/.bin/terraform.exe")!=0||strcmp(md5sum,MD5_TF_EXEC)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (1/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(REPO_LOC_FLAG==1){
            sprintf(cmdline,"/bin/cp %s/terraform/terraform /usr/.hpc-now/.bin/terraform.exe",URL_REPO_ROOT)；
        }
        else{
            sprintf(cmdline,"curl %sterraform/terraform -o /usr/.hpc-now/.bin/terraform.exe",URL_REPO_ROOT);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }
    system("chmod +x /usr/.hpc-now/.bin/terraform.exe");

    if(file_exist_or_not("/usr/.hpc-now/.bin/now-crypto.exe")==0){
        get_crypto_key("/usr/.hpc-now/.bin/now-crypto.exe",md5sum);
    }
    if(file_exist_or_not("/usr/.hpc-now/.bin/now-crypto.exe")!=0||strcmp(md5sum,MD5_NOW_CRYPTO)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (2/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(NOW_CRYPTO_LOC_FLAG==1){
            sprintf(cmdline,"/bin/cp %s /usr/.hpc-now/.bin/now-crypto.exe",URL_NOW_CRYPTO);
        }
        else{
            sprintf(cmdline,"curl %s -o /usr/.hpc-now/.bin/now-crypto.exe",URL_NOW_CRYPTO);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }
    system("chmod +x /usr/.hpc-now/.bin/now-crypto.exe");

    if(file_exist_or_not("/home/hpc-now/.terraformrc")!=0){
        file_p=fopen("/home/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"/home/hpc-now/.terraform.d/plugins\"\n");
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }

    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/aliyun/alicloud/%s/linux_amd64/",ali_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-alicloud_v%s",dirname_temp,ali_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (3/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/home/hpc-now/.terraform.d/terraform-provider-alicloud_v%s.tar.xz",ali_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform/terraform-provider-alicloud_v%s.tar.xz %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform/terraform-provider-alicloud_v%s.tar.xz -o %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"tar xf %s -C %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);
    }

    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/tencentcloudstack/tencentcloud/%s/linux_amd64/",qcloud_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-tencentcloud_v%s",dirname_temp,qcloud_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (4/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/home/hpc-now/.terraform.d/terraform-provider-tencentcloud_v%s.tar.xz",qcloud_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform/terraform-provider-tencentcloud_v%s.tar.xz %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform/terraform-provider-tencentcloud_v%s.tar.xz -o %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }  
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"tar xf %s -C %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);   
    }

    sprintf(dirname_temp,"/home/hpc-now/.terraform.d/plugins/registry.terraform.io/hashicorp/aws/%s/linux_amd64/",aws_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-aws_v%s_x5",dirname_temp,aws_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (5/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/home/hpc-now/.terraform.d/terraform-provider-aws_v%s_x5.tar.xz",aws_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform/terraform-provider-aws_v%s_x5.tar.xz %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform/terraform-provider-aws_v%s_x5.tar.xz -o %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"tar xf %s -C %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);   
    }

    if(file_exist_or_not(usage_logfile)!=0){
        file_p=fopen(usage_logfile,"w+");
        fprintf(file_p,"UCID,CLOUD_VENDOR,NODE_NAME,vCPU,START_DATE,START_TIME,STOP_DATE,STOP_TIME,RUNNING_HOURS,CPUxHOURS,CPU_MODEL,CLOUD_REGION\n");
        fclose(file_p);
    }
    
    sprintf(filename_temp,"%s/known_hosts",sshkey_dir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
        system(cmdline);
    }

// create the syslog file.
    if(file_exist_or_not(operation_logfile)!=0){
        sprintf(cmdline,"echo \"\" > %s",operation_logfile);
        system(cmdline);
    }
    if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/home/hpc-now/.bin/:$PATH >> /home/hpc-now/.bashrc");
        system(cmdline);
    }

#elif __APPLE__
    system("rm -rf /Users/hpc-now/.ssh/known_hosts >> /dev/null 2>&1");
    system("mkdir -p /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1");
    system("mkdir -p /Applications/.hpc-now/.bin/ >> /dev/null 2>&1");
    system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    
    if(file_exist_or_not("/Applications/.hpc-now/.bin/terraform")==0){
        get_crypto_key("/Applications/.hpc-now/.bin/terraform",md5sum);
    }
    if(file_exist_or_not("/Applications/.hpc-now/.bin/terraform")!=0||strcmp(md5sum,MD5_TF_EXEC)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (1/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(REPO_LOC_FLAG==1){
            sprintf(cmdline,"/bin/cp %s/terraform-darwin/terraform /Applications/.hpc-now/.bin/terraform",URL_REPO_ROOT);
        }
        else{
            sprintf(cmdline,"curl %sterraform-darwin/terraform -o /Applications/.hpc-now/.bin/terraform",URL_REPO_ROOT);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }
    system("chmod +x /Applications/.hpc-now/.bin/terraform");

    if(file_exist_or_not("/Applications/.hpc-now/.bin/now-crypto.exe")==0){
        get_crypto_key("/Applications/.hpc-now/.bin/now-crypto.exe",md5sum);
    }
    if(file_exist_or_not("/Applications/.hpc-now/.bin/now-crypto.exe")!=0||strcmp(md5sum,MD5_NOW_CRYPTO)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (2/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        if(NOW_CRYPTO_LOC_FLAG==1){
            sprintf(cmdline,"/bin/cp %s /Applications/.hpc-now/.bin/now-crypto.exe",URL_NOW_CRYPTO);
        }
        else{
            sprintf(cmdline,"curl %s -o /Applications/.hpc-now/.bin/now-crypto.exe",URL_NOW_CRYPTO);
        }
        flag=system(cmdline);
        if(flag!=0){
            printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now.\n");
            return 3;
        }
    }
    system("chmod +x /Applications/.hpc-now/.bin/now-crypto.exe");
    
    if(file_exist_or_not("/Users/hpc-now/.terraformrc")!=0){
        file_p=fopen("/Users/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"/Users/hpc-now/.terraform.d/plugins\"\n");
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }

    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/aliyun/alicloud/%s/darwin_amd64/",ali_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-alicloud_v%s",dirname_temp,ali_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (3/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/Users/hpc-now/.terraform.d/terraform-provider-alicloud_%s_darwin_amd64.zip",ali_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_ALI_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform-darwin/terraform-provider-alicloud_%s_darwin_amd64.zip %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-darwin/terraform-provider-alicloud_%s_darwin_amd64.zip -o %s",URL_REPO_ROOT,ali_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"unzip -q %s -d %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);
    }

    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/tencentcloudstack/tencentcloud/%s/darwin_amd64/",qcloud_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-tencentcloud_v%s",dirname_temp,qcloud_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (4/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/Users/hpc-now/.terraform.d/terraform-provider-tencentcloud_%s_darwin_amd64.zip",qcloud_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_QCLOUD_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform-darwin/terraform-provider-tencentcloud_%s_darwin_amd64.zip %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-darwin/terraform-provider-tencentcloud_%s_darwin_amd64.zip -o %s",URL_REPO_ROOT,qcloud_plugin_version,filename_temp);
            }
            flag=system(cmdline);
            if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"unzip -q %s -d %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);
    }

    sprintf(dirname_temp,"/Users/hpc-now/.terraform.d/plugins/registry.terraform.io/hashicorp/aws/%s/darwin_amd64/",aws_plugin_version);
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1", dirname_temp);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/terraform-provider-aws_v%s_x5",dirname_temp,aws_plugin_version);
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(filename_temp,md5sum);
    }
    if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF)!=0){
        printf("[ -INFO- ] Downloading/Copying and installing necessary tools (5/5) ...\n");
        printf("           Usually *ONLY* for the first time of running hpcopr.\n\n");
        sprintf(filename_temp,"/Users/hpc-now/.terraform.d/terraform-provider-aws_%s_x5_darwin_amd64.zip",aws_plugin_version);
        if(file_exist_or_not(filename_temp)==0){
            get_crypto_key(filename_temp,md5sum);
        }
        if(file_exist_or_not(filename_temp)!=0||strcmp(md5sum,MD5_AWS_TF_ZIP)!=0){
            if(REPO_LOC_FLAG==1){
                sprintf(cmdline,"/bin/cp %s/terraform-darwin/terraform-provider-aws_%s_darwin_amd64.zip %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            else{
                sprintf(cmdline,"curl %sterraform-darwin/terraform-provider-aws_%s_darwin_amd64.zip -o %s",URL_REPO_ROOT,aws_plugin_version,filename_temp);
            }
            flag=system(cmdline);
                if(flag!=0){
                printf("[ FATAL: ] Failed to download/copy necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now.\n");
                return 3;
            }
        }
        sprintf(cmdline,"unzip -q %s -d %s >> /dev/null 2>&1",filename_temp,dirname_temp);
        system(cmdline);
    }
    if(file_exist_or_not(usage_logfile)!=0){
        file_p=fopen(usage_logfile,"w+");
        fprintf(file_p,"UCID,CLOUD_VENDOR,NODE_NAME,vCPU,START_DATE,START_TIME,STOP_DATE,STOP_TIME,RUNNING_HOURS,CPUxHOURS,CPU_MODEL,CLOUD_REGION\n");
        fclose(file_p);
    }
    sprintf(filename_temp,"%s/known_hosts",sshkey_dir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
        system(cmdline);
    }
    if(file_exist_or_not(operation_logfile)!=0){
        sprintf(cmdline,"echo \"\" > %s",operation_logfile);
        system(cmdline);
    }
    if(system("cat /Users/hpc-now/.bashrc | grep PATH=/Users/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/Users/hpc-now/.bin/:$PATH >> /Users/hpc-now/.bashrc");
        system(cmdline);
    }

#endif
    printf("[ -INFO- ] Running environment successfully checked. HPC-NOW services are ready.\n");
    return 0;
}