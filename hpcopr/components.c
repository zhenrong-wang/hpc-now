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

extern char URL_CODE_ROOT[LOCATION_LENGTH];
extern char URL_TF_ROOT[LOCATION_LENGTH];
extern char URL_SHELL_SCRIPTS[LOCATION_LENGTH];
extern char URL_NOW_CRYPTO[LOCATION_LENGTH];
extern int TF_LOC_FLAG;
extern int CODE_LOC_FLAG;
extern int NOW_CRYPTO_LOC_FLAG;

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
    get_seq_string(vers_md5_line,'\t',1,terraform_version_var);
    get_seq_string(vers_md5_line,'\t',2,ali_tf_plugin_version_var);
    get_seq_string(vers_md5_line,'\t',3,qcloud_tf_plugin_version_var);
    get_seq_string(vers_md5_line,'\t',4,aws_tf_plugin_version_var);
    if(valid_ver_or_not(terraform_version_var)!=0||valid_ver_or_not(ali_tf_plugin_version_var)!=0||valid_ver_or_not(qcloud_tf_plugin_version_var)!=0||valid_ver_or_not(aws_tf_plugin_version_var)!=0){
        return 1;
    }
    while(fgetline(file_p,vers_md5_line)==0){
        get_seq_string(vers_md5_line,' ',1,header);
        get_seq_string(vers_md5_line,' ',2,tail);
        if(valid_md5_or_not(tail)!=0){
            return 1;
        }
        if(strcmp(header,"TF_EXEC:")==0){
            strcpy(md5_tf_exec_var,tail);
        }
        else if(strcmp(header,"TF_EXEC_ZIP:")==0){
            strcpy(md5_tf_zip_var,tail);
        }
        else if(strcmp(header,"NOW_CRYPTO:")==0){
            strcpy(md5_now_crypto_var,tail);
        }
        else if(strcmp(header,"ALI_TF:")==0){
            strcpy(md5_ali_tf_var,tail);
        }
        else if(strcmp(header,"ALI_TF_ZIP:")==0){
            strcpy(md5_ali_tf_zip_var,tail);
        }
        else if(strcmp(header,"QCLOUD_TF:")==0){
            strcpy(md5_qcloud_tf_var,tail);
        }
        else if(strcmp(header,"QCLOUD_TF_ZIP:")==0){
            strcpy(md5_qcloud_tf_zip_var,tail);
        }
        else if(strcmp(header,"AWS_TF:")==0){
            strcpy(md5_aws_tf_var,tail);
        }
        else if(strcmp(header,"AWS_TF_ZIP:")==0){
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
        return -1;
    }
    fprintf(file_p,"%s\t%s\t\t%s\t\t%s\n",TERRAFORM_VERSION,ALI_TF_PLUGIN_VERSION,QCLOUD_TF_PLUGIN_VERSION,AWS_TF_PLUGIN_VERSION);
#ifdef _WIN32
    fprintf(file_p,"TF_EXEC:       %s\n",MD5_TF_EXEC_WIN);
    fprintf(file_p,"TF_EXEC_ZIP:   %s\n",MD5_TF_ZIP_WIN);
    fprintf(file_p,"NOW_CRYPTO:    %s\n",MD5_NOW_CRYPTO_WIN);
    fprintf(file_p,"ALI_TF:        %s\n",MD5_ALI_TF_WIN);
    fprintf(file_p,"ALI_TF_ZIP:    %s\n",MD5_ALI_TF_ZIP_WIN);
    fprintf(file_p,"QCLOUD_TF:     %s\n",MD5_QCLOUD_TF_WIN);
    fprintf(file_p,"QCLOUD_TF_ZIP: %s\n",MD5_QCLOUD_TF_ZIP_WIN);
    fprintf(file_p,"AWS_TF:        %s\n",MD5_AWS_TF_WIN);
    fprintf(file_p,"AWS_TF_ZIP:    %s\n",MD5_AWS_TF_ZIP_WIN);
#elif __linux__
    fprintf(file_p,"TF_EXEC:       %s\n",MD5_TF_EXEC_LIN);
    fprintf(file_p,"TF_EXEC_ZIP:   %s\n",MD5_TF_ZIP_LIN);
    fprintf(file_p,"NOW_CRYPTO:    %s\n",MD5_NOW_CRYPTO_LIN);
    fprintf(file_p,"ALI_TF:        %s\n",MD5_ALI_TF_LIN);
    fprintf(file_p,"ALI_TF_ZIP:    %s\n",MD5_ALI_TF_ZIP_LIN);
    fprintf(file_p,"QCLOUD_TF:     %s\n",MD5_QCLOUD_TF_LIN);
    fprintf(file_p,"QCLOUD_TF_ZIP: %s\n",MD5_QCLOUD_TF_ZIP_LIN);
    fprintf(file_p,"AWS_TF:        %s\n",MD5_AWS_TF_LIN);
    fprintf(file_p,"AWS_TF_ZIP:    %s\n",MD5_AWS_TF_ZIP_LIN);
#elif __APPLE__
    fprintf(file_p,"TF_EXEC:       %s\n",MD5_TF_EXEC_DWN);
    fprintf(file_p,"TF_EXEC_ZIP:   %s\n",MD5_TF_ZIP_DWN);
    fprintf(file_p,"NOW_CRYPTO:    %s\n",MD5_NOW_CRYPTO_DWN);
    fprintf(file_p,"ALI_TF:        %s\n",MD5_ALI_TF_DWN);
    fprintf(file_p,"ALI_TF_ZIP:    %s\n",MD5_ALI_TF_ZIP_DWN);
    fprintf(file_p,"QCLOUD_TF:     %s\n",MD5_QCLOUD_TF_LIN);
    fprintf(file_p,"QCLOUD_TF_ZIP: %s\n",MD5_QCLOUD_TF_ZIP_DWN);
    fprintf(file_p,"AWS_TF:        %s\n",MD5_AWS_TF_DWN);
    fprintf(file_p,"AWS_TF_ZIP:    %s\n",MD5_AWS_TF_ZIP_DWN);
#endif
    fclose(file_p);
    return 0;
}

