/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "now_macros.h"
#include "general_funcs.h"
#include "general_print_info.h"
#include "components.h"
#include "cluster_general_funcs.h"
#include "userman.h"
#include "prereq_check.h"

extern char url_tf_root_var[LOCATION_LENGTH];
extern char url_now_crypto_var[LOCATION_LENGTH];
extern int tf_loc_flag_var;
extern int now_crypto_loc_flag_var;

extern char terraform_version_var[32];
extern char tofu_version_var[32];

extern char ali_tf_plugin_version_var[32];
extern char qcloud_tf_plugin_version_var[32];
extern char aws_tf_plugin_version_var[32];
extern char hw_tf_plugin_version_var[32];
extern char bd_tf_plugin_version_var[32];
extern char azrm_tf_plugin_version_var[32];
extern char azad_tf_plugin_version_var[32];
extern char gcp_tf_plugin_version_var[32];

extern char md5_tf_exec_var[64];
extern char md5_tf_zip_var[64];

extern char md5_tofu_exec_var[64]; //Added openTofu md5
extern char md5_tofu_zip_var[64];  //Added openTofu zip md5


extern char md5_now_crypto_var[64];
extern char md5_ali_tf_var[64];
extern char md5_ali_tf_zip_var[64];
extern char md5_qcloud_tf_var[64];
extern char md5_qcloud_tf_zip_var[64];
extern char md5_aws_tf_var[64];
extern char md5_aws_tf_zip_var[64];
extern char md5_hw_tf_var[64];
extern char md5_hw_tf_zip_var[64];
extern char md5_bd_tf_var[64];
extern char md5_bd_tf_zip_var[64];
extern char md5_azrm_tf_var[64];
extern char md5_azrm_tf_zip_var[64];
extern char md5_azad_tf_var[64];
extern char md5_azad_tf_zip_var[64];
extern char md5_gcp_tf_var[64];
extern char md5_gcp_tf_zip_var[64];

extern int batch_flag;
extern char final_command[512];
extern tf_exec_config tf_this_run;

extern char commands[COMMAND_NUM][COMMAND_STRING_LENGTH_MAX];

int check_internet(void){
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    sprintf(cmdline,"ping -n 1 www.baidu.com %s",SYSTEM_CMD_REDIRECT_NULL);
#else
    sprintf(cmdline,"ping -c 1 www.baidu.com %s",SYSTEM_CMD_REDIRECT_NULL);
#endif
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Internet connectivity check failed. Please either check your DNS\n");
        printf("|          service or check your internet connectivity and retry later.\n");
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
        return 1;
    }
    return 0;
}

/* 
 * Check whether the current device can connect to google 
 * This function is critical for GCP
 */
int check_internet_google(void){
    char cmdline[CMDLINE_LENGTH]="";
    char google_connectivity_flag[FILENAME_LENGTH];
    sprintf(google_connectivity_flag,"%s%sgoogle_check.dat",GENERAL_CONF_DIR,PATH_SLASH);
    FILE* file_p=fopen(google_connectivity_flag,"w+");
    if(file_p==NULL){
        return -1;
    }
#ifdef _WIN32
    sprintf(cmdline,"ping -n 1 api.google.com %s",SYSTEM_CMD_REDIRECT_NULL);
#else
    sprintf(cmdline,"ping -c 1 api.google.com %s",SYSTEM_CMD_REDIRECT_NULL);
#endif
    if(system(cmdline)!=0){
        fprintf(file_p,"api.google.com_connectivity_check: FAILED\n");
        fclose(file_p);
        return 1;
    }
    fprintf(file_p,"api.google.com_connectivity_check: SUCCEEDED\n");
    fclose(file_p);
    return 0;
}

