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
#include "cluster_general_funcs.h"
#include "general_print_info.h"
#include "cluster_operations.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern int code_loc_flag_var;

void get_workdir(char* cluster_workdir, char* cluster_name){
#ifdef _WIN32
    sprintf(cluster_workdir,"%s\\workdir\\%s\\",HPC_NOW_ROOT_DIR,cluster_name);
#else
    sprintf(cluster_workdir,"%s/workdir/%s/",HPC_NOW_ROOT_DIR,cluster_name);
#endif
}

int create_cluster_registry(void){
    FILE* file_p=NULL;
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)==0){
        return 0;
    }
    file_p=fopen(ALL_CLUSTER_REGISTRY,"w+");
    if(file_p==NULL){
        return 1;
    }
    else{
        fclose(file_p);
        return 0;
    }
}

/*  
 * If silent_flag==1, verbose. Will tell the user which cluster is active
 * If silent_flag==0, silent. Will print nothing
 * If silent_flag== other_number, Will only show the warning
 */

int show_current_cluster(char* cluster_workdir, char* current_cluster_name, int silent_flag){
    FILE* file_p=NULL;
    if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0||file_empty_or_not(CURRENT_CLUSTER_INDICATOR)==0){
        if(silent_flag!=0){
            printf("[ -WARN- ] Currently you are not operating any cluster.\n");
        }
        return 1;
    }
    else{
        file_p=fopen(CURRENT_CLUSTER_INDICATOR,"r");
        fscanf(file_p,"%s",current_cluster_name);
        if(silent_flag==1){
            printf("[ -INFO- ] Current cluster: %s\n",current_cluster_name);
        }
        fclose(file_p);
        get_workdir(cluster_workdir,current_cluster_name);
        return 0;
    }
}

int current_cluster_or_not(char* current_indicator, char* cluster_name){
    char current_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    FILE* file_p=fopen(current_indicator,"r");
    if(file_p==NULL){
        return 1;
    }
    fscanf(file_p,"%s",current_cluster_name);
    if(strcmp(current_cluster_name,cluster_name)!=0){
        return -1;
    }
    return 0;
}

int cluster_name_check_and_fix(char* cluster_name, char* cluster_name_output){
    int i, name_flag;
    char real_cluster_name_with_prefix[LINE_LENGTH_SHORT]="";
    if(strlen(cluster_name)==0){
        generate_random_string(cluster_name_output);
        name_flag=-1;
    }
    for(i=0;i<strlen(cluster_name);i++){
        if(*(cluster_name+i)=='-'||*(cluster_name+i)=='0'||*(cluster_name+i)=='9'){
            continue;
        }
        if(*(cluster_name+i)>'0'&&*(cluster_name+i)<'9'){
            continue;
        }
        if(*(cluster_name+i)<'A'||*(cluster_name+i)>'z'){
            return 127;
        }
        else if(*(cluster_name+i)>'Z'&&*(cluster_name+i)<'a'){
            return 127;
        }
    }
    if(strlen(cluster_name)<CLUSTER_ID_LENGTH_MIN){
        sprintf(cluster_name_output,"%s-hpcnow",cluster_name);
        name_flag=1;
    }
    else if(strlen(cluster_name)>CLUSTER_ID_LENGTH_MAX){
        for(i=0;i<CLUSTER_ID_LENGTH_MAX;i++){
            *(cluster_name_output+i)=*(cluster_name+i);
        }
        *(cluster_name_output+CLUSTER_ID_LENGTH_MAX)='\0';
        name_flag=2;
    }
    else{
        strcpy(cluster_name_output,cluster_name);
        name_flag=0;
    }
    sprintf(real_cluster_name_with_prefix,"< cluster name: %s >",cluster_name_output);
    if(find_multi_keys(ALL_CLUSTER_REGISTRY,real_cluster_name_with_prefix,"","","","")>0){
        return -127;
    }
    return name_flag;
}

int exit_current_cluster(void){
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s > nul 2>&1",CURRENT_CLUSTER_INDICATOR);
#else
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",CURRENT_CLUSTER_INDICATOR);
#endif
    return system(cmdline);
    
}

int switch_to_cluster(char* target_cluster_name){
    char* current_cluster=CURRENT_CLUSTER_INDICATOR;
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_workdir[DIR_LENGTH]="";
    FILE* file_p=NULL;
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        printf("[ FATAL: ] The specified cluster name is not in the registry. Exit now.\n");
        return 1;
    }
    if(show_current_cluster(temp_workdir,temp_cluster_name,0)==0){
        if(strcmp(temp_cluster_name,target_cluster_name)==0){
            printf("[ -INFO- ] You are operating the cluster %s now. No need to switch.\n",target_cluster_name);
            return 1;
        }
    }
    file_p=fopen(current_cluster,"w+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to create current cluster indicator. Exit now.\n");
        return -1;
    }
    fprintf(file_p,"%s",target_cluster_name);
    fclose(file_p);
    printf("[ -INFO- ] Successfully switched to the cluster %s.\n",target_cluster_name);
    return 0;
}

int add_to_cluster_registry(char* new_cluster_name){
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    FILE* file_p=fopen(cluster_registry,"a+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to open/write to the cluster registry. Exit now.");
        return -1;
    }
    fprintf(file_p,"< cluster name: %s >\n",new_cluster_name);
    fclose(file_p);
    return 0;
}

int delete_from_cluster_registry(char* deleted_cluster_name){
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    char deleted_cluster_name_with_prefix[LINE_LENGTH_SHORT]="";
    char filename_temp[FILENAME_LENGTH]="";
    char temp_line[LINE_LENGTH_SHORT]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    FILE* file_p_tmp=NULL;
    int replace_flag;
    sprintf(deleted_cluster_name_with_prefix,"< cluster name: %s >",deleted_cluster_name);
    replace_flag=global_replace(cluster_registry,deleted_cluster_name_with_prefix,"");
    if(replace_flag!=0){
        printf("[ FATAL: ] Failed to delete the cluster %s from the registry.\n",deleted_cluster_name);
    }
    sprintf(filename_temp,"%s.tmp",cluster_registry);
    file_p=fopen(cluster_registry,"r");
    file_p_tmp=fopen(filename_temp,"w+");
    while(fgetline(file_p,temp_line)==0){
        if(strlen(temp_line)!=0){
            fprintf(file_p_tmp,"%s\n",temp_line);
        }
    }
    fprintf(file_p,"%s\n",temp_line);
    fclose(file_p);
    fclose(file_p_tmp);
    if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,deleted_cluster_name)==0){
        exit_current_cluster();
    }
