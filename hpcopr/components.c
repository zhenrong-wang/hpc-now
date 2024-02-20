/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "now_macros.h"
#include "general_funcs.h"
#include "components.h"
#include "cluster_general_funcs.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern char url_tf_root_var[LOCATION_LENGTH];
extern char url_shell_scripts_var[LOCATION_LENGTH];
extern char url_now_crypto_var[LOCATION_LENGTH];
extern char url_initutils_root_var[LOCATION_LENGTH];
extern char url_app_pkgs_root_var[LOCATION_LENGTH];
extern char url_app_inst_root_var[LOCATION_LENGTH];

extern int tf_loc_flag_var;
extern int code_loc_flag_var;
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

extern char sha_tf_exec_var[80];
extern char sha_tf_zip_var[80];
extern char sha_tofu_exec_var[80]; 
extern char sha_tofu_zip_var[80];
extern char sha_now_crypto_var[80];
extern char sha_ali_tf_var[80];
extern char sha_ali_tf_zip_var[80];
extern char sha_qcloud_tf_var[80];
extern char sha_qcloud_tf_zip_var[80];
extern char sha_aws_tf_var[80];
extern char sha_aws_tf_zip_var[80];
extern char sha_hw_tf_var[80];
extern char sha_hw_tf_zip_var[80];
extern char sha_bd_tf_var[80];
extern char sha_bd_tf_zip_var[80];
extern char sha_azrm_tf_var[80];
extern char sha_azrm_tf_zip_var[80];
extern char sha_azad_tf_var[80];
extern char sha_azad_tf_zip_var[80];
extern char sha_gcp_tf_var[80];
extern char sha_gcp_tf_zip_var[80];

int valid_sha_or_not(char* sha_input){
    if(strlen(sha_input)!=64){
        return -1;
    }
    int i;
    for(i=0;i<64;i++){
        if(*(sha_input+i)=='0'||*(sha_input+i)=='9'||*(sha_input+i)=='A'||*(sha_input+i)=='Z'||*(sha_input+i)=='a'||*(sha_input+i)=='z'){
            continue;
        }
        else if(*(sha_input+i)>'0'&&*(sha_input+i)<'9'){
            continue;
        }
        else if(*(sha_input+i)>'A'&&*(sha_input+i)<'Z'){
            continue;
        }
        else if(*(sha_input+i)>'a'&&*(sha_input+i)<'z'){
            continue;
        }
        else{
            return 1;
        }
    }
    return 0;
}

int valid_ver_or_not(char* version_code){
    int length=strlen(version_code);
    int i,j;
    int dots=0;
    if(length>12||length<5){
        return -1;
    }
    if(*(version_code+length-1)=='.'){
        return -1;
    }
    if(*(version_code+0)=='.'||*(version_code+1)!='.'){
        return -1;
    }
    for(i=0;i<length;i++){
        if(*(version_code+i)=='.'){
            dots++;
            continue;
        }
        else if(*(version_code+i)<'0'||*(version_code+i)>'9'){
            return -1;
        }
    }
    if(dots!=2){
        return -1;
    }
    for(i=0;i<length-1;i++){
        j=i+1;
        if(*(version_code+i)=='.'&&*(version_code+j)=='.'){
            return -1;
        }
    }
    return 0;
}

int valid_ver_or_not_tofu(char* version_code){
    char head[32]="";
    char tail[32]="";
    get_seq_nstring(version_code,'-',1,head,32);
    get_seq_nstring(version_code,'-',2,tail,32);
    if(valid_ver_or_not(head)!=0){
        return 1;
    }
    return 0;
}