int get_google_connectivity(void){
    char google_connectivity_flag[FILENAME_LENGTH]="";
    char line_buffer[256]="";
    char connectivity_status[16]="";
    sprintf(google_connectivity_flag,"%s%sgoogle_check.dat",GENERAL_CONF_DIR,PATH_SLASH);
    FILE* file_p=fopen(google_connectivity_flag,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,line_buffer);
    fclose(file_p);
    get_seq_string(line_buffer,' ',2,connectivity_status);
    if(strcmp(connectivity_status,"SUCCEEDED")==0){
        return 0;
    }
    return 1;
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
    int i,slash=0;
    if(system("whoami > c:\\programdata\\current_user.txt.tmp")!=0){
        return 1;
    }
    FILE* file_p_temp=fopen("c:\\programdata\\current_user.txt.tmp","r");
    fscanf(file_p_temp,"%128s",current_user_full);
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

int install_bucket_clis(int silent_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp_zip[FILENAME_LENGTH]="";
    int inst_flag=0;
    if(silent_flag!=0){
        printf(RESET_DISPLAY GENERAL_BOLD "|        . Checking & installing the dataman components: 1/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%sossutil64.exe",NOW_BINARY_DIR,PATH_SLASH);
    sprintf(filename_temp_zip,"%s%soss.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 1 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_OSSUTIL,filename_temp_zip);
#else
            sprintf(cmdline,"curl %s -o '%s'",URL_OSSUTIL,filename_temp_zip);
#endif
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 1/7." RESET_DISPLAY "\n");
                }
                inst_flag=1;
                goto coscli;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s",filename_temp_zip,NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"%s %s%sossutil-v1.7.16-windows-amd64%sossutil64.exe %s %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#elif __linux__   
        sprintf(cmdline,"unzip -q -o '%s' -d %s",filename_temp_zip,NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"%s %s%sossutil-v1.7.16-linux-amd64%sossutil64 %s %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s %s",filename_temp,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
#elif __APPLE__
        sprintf(cmdline,"unzip -q -o '%s' -d %s",filename_temp_zip,NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"%s %s%sossutil-v1.7.16-mac-amd64%sossutilmac64 %s %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s %s",filename_temp,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
#endif
        sprintf(cmdline,"%s %s%sossutil-v1.* %s",DELETE_FOLDER_CMD,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);  
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 1/7 .\n");
    }

coscli:
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 2/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%scoscli.exe",NOW_BINARY_DIR,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 2 not found. Downloading and installing ..." GREY_LIGHT "\n");
        sprintf(cmdline,"curl %s -o %s",URL_COSCLI,filename_temp);
        if(system(cmdline)!=0){
            if(silent_flag!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 2/7." RESET_DISPLAY "\n");
            }
            inst_flag=2;
            goto awscli;
        }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s %s",filename_temp,SYSTEM_CMD_REDIRECT_NULL);
        system(cmdline);
#endif
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 2/7 .\n");
    }

awscli: 
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 3/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%saws",NOW_BINARY_DIR,PATH_SLASH);
#ifdef __linux__
    sprintf(filename_temp_zip,"%s%sawscliv2.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 3 not found. Downloading and installing ..." GREY_LIGHT "\n");
        sprintf(cmdline,"%s %s%saws* %s",DELETE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        if(file_exist_or_not(filename_temp_zip)!=0){
            sprintf(cmdline,"curl %s -o '%s'",URL_AWSCLI,filename_temp_zip);
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(RESET_DISPLAY WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 3/7." RESET_DISPLAY "\n");
                }
                inst_flag=3;
                goto obsutil;
            }
        }
        sprintf(cmdline,"unzip -q -o '%s' -d /tmp",filename_temp_zip);
        system(cmdline);
        sprintf(cmdline,"/tmp/aws/install -i %s%sawscli -b %s %s",NOW_BINARY_DIR,PATH_SLASH,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        printf(RESET_DISPLAY);
    }
#elif __APPLE__
    sprintf(filename_temp_zip,"%s%sAWSCLIV2.pkg",TF_LOCAL_PLUGINS,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 3 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
            sprintf(cmdline,"curl %s -o '%s'",URL_AWSCLI,filename_temp_zip);
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(RESET_DISPLAY WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 3/7." RESET_DISPLAY "\n");
                }
                inst_flag=3;
                goto obsutil;
            }
        }
        if(system("/Applications/aws-cli/aws --version >> /dev/null 2>&1")!=0){
            FILE* file_p=fopen("/tmp/choices.xml","w+");
            if(file_p==NULL){
                if(silent_flag!=0){
                    printf(RESET_DISPLAY FATAL_RED_BOLD "[ FATAL: ] File I/O error. Failed to create tmp files." RESET_DISPLAY "\n");
                }
                inst_flag=3;
                goto obsutil;
            }
            fprintf(file_p,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            fprintf(file_p,"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
            fprintf(file_p,"<plist version=\"1.0\">\n");
            fprintf(file_p,"  <array>\n    <dict>\n      <key>choiceAttribute</key>\n      <string>customLocation</string>\n      <key>attributeSetting</key>\n");
            fprintf(file_p,"      <string>/Applications/</string>\n      <key>choiceIdentifier</key>\n      <string>default</string>\n");
            fprintf(file_p,"    </dict>\n  </array>\n</plist>\n");
            fclose(file_p);
            sprintf(cmdline,"installer -pkg '%s' -target CurrentUserHomeDirectory -applyChoiceChangesXML /tmp/choices.xml %s &",filename_temp_zip,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            int i=0;
            while(file_exist_or_not("/Applications/aws-cli/aws")!=0||file_exist_or_not("/Applications/aws-cli/aws_completer")!=0){
                printf(RESET_DISPLAY GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " Installing additional component, %d sec(s) of max 120s passed ... \r",i);
                fflush(stdout);
                i++;
                sleep(1);
                if(i==120){
                    printf(RESET_DISPLAY WARN_YELLO_BOLD "[ -WARN- ] Failed to install component. HPC-NOW dataman services may not work properly.");
                    inst_flag=3;
                    goto obsutil;
                }
            }
            printf("\n");
        }
        sprintf(cmdline,"/bin/cp -r /Applications/aws-cli %s",NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"ln -s %s%saws-cli%saws %s%saws %s",NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"ln -s %s%saws-cli%saws_completer %s%saws_completer %s",NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        printf(RESET_DISPLAY);
    }
#elif _WIN32
    if(file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws.exe")!=0||file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws_completer.exe")!=0){
        if(silent_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please run the installer update to fix this issue." RESET_DISPLAY "\n");
        }
        inst_flag=3;
        goto obsutil;
    }
#endif
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 3/7 .\n");
    }

obsutil:
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 4/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%sobsutil.exe",NOW_BINARY_DIR,PATH_SLASH);
#ifdef _WIN32
    sprintf(filename_temp_zip,"%s%sobsutil_amd64.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#else
    sprintf(filename_temp_zip,"%s%sobsutil_amd64.tar.gz",TF_LOCAL_PLUGINS,PATH_SLASH);
#endif
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 4 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_OBSUTIL,filename_temp_zip);
#else
            sprintf(cmdline,"curl %s -o '%s'",URL_OBSUTIL,filename_temp_zip);
#endif
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 4/7." RESET_DISPLAY "\n");
                }
                inst_flag=4;
                goto bcecmd;
            }
        }
#ifdef __APPLE__
        sprintf(cmdline,"tar zxf '%s' -C %s",filename_temp_zip,NOW_BINARY_DIR);
#else
        sprintf(cmdline,"tar zxf %s -C %s",filename_temp_zip,NOW_BINARY_DIR);
#endif
        system(cmdline);