#ifdef _WIN32
    sprintf(cmdline,"move /y %s %s > nul 2>&1",filename_temp,cluster_registry);
#else
    sprintf(cmdline,"mv %s %s > /dev/null 2>&1",filename_temp,cluster_registry);
#endif
    return system(cmdline);
}

int list_all_cluster_names(void){
    FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"r");
    char registry_line[LINE_LENGTH_SHORT]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
//    int getline_flag=0;
    if(file_p==NULL){
        printf("[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now.\n");
        return -1;
    }
    printf("[ -INFO- ] List of all the clusters:\n");
    while(fgetline(file_p,registry_line)!=1){
        if(strlen(registry_line)!=0){
            if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0){
                printf("|          %s\n",registry_line);
            }
            else{
                get_seq_string(registry_line,' ',4,temp_cluster_name);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf("|  active: %s\n",registry_line);
                }
                else{
                    printf("|          %s\n",registry_line);
                }
            }
        }
    }
    return 0;
}

int glance_clusters(char* target_cluster_name, char* crypto_keyfile){
    FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"r");
    char registry_line[LINE_LENGTH_SHORT]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_cluster_workdir[DIR_LENGTH]="";
    if(file_p==NULL){
        printf("[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now.\n");
        return -1;
    }
    if(strlen(target_cluster_name)==0){
        if(show_current_cluster(temp_cluster_workdir,temp_cluster_name,0)==1){
            return 1;
        }
        else{
            decrypt_files(temp_cluster_workdir,crypto_keyfile);
            printf("|  active: <> %s | ",temp_cluster_name);
            if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
                printf(" * EMPTY CLUSTER *\n");
            }
            delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            return 0;
        }
    }
    if(strcmp(target_cluster_name,"all")==0||strcmp(target_cluster_name,"ALL")==0||strcmp(target_cluster_name,"All")==0){
        while(fgetline(file_p,registry_line)==0){
//            printf("test### %s\n",registry_line);
            if(strlen(registry_line)!=0){
                get_seq_string(registry_line,' ',4,temp_cluster_name);
//                printf("test### %s\n",registry_line);
                get_workdir(temp_cluster_workdir,temp_cluster_name);
//                printf("test### %s\n",registry_line);
                decrypt_files(temp_cluster_workdir,crypto_keyfile);
//                printf("test### %s\n",registry_line);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf("|  active: <> %s | ",temp_cluster_name);
                }
                else{
                    printf("|          <> %s | ",temp_cluster_name);
                }
                if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
                    printf("* EMPTY CLUSTER *\n");
                }
                delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            }
        }
        fclose(file_p);
        return 0;
    }
    fclose(file_p);
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        return -1;
    }
    else{
        get_workdir(temp_cluster_workdir,target_cluster_name);
        decrypt_files(temp_cluster_workdir,crypto_keyfile);
        if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
            printf("|  active: <> %s | ",temp_cluster_name);
        }
        else{
            printf("|          <> %s | ",temp_cluster_name);
        }
        if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
            printf(" * EMPTY CLUSTER *\n");
        }
        delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        return 0;
    }
}

int remove_cluster(char* target_cluster_name, char*crypto_keyfile){
    char cluster_workdir[DIR_LENGTH]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        printf("[ FATAL: ] The specified cluster name %s is not in the registry.\n",target_cluster_name);
        list_all_cluster_names();
        return 1;
    }
    get_workdir(cluster_workdir,target_cluster_name);
    if(cluster_empty_or_not(cluster_workdir)!=0){
        printf("[ -WARN- ] The specified cluster is *NOT* empty!\n");
        glance_clusters(target_cluster_name,crypto_keyfile);
        printf("[ -WARN- ] Would you like to remove it anyway? This operation is *NOT* recoverable!\n");
        printf("[ INPUT: ] Only 'y-e-s' is accepted to continuie: ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")==0){
            printf("[ -WARN- ] Please type the cluster name %s to confirm. This opeartion is\n",target_cluster_name);
            printf("|          absolutely *NOT* recoverable!\n");
            printf("[ INPUT: ] ");
            fflush(stdin);
            scanf("%s",doubleconfirm);
            if(strcmp(doubleconfirm,target_cluster_name)==0){
                if(cluster_destroy(cluster_workdir,crypto_keyfile,0)==0){
                    delete_from_cluster_registry(target_cluster_name);
                }
                else{
                    return 1;
                }
            }
            else{
                printf("[ -INFO- ] Only %s is accepted to confirm. You chose to deny this operation.\n",target_cluster_name);
                printf("|          Nothing changed.\n");
                return 1;
            }
        }
        else{
            printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
            printf("|          Nothing changed.\n");
            return 1;
        }
    }
    else{
        printf("[ -INFO- ] The specified cluster is empty. This operation will remove all the related files\n");
        printf("|          from your system and registry. Would you like to continue?\n");
        printf("[ INPUT: ] Only 'y-e-s' is accepted to continuie: ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
            printf("|          Nothing changed.\n");
            return 1;
        }
    }
#ifdef _WIN32
    sprintf(cmdline,"rd /q /s %s > nul 2>&1",cluster_workdir);
#else
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",cluster_workdir);
#endif
    printf("[ -INFO- ] Removing all the related files ...\n");
    system(cmdline);
    printf("[ -INFO- ] Deleting the cluster from the registry ...\n");
    delete_from_cluster_registry(target_cluster_name);
    printf("[ -DONE- ] The cluster %s has been removed completely.\n",target_cluster_name);
    return 0;
}

