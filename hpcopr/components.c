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

#include "now_macros.h"
#include "general_funcs.h"
#include "components.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern char url_tf_root_var[LOCATION_LENGTH];
extern char url_shell_scripts_var[LOCATION_LENGTH];
extern char url_now_crypto_var[LOCATION_LENGTH];
extern char url_initutils_root_var[LOCATION_LENGTH];
extern int tf_loc_flag_var;
extern int code_loc_flag_var;
extern int now_crypto_loc_flag_var;

extern char terraform_version_var[16];
extern char ali_tf_plugin_version_var[16];
extern char qcloud_tf_plugin_version_var[16];
extern char aws_tf_plugin_version_var[16];

extern char md5_tf_exec_var[64];
extern char md5_tf_zip_var[64];
extern char md5_now_crypto_var[64];
extern char md5_ali_tf_var[64];
extern char md5_ali_tf_zip_var[64];
extern char md5_qcloud_tf_var[64];
extern char md5_qcloud_tf_zip_var[64];
extern char md5_aws_tf_var[64];
extern char md5_aws_tf_zip_var[64];

int valid_md5_or_not(char* md5_input){
    if(strlen(md5_input)!=32){
        return -1;
    }
    int i;
    for(i=0;i<32;i++){
        if(*(md5_input+i)=='0'||*(md5_input+i)=='9'||*(md5_input+i)=='A'||*(md5_input+i)=='Z'||*(md5_input+i)=='a'||*(md5_input+i)=='z'){
            continue;
        }
        else if(*(md5_input+i)>'0'&&*(md5_input+i)<'9'){
            continue;
        }
        else if(*(md5_input+i)>'A'&&*(md5_input+i)<'Z'){
            continue;
        }
        else if(*(md5_input+i)>'a'&&*(md5_input+i)<'z'){
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

int get_vers_md5_vars(void){
    char vers_md5_line[128]="";
    char header[16]="";
    char tail[64]="";
    FILE* file_p=fopen(VERS_MD5_CONF_FILE,"r");
    if(file_p==NULL){
        return -1;
    }
    if(fgetline(file_p,vers_md5_line)!=0){
        return 1;
    }
//    printf("\n\n%s\n\n",vers_md5_line);
    get_seq_string(vers_md5_line,'\t',1,terraform_version_var);
    get_seq_string(vers_md5_line,'\t',2,ali_tf_plugin_version_var);
    get_seq_string(vers_md5_line,'\t',3,qcloud_tf_plugin_version_var);
    get_seq_string(vers_md5_line,'\t',4,aws_tf_plugin_version_var);
    if(valid_ver_or_not(terraform_version_var)!=0||valid_ver_or_not(ali_tf_plugin_version_var)!=0||valid_ver_or_not(qcloud_tf_plugin_version_var)!=0||valid_ver_or_not(aws_tf_plugin_version_var)!=0){
//        printf("%s||%s||%s||%sERROR HERE!.2.\n",terraform_version_var,ali_tf_plugin_version_var,qcloud_tf_plugin_version_var,aws_tf_plugin_version_var);
//        printf("ERROR HERE!.1.\n");
        return 1;
    }
    while(fgetline(file_p,vers_md5_line)==0){
        get_seq_string(vers_md5_line,' ',1,header);
        get_seq_string(vers_md5_line,' ',2,tail);
//        printf("%s||%s||%sCHECKIT HERE!.2.\n\n",vers_md5_line,header,tail);
        if(strcmp(header,"TF_EXEC:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_tf_exec_var,tail);
        }
        else if(strcmp(header,"TF_EXEC_ZIP:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }            
            strcpy(md5_tf_zip_var,tail);
        }
        else if(strcmp(header,"NOW_CRYPTO:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_now_crypto_var,tail);
        }
        else if(strcmp(header,"ALI_TF:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_ali_tf_var,tail);
        }
        else if(strcmp(header,"ALI_TF_ZIP:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_ali_tf_zip_var,tail);
        }
        else if(strcmp(header,"QCLOUD_TF:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_qcloud_tf_var,tail);
        }
        else if(strcmp(header,"QCLOUD_TF_ZIP:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_qcloud_tf_zip_var,tail);
        }
        else if(strcmp(header,"AWS_TF:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_aws_tf_var,tail);
        }
        else if(strcmp(header,"AWS_TF_ZIP:")==0){
            if(valid_md5_or_not(tail)!=0){
//                printf("%s||%s||%sERROR HERE!.4.\n",vers_md5_line,header,tail);
                return 1;
            }
            strcpy(md5_aws_tf_zip_var,tail);
        }
        else{
            continue;
        }
    }
    return 0;
}

int reset_vers_md5_vars(void){
    FILE* file_p=fopen(VERS_MD5_CONF_FILE,"w+");
    if(file_p==NULL){
        return -127;
    }
    char tf_md5_file[FILENAME_LENGTH]="";
    char crypto_md5_file[FILENAME_LENGTH]="";
    char cmdline1[CMDLINE_LENGTH]="";
    char cmdline2[CMDLINE_LENGTH]="";
    char md5_line[LINE_LENGTH_SHORT]="";
    FILE* file_p_1=NULL;
    FILE* file_p_2=NULL;
    if(strlen(url_tf_root_var)==0||strlen(url_now_crypto_var)==0){
        fclose(file_p);
        return -1;
    }
    if(tf_loc_flag_var==1&&now_crypto_loc_flag_var==1){
        sprintf(tf_md5_file,"%s%stf-md5-%s.dat",url_tf_root_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        sprintf(crypto_md5_file,"%s%scrypto-md5-%s.dat",url_now_crypto_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(tf_md5_file)!=0||file_exist_or_not(crypto_md5_file)!=0){
            fclose(file_p);
            return -1;
        }
        file_p_1=fopen(tf_md5_file,"r");
        file_p_2=fopen(crypto_md5_file,"r");
        while(fgetline(file_p_1,md5_line)==0){
            fprintf(file_p,"%s\n",md5_line);
        }
        fclose(file_p_1);
        fgetline(file_p_2,md5_line);
        fprintf(file_p,"%s\n",md5_line);
        fclose(file_p_2);
        fclose(file_p);
    }
    else if(tf_loc_flag_var==0&&now_crypto_loc_flag_var==0){
        fclose(file_p);
        sprintf(cmdline1,"curl -s %stf-md5-%s.dat >> %s",url_tf_root_var,FILENAME_SUFFIX_SHORT,VERS_MD5_CONF_FILE);
        sprintf(cmdline2,"curl -s %scrypto-md5-%s.dat >> %s",url_now_crypto_var,FILENAME_SUFFIX_SHORT,VERS_MD5_CONF_FILE);
        if(system(cmdline1)!=0||system(cmdline2)!=0){
            return 1;
        }
    }
    else if(tf_loc_flag_var==0&&now_crypto_loc_flag_var==1){
        fclose(file_p);
        sprintf(cmdline1,"curl -s %stf-md5-%s.dat >> %s",url_tf_root_var,FILENAME_SUFFIX_SHORT,VERS_MD5_CONF_FILE);
        if(system(cmdline1)!=0){
            return 1;
        }
        sprintf(crypto_md5_file,"%s%scrypto-md5-%s.dat",url_now_crypto_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(crypto_md5_file)!=0){
            return -1;
        }  
        file_p=fopen(VERS_MD5_CONF_FILE,"a");
        file_p_2=fopen(crypto_md5_file,"r");
        fgetline(file_p_2,md5_line);
        fprintf(file_p,"%s\n",md5_line);
        fclose(file_p_2);
        fclose(file_p);
    }
    else if(tf_loc_flag_var==1&&now_crypto_loc_flag_var==0){
        sprintf(tf_md5_file,"%s%stf-md5-%s.dat",url_tf_root_var,PATH_SLASH,FILENAME_SUFFIX_SHORT);
        if(file_exist_or_not(tf_md5_file)!=0){
            fclose(file_p);
            return -1;
        }
        file_p_1=fopen(tf_md5_file,"r");
        while(fgetline(file_p_1,md5_line)==0){
            fprintf(file_p,"%s\n",md5_line);
        }
        fclose(file_p_1);
        fclose(file_p);
        sprintf(cmdline2,"curl -s %scrypto-md5-%s.dat >> %s",url_now_crypto_var,FILENAME_SUFFIX_SHORT,VERS_MD5_CONF_FILE);
        if(system(cmdline2)!=0){
            return 1;
        }
    }
    return 0;
}

int show_vers_md5vars(void){
    FILE* file_p=fopen(VERS_MD5_CONF_FILE,"r");
    char vers_and_md5[64]="";
    if(file_p==NULL){
        printf("[ -FATAL- ] Failed to open the md5 file. Please try 'hpcopr envcheck',\n");
        printf("|           or 'hpcopr configloc'. Or run 'hpcopr repair',\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Versions and md5sum values\n");
    printf("|  Vers:   Terraform\tAliCloudProvider TencentCloudProvider AWSProvider\n");
    while(fgetline(file_p,vers_and_md5)==0){
        printf("|          %s\n",vers_and_md5);
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
    fprintf(file_p,"TF_BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_TF_ROOT);
    fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",DEFAULT_URL_CODE_ROOT);
    fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",DEFAULT_URL_NOW_CRYPTO);
    fprintf(file_p,"ONLINE_URL_INITUTILS_ROOT %s\n",DEFAULT_INITUTILS_REPO_ROOT);
    fclose(file_p);
    return 0;
}

int get_locations(void){
    char location_line[LOCATION_LENGTH_EXTENDED]="";
    char header_string[64]="";
    char loc_string[LOCATION_LENGTH]="";
    char title_string[256]="";
    if(file_exist_or_not(LOCATION_CONF_FILE)!=0){
        return -1;
    }
    FILE* file_p=fopen(LOCATION_CONF_FILE,"r");
    fgetline(file_p,title_string);
    while(fgetline(file_p,location_line)==0){
        get_seq_string(location_line,' ',1,header_string);
        get_seq_string(location_line,' ',2,loc_string);
        if(strcmp(header_string,"TF_BINARY_AND_PROVIDERS_LOC_ROOT")==0){
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
        }
        else if(strcmp(header_string,"CLOUD_IAC_TEMPLATES_LOC_ROOT")==0){
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
        }
        else if(strcmp(header_string,"ONLINE_SHELL_SCRIPTS_LOC_ROOT")==0){
            strcpy(url_shell_scripts_var,loc_string);
        }
        else if(strcmp(header_string,"NOW_CRYPTO_BINARY_LOC")==0){
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
        }
        else if(strcmp(header_string,"ONLINE_URL_INITUTILS_ROOT")==0){
            strcpy(url_initutils_root_var,loc_string);
        }
        else{
            continue;
        }
    }
    return 0;       
}

int show_locations(void){
    FILE* file_p=NULL;
    file_p=fopen(LOCATION_CONF_FILE,"r");
    char header[64]="";
    char loc_string[LOCATION_LENGTH]="";
    int i;
    if(file_p==NULL){
        printf("[ -FATAL- ] Failed to open the location config. Please try 'hpcopr envcheck',\n");
        printf("|           or 'hpcopr configloc'. Or run 'hpcopr repair',\n");
        return -1;
    }
    fgetline(file_p,loc_string);
    printf("\n");
    for(i=0;i<LOCATION_LINES;i++){
        fscanf(file_p,"%s%s",header,loc_string);
        printf("%s -> %s\n",header,loc_string);
    }
    return 0;
}

int configure_locations(void){
    char doubleconfirm[32]="";
    char loc_string[LOCATION_LENGTH]="";
    int format_flag=0;
    FILE* file_p=NULL;
    printf(GENERAL_BOLD "\n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   YOU ARE MODIFYING THE LOCATIONS OF COMPONENTS FOR THE HPC-NOW SERVICES!       \n");
    printf("|*   YOUR NEED TO MAKE SURE:                                                       \n");
    printf("|*   1. The locations - either URLs or local filesystem paths are valid.           \n");
    printf("|*        URLs       : *MUST* start with 'http://' or 'https://' , root locations  \n");
    printf("|*                     *MUST* end with '/'                                         \n");
    printf("|*        Local Paths: *MUST* be absolute paths. For GNU/Linux and macOS, the      \n");
    printf("|                       locations must start with '/'; for Microsoft Windows, the  \n");
    printf("|                       locations must start with DRIVE_LETTER:\\                   \n");              
    printf("|*   2. The structures of the location are valid. Please refer to the docs and     \n");
    printf("|*      confirm your structure in advance.                                         \n");
    printf("|*                                                                                 \n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   THE HPCOPR WILL ONLY CHECK THE FORMAT OF YOUR INPUTS, WILL *NOT* CHECK        \n");
    printf("|*   WHETHER LOCATIONS ARE VALID OR NOT. IT'S YOUR JOB TO GUARANTEE THE VALIDITY!  \n");
    printf("|*   INVALID LOCATIONS MAY DAMAGE THE HPC-NOW SERVICES! YOU MAY NEED TO RESET TO   \n");
    printf("|*   THE DEFAULT LOCATIONS IF YOUR LOCATIONS FAIL TO WORK PROPERLY!                \n");
    printf("|*                                                                                 \n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("| ARE YOU SURE? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY GENERAL_BOLD " is accepted to double confirm this operation:\n\n" RESET_DISPLAY);
    fflush(stdin);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    scanf("%s",doubleconfirm);
    getchar();
    if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    printf("[ LOC1/5 ] Please specify the root location of the terraform binary and providers. \n");
    printf("|          You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_TF_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location.\n" RESET_DISPLAY);
        }
        else{
            strcpy(url_tf_root_var,loc_string);
        }
    }
    printf("[ LOC2/5 ] Please specify the root location of the terraform templates. \n");
    printf("|          You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_CODE_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location.\n" RESET_DISPLAY);
        }
        else{
            strcpy(url_code_root_var,loc_string);
        }
    }
    printf("[ LOC3/5 ] Please specify the root location of the *online* shell scripts.\n");
    printf("|          You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_SHELL_SCRIPTS);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location.\n" RESET_DISPLAY);
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify.\n" RESET_DISPLAY);
        }
        else{
            strcpy(url_shell_scripts_var,loc_string);
        }
    }
    printf("[ LOC4/5 ] Please input the root location of the now-crypto binary.\n");
    printf("|          You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_NOW_CRYPTO);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location.\n" RESET_DISPLAY);
        }
        else{
            strcpy(url_now_crypto_var,loc_string);
        }
    }
    printf("[ LOC5/5 ] Please specify the location of the *online* repo for utils and apps.\n");
    printf("|          You can input " HIGH_CYAN_BOLD "default" RESET_DISPLAY " to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_INITUTILS_REPO_ROOT);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",loc_string);
    getchar();
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Invalid format. Will not modify this location.\n" RESET_DISPLAY);
        }
        else if(format_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] This location must be a public URL. Will not modify.\n" RESET_DISPLAY);
        }
        else{
            strcpy(url_initutils_root_var,loc_string);
        }
    }

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Updating the location configuration file now ... \n");
    file_p=fopen(LOCATION_CONF_FILE,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create or modify the target file. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    fprintf(file_p,"*VERY IMPORTANT*: THIS FILE IS GENERATED AND MANAGED BY THE HPC-NOW SERVICES! *DO NOT* MODIFY OR HANDLE THIS FILE MANUALLY!\n");
    if(strlen(url_tf_root_var)==0){
        fprintf(file_p,"TF_BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_TF_ROOT);
    }
    else{
        fprintf(file_p,"TF_BINARY_AND_PROVIDERS_LOC_ROOT %s\n",url_tf_root_var);
    }
    if(strlen(url_code_root_var)==0){
        fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",DEFAULT_URL_CODE_ROOT);
    }
    else{
        fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",url_code_root_var);
    }
    if(strlen(url_shell_scripts_var)==0){
        fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    }
    else{
        fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",url_shell_scripts_var);
    }
    if(strlen(url_now_crypto_var)==0){
        fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",DEFAULT_URL_NOW_CRYPTO);
    }
    else{
        fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",url_now_crypto_var);
    }
    if(strlen(url_initutils_root_var)==0){
        fprintf(file_p,"ONLINE_URL_INITUTILS_ROOT %s\n",DEFAULT_INITUTILS_REPO_ROOT);
    }
    else{
        fprintf(file_p,"ONLINE_URL_INITUTILS_ROOT %s\n",url_initutils_root_var);
    }
    fclose(file_p);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Locations are modified and saved. The latest locations:\n");
    show_locations();
    return 0;
}