#ifndef _WIN32
        sprintf(cmdline,"chmod -R 711 %s%sobsutil_* %s",NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#endif
        sprintf(cmdline,"%s %s%sobsutil_%s_amd64_* %s%sobsutil %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,FILENAME_SUFFIX_FULL,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#ifdef _WIN32
        sprintf(cmdline,"%s %s%sobsutil%sobsutil.exe %s%sobsutil.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
#else
        sprintf(cmdline,"%s %s%sobsutil%sobsutil %s%sobsutil.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
#endif
        system(cmdline);
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 4/7 .\n");
    }
bcecmd:
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 5/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%sbcecmd.exe",NOW_BINARY_DIR,PATH_SLASH);
#ifdef _WIN32
    sprintf(filename_temp_zip,"%s%swindows-bcecmd-0.4.1.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __linux__
    sprintf(filename_temp_zip,"%s%slinux-bcecmd-0.4.1.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __APPLE__
    sprintf(filename_temp_zip,"%s%smac-bcecmd-0.4.1.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#endif
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 5 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_BCECMD,filename_temp_zip);
#else
            sprintf(cmdline,"curl %s -o '%s'",URL_BCECMD,filename_temp_zip);
#endif
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 5/7." RESET_DISPLAY "\n");
                }
                inst_flag=5;
                goto azcopy;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s",filename_temp_zip,NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"%s %s%swindows-bcecmd-0.4.1%sbcecmd.exe %s%sbcecmd.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#elif __linux__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%slinux-bcecmd-0.4.1%sbcecmd %s%sbcecmd.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s",filename_temp);
        system(cmdline);
#elif __APPLE__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%smac-bcecmd-0.4.1%sbcecmd %s%sbcecmd.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s",filename_temp);
        system(cmdline);
#endif
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 5/7 .\n");
    }
azcopy:
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 6/7 ..." RESET_DISPLAY "\n");
    }
    sprintf(filename_temp,"%s%sazcopy.exe",NOW_BINARY_DIR,PATH_SLASH);
#ifdef _WIN32
    sprintf(filename_temp_zip,"%s%sazcopy_windows_amd64_10.20.1.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __linux__
    sprintf(filename_temp_zip,"%s%sazcopy_linux_amd64_10.20.1.tar.gz",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __APPLE__
    sprintf(filename_temp_zip,"%s%sazcopy_darwin_amd64_10.20.1.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#endif
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 6 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_AZCOPY,filename_temp_zip);
#else
            sprintf(cmdline,"curl %s -o '%s'",URL_AZCOPY,filename_temp_zip);
#endif
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 6/7." RESET_DISPLAY "\n");
                }
                inst_flag=6;
                goto gcloud_cli;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s",filename_temp_zip,NOW_BINARY_DIR);
        system(cmdline);
        sprintf(cmdline,"%s %s%sazcopy_windows_amd64_10.20.1%sazcopy.exe %s%sazcopy.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#elif __linux__
        sprintf(cmdline,"tar zxf '%s' -C %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%sazcopy_linux_amd64_10.20.1%sazcopy %s%sazcopy.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s",filename_temp);
        system(cmdline);
#elif __APPLE__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%sazcopy_darwin_amd64_10.20.1%sazcopy %s%sazcopy.exe %s",MOVE_FILE_CMD,NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,NOW_BINARY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"chmod +x %s",filename_temp);
        system(cmdline);
#endif
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 6/7 .\n");
    }
gcloud_cli:
    if(silent_flag!=0){
        printf(GENERAL_BOLD "|        . Checking & installing the dataman components: 7/7 ..." RESET_DISPLAY "\n");
    }
    if(get_google_connectivity()!=0){
        if(silent_flag!=0){
            printf(WARN_YELLO_BOLD "|        x Failed to connect to api.google.com. Skip installing the gcp component." RESET_DISPLAY "\n");
        }
        goto end_return;
    }
    sprintf(filename_temp,"%s%sgoogle-cloud-sdk%sbin%sgcloud",NOW_BINARY_DIR,PATH_SLASH,PATH_SLASH,PATH_SLASH);
#ifdef _WIN32
    sprintf(filename_temp_zip,"%s%sgoogle-cloud-sdk-449.0.0-windows-x86_64-bundled-python.zip",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __linux__
    sprintf(filename_temp_zip,"%s%sgoogle-cloud-cli-449.0.0-linux-x86_64.tar.gz",TF_LOCAL_PLUGINS,PATH_SLASH);
#elif __APPLE__
    sprintf(filename_temp_zip,"%s%sgoogle-cloud-cli-449.0.0-darwin-x86_64.tar.gz",TF_LOCAL_PLUGINS,PATH_SLASH);
#endif
    if(file_exist_or_not(filename_temp)!=0){
        printf("|          Dataman component 7 not found. Downloading and installing ..." GREY_LIGHT "\n");
        if(file_exist_or_not(filename_temp_zip)!=0){
#ifdef _WIN32
            sprintf(cmdline,"curl %s -o %s",URL_GCLOUD,filename_temp_zip);
#else
            sprintf(cmdline,"curl %s -o '%s'",URL_GCLOUD,filename_temp_zip);
#endif
            if(system(cmdline)!=0){
                if(silent_flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download dataman component 7/7." RESET_DISPLAY "\n");
                }
                inst_flag=7;
                goto end_return;
            }
        }
#ifdef _WIN32
        sprintf(cmdline,"tar xf %s -C %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#else
        sprintf(cmdline,"tar zxf '%s' -C %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
#endif
    }
    if(silent_flag!=0){
        printf(RESET_DISPLAY "|        v Installed the dataman components: 7/7 .\n");
    }
end_return:
    return inst_flag;
}

int repair_provider(char* plugin_root_path, char* cloud_name, char* provider_version, char* md5_exec, char* md5_zip, int force_repair_flag, char* seq_code){
    if(valid_md5_or_not(md5_exec)!=0||valid_md5_or_not(md5_zip)!=0){
        return 1;
    }

    char provider_zip[FILENAME_LENGTH]="";
    char provider_dir_tf[DIR_LENGTH_EXT]="";
    char provider_dir_tofu[DIR_LENGTH_EXT]="";
    char provider_exec_tf[FILENAME_LENGTH]="";
    char provider_exec_tofu[FILENAME_LENGTH]="";
    char provider_prefix[32]="";
    char provider_exec_suffix[8]="";
    
    int file_check_flag;
    int file_check_flag_tf;
    int file_check_flag_tofu;
    int run_flag;
    char cmdline[CMDLINE_LENGTH]="";

    if(strcmp(cloud_name,"alicloud")==0){
        strcpy(provider_prefix,"aliyun");
#ifdef _WIN32
        strcpy(provider_exec_suffix,".exe");
#endif
    }
    else if(strcmp(cloud_name,"tencentcloud")==0){
        strcpy(provider_prefix,"tencentcloudstack");
#ifdef _WIN32
        strcpy(provider_exec_suffix,".exe");
#endif
    }
    else if(strcmp(cloud_name,"aws")==0||strcmp(cloud_name,"azuread")==0||strcmp(cloud_name,"azurerm")==0||strcmp(cloud_name,"google")==0){
        strcpy(provider_prefix,"hashicorp");
#ifdef _WIN32
        strcpy(provider_exec_suffix,"_x5.exe");
#else
        strcpy(provider_exec_suffix,"_x5");
#endif
    }
    else if(strcmp(cloud_name,"baiducloud")==0){
        strcpy(provider_prefix,"baiubce");
#ifdef _WIN32
        strcpy(provider_exec_suffix,".exe");
#endif
    }
    else if(strcmp(cloud_name,"huaweicloud")==0){
        strcpy(provider_prefix,"huaweicloud");
#ifdef _WIN32
        strcpy(provider_exec_suffix,".exe");
#endif
    }
    else{
        return 1;
    }

    sprintf(provider_zip,"%s%sterraform-provider-%s_%s_%s_amd64.zip",TF_LOCAL_PLUGINS,PATH_SLASH,cloud_name,provider_version,FILENAME_SUFFIX_FULL);
    sprintf(provider_dir_tf,"%s%sregistry.terraform.io%s%s%s%s%s%s%s%s_amd64%s",plugin_root_path,PATH_SLASH,PATH_SLASH,provider_prefix,PATH_SLASH,cloud_name,PATH_SLASH,provider_version,PATH_SLASH,FILENAME_SUFFIX_FULL,PATH_SLASH);
    sprintf(provider_dir_tofu,"%s%sregistry.opentofu.org%s%s%s%s%s%s%s%s_amd64%s",plugin_root_path,PATH_SLASH,PATH_SLASH,provider_prefix,PATH_SLASH,cloud_name,PATH_SLASH,provider_version,PATH_SLASH,FILENAME_SUFFIX_FULL,PATH_SLASH);
    sprintf(provider_exec_tf,"%s%sterraform-provider-%s_v%s%s",provider_dir_tf,PATH_SLASH,cloud_name,provider_version,provider_exec_suffix);
    sprintf(provider_exec_tofu,"%s%sterraform-provider-%s_v%s%s",provider_dir_tofu,PATH_SLASH,cloud_name,provider_version,provider_exec_suffix);

    if(folder_exist_or_not(provider_dir_tf)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,provider_dir_tf,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(provider_dir_tofu)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,provider_dir_tofu,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    file_check_flag_tf=file_validity_check(provider_exec_tf,force_repair_flag,md5_exec);
    file_check_flag_tofu=file_validity_check(provider_exec_tofu,force_repair_flag,md5_exec);
    if(file_check_flag_tf==1||file_check_flag_tofu==1){
        printf(RESET_DISPLAY GENERAL_BOLD "[ -INFO- ] Downloading/Copying the cloud provider for %s (%s) ...\n",cloud_name,seq_code);
        printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode." RESET_DISPLAY "\n" GREY_LIGHT "\n");
        file_check_flag=file_validity_check(provider_zip,force_repair_flag,md5_zip);
        if(file_check_flag==1){
            if(tf_loc_flag_var==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform-provider-%s_%s_windows_amd64.zip %s",url_tf_root_var,cloud_name,provider_version,provider_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform-provider-%s_%s_linux_amd64.zip '%s'",url_tf_root_var,cloud_name,provider_version,provider_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform-provider-%s_%s_darwin_amd64.zip '%s'",url_tf_root_var,cloud_name,provider_version,provider_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform-provider-%s_%s_windows_amd64.zip -o %s",url_tf_root_var,cloud_name,provider_version,provider_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform-provider-%s_%s_linux_amd64.zip -o '%s'",url_tf_root_var,cloud_name,provider_version,provider_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform-provider-%s_%s_darwin_amd64.zip -o '%s'",url_tf_root_var,cloud_name,provider_version,provider_zip);
#endif
            }
            run_flag=system(cmdline);
            if(run_flag!=0){
                printf(RESET_DISPLAY FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy or install the provider." RESET_DISPLAY "\n");
                return 3;
            }
        }
        if(file_check_flag_tf==1){
#ifdef _WIN32
            sprintf(cmdline,"tar zxf %s -C %s %s",provider_zip,provider_dir_tf,SYSTEM_CMD_REDIRECT);
#else
            sprintf(cmdline,"unzip -o -q '%s' -d '%s' %s",provider_zip,provider_dir_tf,SYSTEM_CMD_REDIRECT);
#endif
            run_flag=system(cmdline);
            if(run_flag!=0){
                printf(RESET_DISPLAY FATAL_RED_BOLD "[ FATAL: ] Failed to unzip the provider file. Exit now." RESET_DISPLAY "\n");
                return 3;
            }
        }
        if(file_check_flag_tofu==1){
#ifdef _WIN32
            sprintf(cmdline,"tar zxf %s -C %s %s",provider_zip,provider_dir_tofu,SYSTEM_CMD_REDIRECT);
#else
            sprintf(cmdline,"unzip -o -q '%s' -d '%s' %s",provider_zip,provider_dir_tofu,SYSTEM_CMD_REDIRECT);
#endif
            run_flag=system(cmdline);
            if(run_flag!=0){
                printf(RESET_DISPLAY FATAL_RED_BOLD "[ FATAL: ] Failed to unzip the provider file. Exit now." RESET_DISPLAY "\n");
                return 3;
            }
        }
    }
    return 0;
}

int check_and_install_prerequisitions(int repair_flag){
    char cmdline[CMDLINE_LENGTH]="";

    char filename_temp[FILENAME_LENGTH]="";
    char dirname_temp[DIR_LENGTH]="";
    char filename_temp_zip[FILENAME_LENGTH]="";

    int flag=0;
    int gcp_flag=0;
    int file_check_flag=0;
    int force_repair_flag;

    FILE* file_p=NULL;
    char plugin_dir_root[DIR_LENGTH]="";

#ifdef _WIN32
    char appdata_dir[128]="";
    char home_path[64]="";
    char dotssh_dir[128]="";
    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%128s",appdata_dir);
    fclose(file_p);
    sprintf(cmdline,"del /f /s /q c:\\programdata\\appdata.txt.tmp %s",SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_seq_string(appdata_dir,'\\',3,home_path);
    sprintf(dotssh_dir,"c:\\users\\%s\\.ssh",home_path);
    if(folder_exist_or_not(TF_LOCAL_PLUGINS)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,TF_LOCAL_PLUGINS,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
#endif

    if(file_exist_or_not(USAGE_LOG_FILE)!=0){
        force_repair_flag=1;
    }
    else{
        if(repair_flag==1){
            force_repair_flag=repair_flag;
        }
        else{
            force_repair_flag=0;
        }
    }

    sprintf(cmdline,"%s %s%sworkdir %s",MKDIR_CMD,HPC_NOW_ROOT_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);

    if(repair_flag==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Start checking and repairing the HPC-NOW services now ... \n");
        printf("|        . Checking and repairing the registry now ...\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking the environment for HPC-NOW services ...\n");
    }

    sprintf(filename_temp,"%s%sgoogle_check.dat",GENERAL_CONF_DIR,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0||repair_flag==1||repair_flag==2){
        printf("|        . Checking whether Google Cloud Platform (GCP) is accessible ...\n");
        check_internet_google();
    }
    gcp_flag=get_google_connectivity();
    if(gcp_flag==1){
        if(repair_flag==1){
            printf(WARN_YELLO_BOLD "|        x Failed to call GCP's API. GCP is unavailable currently." RESET_DISPLAY "\n");
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to call GCP's API. GCP is unavailable currently." RESET_DISPLAY "\n");
        }
    }
    else if(gcp_flag==-1){
        if(repair_flag==1){
            printf(WARN_YELLO_BOLD "|        x Internal error (GCP connectivity status is absent)." RESET_DISPLAY "\n");
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Internal error (GCP connectivity status is absent)." RESET_DISPLAY "\n");
        }
    }
    if(repair_flag==1){
        printf("|        . Checking and repairing the registry now ...\n");
    }
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " No registry file found. Creating a blank cluster registry now ...\n");
        file_p=fopen(ALL_CLUSTER_REGISTRY,"w+");
        if(file_p==NULL){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry. Exit now." RESET_DISPLAY "\n");
            return -1;
        }
        fclose(file_p);
    }
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The registry has been repaired.\n");
        printf("|        . Checking and repairing the location configuration file now ...\n");
        if(reset_locations()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to reset the locations for binaries and templates. Exit now." RESET_DISPLAY "\n");
            return -3;
        }
        printf( RESET_DISPLAY "|        v All the locations has been reset to the default ones.\n");
    }
    flag=get_locations();
    if(flag!=0){
        if(flag==-1){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Location configuration file not found.\n");
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Location configuration format incorrect.\n");
        }
        if(batch_flag!=0&&prompt_to_confirm("Use the default locations?",CONFIRM_STRING,batch_flag)==1){
            if(prompt_to_confirm("Configure the locations now?",CONFIRM_STRING,1)==1){
                printf(FATAL_RED_BOLD "[ FATAL: ] Prerequisites check abort." RESET_DISPLAY "\n");
                return 1;
            }
            if(configure_locations(0)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to configure the locations. Exit now." RESET_DISPLAY "\n");
                return 5;
            }
        }
        else{
            if(reset_locations()!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Fatal error (FILE I/O). Please submit issues to the source repository." RESET_DISPLAY "\n");
                return 5;
            }
        }
        if(get_locations()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Fatal error. Please submit issues to the source repository." RESET_DISPLAY "\n");
            return 5;
        }
    }

    if(repair_flag==1){
        printf( RESET_DISPLAY "|        v Location configuration has been repaired.\n");
        printf("|        . Checking and repairing the versions and md5sums ...\n");
        if(reset_vers_md5_vars()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to reset the versions and md5sums. Exit now." RESET_DISPLAY "\n");
            return 7;
        }
        printf( RESET_DISPLAY "|        v Versions and md5sums been repaired.\n");
        printf("|        . Setting TF running configurations ...\n");
        if(reset_tf_running()!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to create tf_running_config file." RESET_DISPLAY "\n");
        }
        printf("|        . Checking and repairing the key directories and files ...\n");
    }
    flag=get_vers_md5_vars();
    if(flag!=0){
        if(flag==-1){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Versions and md5sums not found. Trying to fix ...\n");
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Versions and md5sums format incorrect. Trying to fix ...\n");
        }
        if(reset_vers_md5_vars()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to reset the versions and md5sums. Exit now." RESET_DISPLAY "\n");
            return 7;
        }
        if(get_vers_md5_vars()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to configure versions and md5sums of core components.\n");
            printf("|          Please check the format of md5 files. Exit now." RESET_DISPLAY "\n");
            return 7;
        }
    }
    if(folder_exist_or_not(DESTROYED_DIR)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,DESTROYED_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    else{
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(folder_exist_or_not(NOW_BINARY_DIR)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(dirname_temp,"%s\\terraform.d\\",appdata_dir);
    sprintf(filename_temp_zip,"%s\\terraform_%s_windows_amd64.zip",TF_LOCAL_PLUGINS,terraform_version_var);
#elif __linux__
    strcpy(dirname_temp,TF_LOCAL_PLUGINS);
    sprintf(filename_temp_zip,"%sterraform_%s_linux_amd64.zip",dirname_temp,terraform_version_var);
#elif __APPLE__
    strcpy(dirname_temp,TF_LOCAL_PLUGINS);
    sprintf(filename_temp_zip,"%sterraform_%s_darwin_amd64.zip",dirname_temp,terraform_version_var);
#endif
    if(folder_exist_or_not(dirname_temp)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    file_check_flag=file_validity_check(TERRAFORM_EXEC,force_repair_flag,md5_tf_exec_var);
    if(file_check_flag==1){
        file_check_flag=file_validity_check(filename_temp_zip,force_repair_flag,md5_tf_zip_var);
        if(file_check_flag==1){
            printf(GENERAL_BOLD "[ -INFO- ] Downloading/Copying the Terraform binary ...\n");
            printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode." RESET_DISPLAY "\n" GREY_LIGHT "\n");
            if(tf_loc_flag_var==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\terraform_%s_windows_amd64.zip %s",url_tf_root_var,terraform_version_var,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/terraform_%s_linux_amd64.zip '%s'",url_tf_root_var,terraform_version_var,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/terraform_%s_darwin_amd64.zip '%s'",url_tf_root_var,terraform_version_var,filename_temp_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/terraform_%s_windows_amd64.zip -o %s",url_tf_root_var,terraform_version_var,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/terraform_%s_linux_amd64.zip -o '%s'",url_tf_root_var,terraform_version_var,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/terraform_%s_darwin_amd64.zip -o '%s'",url_tf_root_var,terraform_version_var,filename_temp_zip);
#endif
            }
            flag=system(cmdline);
            if(flag!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now." RESET_DISPLAY "\n");
                return 3;
            }
        }
//        printf("%s,,,,,\"\n",cmdline);
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#elif __linux__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#elif __APPLE__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#endif
//        printf("%s,,,,,\"\n",cmdline);
        flag=system(cmdline);
        if(flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to unzip the terraform binary file. Exit now." RESET_DISPLAY "\n");
            return 3;
        }        
    }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s",TERRAFORM_EXEC);
        system(cmdline);
#endif
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The Terraform executable has been repaired.\n");
    }

#ifdef _WIN32
    sprintf(filename_temp_zip,"%s\\tofu_%s_windows_amd64.zip",TF_LOCAL_PLUGINS,tofu_version_var);
#elif __linux__
    sprintf(filename_temp_zip,"%stofu_%s_linux_amd64.zip",TF_LOCAL_PLUGINS,tofu_version_var);
#elif __APPLE__
    sprintf(filename_temp_zip,"%stofu_%s_darwin_amd64.zip",TF_LOCAL_PLUGINS,tofu_version_var);
#endif
    file_check_flag=file_validity_check(TOFU_EXEC,force_repair_flag,md5_tofu_exec_var);
    if(file_check_flag==1){
        file_check_flag=file_validity_check(filename_temp_zip,force_repair_flag,md5_tofu_zip_var);
        if(file_check_flag==1){
            printf(RESET_DISPLAY GENERAL_BOLD "[ -INFO- ] Downloading/Copying the openTofu binary ...\n");
            printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode." RESET_DISPLAY "\n" GREY_LIGHT "\n");
            if(tf_loc_flag_var==1){
#ifdef _WIN32
                sprintf(cmdline,"copy /y %s\\tf-win\\tofu_%s_windows_amd64.zip %s",url_tf_root_var,tofu_version_var,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"/bin/cp %s/tf-linux/tofu_%s_linux_amd64.zip '%s'",url_tf_root_var,tofu_version_var,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"/bin/cp %s/tf-darwin/tofu_%s_darwin_amd64.zip '%s'",url_tf_root_var,tofu_version_var,filename_temp_zip);
#endif
            }
            else{
#ifdef _WIN32
                sprintf(cmdline,"curl %stf-win/tofu_%s_windows_amd64.zip -o %s",url_tf_root_var,tofu_version_var,filename_temp_zip);
#elif __linux__
                sprintf(cmdline,"curl %stf-linux/tofu_%s_linux_amd64.zip -o '%s'",url_tf_root_var,tofu_version_var,filename_temp_zip);
#elif __APPLE__
                sprintf(cmdline,"curl %stf-darwin/tofu_%s_darwin_amd64.zip -o '%s'",url_tf_root_var,tofu_version_var,filename_temp_zip);
#endif
            }
            flag=system(cmdline);
            if(flag!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
                printf("|          info@hpc-now.com for support. Exit now." RESET_DISPLAY "\n");
                return 3;
            }
        }
//        printf("%s,,,,,\"\n",cmdline);
#ifdef _WIN32
        sprintf(cmdline,"tar zxf %s -C %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#elif __linux__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#elif __APPLE__
        sprintf(cmdline,"unzip -o -q '%s' -d %s %s",filename_temp_zip,NOW_BINARY_DIR,SYSTEM_CMD_REDIRECT);
#endif
//        printf("%s,,,,,\"\n",cmdline);
        flag=system(cmdline);
        if(flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to unzip the openTofu binary file. Exit now." RESET_DISPLAY "\n");
            return 3;
        }        
    }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s",TOFU_EXEC);
        system(cmdline);
#endif
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The openTofu executable has been repaired.\n");
    }

    file_check_flag=file_validity_check(NOW_CRYPTO_EXEC,repair_flag,md5_now_crypto_var);
    if(file_check_flag==1){
        printf(GENERAL_BOLD "[ -INFO- ] Downloading/Copying the now-crypto.exe ...\n");
        printf("|          Usually *ONLY* for the first time of running hpcopr or repair mode." RESET_DISPLAY "\n" GREY_LIGHT "\n");
        if(now_crypto_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\now-crypto-win.exe %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#elif __linux__
            sprintf(cmdline,"/bin/cp %s/now-crypto-lin.exe %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#elif __APPLE__
            sprintf(cmdline,"/bin/cp %s/now-crypto-dwn.exe %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %snow-crypto-win.exe -o %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#elif __linux__
            sprintf(cmdline,"curl %snow-crypto-lin.exe -o %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#elif __APPLE__
            sprintf(cmdline,"curl %snow-crypto-dwn.exe -o %s",url_now_crypto_var,NOW_CRYPTO_EXEC);
#endif
        }
//        printf("%s,,,,,\"\n",cmdline);
        flag=system(cmdline);
        if(flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy or install necessary tools. Please contact\n");
            printf("|          info@hpc-now.com for support. Exit now." RESET_DISPLAY "\n");
            return 3;
        }
    }
#ifndef _WIN32
        sprintf(cmdline,"chmod +x %s",NOW_CRYPTO_EXEC);
        system(cmdline);
#endif
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The now-crypto executable has been repaired.\n");
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\.terraformrc",appdata_dir);
    if(file_exist_or_not(filename_temp)!=0){
        file_p=fopen(filename_temp,"w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"%s\\.terraform.d/plugins\"\n",appdata_dir);
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\",\"registry.opentofu.org/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }
#elif __linux__
    if(file_exist_or_not("/home/hpc-now/.terraformrc")!=0){
        file_p=fopen("/home/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"%splugins\"\n",TF_LOCAL_PLUGINS);
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\",\"registry.opentofu.org/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }
#elif __APPLE__
    if(file_exist_or_not("/Users/hpc-now/.terraformrc")!=0){
        file_p=fopen("/Users/hpc-now/.terraformrc","w+");
        fprintf(file_p,"privider_installation {\n");
        fprintf(file_p,"  filesystem_mirror {\n");
        fprintf(file_p,"    path    = \"%splugins\"\n",TF_LOCAL_PLUGINS);
        fprintf(file_p,"    include = [\"registry.terraform.io/*/*\",\"registry.opentofu.org/*/*\"]\n");
        fprintf(file_p,"  }\n}\n");
        fclose(file_p);
    }
#endif
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The terraformrc file has been repaired.\n");
        printf("|        . Checking and repairing the TF Providers ... \n");
    }
#ifdef _WIN32
    sprintf(plugin_dir_root,"%s\\terraform.d\\plugins\\",appdata_dir);
#else
    sprintf(plugin_dir_root,"%s/plugins/",TF_LOCAL_PLUGINS);
#endif
    if(repair_provider(plugin_dir_root,"alicloud",ali_tf_plugin_version_var,md5_ali_tf_var,md5_ali_tf_zip_var,force_repair_flag,"1/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"tencentcloud",qcloud_tf_plugin_version_var,md5_qcloud_tf_var,md5_qcloud_tf_zip_var,force_repair_flag,"2/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"aws",aws_tf_plugin_version_var,md5_aws_tf_var,md5_aws_tf_zip_var,force_repair_flag,"3/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"huaweicloud",hw_tf_plugin_version_var,md5_hw_tf_var,md5_hw_tf_zip_var,force_repair_flag,"4/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"baiducloud",bd_tf_plugin_version_var,md5_bd_tf_var,md5_bd_tf_zip_var,force_repair_flag,"5/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"azuread",azad_tf_plugin_version_var,md5_azad_tf_var,md5_azad_tf_zip_var,force_repair_flag,"6a/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"azurerm",azrm_tf_plugin_version_var,md5_azrm_tf_var,md5_azrm_tf_zip_var,force_repair_flag,"6b/7")!=0){
        return 3;
    }
    if(repair_provider(plugin_dir_root,"google",gcp_tf_plugin_version_var,md5_gcp_tf_var,md5_gcp_tf_zip_var,force_repair_flag,"7/7")!=0){
        return 3;
    }
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v The Terraform Providers have been repaired.\n");
    }
    printf(RESET_DISPLAY);
    flag=install_bucket_clis(force_repair_flag);
    if(flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] IMPORTANT! The dataman component %d may not work properly." RESET_DISPLAY "\n",flag);
    }
    if(folder_exist_or_not(SSHKEY_DIR)!=0){
        sprintf(cmdline,"%s \"%s\" %s",MKDIR_CMD,SSHKEY_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(file_exist_or_not(USAGE_LOG_FILE)!=0){
        file_p=fopen(USAGE_LOG_FILE,"w+");
        fprintf(file_p,"UCID,CLOUD_VENDOR,NODE_NAME,vCPU,START_DATE,START_TIME,STOP_DATE,STOP_TIME,RUNNING_HOURS,CPUxHOURS,CPU_MODEL,CLOUD_REGION\n");
        fclose(file_p);
        file_p=fopen(OPERATION_LOG_FILE,"w+");
        fclose(file_p);
    }
    if(repair_flag==1){
        printf("|        . Checking and repairing the key folders and environment variables ... \n");
    }
#ifdef _WIN32
    sprintf(filename_temp,"c:\\users\\%s\\.cos.yaml",home_path);
    if(file_exist_or_not(filename_temp)!=0){
        sprintf(cmdline,"type nul > %s",filename_temp);
        system(cmdline);
    }
    sprintf(cmdline,"takeown /f %s /r /d y %s",SSHKEY_DIR,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\known_hosts* >nul 2>&1",dotssh_dir);
#elif __linux__
    if(file_exist_or_not("/home/hpc-now/.cos.yaml")!=0){
        system("echo \"\" > /home/hpc-now/.cos.yaml");
    }
    if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/home/hpc-now/.bin/:$PATH >> /home/hpc-now/.bashrc");
        system(cmdline);
    }
    sprintf(cmdline,"rm -rf /home/hpc-now/.ssh/known_hosts %s",SYSTEM_CMD_REDIRECT);
#elif __APPLE__
    if(file_exist_or_not("/Users/hpc-now/.cos.yaml")!=0){
        system("echo \"\" > /Users/hpc-now/.cos.yaml");
    }
    if(system("cat /Users/hpc-now/.bashrc | grep PATH=/Users/hpc-now/.bin/ > /dev/null 2>&1")!=0){
        strcpy(cmdline,"export PATH=/Users/hpc-now/.bin/:$PATH >> /Users/hpc-now/.bashrc");
        system(cmdline);
    }
    sprintf(cmdline,"rm -rf /Users/hpc-now/.ssh/known_hosts %s",SYSTEM_CMD_REDIRECT);
#endif
    system(cmdline);
    sprintf(dirname_temp,"%s%s.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,dirname_temp,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    
    if(repair_flag==1){
        printf(RESET_DISPLAY "|        v Environment variables have been repaired.\n");
        printf("|        v SSH files have been repaired. \n");
        if(gcp_flag==0){
            printf(HIGH_GREEN_BOLD "[ -INFO- ] Running environment successfully checked and repaired." RESET_DISPLAY "\n");
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Running environment checked and repaired with a warning.\n");
        }
    }
    else{
        if(gcp_flag==0){
            printf(RESET_DISPLAY HIGH_GREEN_BOLD "[ -INFO- ] Running environment successfully checked." RESET_DISPLAY "\n");
        }
        else{
            printf(RESET_DISPLAY GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Running environment checked with a warning.\n");
        }
    }
    return 0;
}

int command_name_check(char* command_name_input, char* command_prompt, char* role_flag, char* cu_flag){
    int i;
    int j;
    int diff_current=0;
    int diff_prev=1024;
    int equal_flag;
    int equal_flag_prev=0;
    int compare_length=0;
    char command_temp[32]="";
    int closest=0;
    for(i=0;i<COMMAND_NUM;i++){
        get_seq_string(commands[i],',',1,command_temp);
        if(strcmp(command_name_input,command_temp)==0){
            get_seq_string(commands[i],',',2,role_flag);
            get_seq_string(commands[i],',',3,cu_flag);
            return 0;
        }
        diff_current=0;
        equal_flag=0;
        if(strlen(command_temp)<strlen(command_name_input)){
            compare_length=strlen(command_temp);
        }
        else{
            compare_length=strlen(command_name_input);
        }
        for(j=0;j<compare_length-1;j++){
            if(*(command_name_input+j)==*(command_temp+j)&&*(command_name_input+j+1)==*(command_temp+j+1)){
                equal_flag++;
            }
            diff_current+=abs(*(command_name_input+j)-*(command_temp+j));  
        }
        if(*(command_name_input+j+1)==*(command_temp+j+1)){
            equal_flag++;
        }
//        printf("%s,%d,%d,%d,%d\n",commands[i],equal_flag,closest,diff_current,diff_prev);
        if(equal_flag>equal_flag_prev&&diff_current<diff_prev){
            closest=i;
            equal_flag_prev=equal_flag;
            diff_prev=diff_current;
            continue;
        }
    }
    get_seq_string(commands[closest],',',1,command_temp);
    strcpy(role_flag,"");
    strcpy(cu_flag,"");
    strcpy(command_prompt,command_temp);
    return 200+closest;
}

int command_parser(int argc, char** argv, char* command_name_prompt, char* workdir, char* cluster_name, char* user_name, char* cluster_role){
    int command_flag=0;
    int max_time_temp=0;
    char temp_cluster_name_specified[128]="";
    int flag1,flag2;
    char temp_cluster_name_switched[128]="";
    char temp_cluster_name[128]="";
    char temp_workdir[DIR_LENGTH]="";
    char string_temp[256]="";
    char cluster_name_source[16]="";

    if(argc<2){
        list_all_commands();
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input a " HIGH_GREEN_BOLD "command" RESET_DISPLAY " : " HIGH_GREEN_BOLD);
        fflush(stdin);
        scanf("%512s",final_command);
        getchar();
        printf(RESET_DISPLAY);
    }
    else if(argc==2){
        if(strcmp(argv[1],"-b")==0){
            strcpy(final_command,"");
            return -1;
        }
        else{
            strncpy(final_command,argv[1],512);
        }
    }
    else{
        if(strcmp(argv[1],"-b")==0){
            batch_flag=0;
            strncpy(final_command,argv[2],512);
        }
        else{
            if(cmd_flag_check(argc,argv,"-b")==0){
                batch_flag=0;
            }
            strncpy(final_command,argv[1],512);
        }
    }

    char role_flag[16]="";
    char cluster_role_ext[32]="";
    char cu_flag[16]="";
    int tf_local_config_flag=127; //will be reset to -1~3 after get_tf_running_config()function
    char filename_temp[FILENAME_LENGTH]="";

    command_flag=command_name_check(final_command,command_name_prompt,role_flag,cu_flag);
    if(command_flag!=0){
        return command_flag;
    }
    if(strcmp(cu_flag,"UNAME")==0||strcmp(cu_flag,"CNAME")==0){
        flag1=cmd_keyword_check(argc,argv,"-c",temp_cluster_name_specified);
        if(flag1==0){
            strcpy(temp_cluster_name,temp_cluster_name_specified);
            strcpy(cluster_name_source,"specified");
        }
        else{
            flag2=show_current_cluster(temp_workdir,temp_cluster_name_switched,0);
            if(flag2==0){
                strcpy(temp_cluster_name,temp_cluster_name_switched);
                strcpy(cluster_name_source,"switched");
            }
        }
        if(cluster_name_check(temp_cluster_name)!=-127){
            if(batch_flag!=0){
                if(strlen(temp_cluster_name)==0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ]" RESET_DISPLAY " No specified or switched cluster. Please select one from the list:\n");
                }
                else{
                    printf(WARN_YELLO_BOLD "[ -WARN- ]" RESET_DISPLAY " The specified cluster name " WARN_YELLO_BOLD "%s" RESET_DISPLAY " is invalid. Please choose one from the list:\n",temp_cluster_name);
                }
                list_all_cluster_names(1);
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%128s",temp_cluster_name);
                getchar();
                if(cluster_name_check(temp_cluster_name)!=-127){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The input cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid. Exit now.\n" RESET_DISPLAY,temp_cluster_name);
                    return -3;
                }
                strcpy(cluster_name_source,"input");
            }
            else{
                list_all_cluster_names(1);
                if(strlen(temp_cluster_name)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid. Exit now.\n" RESET_DISPLAY,temp_cluster_name);
                }
                else{
                    printf(FATAL_RED_BOLD "[ FATAL: ] No cluster specified or switched. Use " WARN_YELLO_BOLD "-c" FATAL_RED_BOLD " or " WARN_YELLO_BOLD "switch" FATAL_RED_BOLD " to one." RESET_DISPLAY "\n");
                }
                return -3;
            }
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " cluster name " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",cluster_name_source,temp_cluster_name);
        strcpy(cluster_name,temp_cluster_name);
        get_workdir(workdir,cluster_name);
        cluster_role_detect(workdir,cluster_role,cluster_role_ext);
        if(strcmp(role_flag,"opr")==0&&strcmp(cluster_role,"opr")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " needs the " WARN_YELLO_BOLD "operator" FATAL_RED_BOLD " to execute.\n",argv[1]);
            printf(RESET_DISPLAY GENERAL_BOLD "[ -INFO- ] Current role: %s . Please contact the operator.\n",cluster_role);
            return 1;
        }
        else if(strcmp(role_flag,"admin")==0&&strcmp(cluster_role,"opr")!=0&&strcmp(cluster_role,"admin")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " needs the " WARN_YELLO_BOLD "operator" FATAL_RED_BOLD " or " WARN_YELLO_BOLD "admin" FATAL_RED_BOLD " to execute.\n",argv[1]); 
            printf(RESET_DISPLAY GENERAL_BOLD "[ -INFO- ] Current role: %s . Please contact the operator.\n",cluster_role);
            return 1;
        }
        if(check_local_tf_config(workdir,filename_temp)==0){
            tf_local_config_flag=get_tf_running(&tf_this_run,filename_temp);
        }
    }
    if(strcmp(cu_flag,"UNAME")==0){
        if(cluster_empty_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is empty. Please init first." RESET_DISPLAY "\n",cluster_name);
            return -7;
        }
        if(cmd_keyword_check(argc,argv,"-u",string_temp)!=0){
            if(batch_flag!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid user name from the list below. \n");
                hpc_user_list(workdir,CRYPTO_KEY_FILE,0);
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%256s",string_temp);
                getchar();
                if(user_name_quick_check(cluster_name,string_temp,SSHKEY_DIR)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The input user name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid. Exit now.\n" RESET_DISPLAY,string_temp);
                    return -5;
                }
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Cluster user not specified. Use " HIGH_CYAN_BOLD "-u USER_NAME" RESET_DISPLAY " or " HIGH_CYAN_BOLD "-i" RESET_DISPLAY " (interactive).\n");
                hpc_user_list(workdir,CRYPTO_KEY_FILE,0);
                return -5;
            }
        }
        if(user_name_quick_check(cluster_name,string_temp,SSHKEY_DIR)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified user name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid. Exit now.\n" RESET_DISPLAY,string_temp);
            hpc_user_list(workdir,CRYPTO_KEY_FILE,0);
            return -5;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the user name " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",string_temp);
        strcpy(user_name,string_temp);
    }
    
    if(tf_local_config_flag==127){ // If not get_tf_running for local workdir
        get_tf_running(&tf_this_run,TF_RUNNING_CONFIG); //goes to the global static one
        // the command args have the higher priority
        // flush any values previously get
        cmd_keyword_check(argc,argv,"--tf-run",string_temp);
        if(strcmp(string_temp,"tofu")==0){
            strcpy(tf_this_run.tf_runner,TOFU_EXEC);
            strcpy(tf_this_run.tf_runner_type,"tofu");
        }
        else if(strcmp(string_temp,"terraform")==0){
            strcpy(tf_this_run.tf_runner,TERRAFORM_EXEC);
            strcpy(tf_this_run.tf_runner_type,"terraform");
        }
        cmd_keyword_check(argc,argv,"--dbg-level",string_temp); //Get the global option: debug level
        if(strcmp(string_temp,"trace")==0||strcmp(string_temp,"debug")==0||strcmp(string_temp,"info")==0||strcmp(string_temp,"warn")==0||strcmp(string_temp,"error")==0||strcmp(string_temp,"off")==0||strcmp(string_temp,"TRACE")==0||strcmp(string_temp,"DEBUG")==0||strcmp(string_temp,"INFO")==0||strcmp(string_temp,"WARN")==0||strcmp(string_temp,"ERROR")==0||strcmp(string_temp,"OFF")==0){
            strcpy(tf_this_run.dbg_level,"warn");
        }
        cmd_keyword_check(argc,argv,"--max-time",string_temp); //Get the global option: tf maximum execution time.
        max_time_temp=string_to_positive_num(string_temp);
        if(max_time_temp>MAXIMUM_WAIT_TIME-1&&max_time_temp<MAXIMUM_WAIT_TIME_EXT+1){
            tf_this_run.max_wait_time=max_time_temp;
        }
    }
    //If the tf_this_run is still invalid, throw out a serious warning.
    //Anyway, the operations except cluster tf operations may still work.
    if(tf_exec_config_validation(&tf_this_run)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] ATTENTION! The tf config is not working! Please repair the hpcopr!" RESET_DISPLAY "\n");
    }
    return 0;
}