int create_new_cluster(char* crypto_keyfile, char* cluster_name, char* cloud_ak, char* cloud_sk){
    char cmdline[CMDLINE_LENGTH]="";
    char real_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char input_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char filename_temp[FILENAME_LENGTH]="";
    int cluster_name_check_flag=0;
    FILE* file_p=NULL;
    char new_workdir[DIR_LENGTH]="";
    char new_vaultdir[DIR_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    int ak_length,sk_length;
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    char* current_cluster=CURRENT_CLUSTER_INDICATOR;
    char doubleconfirm[64]="";
    if(file_exist_or_not(crypto_keyfile)!=0){
        return -1;
    }
    file_p=fopen(cluster_registry,"a+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to open/write to the cluster registry. Exit now.");
        return -1;
    }
    fclose(file_p);
    file_p=fopen(current_cluster,"w+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to create the current cluster indicator. Exit now.");
        return -1;
    }
    fclose(file_p);
    if(strlen(cluster_name)==0){
        printf("[ -INFO- ] Please input the cluster name (A-Z | a-z | 0-9 | - , maximum length %d):\n",CLUSTER_ID_LENGTH_MAX);
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",input_cluster_name);
    }
    else{
        strcpy(input_cluster_name,cluster_name);
    }
    cluster_name_check_flag=cluster_name_check_and_fix(input_cluster_name,real_cluster_name);
    if(cluster_name_check_flag==127){
        printf("[ FATAL: ] The cluster name only accepts English letters 'A-Z', 'a-z', '0-9' and '-'.\n");
        printf("|          The specified name %s contains illegal characters.\n",input_cluster_name);
        printf("|          Please check and retry. Exit now.\n");
        return 1;
    }
    else if(cluster_name_check_flag==-127){
        printf("[ FATAL: ] The specified cluster name %s already exists in the registry.\n",input_cluster_name);
        printf("|          Please check and retry. Exit now.\n");
        return 1;
    }
    else if(cluster_name_check_flag==1){
        printf("[ -WARN- ] The specified cluster name length <%d, add '-hpcnow'.\n",CLUSTER_ID_LENGTH_MIN);
    }
    else if(cluster_name_check_flag==2){
        printf("[ -WARN- ] The specified cluster name length > %d, cut to %d.\n",CLUSTER_ID_LENGTH_MAX,CLUSTER_ID_LENGTH_MAX);
    }
    else if(cluster_name_check_flag==-1){
        printf("[ -WARN- ] Would you like to use the random string %s as a cluster name? \n",real_cluster_name);
        printf("|          Only 'y-e-s' is accepted as a confirmation. \n");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
            printf("|          Nothing changed.\n");
            return 3;
        }
    }
    printf("[ -INFO- ] Using the cluster name %s.\n",real_cluster_name);
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
#ifdef _WIN32
    strcpy(filename_temp,"c:\\programdata\\secret.tmp.txt");
#else
    strcpy(filename_temp,"/tmp/secret.tmp.txt");