int show_vers_md5vars(int file_flag){
    if(file_flag!=0){
        printf("Sample format - Please strictly follow it.\n\n");
        printf("The versions in the first row must follow the sequence below:\n");
        printf("1. Terraform binary version\n");
        printf("2. AliCloud provider version\n");
        printf("3. TencentCloud provider version.\n");
        printf("4. AWS provider version.\n\n"); 
        printf("%s\t%s\t%s\t%s\n",TERRAFORM_VERSION,ALI_TF_PLUGIN_VERSION,QCLOUD_TF_PLUGIN_VERSION,AWS_TF_PLUGIN_VERSION);
        printf("TF_EXEC:       %s\n",MD5_TF_EXEC_LIN);
        printf("TF_EXEC_ZIP:   %s\n",MD5_TF_ZIP_LIN);
        printf("NOW_CRYPTO:    %s\n",MD5_NOW_CRYPTO_LIN);
        printf("ALI_TF:        %s\n",MD5_ALI_TF_LIN);
        printf("ALI_TF_ZIP:    %s\n",MD5_ALI_TF_ZIP_LIN);
        printf("QCLOUD_TF:     %s\n",MD5_QCLOUD_TF_LIN);
        printf("QCLOUD_TF_ZIP: %s\n",MD5_QCLOUD_TF_ZIP_LIN);
        printf("AWS_TF:        %s\n",MD5_AWS_TF_LIN);
        printf("AWS_TF_ZIP:    %s\n",MD5_AWS_TF_ZIP_LIN);
        return 0;
    }
    FILE* file_p=fopen(VERS_MD5_CONF_FILE,"r");
    char vers_and_md5[64]="";
    if(file_p==NULL){
        return -1;
    }
    printf("Terraform\tAliCloud Provider\tTencentCloud Provider\tAWS Provider\n");
    while(fgetline(file_p,vers_and_md5)==0){
        printf("%s\n",vers_and_md5);
    }
    fclose(file_p);
    return 0;
}