int get_vers_sha_vars(void){
    char vers_sha_line[LINE_LENGTH_SMALL]=""; 
    char header[64]="";
    char version[32]="";
    char exec_sha[80]="";
    char zip_sha[80]="";
    int i=0;
    FILE* file_p=fopen(VERS_SHA_CONF_FILE,"r");
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fngetline(file_p,vers_sha_line,LINE_LENGTH_SMALL);
        get_seq_nstring(vers_sha_line,' ',1,header,64);
        get_seq_nstring(vers_sha_line,' ',2,version,32);
        get_seq_nstring(vers_sha_line,' ',3,exec_sha,80);
        get_seq_nstring(vers_sha_line,' ',4,zip_sha,80);    
        if(strcmp(header,"terraform:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(terraform_version_var,version);
                strcpy(sha_tf_exec_var,exec_sha);
                strcpy(sha_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"tofu:")==0){
            if(valid_ver_or_not_tofu(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(tofu_version_var,version);
                strcpy(sha_tofu_exec_var,exec_sha);
                strcpy(sha_tofu_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"alicloud_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(ali_tf_plugin_version_var,version);
                strcpy(sha_ali_tf_var,exec_sha);
                strcpy(sha_ali_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"tencentcloud_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(qcloud_tf_plugin_version_var,version);
                strcpy(sha_qcloud_tf_var,exec_sha);
                strcpy(sha_qcloud_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"aws_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(aws_tf_plugin_version_var,version);
                strcpy(sha_aws_tf_var,exec_sha);
                strcpy(sha_aws_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"huaweicloud_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(hw_tf_plugin_version_var,version);
                strcpy(sha_hw_tf_var,exec_sha);
                strcpy(sha_hw_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"baidubce_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(bd_tf_plugin_version_var,version);
                strcpy(sha_bd_tf_var,exec_sha);
                strcpy(sha_bd_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"azurerm_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(azrm_tf_plugin_version_var,version);
                strcpy(sha_azrm_tf_var,exec_sha);
                strcpy(sha_azrm_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"azuread_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(azad_tf_plugin_version_var,version);
                strcpy(sha_azad_tf_var,exec_sha);
                strcpy(sha_azad_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"gcp_tf:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0&&valid_sha_or_not(zip_sha)==0){
                strcpy(gcp_tf_plugin_version_var,version);
                strcpy(sha_gcp_tf_var,exec_sha);
                strcpy(sha_gcp_tf_zip_var,zip_sha);
            }
        }
        else if(strcmp(header,"now_crypto:")==0){
            if(valid_ver_or_not(version)==0&&valid_sha_or_not(exec_sha)==0){
                strcpy(sha_now_crypto_var,exec_sha);
            }
        }
        else{
            continue;
        }
        i++;
    }
    if(i==VERS_SHA_LINES){
        return 0;
    }
    else{
        return 1;
    }
}

int reset_vers_sha_vars(void){
    FILE* file_p=fopen(VERS_SHA_CONF_FILE,"w+");
    if(file_p==NULL){
        return -3;
    }
    char tf_sha_file[FILENAME_LENGTH]="";
    char crypto_sha_file[FILENAME_LENGTH]="";
    char cmdline1[CMDLINE_LENGTH]="";
    char cmdline2[CMDLINE_LENGTH]="";
    char sha_line[LINE_LENGTH_SMALL]="";
    FILE* file_p_1=NULL;
    FILE* file_p_2=NULL;
    if(strlen(url_tf_root_var)==0||strlen(url_now_crypto_var)==0){
        fclose(file_p);
        return -1;
    }
    if(tf_loc_flag_var==1&&now_crypto_loc_flag_var==1){
        snprintf(tf_sha_file,FILENAME_LENGTH-1,"%s%stf-sha-%s.dat",url_tf_root_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        snprintf(crypto_sha_file,FILENAME_LENGTH-1,"%s%scrypto-sha-%s.dat",url_now_crypto_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(tf_sha_file)!=0||file_exist_or_not(crypto_sha_file)!=0){
            fclose(file_p);
            return -1;
        }
        file_p_1=fopen(tf_sha_file,"r");
        file_p_2=fopen(crypto_sha_file,"r");
        while(fngetline(file_p_1,sha_line,LINE_LENGTH_SMALL)!=1){
            fprintf(file_p,"%s\n",sha_line);
        }
        fclose(file_p_1);
        fngetline(file_p_2,sha_line,LINE_LENGTH_SMALL);
        fprintf(file_p,"%s\n",sha_line);
        fclose(file_p_2);
        fclose(file_p);
    }
    else if(tf_loc_flag_var==0&&now_crypto_loc_flag_var==0){
        fclose(file_p);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %stf-sha-%s.dat >> %s",url_tf_root_var,FILENAME_SUFFIX_SHORT,VERS_SHA_CONF_FILE);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %scrypto-sha-%s.dat >> %s",url_now_crypto_var,FILENAME_SUFFIX_SHORT,VERS_SHA_CONF_FILE);
        if(system(cmdline1)!=0||system(cmdline2)!=0){
            return 1;
        }
    }
    else if(tf_loc_flag_var==0&&now_crypto_loc_flag_var==1){
        fclose(file_p);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %stf-sha-%s.dat >> %s",url_tf_root_var,FILENAME_SUFFIX_SHORT,VERS_SHA_CONF_FILE);
        if(system(cmdline1)!=0){
            return 1;
        }
        snprintf(crypto_sha_file,FILENAME_LENGTH-1,"%s%scrypto-sha-%s.dat",url_now_crypto_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(crypto_sha_file)!=0){
            return -1;
        }  
        file_p=fopen(VERS_SHA_CONF_FILE,"a");
        file_p_2=fopen(crypto_sha_file,"r");
        fngetline(file_p_2,sha_line,LINE_LENGTH_SMALL);
        fprintf(file_p,"%s\n",sha_line);
        fclose(file_p_2);
        fclose(file_p);
    }
    else if(tf_loc_flag_var==1&&now_crypto_loc_flag_var==0){
        snprintf(tf_sha_file,FILENAME_LENGTH-1,"%s%stf-sha-%s.dat",url_tf_root_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(tf_sha_file)!=0){
            fclose(file_p);
            return -1;
        }
        file_p_1=fopen(tf_sha_file,"r");
        while(fngetline(file_p_1,sha_line,LINE_LENGTH_SMALL)!=1){
            fprintf(file_p,"%s\n",sha_line);
        }
        fclose(file_p_1);
        fclose(file_p);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %scrypto-sha-%s.dat >> %s",url_now_crypto_var,FILENAME_SUFFIX_SHORT,VERS_SHA_CONF_FILE);
        if(system(cmdline2)!=0){
            return 1;
        }
    }
    return 0;
}

int show_vers_sha_vars(void){
    FILE* file_p=fopen(VERS_SHA_CONF_FILE,"r");
    char vers_and_sha[LINE_LENGTH_SMALL]="";
    if(file_p==NULL){
        printf("[ -FATAL- ] Failed to open the hash file. Please try 'hpcopr envcheck',\n");
        printf("[  ****  ]  or 'hpcopr configloc'. Or run 'hpcopr repair',\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Col.1-Component  Col.2-Version  Col.3-Exec_sha256  Col.4-Zip_sha256\n");
    while(fngetline(file_p,vers_and_sha,LINE_LENGTH_SMALL)!=1){
        printf("%s\n",vers_and_sha);
    }
    fclose(file_p);
    return 0;
}

int valid_loc_format_or_not(char* loc_string){
    int length;
    length=strlen(loc_string);
    if(length<3){
        return -1;
    }
#ifdef _WIN32
    if(length<8){
        if(*(loc_string+0)<'A'||*(loc_string+0)>'z'){
            return -1;
        }
        if(*(loc_string+0)>'Z'&&*(loc_string+0)<'a'){
            return -1;
        }
        if(*(loc_string+1)!=':'){
            return -1;
        }
        if(*(loc_string+2)!='\\'){
            return -1;
        }
        return 1;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)==':'&&*(loc_string+5)=='/'&&*(loc_string+6)=='/'&&*(loc_string+length-1)=='/'){
        return 0;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)=='s'&&*(loc_string+5)==':'&&*(loc_string+6)=='/'&&*(loc_string+7)=='/'&&*(loc_string+length-1)=='/'){
        return 0;
    }
    if(*(loc_string+0)=='f'&&*(loc_string+1)=='t'&&*(loc_string+2)=='p'&&*(loc_string+3)==':'&&*(loc_string+4)=='/'&&*(loc_string)=='/'&&*(loc_string+length-1)=='/'){
        return 0;
    }
    if(*(loc_string+0)<'A'||*(loc_string+0)>'z'){
        return -1;
    }
    if(*(loc_string+0)>'Z'&&*(loc_string+0)<'a'){
        return -1;
    }
    if(*(loc_string+1)!=':'){
        return -1;
    }
    if(*(loc_string+2)!='\\'){
        return -1;
    }
    return 1;
#else
    if(length<8){
        if(*(loc_string+0)!='/'){
            return -1;
        }
    }
    if(*(loc_string+0)!='/'){
        if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)==':'&&*(loc_string+5)=='/'&&*(loc_string+6)=='/'&&*(loc_string+length-1)=='/'){
            return 0;
        }
        if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)=='s'&&*(loc_string+5)==':'&&*(loc_string+6)=='/'&&*(loc_string+7)=='/'&&*(loc_string+length-1)=='/'){
            return 0;
        }
        if(*(loc_string+0)=='f'&&*(loc_string+1)=='t'&&*(loc_string+2)=='p'&&*(loc_string+3)==':'&&*(loc_string+4)=='/'&&*(loc_string)=='/'&&*(loc_string+length-1)=='/'){
            return 0;
        }
        return -1;
    }
    return 1;
#endif
}

int reset_locations(void){
    FILE* file_p=fopen(LOCATION_CONF_FILE,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"*VERY IMPORTANT*: THIS FILE IS GENERATED AND MANAGED BY THE HPC-NOW SERVICES! *DO NOT* MODIFY OR HANDLE THIS FILE MANUALLY!\n");
    fprintf(file_p,"tf_binary_root:   %s\n",DEFAULT_URL_TF_ROOT);
    fprintf(file_p,"cloud_iac_root:   %s\n",DEFAULT_URL_CODE_ROOT);
    fprintf(file_p,"online_scripts:   %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    fprintf(file_p,"now_crypto_bin:   %s\n",DEFAULT_URL_NOW_CRYPTO);
    fprintf(file_p,"online_initutl:   %s\n",DEFAULT_INITUTILS_REPO_ROOT);
    fprintf(file_p,"online_apps_pkgs: %s\n",DEFAULT_APPS_PKGS_REPO_ROOT);
    fprintf(file_p,"online_apps_inst: %s\n",DEFAULT_URL_APPS_INST_SCRIPTS);
    fclose(file_p);
    return 0;
}

int get_locations(void){
    char location_line[LOCATION_LENGTH_EXTENDED]="";
    char header_string[LINE_LENGTH_TINY]="";
    char loc_string[LOCATION_LENGTH]="";
    char title_string[LINE_LENGTH_SHORT]="";
    int i=0;
    if(file_exist_or_not(LOCATION_CONF_FILE)!=0){
        return -1;
    }
    FILE* file_p=fopen(LOCATION_CONF_FILE,"r");
    fngetline(file_p,title_string,LINE_LENGTH_SHORT);
    while(fngetline(file_p,location_line,LOCATION_LENGTH_EXTENDED)!=1){
        get_seq_nstring(location_line,' ',1,header_string,LINE_LENGTH_TINY);
        get_seq_nstring(location_line,' ',2,loc_string,LOCATION_LENGTH);
        if(strcmp(header_string,"tf_binary_root:")==0){
            strcpy(url_tf_root_var,loc_string);
#ifdef _WIN32
            if(loc_string[1]==':'){
                tf_loc_flag_var=1;
            }
#else
            if(loc_string[0]=='/'){
                tf_loc_flag_var=1;
            }
#endif
            i++;
        }
        else if(strcmp(header_string,"cloud_iac_root:")==0){
            strcpy(url_code_root_var,loc_string);
#ifdef _WIN32
            if(loc_string[1]==':'){
                code_loc_flag_var=1;
            }
#else
            if(loc_string[0]=='/'){
                code_loc_flag_var=1;
            }
#endif
            i++;
        }
        else if(strcmp(header_string,"online_scripts:")==0){
            strcpy(url_shell_scripts_var,loc_string);
            i++;
        }
        else if(strcmp(header_string,"online_apps_pkgs:")==0){
            strcpy(url_app_pkgs_root_var,loc_string);
            i++;
        }
        else if(strcmp(header_string,"online_apps_inst:")==0){
            strcpy(url_app_inst_root_var,loc_string);
            i++;
        }
        else if(strcmp(header_string,"now_crypto_bin:")==0){
            strcpy(url_now_crypto_var,loc_string);
#ifdef _WIN32
            if(loc_string[1]==':'){
                now_crypto_loc_flag_var=1;
            }
#else
            if(loc_string[0]=='/'){
                now_crypto_loc_flag_var=1;
            }
#endif
            i++;
        }
        else if(strcmp(header_string,"online_initutl:")==0){
            strcpy(url_initutils_root_var,loc_string);
            i++;
        }
        else{
            continue;
        }
    }
    if(i==DEFAULT_LOCATIONS_COUNT){
        return 0;
    }
    else{
        return 1;
    }    
}

int show_locations(void){
    FILE* file_p=NULL;
    file_p=fopen(LOCATION_CONF_FILE,"r");
    char header[64]="";
    char loc_string[LOCATION_LENGTH]="";
    int i;
    if(file_p==NULL){
        printf("[ -FATAL- ] Failed to open the location config. Please try 'hpcopr envcheck',\n");
        printf("[  ****  ]  or 'hpcopr configloc'. Or run 'hpcopr repair',\n");
        return -1;
    }
    fngetline(file_p,loc_string,LOCATION_LENGTH);
    for(i=0;i<DEFAULT_LOCATIONS_COUNT;i++){
        fscanf(file_p,"%63s%383s",header,loc_string);
        printf("%s -> %s\n",header,loc_string);
    }
    return 0;
}

int reset_tf_running(void){
    FILE* file_p=fopen(TF_RUNNING_CONFIG,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"tf_execution:  %s\n",TERRAFORM_EXEC);
    fprintf(file_p,"tf_dbg_level:  warn\n");
    fprintf(file_p,"max_wait_sec:  %d\n",MAXIMUM_WAIT_TIME);
    fclose(file_p);
    return 0;
}

//return -1: file_not_exist
//return 0: file exist and format correct
//return 1: one config failed
//return 2: two configs failed
//return 3: three configs failed
//Any failed config will be reset to the default one
int get_tf_running(tf_exec_config* tf_config, char* tf_config_file){
    FILE* file_p=fopen(tf_config_file,"r");
    char conf_line[LINE_LENGTH_SHORT]="";
    char header[256]="";
    char tail[512]="";
    int time,get_flag=0;
    if(file_p==NULL){
        strcpy(tf_config->tf_runner_type,"terraform");
        strcpy(tf_config->tf_runner,TERRAFORM_EXEC);
        strcpy(tf_config->dbg_level,"warn");
        tf_config->max_wait_time=MAXIMUM_WAIT_TIME;
        return -1;
    }
    while(!feof(file_p)){
        if(fngetline(file_p,conf_line,LINE_LENGTH_SHORT)!=0){
            continue;
        }
        get_seq_nstring(conf_line,' ',1,header,256);
        get_seq_nstring(conf_line,' ',2,tail,511);
        if(strcmp(header,"tf_execution:")==0){
            if(strcmp(tail,TOFU_EXEC)==0){
                strcpy(tf_config->tf_runner,TOFU_EXEC);
                strcpy(tf_config->tf_runner_type,"tofu");
            }
            else{
                strcpy(tf_config->tf_runner,TERRAFORM_EXEC);
                strcpy(tf_config->tf_runner_type,"terraform");
            }
        }
        else if(strcmp(header,"tf_dbg_level:")==0){
            if(strcmp(tail,"trace")!=0&&strcmp(tail,"debug")!=0&&strcmp(tail,"info")!=0&&strcmp(tail,"warn")!=0&&strcmp(tail,"error")!=0&&strcmp(tail,"off")!=0&&strcmp(tail,"TRACE")!=0&&strcmp(tail,"DEBUG")!=0&&strcmp(tail,"INFO")!=0&&strcmp(tail,"WARN")!=0&&strcmp(tail,"ERROR")!=0&&strcmp(tail,"OFF")!=0){
                strcpy(tf_config->dbg_level,"warn");
            }
            else{
                strcpy(tf_config->dbg_level,tail);
            }
        }
        else if(strcmp(header,"max_wait_sec:")==0){
            time=string_to_positive_num(tail);
            if(time<MAXIMUM_WAIT_TIME||time>MAXIMUM_WAIT_TIME_EXT){
                tf_config->max_wait_time=MAXIMUM_WAIT_TIME;
            }
            else{
                tf_config->max_wait_time=time;
            }
        }
        else{
            continue;
        }
    }
    fclose(file_p);
    if(strcmp(tf_config->tf_runner,TERRAFORM_EXEC)!=0&&strcmp(tf_config->tf_runner,TOFU_EXEC)!=0){
        strcpy(tf_config->tf_runner,TERRAFORM_EXEC);
        strcpy(tf_config->tf_runner_type,"terraform");
        get_flag++;
    }
    if(strcmp(tf_config->dbg_level,"trace")!=0&&strcmp(tf_config->dbg_level,"debug")!=0&&strcmp(tf_config->dbg_level,"info")!=0&&strcmp(tf_config->dbg_level,"warn")!=0&&strcmp(tf_config->dbg_level,"error")!=0&&strcmp(tf_config->dbg_level,"off")!=0&&strcmp(tf_config->dbg_level,"TRACE")!=0&&strcmp(tf_config->dbg_level,"DEBUG")!=0&&strcmp(tf_config->dbg_level,"INFO")!=0&&strcmp(tf_config->dbg_level,"WARN")!=0&&strcmp(tf_config->dbg_level,"ERROR")!=0&&strcmp(tf_config->dbg_level,"OFF")!=0){
        strcpy(tf_config->dbg_level,"warn");
        get_flag++;
    }
    if(tf_config->max_wait_time<MAXIMUM_WAIT_TIME||tf_config->max_wait_time>MAXIMUM_WAIT_TIME_EXT){
        tf_config->max_wait_time=MAXIMUM_WAIT_TIME;
        get_flag++;
    }
    return get_flag;
}

int show_tf_running_config(void){
    FILE* file_p=fopen(TF_RUNNING_CONFIG,"r");
    char conf_line[LINE_LENGTH_SHORT]="";
    char header[LINE_LENGTH_TINY]="";
    char tail[LINE_LENGTH_SHORT]="";
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        if(fngetline(file_p,conf_line,LINE_LENGTH_SHORT)!=0){
            continue;
        }
        get_seq_nstring(conf_line,' ',1,header,LINE_LENGTH_TINY);
        get_seq_nstring(conf_line,' ',2,tail,LINE_LENGTH_SHORT);
        if(strcmp(header,"tf_execution:")==0||strcmp(header,"tf_dbg_level:")==0||strcmp(header,"max_wait_sec:")==0){
            printf("|   " GENERAL_BOLD "%s" RESET_DISPLAY "  %s\n",header,tail);
        }
        else{
            continue;
        }
    }
    fclose(file_p);
    return 0;
}

int update_tf_running(char* new_tf_runner, char* new_dbg_level, int new_max_time){
    if(file_empty_or_not(TF_RUNNING_CONFIG)<1){
        return -1;
    }
    char prev_config[128]="";
    char new_max_time_string[8]="";
    char new_tf_runner_path[256]="";
    char prev_runner_assume[16]="";
    int i=0;
    if(strcmp(new_tf_runner,"terraform")==0||strcmp(new_tf_runner,"tofu")==0){
        if(strcmp(new_tf_runner,"terraform")==0){
            strcpy(new_tf_runner_path,TERRAFORM_EXEC);
            strcpy(prev_runner_assume,"tofu");
        }
        else{
            strcpy(new_tf_runner_path,TOFU_EXEC);
            strcpy(prev_runner_assume,"terraform");
        }
        find_and_nget(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"tf_execution:","","",1,"tf_execution:","","",' ',2,prev_config,128);
        if(strcmp(prev_config,new_tf_runner_path)!=0){
            find_and_nreplace(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"tf_execution:","","","","",prev_config,new_tf_runner_path);
            i++;
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updated tf execution from " GENERAL_BOLD "%s" RESET_DISPLAY " to " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",prev_runner_assume,new_tf_runner);
        }
    }
    if(strcmp(new_dbg_level,"trace")==0||strcmp(new_dbg_level,"debug")==0||strcmp(new_dbg_level,"info")==0||strcmp(new_dbg_level,"warn")==0||strcmp(new_dbg_level,"error")==0||strcmp(new_dbg_level,"off")==0||strcmp(new_dbg_level,"TRACE")==0||strcmp(new_dbg_level,"DEBUG")==0||strcmp(new_dbg_level,"INFO")==0||strcmp(new_dbg_level,"WARN")==0||strcmp(new_dbg_level,"ERROR")==0||strcmp(new_dbg_level,"OFF")==0){
        find_and_nget(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"tf_dbg_level:","","",1,"tf_dbg_level:","","",' ',2,prev_config,128);
        if(strcmp(prev_config,new_dbg_level)!=0){
            find_and_nreplace(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"tf_dbg_level:","","","","",prev_config,new_dbg_level);
            i++;
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updated debug log level from " GENERAL_BOLD "%s" RESET_DISPLAY " to " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",prev_config,new_dbg_level);
        }
    }
    if(new_max_time>MAXIMUM_WAIT_TIME-1&&new_max_time<MAXIMUM_WAIT_TIME_EXT+1){
        find_and_nget(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"max_wait_sec:","","",1,"max_wait_sec:","","",' ',2,prev_config,128);
        snprintf(new_max_time_string,7,"%d",new_max_time);
        if(strcmp(prev_config,new_max_time_string)!=0){
            find_and_nreplace(TF_RUNNING_CONFIG,LINE_LENGTH_SHORT,"max_wait_sec:","","","","",prev_config,new_max_time_string);
            i++;
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updated max wait time from " GENERAL_BOLD "%s" RESET_DISPLAY " to " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",prev_config,new_max_time_string);
        }
    }
    if(i==0){
        printf(GENERAL_BOLD "\n[ -INFO- ]" RESET_DISPLAY " Same configurations specified. Nothing updated.\n");
    }
    else{
        printf(GENERAL_BOLD "\n[ -INFO- ] New tf running configurations:" RESET_DISPLAY "\n\n");
        show_tf_running_config();
    }
    return 0;
}

int configure_locations(int batch_flag_local){
    char loc_string[LOCATION_LENGTH]="";
    int format_flag=0;
    FILE* file_p=NULL;

    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("[  ****  ] YOU ARE MODIFYING THE LOCATIONS OF COMPONENTS, DO MAKE SURE:\n");
    printf("[  ****  ] 1. The locations - URLs or local filesystem paths are valid.\n");
    printf("[  ****  ]    URLs : *MUST* start with 'http://' or 'https://', root\n");
    printf("[  ****  ]           locations *MUST* end with '/'\n");
    printf("[  ****  ]    Paths: *MUST* be absolute paths. For GNU/Linux and macOS,\n");
    printf("[  ****  ]           the locations must start with '/'; for Windows, the\n");
    printf("[  ****  ]           locations must start with DRIVE_LETTER:\\\n");              
    printf("[  ****  ] 2. The structures of the location are valid. Please refer to\n");
    printf("[  ****  ]     the docs and confirm your structure in advance.\n");
    printf("[  ****  ] THE HPCOPR WILL ONLY CHECK THE FORMAT OF YOUR INPUTS, WILL\n");
    printf("[  ****  ] *NOT* CHECK WHETHER LOCATIONS ARE VALID OR NOT. INVALID \n");
    printf("[  ****  ] LOCATIONS MAY DAMAGE THE HPC-NOW SERVICES!" RESET_DISPLAY "\n");
    
    if(prompt_to_confirm("ARE YOU SURE ?",CONFIRM_STRING,batch_flag_local)==1){
        return 1;
    }
    printf("[ LOC1/7 ] Please specify the root location of the tf binary and providers. \n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_URL_TF_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_tf_root_var,loc_string);
        }
    }
    printf("[ LOC2/7 ] Please specify the root location of the infra-as-code templates. \n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_URL_CODE_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_code_root_var,loc_string);
        }
    }
    printf("[ LOC3/7 ] Please specify the root location of the *online* shell scripts.\n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_URL_SHELL_SCRIPTS);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_shell_scripts_var,loc_string);
        }
    }
    printf("[ LOC4/7 ] Please input the root location of the now-crypto binary.\n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_URL_NOW_CRYPTO);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_now_crypto_var,loc_string);
        }
    }
    printf("[ LOC5/7 ] Please specify the location of the *online* repo for utils and apps.\n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_INITUTILS_REPO_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_initutils_root_var,loc_string);
        }
    }

    printf("[ LOC6/7 ] Please specify the location of the *online* repo for application install scripts.\n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_INITUTILS_REPO_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_app_inst_root_var,loc_string);
        }
    }

    printf("[ LOC7/7 ] Please specify the location of the *online* repo for application packages.\n");
    printf("[  ****  ] You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("[  ****  ] -> %s \n",DEFAULT_INITUTILS_REPO_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%383s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location." RESET_DISPLAY "\n");
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify." RESET_DISPLAY "\n");
        }
        else{
            strcpy(url_app_pkgs_root_var,loc_string);
        }
    }

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updating the location configuration file now ... \n");
    file_p=fopen(LOCATION_CONF_FILE,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create or modify the target file." RESET_DISPLAY "\n");
        return -1;
    }
    fprintf(file_p,"*VERY IMPORTANT*: THIS FILE IS GENERATED AND MANAGED BY THE HPC-NOW SERVICES! *DO NOT* MODIFY OR HANDLE THIS FILE MANUALLY!\n");
    if(strlen(url_tf_root_var)==0){
        fprintf(file_p,"tf_binary_root:   %s\n",DEFAULT_URL_TF_ROOT);
    }
    else{
        fprintf(file_p,"tf_binary_root:   %s\n",url_tf_root_var);
    }
    if(strlen(url_code_root_var)==0){
        fprintf(file_p,"cloud_iac_root:   %s\n",DEFAULT_URL_CODE_ROOT);
    }
    else{
        fprintf(file_p,"cloud_iac_root:   %s\n",url_code_root_var);
    }
    if(strlen(url_shell_scripts_var)==0){
        fprintf(file_p,"online_scripts:   %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    }
    else{
        fprintf(file_p,"online_scripts:   %s\n",url_shell_scripts_var);
    }
    if(strlen(url_now_crypto_var)==0){
        fprintf(file_p,"now_crypto_bin:   %s\n",DEFAULT_URL_NOW_CRYPTO);
    }
    else{
        fprintf(file_p,"now_crypto_bin:   %s\n",url_now_crypto_var);
    }
    if(strlen(url_initutils_root_var)==0){
        fprintf(file_p,"online_initutl:   %s\n",DEFAULT_INITUTILS_REPO_ROOT);
    }
    else{
        fprintf(file_p,"online_initutl:   %s\n",url_initutils_root_var);
    }
    if(strlen(url_app_pkgs_root_var)==0){
        fprintf(file_p,"online_apps_pkgs: %s\n",DEFAULT_APPS_PKGS_REPO_ROOT);
    }
    else{
        fprintf(file_p,"online_apps_pkgs: %s\n",url_app_pkgs_root_var);
    }
    if(strlen(url_app_inst_root_var)==0){
        fprintf(file_p,"online_apps_inst: %s\n",DEFAULT_URL_APPS_INST_SCRIPTS);
    }
    else{
        fprintf(file_p,"online_apps_inst: %s\n",url_app_inst_root_var);
    }
    fclose(file_p);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Locations are modified and saved. The latest locations:\n");
    show_locations();
    return 0;
}