#endif
    }
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return -1;
    }
    printf("[ -INFO- ] Please input your secrets key pair:\n");
    printf("[ INPUT: ] Access key ID :");
    fflush(stdin);
    scanf("%s",access_key);
    printf("[ INPUT: ] Access secrets:");
    fflush(stdin);
    scanf("%s",secret_key);
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);
    if(ak_length==24&&sk_length==30){
        fprintf(file_p,"%s\n%s\nCLOUD_A",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        fprintf(file_p,"%s\n%s\nCLOUD_B",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        fprintf(file_p,"%s\n%s\nCLOUD_C",access_key,secret_key);
        fclose(file_p);
    }
    else{
        printf("[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.\n");
        fclose(file_p);
#ifdef _WIN32
        sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
        system(cmdline);
        return 5;
    }
#ifdef _WIN32
    sprintf(new_workdir,"%s\\workdir\\%s\\",HPC_NOW_ROOT_DIR,real_cluster_name);
    sprintf(cmdline,"mkdir %s > nul 2>&1",new_workdir);
    system(cmdline);
    create_and_get_vaultdir(new_workdir,new_vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s\\.secrets.txt %s",now_crypto_exec,filename_temp,new_vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#elif __linux__
    sprintf(new_workdir,"%s/workdir/%s",HPC_NOW_ROOT_DIR,real_cluster_name);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",new_workdir);
    system(cmdline);
    create_and_get_vaultdir(new_workdir,new_vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s/.secrets.txt %s",now_crypto_exec,filename_temp,new_vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#elif __APPLE__
    sprintf(new_workdir,"%s/workdir/%s",HPC_NOW_ROOT_DIR,real_cluster_name);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",new_workdir);
    system(cmdline);
    create_and_get_vaultdir(new_workdir,new_vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s/.secrets.txt %s",now_crypto_exec,filename_temp,new_vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    add_to_cluster_registry(real_cluster_name);
    switch_to_cluster(real_cluster_name);
    printf("[ -INFO- ] The secrets key pair has been encrypted and stored locally. You can either:\n");
    printf("|          1. run 'hpcopr init' to create a default cluster. OR\n");
    printf("|          2. run 'hpcopr get-conf' to get the default cluster configuration, and run\n");
    printf("|              'hpcopr init' to create a customized cluster.\n");
    printf("|          You can also switch to this cluster name and operate this cluster later.\n");
    printf("[ -DONE- ] Exit now.\n");
    return 0;
}

int rotate_new_keypair(char* workdir, char* cloud_ak, char* cloud_sk, char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char *now_crypto_exec=NOW_CRYPTO_EXEC;
    int ak_length,sk_length;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[32]="";
    char doubleconfirm[32]="";

    char access_key_prev[AKSK_LENGTH]="";
    char secret_key_prev[AKSK_LENGTH]="";
    char cloud_flag_prev[32]="";
    char md5sum[33]="";

#ifdef _WIN32
    if(folder_exist_or_not("c:\\programdata\\hpc-now")==0){
        strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
    }
    else{
        strcpy(filename_temp,"c:\\programdata\\secret.tmp.txt");
    }
#else
    strcpy(filename_temp,"/tmp/secret.tmp.txt");
#endif
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf("[ FATAL: ] Failed to create a temporary file in your system.\n");
        printf("|          Please check the available disk space. Exit now.\n");
        return -1;
    }

    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp2,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp2,"%s/.secrets.txt",vaultdir);
#endif
    if(file_exist_or_not(filename_temp2)!=0){
        printf("[ FATAL: ] Currently there is no secrets keypair. This working directory may be\n");
        printf("|          corrputed, which is very unusual. Please contact us via:\n");
        printf("|          info@hpc-now.com for troubleshooting. Exit now.\n");
        return -1;
    }
    printf("\n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   YOU ARE ROTATING THE CLOUD KEYPAIR, WHICH MAY DAMAGE THIS CLUSTER.            \n");
    printf("|*   BEFORE PROCEEDING, PLEASE MAKE SURE:                                          \n");
    printf("|*                                                                                 \n");
    printf("|*   1. Your new key pair comes from the *SAME* cloud vendor and account.          \n");
    printf("|*      This is * !!! EXTREMELY IMPORTANT !!! *                                    \n");
    printf("|*   2. Your new key pair is valid and able to manage cloud resources.             \n");
    printf("|*      This is * !!! VERY IMPORTANT !!! *                                         \n");
    printf("|*                                                                                 \n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          \n");
    printf("|*                                                                                 \n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("| ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:\n");
    printf("[ INPUT: ] ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }

    get_ak_sk(filename_temp2,crypto_keyfile,access_key_prev,secret_key_prev,cloud_flag_prev);
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
        printf("[ -INFO- ] Please input your new secrets key pair:\n");
        printf("[ INPUT: ] Access key ID :");
        fflush(stdin);
        scanf("%s",access_key);
        printf("[ INPUT: ] Access secrets:");
        fflush(stdin);
        scanf("%s",secret_key);
    }
    else{
        strcpy(access_key,cloud_ak);
        strcpy(secret_key,cloud_sk);
    }
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);
    if(ak_length==24&&sk_length==30){
        strcpy(cloud_flag,"CLOUD_A");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: AliCloud (HPC-NOW code: CLOUD_A).\n");
            printf("|          Please rotate a keypair from an AliCloud account.\n");
            printf("[ FATAL: ] Exit now.\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_A",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_B");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: TencentCloud (HPC-NOW code: CLOUD_B).\n");
            printf("|          Please rotate a keypair from an TencentCloud account.\n");
            printf("[ FATAL: ] Exit now.\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_B",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        strcpy(cloud_flag,"CLOUD_C");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf("[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: Amazon Web Services (HPC-NOW code: CLOUD_C).\n");
            printf("|          Please rotate a keypair from an Amazon Web Services account.\n");
            printf("[ FATAL: ] Exit now.\n");
            return 1;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_C",access_key,secret_key);
        fclose(file_p);
    }
    else{
        printf("[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.\n");
        fclose(file_p);
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
        system(cmdline);
        return 1;
    }
    get_crypto_key(crypto_keyfile,md5sum);
#ifdef _WIN32
    sprintf(cmdline,"%s encrypt %s %s\\.secrets.txt %s",now_crypto_exec,filename_temp,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
    sprintf(cmdline,"%s encrypt %s %s/.secrets.txt %s",now_crypto_exec,filename_temp,vaultdir,md5sum);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(filename_temp2,"%s\\hpc_stack_base.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf.tmp",stackdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(filename_temp2,"%s/hpc_stack_base.tf",stackdir);
#endif
        sprintf(cmdline,"%s decrypt %s %s %s",now_crypto_exec,filename_temp,filename_temp2,md5sum);
        system(cmdline);
        global_replace(filename_temp2,access_key_prev,access_key);
        global_replace(filename_temp2,secret_key_prev,secret_key);
        sprintf(cmdline,"%s encrypt %s %s %s",now_crypto_exec,filename_temp2,filename_temp,md5sum);
        system(cmdline);
    }
    printf("[ -INFO- ] The new secrets key pair has been encrypted and rotated locally.\n");
    printf("[ -DONE- ] Exit now.\n");
    return 0;
}

int cluster_destroy(char* workdir, char* crypto_keyfile, int force_flag){
    char doubleconfirm[32]="";
    char cloud_flag[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cmdline[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[LINE_LENGTH];
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_folder=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    char md5sum[33]="";
    char master_address[32]="";
    char bucket_address[32]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    int i;
    int compute_node_num=0;
    printf("\n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   YOU ARE DELETING THE WHOLE CLUSTER - INCLUDING ALL THE NODES AND *DATA*!      \n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          \n");
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
    else{
        printf("[ -INFO- ] Cluster operation started ...\n");
    }
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    reset_string(buffer1);
    reset_string(buffer2);
    get_crypto_key(crypto_keyfile,md5sum);
#ifdef _WIN32
    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    find_and_get(filename_temp,"Master Node IP:","","",1,"Master Node IP:","","",' ',4,master_address);
    find_and_get(filename_temp,"NetDisk Address:","","",1,"NetDisk Address:","","",' ',4,bucket_address);
    sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_temp);
#else
    sprintf(cmdline,"%s decrypt %s/_CLUSTER_SUMMARY.txt %s/_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
    system(cmdline);
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
    find_and_get(filename_temp,"Master Node IP:","","",1,"Master Node IP:","","",' ',4,master_address);
    find_and_get(filename_temp,"NetDisk Address:","","",1,"NetDisk Address:","","",' ',4,bucket_address);
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_temp);
#endif
    system(cmdline);
    if(strcmp(cloud_flag,"CLOUD_C")==0&&force_flag==1){
#ifdef _WIN32
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/bin/s3cmd del -rf s3://%s\" > nul 2>&1",sshkey_folder,master_address,bucket_address);
#else
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s/now-cluster-login root@%s \"/bin/s3cmd del -rf s3://%s\" >> /dev/null 2>&1",sshkey_folder,master_address,bucket_address);
#endif
        system(cmdline);
    }
    else if(strcmp(cloud_flag,"CLOUD_A")==0&&force_flag==1){
#ifdef _WIN32
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/usr/bin/ossutil64 rm -rf oss://%s\" > nul 2>&1",sshkey_folder,master_address,bucket_address);
#else
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s/now-cluster-login root@%s \"/usr/bin/ossutil64 rm -rf oss://%s\" >> /dev/null 2>&1",sshkey_folder,master_address,bucket_address);
#endif
        system(cmdline);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0&&force_flag==1){
#ifdef _WIN32
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s\\now-cluster-login root@%s \"/usr/local/bin/coscmd delete -rf /\" > nul 2>&1",sshkey_folder,master_address);
#else
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s/now-cluster-login root@%s \"/usr/local/bin/coscmd delete -rf /\" >> /dev/null 2>&1",sshkey_folder,master_address);
#endif
        system(cmdline);
    }
    decrypt_files(workdir,crypto_keyfile);
    create_and_get_stackdir(workdir,stackdir);
    if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)!=0){
        printf("[ -WARN- ] Some problems occoured. Retrying destroy now (1/2)...\n");
        sleep(2);
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)!=0){
            printf("[ -WARN- ] Some problems occoured. Retrying destroy now (2/2)...\n");
            sleep(2);
            if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log)!=0){
                printf("[ FATAL: ] Failed to destroy your cluster. This usually caused by either Terraform or\n");
                printf("|          the providers developed and maintained by cloud service providers.\n");
                printf("|          You *MUST* manually destroy the remaining cloud resources of this cluster.\n");
                printf("|          Exit now.\n");
                return -1;
            }
        }
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    compute_node_num=get_compute_node_num(filename_temp,"all");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
    for(i=0;i<compute_node_num;i++){
        sprintf(string_temp,"compute%d",i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    delete_decrypted_files(workdir,crypto_keyfile);
#ifdef _WIN32
    system("del /f /q /s c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
    sprintf(cmdline,"move %s\\*.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\*.tmp c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\currentstate > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"del /f /q %s\\compute_template > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\hostfile_latest c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\_CLUSTER_SUMMARY.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\UCID_LATEST.txt c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"move %s\\conf\\tf_prep.conf %s\\conf\\tf_prep.conf.destroyed > nul 2>&1",workdir,workdir);
    system(cmdline);
#elif __APPLE__
    system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    sprintf(cmdline,"mv %s/*.tf /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/*.tmp /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/currentstate >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/compute_template >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hostfile_latest /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/_CLUSTER_SUMMARY.txt /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/UCID_LATEST.txt /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
    system(cmdline);
#elif __linux__
    system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
    sprintf(cmdline,"mv %s/*.tf /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/*.tmp /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/currentstate >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"rm -rf %s/compute_template >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/hostfile_latest /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/_CLUSTER_SUMMARY.txt /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/UCID_LATEST.txt /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"mv %s/conf/tf_prep.conf %s/conf/tf_prep.conf.destroyed >> /dev/null 2>&1",workdir,workdir);
    system(cmdline);
#endif
    printf("[ -DONE- ] The whole cluster has been destroyed successfully.\n");
    printf("|          You can run 'init' command to rebuild it.\n");
    printf("|          However, all the data has been erased.\n");
    return 0;
}

int delete_compute_node(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    int del_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing deleted. Exit now.\n");
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
                return -1;
            }
            del_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(del_num==0){
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
            return 1;
        }
        if(del_num>compute_node_num){
            printf("[ -INFO- ] You specified a number larger than the quantity of compute nodes.\n");
            printf("           Do you mean deleting *ALL* the compute nodes?\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("[ -INFO- ] You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You specified to delete %d from %d compute node(s).",del_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);

#ifdef _WIN32
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                system("del /f /s /q c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
                sprintf(cmdline,"move %s\\hpc_stack_compute%d.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1", stackdir,i);
                system(cmdline);
            }
#elif __APPLE__
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
                sprintf(cmdline,"mv %s/hpc_stack_compute%d.tf /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1", stackdir,i);
                system(cmdline);
            }
#elif __linux__
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
                sprintf(cmdline,"mv %s/hpc_stack_compute%d.tf /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1", stackdir,i);
                system(cmdline);
            }
#endif
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
                return -1;
            }
            printf("[ -INFO- ] After the cluster operation:\n|\n");
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.\n");
            return 0;
        }
    }
    sprintf(string_temp,"[ -INFO- ] You specified to delete *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
#ifdef _WIN32
    for(i=1;i<compute_node_num+1;i++){
        system("del /f /s /q c:\\programdata\\hpc-now\\.destroyed\\* > nul 2>&1");
        sprintf(cmdline,"move %s\\hpc_stack_compute%d.tf c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1", stackdir,i);
        system(cmdline);
    }
#elif __APPLE__
    for(i=1;i<compute_node_num+1;i++){
        system("rm -rf /Applications/.hpc-now/.destroyed/* >> /dev/null 2>&1");
        sprintf(cmdline,"mv %s/hpc_stack_compute%d.tf /Applications/.hpc-now/.destroyed/ >> /dev/null 2>&1", stackdir,i);
        system(cmdline);
    }
#elif __linux__
    for(i=1;i<compute_node_num+1;i++){
        system("rm -rf /usr/.hpc-now/.destroyed/* >> /dev/null 2>&1");
        sprintf(cmdline,"mv %s/hpc_stack_compute%d.tf /usr/.hpc-now/.destroyed/ >> /dev/null 2>&1", stackdir,i);
        system(cmdline);
    }
#endif
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.\n");
    return 0;
}