int configure_vers_md5_vars(char* import_filename, int interactive_flag){
    char doubleconfirm[64]="";
    char input_filename[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(VERS_MD5_CONF_FILE)!=0){
        return -1;
    }
    printf("\n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   YOU ARE MODIFYING THE VERSIONS AND RELATED CHECKSUMS OF CORE COMPONENTS!      \n");
    printf("|*   YOUR NEED TO MAKE SURE:                                                       \n");
    printf("|*   1. Provide a plain text file which is readable or downloadable.               \n");
    printf("|*   2. Strictly follow the sample format below:                                   \n\n");
    show_vers_md5vars(1);
    printf("\n|*   3. Make *absolutely* sure that the md5sum values matches the actual files.    \n");
    printf("|*   4. How to create/get the files:                                               \n");
    printf("|*      -> Download terraform providers from https://releases.hashicorp.com/       \n");
    printf("|*      -> Download terraform binary from:                                         \n");
    printf("|*         -> https://developer.hashicorp.com/terraform/downloads                  \n");
    printf("|*      -> Build your own now-crypto.exe by using gcc or clang.                    \n");
    printf("|*   4. Build your own component repository (either URL or local path), and the    \n");
    printf("|*      tree structure is correct! (See the docs), then run 'hpcopr configloc'.    \n");
    printf("|*                                                                                 \n");
    printf("|*   You may need to run 'hpcopr repair' if your settings failed to work.          \n\n");
    printf("|*   Would you like to continue? Only 'y-e-s' is accepted to confirm:\n");

    fflush(stdin);
    printf("[ INPUT: ] ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    if(interactive_flag==0){
        printf("[ -INFO- ] Please input the path or URL of your file.\n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",input_filename);
    }
    else{
        strcpy(input_filename,import_filename);
    }
    if(*(input_filename+0)=='h'&&*(input_filename+1)=='t'&&*(input_filename+2)=='t'&&*(input_filename+3)=='p'&&*(input_filename+4)==':'){
        sprintf(cmdline,"curl -s %s -o %s",input_filename,VERS_MD5_CONF_FILE);
    }
    else if(*(input_filename+0)=='h'&&*(input_filename+1)=='t'&&*(input_filename+2)=='t'&&*(input_filename+3)=='p'&&*(input_filename+4)=='s'&&*(input_filename+5)==':'){
        sprintf(cmdline,"curl -s %s -o %s",input_filename,VERS_MD5_CONF_FILE);
    }
    else{
#ifdef _WIN32
        sprintf(cmdline,"copy /y %s %s > nul 2>&1",input_filename,VERS_MD5_CONF_FILE);
#else
        sprintf(cmdline,"/bin/cp %s %s >> /dev/null 2>&1",input_filename,VERS_MD5_CONF_FILE);
#endif
    }
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Failed to download/copy the file to the target place. Exit now.\n");
        return 1;
    }
    else{
        printf("[ -INFO- ] Successfully copied your file to the target place.\n");
        printf("|          Now checking the formats ...\n"); 
        if(get_vers_md5_vars()!=0){
            printf("[ FATAL: ] The format of the provided file is incorrect. Please check and retry.\n");
            printf("|          You may need to run 'hpcopr repair' later to recover.\n");
            return 1;
        }
        printf("[ -INFO- ] The format of the provided file is correct. Exit now.\n");
        return 0;
    }
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
    fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_TF_ROOT);
    fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",DEFAULT_URL_CODE_ROOT);
    fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",DEFAULT_URL_NOW_CRYPTO);
    fclose(file_p);
    return 0;
}

