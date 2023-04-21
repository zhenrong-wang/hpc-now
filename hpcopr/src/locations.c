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

#ifndef LOCATION_CONF_TOTAL_LINES
#define LOCATION_CONF_TOTAL_LINES 5
#endif
#ifndef LOCATION_LINES
#define LOCATION_LINES 4
#endif

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
    FILE* file_p=NULL;
    file_p=fopen(LOCATION_CONF_FILE,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"*VERY IMPORTANT*: THIS FILE IS GENERATED AND MANAGED BY THE HPC-NOW SERVICES! *DO NOT* MODIFY OR HANDLE THIS FILE MANUALLY!\n");
    fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_REPO_ROOT);
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
                strcpy(URL_REPO_ROOT,loc_string);
#ifdef _WIN32
                if(loc_string[1]==':'){
                    REPO_LOC_FLAG=1;
                }
#else
                if(loc_string[0]=='/'){
                    REPO_LOC_FLAG=1;
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
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|*                                                                                 *\n");
    printf("|*   YOU ARE MODIFYING THE LOCATIONS OF COMPONENTS FOR THE HPC-NOW SERVICES!       *\n");
    printf("|*   YOUR NEED TO MAKE SURE:                                                       *\n");
    printf("|*   1. The locations - either URLs or local filesystem paths are valid.           *\n");
    printf("|*        URLs       : *MUST* start with 'http://' or 'https://' , root locations  *\n");
    printf("|*                     *MUST* end with '/'                                         *\n");
    printf("|*        Local Paths: *MUST* be absolute paths. For GNU/Linux and macOS, the      *\n");
    printf("|                       locations must start with '/'; for Microsoft Windows, the  *\n");
    printf("|                       locations must start with DRIVE_LETTER:\\                   *\n");              
    printf("|*   2. The structures of the location are valid. Please refer to the docs and     *\n");
    printf("|*      confirm your structure in advance.                                         *\n");
    printf("|*                                                                                 *\n");
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|*                                                                                 *\n");
    printf("|*   THE HPCOPR WILL ONLY CHECK THE FORMAT OF YOUR INPUTS, WILL *NOT* CHECK        *\n");
    printf("|*   WHETHER LOCATIONS ARE VALID OR NOT. IT'S YOUR JOB TO GUARANTEE THE VALIDITY!  *\n");
    printf("|*   INVALID LOCATIONS MAY DAMAGE THE HPC-NOW SERVICES! YOU MAY NEED TO RESET TO   *\n");
    printf("|*   THE DEFAULT LOCATIONS IF YOUR LOCATIONS FAIL TO WORK PROPERLY!                *\n");
    printf("|*                                                                                 *\n");
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:\n");
    fflush(stdin);
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    printf("[ LOC1/4 ] Please specify the root location of the terraform binary and providers. \n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_REPO_ROOT);
    printf("[ INPUT: ]  ");
    fflush(stdin);
    scanf("%s",loc_string);
    if(strcmp(loc_string,"default")!=0){
        format_flag=valid_loc_format_or_not(loc_string);
        if(format_flag==-1){
            printf("[ -WARN- ] Invalid format. Will not modify this location.\n");
        }
        else{
            strcpy(URL_REPO_ROOT,loc_string);
        }
    }
    printf("[ LOC2/4 ] Please specify the root location of the terraform templates. \n");
    printf("|          You can input 'defaut' to use default location below: \n");
    printf("|          -> %s \n",DEFAULT_URL_CODE_ROOT);
    printf("[ INPUT: ]  ");
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
    printf("[ INPUT: ]  ");
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
    printf("[ INPUT: ]  ");
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
    if(strlen(URL_REPO_ROOT)==0){
        fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",DEFAULT_URL_REPO_ROOT);
    }
    else{
        fprintf(file_p,"BINARY_AND_PROVIDERS_LOC_ROOT %s\n",URL_REPO_ROOT);
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