int add_compute_node(char* workdir, char* crypto_keyfile, char* add_number_string){
    char string_temp[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    int add_number=0;
    int current_node_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    if(strlen(add_number_string)>2||strlen(add_number_string)<1){
        printf("[ FATAL: ] The number of nodes to be added is invalid. A number (1-%d) is needed.\n",MAXIMUM_ADD_NODE_NUMBER);
        printf("           Exit now.\n");
        return -1;
    }
    for(i=0;i<strlen(add_number_string);i++){
        if(*(add_number_string+i)<'0'||*(add_number_string+i)>'9'){
            printf("[ FATAL: ] The number of nodes to be added is invalid. A number (1-%d) is needed.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("           Exit now.\n");
            return -1;
        }
        else{
            add_number+=(*(add_number_string+i)-'0')*pow(10,strlen(add_number_string)-i-1);
        }
    }

    if(add_number>MAXIMUM_ADD_NODE_NUMBER||add_number<1){
        printf("[ FATAL: ] The number of nodes to be added is out of range (1-%d). Exit now.\n",MAXIMUM_ADD_NODE_NUMBER);
        return -1;
    }
    sprintf(string_temp,"[ -INFO- ] You specified to add %d compute node(s).",add_number);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    printf("[ -INFO- ] The cluster operation is in progress ...\n");
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    current_node_num=get_compute_node_num(filename_temp,"all");
    for(i=0;i<add_number;i++){
#ifdef _WIN32
        sprintf(cmdline,"copy /y %s\\compute_template %s\\hpc_stack_compute%d.tf > nul 2>&1",stackdir,stackdir,i+1+current_node_num);
        system(cmdline);
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i+1+current_node_num);
#else
        sprintf(cmdline,"/bin/cp %s/compute_template %s/hpc_stack_compute%d.tf >> /dev/null 2>&1",stackdir,stackdir,i+1+current_node_num);
        system(cmdline);
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i+1+current_node_num);
#endif
        sprintf(string_temp,"compute%d",i+1+current_node_num);
        global_replace(filename_temp,"compute1",string_temp);
        sprintf(string_temp,"comp%d",i+1+current_node_num);
        global_replace(filename_temp,"comp1",string_temp);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=0;i<add_number;i++){
        sprintf(string_temp,"compute%d",current_node_num+i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been added.\n");
    return 0;
}

int shutdown_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    int down_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing to be shutdown. Exit now.\n");
        return -1;
    }
    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
                return -1;
            }
            down_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(down_num==0){
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
            return 1;
        }
        if(down_num>compute_node_num){
            printf("[ -INFO- ] You specified a number larger than the quantity of compute nodes.\n");
            printf("           Do you mean shutting down *ALL* the compute nodes?\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("[ -INFO- ] You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You planned to shutdown %d from %d compute node(s).",down_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
#ifdef _WIN32
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
                sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
                if(strcmp(cloud_flag,"CLOUD_A")==0){
                    global_replace(filename_temp,"Running","Stopped");
                }
                else if(strcmp(cloud_flag,"CLOUD_B")==0){
                    find_and_replace(filename_temp,"running_flag","","","","","true","false");
                }
                else if(strcmp(cloud_flag,"CLOUD_C")==0){
                    global_replace(filename_temp,"running","stopped");
                }
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
                return -1;
            }
            printf("[ -INFO- ] After the cluster operation:\n|\n");
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been deleted.\n");
            return 0;
        }
    }
    sprintf(string_temp,"[ -INFO- ] You planned to shutdown *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Running","Stopped");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","true","false");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"running","stopped");
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been shut down.\n");
    return 0;
}