int get_locations(void){
    char header_string[64]="";
    char loc_string[LOCATION_LENGTH]="";
    char title_string[256]="";
    FILE* file_p=NULL;
    int line_location_conf_file=0;
    int i;
    if(file_exist_or_not(LOCATION_CONF_FILE)!=0){
        return -1;
    }
    line_location_conf_file=file_empty_or_not(LOCATION_CONF_FILE);
    if(file_exist_or_not(LOCATION_CONF_FILE)==0&&line_location_conf_file==LOCATION_CONF_TOTAL_LINES){
        file_p=fopen(LOCATION_CONF_FILE,"r");
        fgetline(file_p,title_string);
        for(i=0;i<LOCATION_LINES;i++){
            fscanf(file_p,"%s%s",header_string,loc_string);
            if(strcmp(header_string,"BINARY_AND_PROVIDERS_LOC_ROOT")==0){
                strcpy(URL_TF_ROOT,loc_string);
#ifdef _WIN32
                if(loc_string[1]==':'){
                    TF_LOC_FLAG=1;
                }
#else
                if(loc_string[0]=='/'){
                    TF_LOC_FLAG=1;
                }
#endif
            }
            else if(strcmp(header_string,"CLOUD_IAC_TEMPLATES_LOC_ROOT")==0){
                strcpy(URL_CODE_ROOT,loc_string);
#ifdef _WIN32
                if(loc_string[1]==':'){
                    CODE_LOC_FLAG=1;
                }
#else
                if(loc_string[0]=='/'){
                    CODE_LOC_FLAG=1;
                }
#endif
            }
            else if(strcmp(header_string,"ONLINE_SHELL_SCRIPTS_LOC_ROOT")==0){
                strcpy(URL_SHELL_SCRIPTS,loc_string);
            }
            else if(strcmp(header_string,"NOW_CRYPTO_BINARY_LOC")==0){
                strcpy(URL_NOW_CRYPTO,loc_string);
#ifdef _WIN32
                if(loc_string[1]==':'){
                    NOW_CRYPTO_LOC_FLAG=1;
                }
#else
                if(loc_string[0]=='/'){
                    NOW_CRYPTO_LOC_FLAG=1;
                }
#endif
            }
        }
        fclose(file_p);
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
        printf("[ FATAL: ] Failed to open the location configuration file. Exit now.\n");
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
    printf("\n");
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
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:\n");
    fflush(stdin);
    printf("[ INPUT: ] ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    printf("[ LOC1/4 ] Please specify the root location of the terraform binary and providers. \n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_TF_ROOT);
    printf("[ INPUT: ] ");
    fflush(stdin);
    scanf("%s",loc_string);
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf("[ -WARN- ] Invalid format. Will not modify this location.\n");
        }
        else{
            strcpy(URL_TF_ROOT,loc_string);
        }
    }
    printf("[ LOC2/4 ] Please specify the root location of the terraform templates. \n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_CODE_ROOT);
    printf("[ INPUT: ] ");
    fflush(stdin);
    scanf("%s",loc_string);
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf("[ -WARN- ] Invalid format. Will not modify this location.\n");
        }
        else{
            strcpy(URL_CODE_ROOT,loc_string);
        }
    }
    printf("[ LOC3/4 ] Please specify the root location of the *online* shell scripts.\n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_SHELL_SCRIPTS);
    printf("[ INPUT: ] ");
    fflush(stdin);
    scanf("%s",loc_string);
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf("[ -WARN- ] Invalid format. Will not modify this location.\n");
        }
        else if(format_flag==1){
            printf("[ -WARN- ] This location must be a public URL. Will not modify.\n");
        }
        else{
            strcpy(URL_SHELL_SCRIPTS,loc_string);
        }
    }

    printf("[ LOC4/4 ] Please input the root location of the now-crypto binary.\n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_NOW_CRYPTO);
    printf("[ INPUT: ] ");
    fflush(stdin);
    scanf("%s",loc_string);
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf("[ -WARN- ] Invalid format. Will not modify this location.\n");
        }
        else{
            strcpy(URL_NOW_CRYPTO,loc_string);
        }
    }

    printf("[ -INFO- ] Updating the location configuration file now ... \n");
    file_p=fopen(LOCATION_CONF_FILE,"w+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to create or modify the target file. Exit now.\n");
        return -1;
    }
    fprintf(file_p,"*VERY IMPORTANT*: THIS FILE IS GENERATED AND MANAGED BY THE HPC-NOW SERVICES! *DO NOT* MODIFY OR HANDLE THIS FILE MANUALLY!\n");
    if(strlen(URL_TF_ROOT)==0){
        fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_TF_ROOT);
    }
    else{
        fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",URL_TF_ROOT);
    }
    if(strlen(URL_CODE_ROOT)==0){
        fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",DEFAULT_URL_CODE_ROOT);
    }
    else{
        fprintf(file_p,"CLOUD_IAC_TEMPLATES_LOC_ROOT %s\n",URL_CODE_ROOT);
    }
    if(strlen(URL_SHELL_SCRIPTS)==0){
        fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",DEFAULT_URL_SHELL_SCRIPTS);
    }
    else{
        fprintf(file_p,"ONLINE_SHELL_SCRIPTS_LOC_ROOT %s\n",URL_SHELL_SCRIPTS);
    }
    if(strlen(URL_NOW_CRYPTO)==0){
        fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",DEFAULT_URL_NOW_CRYPTO);
    }
    else{
        fprintf(file_p,"NOW_CRYPTO_BINARY_LOC %s\n",URL_NOW_CRYPTO);
    }
    fclose(file_p);
    printf("[ -DONE- ] Locations are modified and saved. The latest locations:\n");
    show_locations();
    return 0;
}