int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    int on_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    int compute_node_num_on=0;
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_num_on=get_compute_node_num(filename_temp,"on");
    if(compute_node_num==0){
        printf("[ FATAL: ] Currently, there is no compute nodes, nothing to be turned on. Exit now.\n");
        return -1;
    }

    if(compute_node_num==compute_node_num_on){
        printf("[ FATAL: ] Currently, all the compute nodes are in the state of running.\n");
        printf("|          No compute node needs to be turned on. Exit now.\n");
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
                return -1;
            }
            on_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(on_num==0){
            printf("[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n");
            return 1;
        }
        if(on_num+compute_node_num_on>compute_node_num){
            printf("[ -INFO- ] You specified a number larger than the number of currently down nodes.\n");
            printf("           Do you mean turning on *ALL* the compute nodes?\n");
            printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            if(strcmp(string_temp,"y-e-s")!=0){
                printf("[ -INFO- ] You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,"[ -INFO- ] You planned to turn on %d compute node(s).",on_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
#ifdef _WIN32
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
                sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
                if(strcmp(cloud_flag,"CLOUD_A")==0){
                    global_replace(filename_temp,"Stopped","Running");
                }
                else if(strcmp(cloud_flag,"CLOUD_B")==0){
                    find_and_replace(filename_temp,"running_flag","","","","","false","true");
                }
                else if(strcmp(cloud_flag,"CLOUD_C")==0){
                    global_replace(filename_temp,"stopped","running");
                }
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
                return -1;
            }
            printf("[ -INFO- ] After the cluster operation:\n|\n");
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
            }
            printf("[ -DONE- ] Congratulations! The specified compute nodes have been turned on.\n");
            return 0;
        }
    }
    sprintf(string_temp,"[ -INFO- ] You planned to turn on *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
#ifdef _WIN32
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Stopped","Running");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","false","true");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"stopped","running");
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf("[ -DONE- ] Congratulations! The specified compute nodes have been turned on.\n");
    return 0;
}

int check_reconfigure_list(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char single_line[64]="";
    char reconf_list[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(reconf_list,"%s\\reconf.list",stackdir);
#else
    sprintf(reconf_list,"%s/reconf.list",stackdir);
#endif
    if((file_p=fopen(reconf_list,"r"))==NULL){
        return -1;
    }
    while(fgetline(file_p,single_line)==0){
        printf("%s\n",single_line);
    }
    return 0;
}

int reconfigure_compute_node(char* workdir, char* crypto_keyfile, char* new_config, char* htflag){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    char string_temp3[128]="";
    char prev_config[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[16]="";
    int compute_node_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char node_name_temp[32]="";
    char* tf_exec=TERRAFORM_EXEC;
    int cpu_core_num=0;
    char cmdline[CMDLINE_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf("[ -WARN- ] Currently there is no compute nodes in your cluster. Exit now.\n");
        return -1;
    }

    decrypt_files(workdir,crypto_keyfile);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf",stackdir);
#endif
    sprintf(string_temp,"\"%s\"",new_config);
    if(find_multi_keys(filename_temp,string_temp,"","","","")==0||find_multi_keys(filename_temp,string_temp,"","","","")<0){
        printf("[ FATAL: ] Invalid compute configuration.  Exit now.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s\\compute_template",stackdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s/compute_template",stackdir);
#endif
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    if(strcmp(prev_config,new_config)==0){
        if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
            printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
            printf("|          Nothing changed. Exit now.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 1;
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            if(strlen(htflag)==0){
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"hton")!=0&&strcmp(htflag,"htoff")!=0){
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"hton")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
                    sprintf(filename_temp2,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                    sprintf(cmdline,"copy /y %s %s.bak > nul 2>&1",filename_temp2,filename_temp2);
#else
                    sprintf(filename_temp2,"%s/hpc_stack_compute%d.tf",stackdir,i);
                    sprintf(cmdline,"/bin/copy %s %s.bak >>/dev/null 2>&1",filename_temp2,filename_temp2);
#endif
                    system(cmdline);
                    global_replace(filename_temp2,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
                }
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
                    sprintf(filename_temp2,"%s\\hpc_stack_compute%d.tf",stackdir,i);
                    sprintf(cmdline,"copy /y %s %s.bak > nul 2>&1",filename_temp2,filename_temp2);
#else
                    sprintf(filename_temp2,"%s/hpc_stack_compute%d.tf",stackdir,i);
                    sprintf(cmdline,"/bin/cp %s %s.bak >>/dev/null 2>&1",filename_temp2,filename_temp2);
#endif
                    system(cmdline);
                    global_replace(filename_temp2,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
                }
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
                for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
                    sprintf(cmdline,"move /y %s.bak %s > nul 2>&1",filename_temp2,filename_temp2);
#else
                    sprintf(cmdline,"mv %s.bak %s >>/dev/null 2>&1",filename_temp2,filename_temp2);
#endif
                    system(cmdline);
                }
                if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
                    return -127;
                }
                return -1;
            }
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
            }
            update_compute_template(stackdir,cloud_flag);
            printf("[ -INFO- ] After the cluster operation:\n|\n");
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            remote_copy(workdir,sshkey_dir,"hostfile");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
            }
            printf("[ -DONE- ] Congratulations! The compute nodes have been reconfigured.\n");
            return 0;
        }
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
            sprintf(cmdline,"copy /y %s %s.bak > nul 2>&1",filename_temp,filename_temp);
#else
            sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
            sprintf(cmdline,"/bin/cp %s %s.bak >>/dev/nul 2>&1",filename_temp,filename_temp);
#endif
            system(cmdline);
            global_replace(filename_temp,prev_config,new_config);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
            sprintf(cmdline,"copy /y %s %s.bak > nul 2>&1",filename_temp,filename_temp);
#else
            sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
            sprintf(cmdline,"/bin/cp %s %s.bak >>/dev/nul 2>&1",filename_temp,filename_temp);
#endif
            system(cmdline);
            global_replace(filename_temp,prev_config,new_config);
            cpu_core_num=get_cpu_num(new_config)/2;
            find_and_get(filename_temp,"cpu_core_count =","","",1,"cpu_core_count =","","",' ',3,string_temp);
            sprintf(string_temp3,"cpu_core_count = %s",string_temp);
            sprintf(string_temp2,"cpu_core_count = %d",cpu_core_num);
            global_replace(filename_temp,string_temp3,string_temp2);
        }
        if(strcmp(htflag,"hton")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
            for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
                sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
                global_replace(filename_temp,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
            }
        }
        if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
            for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
                sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
                sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
                global_replace(filename_temp,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
            }
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
            sprintf(cmdline,"move /y %s.bak %s > nul 2>&1",filename_temp2,filename_temp2);
#else
            sprintf(cmdline,"mv %s.bak %s >>/dev/null 2>&1",filename_temp2,filename_temp2);
#endif
            system(cmdline);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
            return -127;
        }
        return -1;
    }
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
    }
    update_compute_template(stackdir,cloud_flag);
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
    }
    printf("[ -DONE- ] Congratulations! The compute nodes have been reconfigured.\n");
#ifdef _WIN32
    sprintf(cmdline,"del /q /f %s\\*bak > nul 2>&1",stackdir);
#else
    sprintf(cmdline,"del /q /f %s/*bak >/dev/null 2>&1",stackdir);
#endif
    system(cmdline);
    return 0;
}

int reconfigure_master_node(char* workdir, char* crypto_keyfile, char* new_config){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char string_temp[64]="";
    char prev_config[16]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[16]="";
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char* tf_exec=TERRAFORM_EXEC;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        return -1;
    }

    decrypt_files(workdir,crypto_keyfile);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);z
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf",stackdir);
#endif
    sprintf(string_temp,"\"%s\"",new_config);
    if(find_multi_keys(filename_temp,string_temp,"","","","")==0||find_multi_keys(filename_temp,string_temp,"","","","")<0){
        printf("[ FATAL: ] Invalid master node configuration.  Exit now.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(filename_temp,"%s/hpc_stack_master.tf",stackdir);
#endif
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    if(strcmp(prev_config,new_config)==0){
        printf("[ -INFO- ] The specified configuration is the same as previous configuration.\n");
        printf("|          Nothing changed. Exit now.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
    sprintf(cmdline,"copy /y %s %s.bak > nul 2>&1",filename_temp,filename_temp);
#else
    sprintf(filename_temp,"%s/hpc_stack_master.tf",stackdir);
    sprintf(cmdline,"/bin/cp %s %s.bak >>/dev/null 2>&1",filename_temp,filename_temp);
#endif
    system(cmdline);
    global_replace(filename_temp,prev_config,new_config);
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
#ifdef _WIN32
        sprintf(cmdline,"move /y %s.bak %s > nul 2>&1",filename_temp,filename_temp);
#else
        sprintf(cmdline,"mv %s.bak %s >>/dev/null 2>&1",filename_temp,filename_temp);
#endif
        system(cmdline);
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
            return -127;
        }
        return -3;
    }
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds for remote execution ... \r",GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    remote_copy(workdir,sshkey_dir,"hostfile");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_cluster_summary(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    printf("[ -DONE- ] Congratulations! The master node has been reconfigured.\n");
    return 0;
}

int cluster_sleep(char* workdir, char* crypto_keyfile){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    if(find_multi_keys(filename_temp,"running","","","","")==0&&find_multi_keys(filename_temp,"Running","","","","")==0&&find_multi_keys(filename_temp,"RUNNING","","","","")==0){
        printf("[ -INFO- ] Currently the cluster is in the state of hibernation. No node is running.\n");
        printf("|          If you'd like to make it ready for running, please run 'wakeup' command.\n");
        printf("|          Exit now.\n");
        return 1;
    }
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    sprintf(string_temp,"[ -INFO- ] You planned to shutdown *ALL* the nodes of the current cluster.");
    printf("%s\n",string_temp);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_master.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }

    for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            global_replace(filename_temp,"Running","Stopped");
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_replace(filename_temp,"running_flag","","","","","true","false");
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            global_replace(filename_temp,"running","stopped");
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<10;i++){
            usleep(1000000);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
            return -1;
        }
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");

    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }

    update_cluster_summary(workdir,crypto_keyfile);
    printf("[ -DONE- ] Congratulations! All the nodes of the current cluster have been shutdown.\n");
    return 0;
}

int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option){
    if(strcmp(option,"all")!=0&&strcmp(option,"minimal")!=0){
        printf("[ FATAL: ] Please specify either 'minimal' or 'all' as the second parameter.\n");
        printf("|          Exit now.\n");
        return -1;
    }
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char buffer1[128]="";
    char buffer2[128]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\UCID_LATEST.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/UCID_LATEST.txt",vaultdir);
#endif
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
#endif
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(strcmp(option,"all")==0){
        printf("[ -INFO- ] ALL MODE: Turning on all the nodes of the current cluster.\n");
    }
    else{
        printf("[ -INFO- ] MINIMAL MODE: Turning on the management nodes of the current cluster.\n");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_master.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }
    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
            sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
#else
            sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
#endif
            if(strcmp(cloud_flag,"CLOUD_A")==0){
                global_replace(filename_temp,"Stopped","Running");
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_replace(filename_temp,"running_flag","","","","","false","true");
            }
            else if(strcmp(cloud_flag,"CLOUD_C")==0){
                global_replace(filename_temp,"stopped","running");
            }
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<10;i++){
            usleep(1000000);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log)!=0){
            return -1;
        }
    }
    printf("[ -INFO- ] After the cluster operation:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    update_usage_summary(workdir,crypto_keyfile,"database","start");
    update_usage_summary(workdir,crypto_keyfile,"natgw","start");
    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(string_temp,"compute%d",i);
            update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
        }
    }
    update_cluster_summary(workdir,crypto_keyfile);
    printf("[ -DONE- ] Congratulations! The cluster is in the state of running.\n");
    return 0;
}

int get_default_conf(char* workdir, char* crypto_keyfile, int edit_flag){
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_workdir[DIR_LENGTH]="";
    if(show_current_cluster(temp_workdir,temp_cluster_name,0)!=0){
        return -127;
    }
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[32]="";
    char doubleconfirm[64]="";
    char filename_temp[FILENAME_LENGTH]="";
    char url_aws_root[LOCATION_LENGTH_EXTENDED]="";
    char url_alicloud_root[LOCATION_LENGTH_EXTENDED]="";
    char url_qcloud_root[LOCATION_LENGTH_EXTENDED]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";

    create_and_get_vaultdir(workdir,vaultdir);
    if(code_loc_flag_var==1){
#ifdef _WIN32
        sprintf(url_aws_root,"%s\\aws\\",url_code_root_var);
        sprintf(url_qcloud_root,"%s\\qcloud\\",url_code_root_var);
        sprintf(url_alicloud_root,"%s\\alicloud\\",url_code_root_var);
#else
        sprintf(url_aws_root,"%s/aws/",url_code_root_var);
        sprintf(url_qcloud_root,"%s/qcloud/",url_code_root_var);
        sprintf(url_alicloud_root,"%s/alicloud/",url_code_root_var);
#endif
    }
    else{
        sprintf(url_aws_root,"%saws/",url_code_root_var);
        sprintf(url_qcloud_root,"%sqcloud/",url_code_root_var);
        sprintf(url_alicloud_root,"%salicloud/",url_code_root_var);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(confdir,"%s\\conf\\",workdir);
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1",confdir);
        system(cmdline);
    }
    sprintf(filename_temp,"%s\\tf_prep.conf",confdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"move %s %s.prev > nul 2>&1",filename_temp,filename_temp);
        system(cmdline);
    }
#else
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag);
    sprintf(confdir,"%s/conf/",workdir);
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",confdir);
        system(cmdline);
    }
    sprintf(filename_temp,"%s/tf_prep.conf",confdir);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"mv %s %s.prev >> /dev/null 2>&1",filename_temp,filename_temp);
        system(cmdline);
    }
#endif
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s\\tf_prep.conf > nul 2>&1",url_alicloud_root,confdir);
#else
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s/tf_prep.conf >> /dev/null 2>&1",url_alicloud_root,confdir);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",url_alicloud_root,confdir);
#else
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s/tf_prep.conf",url_alicloud_root,confdir);
#endif
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s\\tf_prep.conf >nul 2>&1",url_qcloud_root,confdir);
#else
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s/tf_prep.conf >> /dev/null 2>&1",url_qcloud_root,confdir);
#endif
        }
        else{
#ifdef _WIN32
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",url_qcloud_root,confdir);
#else
        sprintf(cmdline,"curl %stf_prep.conf -s -o %s/tf_prep.conf",url_qcloud_root,confdir);
#endif
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(code_loc_flag_var==1){
#ifdef _WIN32
            sprintf(cmdline,"copy /y %s\\tf_prep.conf %s\\tf_prep.conf > nul 2>&1",url_aws_root,confdir);
#else
            sprintf(cmdline,"/bin/cp %s/tf_prep.conf %s/tf_prep.conf >> /dev/null 2>&1",url_aws_root,confdir);
#endif
        }
        else{
#ifdef _WIN32
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s\\tf_prep.conf",url_aws_root,confdir);
#else
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s/tf_prep.conf",url_aws_root,confdir);
#endif
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else{
        return 1;
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\tf_prep.conf",confdir);
#else
    sprintf(filename_temp,"%s/tf_prep.conf",confdir);
#endif
    find_and_replace(filename_temp,"CLUSTER_ID","","","","","hpcnow",temp_cluster_name);
    printf("[ -INFO- ] Default configuration file has been downloaded.\n");
    if(edit_flag!=0){
        printf("[ -INFO- ] Would you like to edit it now? Input 'y-e-s' to confirm:\n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
            printf("|          You can still switch to this cluster and run 'hpcopr edit-conf' to \n");
            printf("|          modify and save the default configuration file later. Exit now.\n");
            return 3;
        }
    }
#ifdef _WIN32
    sprintf(cmdline,"notepad %s\\tf_prep.conf",confdir);
#else
    sprintf(cmdline,"vi %s/tf_prep.conf",confdir);
#endif
    system(cmdline);
    return 0;
}

int edit_configuration_file(char* workdir, char* crypto_keyfile){
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char doubleconfirm[64]="";
#ifdef _WIN32
    sprintf(filename_temp,"%s\\conf\\tf_prep.conf",workdir);
#else
    sprintf(filename_temp,"%s/conf/tf_prep.conf",workdir);
#endif
    if(file_exist_or_not(filename_temp)!=0){
        printf("[ -INFO- ] Cluster configuration file not found. Would you like to get one?\n");
        printf("[ INPUT: ] Only 'y-e-s' is accepted to confirm: ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            return 1;
        }
        get_default_conf(workdir,crypto_keyfile,0);
    }
#ifdef _WIN32
    sprintf(cmdline,"notepad %s",filename_temp);
#else
    sprintf(cmdline,"vi %s",filename_temp);
#endif
    system(cmdline);
    return 0;
}