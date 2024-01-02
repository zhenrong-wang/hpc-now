/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef _WIN32
#include <sys/time.h>
#else
#include <conio.h> // This header is not standard! ONLY for mingw
#endif

#include "now_macros.h"
#include "general_funcs.h"
#include "cluster_general_funcs.h"
#include "general_print_info.h"
#include "cluster_operations.h"
#include "prereq_check.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern int code_loc_flag_var;

int switch_to_cluster(char* target_cluster_name){
    char* current_cluster=CURRENT_CLUSTER_INDICATOR;
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_workdir[DIR_LENGTH]="";
    FILE* file_p=NULL;
    if(cluster_name_check(target_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is not in the registry.\n" RESET_DISPLAY, target_cluster_name);
        return 1;
    }
    if(show_current_ncluster(temp_workdir,DIR_LENGTH,temp_cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,0)==0){
        if(strcmp(temp_cluster_name,target_cluster_name)==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Operating the cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " now. No need to switch.\n",target_cluster_name);
            return 3;
        }
    }
    file_p=fopen(current_cluster,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create current cluster indicator. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    fprintf(file_p,"%s",target_cluster_name);
    fclose(file_p);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully switched to the cluster " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",target_cluster_name);
    return 0;
}

int glance_clusters(char* target_cluster_name, char* crypto_keyfile){
    FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"r");
    char registry_line[LINE_LENGTH_SHORT]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    int temp_cluster_name_length=0;
    char temp_cluster_name_column[LINE_LENGTH_SHORT]="";
    char temp_cluster_workdir[DIR_LENGTH]="";
    char cloud_flag[32]="";
    char cluster_role[16]="";
    char cluster_role_ext[32]="";
    int max_cluster_name_length=0;
    int i=0;
    int j=0;
    int status_flag,decrypt_flag;
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    if(strlen(target_cluster_name)==0){
        if(show_current_ncluster(temp_cluster_workdir,DIR_LENGTH,temp_cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,0)==1){
            fclose(file_p);
            return 1;
        }
        get_cloud_flag(temp_cluster_workdir,cloud_flag);
        cluster_role_detect(temp_cluster_workdir,cluster_role,cluster_role_ext);
        decrypt_flag=decryption_status(temp_cluster_workdir);
        status_flag=check_pslock(temp_cluster_workdir,decrypt_flag);
        if(status_flag!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "| switch : <> %s | %s | %s | * OPERATION-IN-PROGRESS * !DECRYPTED! *" RESET_DISPLAY "\n",temp_cluster_name,cluster_role,cloud_flag);
            }
            else{
                printf(GENERAL_BOLD "| switch : <> %s | %s | %s | * OPERATION-IN-PROGRESS *" RESET_DISPLAY "\n",temp_cluster_name,cluster_role,cloud_flag);
            }
            fclose(file_p);
            return 0;
        }
        //decrypt_files(temp_cluster_workdir,crypto_keyfile);
        printf(GENERAL_BOLD "| switch : <> ");
        if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | * EMPTY CLUSTER * !DECRYPTED! *" RESET_DISPLAY "\n",temp_cluster_name,cluster_role,cloud_flag);
            }
            else{
                printf("%s | %s | %s | * EMPTY CLUSTER *" RESET_DISPLAY "\n",temp_cluster_name,cluster_role,cloud_flag);
            }
        }
        //delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        fclose(file_p);
        return 0;
    }
    if(strcmp(target_cluster_name,"all")==0||strcmp(target_cluster_name,"ALL")==0||strcmp(target_cluster_name,"All")==0){
        max_cluster_name_length=get_max_cluster_name_length();
        while(fngetline(file_p,registry_line,LINE_LENGTH_SHORT)!=1){
            if(strlen(registry_line)!=0){
                get_seq_nstring(registry_line,' ',4,temp_cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS);
                if(get_nworkdir(temp_cluster_workdir,DIR_LENGTH,temp_cluster_name)!=0){
                    continue;
                }
                get_cloud_flag(temp_cluster_workdir,cloud_flag);
                cluster_role_detect(temp_cluster_workdir,cluster_role,cluster_role_ext);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf(GENERAL_BOLD "| switch : <> ");
                }
                else{
                    printf(RESET_DISPLAY "|        : <> ");
                }
                temp_cluster_name_length=strlen(temp_cluster_name);
                reset_nstring(temp_cluster_name_column,256);
                if(temp_cluster_name_length<max_cluster_name_length){
                    for(j=0;j<temp_cluster_name_length;j++){
                        *(temp_cluster_name_column+j)=*(temp_cluster_name+j);
                    }
                    for(j=temp_cluster_name_length;j<max_cluster_name_length;j++){
                        *(temp_cluster_name_column+j)=' ';
                    }
                }
                else{
                    strcpy(temp_cluster_name_column,temp_cluster_name);
                }
                decrypt_flag=decryption_status(temp_cluster_workdir);
                status_flag=check_pslock(temp_cluster_workdir,decrypt_flag);
                if(status_flag!=0){
                    if(decrypt_flag!=0){
                        printf(FATAL_RED_BOLD "%s | %s | %s | * OPERATION-IN-PROGRESS * !DECRYPTED! *" RESET_DISPLAY "\n",temp_cluster_name_column,cluster_role_ext,cloud_flag);
                    }
                    else{
                        printf("%s | %s | %s | * OPERATION-IN-PROGRESS *" RESET_DISPLAY "\n",temp_cluster_name_column,cluster_role_ext,cloud_flag);
                    }
                    i++;
                    continue;
                }
                //decrypt_files(temp_cluster_workdir,crypto_keyfile);
                i++;
                if(graph(temp_cluster_workdir,crypto_keyfile,3)!=0){
                    if(decrypt_flag!=0){
                        printf(FATAL_RED_BOLD "%s | %s | %s | * EMPTY CLUSTER * !DECRYPTED! *" RESET_DISPLAY "\n",temp_cluster_name_column,cluster_role_ext,cloud_flag);
                    }
                    else{
                        printf("%s | %s | %s | * EMPTY CLUSTER *" RESET_DISPLAY "\n",temp_cluster_name_column,cluster_role_ext,cloud_flag);
                    }
                }
                //delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            }
        }
        fclose(file_p);
        if(i==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The local cluster registry is empty." RESET_DISPLAY "\n");
            return 0;
        }
        return 0;
    }
    fclose(file_p);
    if(cluster_name_check(target_cluster_name)!=-127){
        return 3;
    }
    else{
        if(get_nworkdir(temp_cluster_workdir,DIR_LENGTH,target_cluster_name)!=0){
            return 3;
        }
        get_cloud_flag(temp_cluster_workdir,cloud_flag);
        cluster_role_detect(temp_cluster_workdir,cluster_role,cluster_role_ext);
        if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,target_cluster_name)==0){
            printf(GENERAL_BOLD "| switch : <> ");
        }
        else{
            printf(RESET_DISPLAY "|        : <> ");
        }
        decrypt_flag=decryption_status(temp_cluster_workdir);
        status_flag=check_pslock(temp_cluster_workdir,decrypt_flag);
        if(status_flag!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | * OPERATION-IN-PROGRESS * !DECRYPTED! *" RESET_DISPLAY "\n",target_cluster_name,cluster_role,cloud_flag);
            }
            else{
                printf("%s | %s | %s | * OPERATION-IN-PROGRESS * " RESET_DISPLAY "\n",target_cluster_name,cluster_role,cloud_flag);
            }
            return 0;
        }
        //decrypt_files(temp_cluster_workdir,crypto_keyfile);
        if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | * EMPTY CLUSTER * !DECRYPTED! *" RESET_DISPLAY "\n",target_cluster_name,cluster_role,cloud_flag);
            }
            else{
                printf("%s | %s | %s | * EMPTY CLUSTER *" RESET_DISPLAY "\n",target_cluster_name,cluster_role,cloud_flag);
            }
        }
        //delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        return 0;
    }
}

int refresh_cluster(char* target_cluster_name, char* crypto_keyfile, char* force_flag, tf_exec_config* tf_run){
    char target_cluster_workdir[DIR_LENGTH]="";
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    if(cluster_name_check(target_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not in the registry. Exit now." RESET_DISPLAY "\n");
        return -3;
    }
    if(get_nworkdir(target_cluster_workdir,DIR_LENGTH,target_cluster_name)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return -3;
    }
    if(strcmp(force_flag,"force")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
        printf("|*         YOU ARE REFRESHING THE CLUSTER WITHOUT CHECKING OPERATION LOCK\n");
        printf("|*         STATUS! PLEASE MAKE SURE THE CLUSTER IS *NOT* IN OPERATION!" RESET_DISPLAY "\n\n");
    }
    else{
        if(cluster_empty_or_not(target_cluster_workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster cannot be refreshed (in init progress or empty)." RESET_DISPLAY "\n");
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please run " HIGH_GREEN_BOLD "hpcopr glance --all" RESET_DISPLAY " to check. Exit now.\n");
            return -5;
        }
        if(check_pslock(target_cluster_workdir,decryption_status(target_cluster_workdir))!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is in operation progress and cannot be refreshed." RESET_DISPLAY "\n");
            return 3;
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Refreshing the target cluster %s now ...\n",target_cluster_name);
    decrypt_files(target_cluster_workdir,crypto_keyfile);
    if(tf_execution(tf_run,"apply",target_cluster_workdir,crypto_keyfile,1)!=0){
        delete_decrypted_files(target_cluster_workdir,crypto_keyfile);
        return 5;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(target_cluster_workdir,crypto_keyfile);
    graph(target_cluster_workdir,crypto_keyfile,0);
    printf("|\n");
    update_cluster_summary(target_cluster_workdir,crypto_keyfile);
    delete_decrypted_files(target_cluster_workdir,crypto_keyfile);
    return 0;
}

int remove_cluster(char* target_cluster_name, char* crypto_keyfile, char* force_flag, tf_exec_config* tf_run){
    if(strlen(target_cluster_name)<CLUSTER_ID_LENGTH_MIN||strlen(target_cluster_name)>CLUSTER_ID_LENGTH_MAX){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is invalid.\n" RESET_DISPLAY,target_cluster_name);
        return 1;
    }
    char cluster_workdir[DIR_LENGTH]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cloud_secrets[FILENAME_LENGTH]="";
    char log_trash[FILENAME_LENGTH]="";
    char tf_realtime_log[FILENAME_LENGTH]="";
    char tf_archive_log[FILENAME_LENGTH]="";
    char tf_realtime_err_log[FILENAME_LENGTH]="";
    char tf_archive_err_log[FILENAME_LENGTH]="";
    char curr_payment_method[16]="";
    FILE* file_p=NULL;
    snprintf(log_trash,FILENAME_LENGTH-1,"%s%slog_trashbin.txt",HPC_NOW_ROOT_DIR,PATH_SLASH);
    if(cluster_name_check(target_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,target_cluster_name);
        list_all_cluster_names(1);
        return 3;
    }
    if(get_nworkdir(cluster_workdir,DIR_LENGTH,target_cluster_name)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return 3;
    }
    get_state_nvalue(cluster_workdir,"payment_method:",curr_payment_method,16);
    if(strcmp(curr_payment_method,"month")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please switch the payment method to " WARN_YELLO_BOLD "od" FATAL_RED_BOLD " first." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(tf_realtime_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.log",cluster_workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_archive_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.log.archive",cluster_workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_realtime_err_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log",cluster_workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_archive_err_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log.archive",cluster_workdir,PATH_SLASH,PATH_SLASH);
    snprintf(cloud_secrets,FILENAME_LENGTH-1,"%s%svault%s.secrets.key",cluster_workdir,PATH_SLASH,PATH_SLASH);
    if(file_empty_or_not(cloud_secrets)<1){
        if(strcmp(force_flag,"force")!=0){
            if(prompt_to_confirm("Removing the *imported* cluster from local.",CONFIRM_STRING,1)==1){
                return 5;
            }
        }
        printf(GENERAL_BOLD "[ -INFO- ] Removing the *imported* cluster from local." RESET_DISPLAY "\n");
        goto remove_files;
    }
    if(strcmp(force_flag,"force")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Removing the specified cluster *WITHOUT* state or resource check." RESET_DISPLAY "\n");
        goto destroy_cluster;
    }
    if(cluster_empty_or_not(cluster_workdir)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster is *NOT* empty!" RESET_DISPLAY "\n");
        glance_clusters(target_cluster_name,crypto_keyfile);
        printf(WARN_YELLO_BOLD "[ -WARN- ] Would you like to remove it anyway? It is *NOT* recoverable!" RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continuie: " );
        fflush(stdin);
        scanf("%63s",doubleconfirm);
        getchar();
        if(strcmp(doubleconfirm,CONFIRM_STRING)==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Please type the cluster name %s to confirm. This opeartion is\n",target_cluster_name);
            printf("|          absolutely *NOT* recoverable!" RESET_DISPLAY "\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
            fflush(stdin);
            scanf("%63s",doubleconfirm);
            getchar();
            if(strcmp(doubleconfirm,target_cluster_name)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only %s is accepted to confirm. Operation denied.\n",target_cluster_name);
                return 5;
            }
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. Operation denied.\n");
            return 5;
        }
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified cluster is empty. This operation will remove all the\n");
        printf("|          related files from your system. Would you like to continue?\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continuie: ");
        fflush(stdin);
        scanf("%63s",doubleconfirm);
        getchar();
        if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. Operation denied.\n");
            return 5;
        }
    }

destroy_cluster:
    if(cluster_destroy(cluster_workdir,crypto_keyfile,"force",0,tf_run)!=0){
        delete_decrypted_files(cluster_workdir,crypto_keyfile);
        return 7;
    }
    file_p=fopen(log_trash,"a+");
    fprintf(file_p,"\n\n###### %s ###### std_archive ######\n\n",target_cluster_name);
    fclose(file_p);
    archive_log(log_trash,tf_archive_log);
    file_p=fopen(log_trash,"a+");
    fprintf(file_p,"\n\n###### %s ###### std_realtime ######\n\n",target_cluster_name);
    fclose(file_p);
    archive_log(log_trash,tf_realtime_log);
    file_p=fopen(log_trash,"a+");
    fprintf(file_p,"\n\n###### %s ###### err_archive ######\n\n",target_cluster_name);
    fclose(file_p);
    archive_log(log_trash,tf_archive_err_log);
    file_p=fopen(log_trash,"a+");
    fprintf(file_p,"\n\n###### %s ###### err_realtime ######\n\n",target_cluster_name);
    fclose(file_p);
    archive_log(log_trash,tf_realtime_err_log);

remove_files:
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Removing all the related files ...\n");
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,cluster_workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s",DELETE_FOLDER_CMD,SSHKEY_DIR,PATH_SLASH,target_cluster_name,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Deleting the cluster from the registry ...\n");
    delete_from_cluster_registry(target_cluster_name);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The cluster %s has been removed completely.\n",target_cluster_name);
    return 0;
}

int create_new_cluster(char* crypto_keyfile, char* cluster_name, char* cloud_ak, char* cloud_sk, char* az_subscription, char* az_tenant, char* echo_flag, char* gcp_flag, int batch_flag_local){
    char cmdline[CMDLINE_LENGTH]="";
    char input_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp_2[FILENAME_LENGTH]="";
    int cluster_name_check_flag=0;
    char cloud_flag[16]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char new_workdir[DIR_LENGTH]="";
    char new_vaultdir[DIR_LENGTH]="";
    char* keypair_temp=NULL;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char az_subscription_id[AKSK_LENGTH]="";
    char az_tenant_id[AKSK_LENGTH]="";
    char md5sum[33]="";
    int ak_length,sk_length;
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    int run_flag;

    if(file_exist_or_not(crypto_keyfile)!=0){
        return -1;
    }
    file_p=fopen(cluster_registry,"a+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    fclose(file_p);
    if(strcmp(gcp_flag,"gcp")==0){
        if(get_google_connectivity()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to " WARN_YELLO_BOLD "api.google.com" FATAL_RED_BOLD " during last check. Please run\n");
            printf("|          the command " WARN_YELLO_BOLD "hpcopr envcheck --gcp" FATAL_RED_BOLD " to re-check and retry." RESET_DISPLAY "\n");
            return 3;
        }
    }
    if(strlen(cluster_name)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Cluster name not specified. Use --cname CLUSTER_NAME ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input the cluster name (A-Z a-z 0-9 -, %d<=length<=%d):\n",CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        scanf("%24s",input_cluster_name);
        getchar();
    }
    else{
        strncpy(input_cluster_name,cluster_name,24);
    }
    cluster_name_check_flag=cluster_name_check(input_cluster_name);
    if(cluster_name_check_flag==-1){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster name cannot begin with '-'. Your input: %s" RESET_DISPLAY "\n",input_cluster_name);
        return 1;
    }
    else if(cluster_name_check_flag==-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " already exists in the registry." RESET_DISPLAY "\n",input_cluster_name);
        return 1;
    }
    else if(cluster_name_check_flag==-3){
        printf(FATAL_RED_BOLD "[ FATAL: ] The length of " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " of out of range %d - %d." RESET_DISPLAY "\n",input_cluster_name,CLUSTER_ID_LENGTH_MIN,CLUSTER_ID_LENGTH_MAX);
        return 1;
    }
    else if(cluster_name_check_flag==-5){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " contains illegal characters." RESET_DISPLAY "\n",input_cluster_name);
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the cluster name %s.\n",input_cluster_name);
    
    if(strcmp(gcp_flag,"gcp")==0){
        if(strlen(cloud_sk)==0){
            if(batch_flag_local==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Key file not specified. Use --sk KEY_FILE_PATH ." RESET_DISPLAY "\n");
                return 17;
            }
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " The JSON-format key file *ABSOLUTE* path: ");
            fflush(stdin);
            scanf("%255s",secret_key);
            getchar();
        }
        else{
            strcpy(secret_key,cloud_sk);
        }
        file_p_2=fopen(secret_key,"r");
        if(file_p_2==NULL){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the key file %s. Exit now." RESET_DISPLAY "\n",secret_key);
            return -1;
        }
        if(find_multi_nkeys(secret_key,LINE_LENGTH_SHORT,"\"project_id\":","","","","")<1||find_multi_nkeys(secret_key,LINE_LENGTH_SHORT,"\"private_key\":","","","","")<1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The provided key file %s is invalid. Exit now." RESET_DISPLAY "\n",secret_key);
            fclose(file_p_2);
            return 3;
        }
        fclose(file_p_2);
        snprintf(new_workdir,DIR_LENGTH-1,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,input_cluster_name,PATH_SLASH);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,new_workdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        create_and_get_vaultdir(new_workdir,new_vaultdir);
        if(get_nmd5sum(crypto_keyfile,md5sum,33)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
            return -3;
        }
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s%s.secrets.key %s %s",NOW_CRYPTO_EXEC,secret_key,new_vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
        run_flag=system(cmdline);
        if(run_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to encrypt the key file. Abort." RESET_DISPLAY "\n");
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,secret_key,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 5;
        }
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scloud_flag.flg",new_vaultdir,PATH_SLASH);
        file_p=fopen(filename_temp,"w+");
        if(file_p!=NULL){
            fprintf(file_p,"CLOUD_G\n");
            fclose(file_p);
        }
        if(strcmp(echo_flag,"echo")==0){
            printf(GREY_LIGHT);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",CAT_FILE_CMD,secret_key);
            system(cmdline);
            printf(RESET_DISPLAY);
        }
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,secret_key,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        add_to_cluster_registry(input_cluster_name,"");
        switch_to_cluster(input_cluster_name);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The key file has been encrypted and stored locally. We recommend you\n");
        printf("|          to delete the original key file to avoid key leakage. Now you can either:\n");
        printf("|          1. run 'hpcopr init' to create a default cluster. OR\n");
        printf("|          2. run 'hpcopr get-conf' to get the default cluster configuration, and run\n");
        printf("|             'hpcopr init' to create a customized cluster.\n");
        printf("|          You can also switch to this cluster name and operate this cluster later.\n");
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
        return 0;
    }

#ifdef _WIN32
    strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
    strcpy(filename_temp_2,"c:\\programdata\\hpc-now\\.az_extra.info");
#elif __linux__
    strcpy(filename_temp,"/home/hpc-now/.secret.tmp.txt");
    strcpy(filename_temp_2,"/home/hpc-now/.az_extra.info");
#elif __APPLE__
    strcpy(filename_temp,"/Users/hpc-now/.secret.tmp.txt");
    strcpy(filename_temp_2,"/Users/hpc-now/.az_extra.info");
#endif
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return -1;
    }
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] AK and/or SK not specified. Use --ak AK --sk SK" RESET_DISPLAY "\n");
            fclose(file_p);
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input/paste your secrets key pair:\n");
        keypair_temp=GETPASS_FUNC("[ INPUT: ] Access key ID  : ");
        strcpy(access_key,keypair_temp);
        reset_string(keypair_temp);
        keypair_temp=GETPASS_FUNC("[ INPUT: ] Access secrets : ");
        strcpy(secret_key,keypair_temp);
        reset_string(keypair_temp);
    }
    else{
        strcpy(access_key,cloud_ak);
        strcpy(secret_key,cloud_sk);
    }
    if(strcmp(echo_flag,"echo")==0){
        printf(GREY_LIGHT "\n|          Access key ID  : %s\n",access_key);
        printf("|          Access secrets : %s\n\n" RESET_DISPLAY,secret_key);
    }
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);
    if(ak_length==24&&sk_length==30){
        strcpy(cloud_flag,"CLOUD_A");
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_B");
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        if(*(access_key+0)=='A'&&*(access_key+1)=='K'&&*(access_key+2)=='I'&&*(access_key+3)=='A'){
            strcpy(cloud_flag,"CLOUD_C");
        }
        else{
            strcpy(cloud_flag,"CLOUD_D");
        }
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==32&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_E");
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==40){
        strcpy(cloud_flag,"CLOUD_F");
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You are using Azure Cloud Service. Subscription and tenant id are needed.\n");
        file_p_2=fopen(filename_temp_2,"w+");
        if(file_p_2==NULL){
            fclose(file_p);
            return -1;
        }
        if(strlen(az_subscription)!=36){
            if(batch_flag_local==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Subscription ID no specified. Use --az-sid ID ." RESET_DISPLAY "\n");
                fclose(file_p_2);
                return 17;
            }
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Subscription id: ");
            fflush(stdin);
            scanf("%255s",az_subscription_id);
            getchar();
        }
        else{
            strcpy(az_subscription_id,az_subscription);
        }
        if(strlen(az_tenant)!=36){
            if(batch_flag_local==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Tenant ID no specified. Use --az-tid ID ." RESET_DISPLAY "\n");
                fclose(file_p_2);
                return 17;
            }
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Azure Tenant id: ");
            fflush(stdin);
            scanf("%255s",az_tenant_id);
            getchar();
        }
        else{
            strcpy(az_tenant_id,az_tenant);
        }
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
        fprintf(file_p_2,"azure_subscription_id: %s\nazure_tenant_id: %s\n",az_subscription_id,az_tenant_id);
        fclose(file_p_2);
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now." RESET_DISPLAY "\n");
        fclose(file_p);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 5;
    }
    snprintf(new_workdir,DIR_LENGTH-1,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,input_cluster_name,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,new_workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    create_and_get_vaultdir(new_workdir,new_vaultdir);
    if(get_nmd5sum(crypto_keyfile,md5sum,33)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
        return -3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s%s.secrets.key %s %s",NOW_CRYPTO_EXEC,filename_temp,new_vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(strcmp(cloud_flag,"CLOUD_F")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s%s %s",MOVE_FILE_CMD,filename_temp_2,new_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scloud_flag.flg",new_vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    if(file_p!=NULL){
        fprintf(file_p,"%s\n",cloud_flag);
        fclose(file_p);
    }
    add_to_cluster_registry(input_cluster_name,"");
    switch_to_cluster(input_cluster_name);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The secrets key pair has been encrypted and stored locally. You can either:\n");
    printf("|          1. run 'hpcopr init' to create a default cluster. OR\n");
    printf("|          2. run 'hpcopr get-conf' to get the default cluster configuration, and run\n");
    printf("|             'hpcopr init' to create a customized cluster.\n");
    printf("|          You can also switch to this cluster name and operate this cluster later.\n");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
    return 0;
}

int rotate_new_keypair(char* workdir, char* cloud_ak, char* cloud_sk, char* crypto_keyfile, char* echo_flag, int batch_flag_local){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    int ak_length,sk_length;
    char* keypair_temp=NULL;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[32]="";
    char access_key_prev[AKSK_LENGTH]="";
    char secret_key_prev[AKSK_LENGTH]="";
    char az_subscription_id[AKSK_LENGTH]="";
    char az_tenant_id[AKSK_LENGTH]="";
    char cloud_flag_prev[32]="";
    char md5sum[33]="";
    FILE* file_p=NULL;
    int run_flag;
    
    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("|*         YOU ARE ROTATING THE CLOUD KEY, WHICH MAY DAMAGE THIS CLUSTER.\n");
    printf("|*         BEFORE PROCEEDING, PLEASE MAKE SURE:\n");
    printf("|*         1. If the current cluster is NOT empty, the new key MUST comes\n");
    printf("|*            from the SAME cloud vendor AND account.\n");
    printf("|*         2. If the current cluster is empty, your new key pair can come\n");
    printf("|*            from another account of the SAME vendor.\n");
    printf("|*         3. Your new key is valid and able to manage cloud resources.\n");
    printf("|*            This is *!!! VERY IMPORTANT !!!*\n");
    printf("|*         THIS OPERATION IS UNRECOVERABLE!" RESET_DISPLAY "\n\n");
    
    if(prompt_to_confirm("ARE YOU SURE ?",CONFIRM_STRING,batch_flag_local)==1){
        return 1;
    }
    get_cloud_flag(workdir,cloud_flag_prev);
    create_and_get_vaultdir(workdir,vaultdir);
    if(strcmp(cloud_flag_prev,"CLOUD_G")==0){
        if(strlen(cloud_sk)==0){
            if(batch_flag_local==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Key file not specified. Use --sk KEY_FILE_PATH ." RESET_DISPLAY "\n");
                return 17;
            }
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " The JSON-format key file *ABSOLUTE* path: ");
            fflush(stdin);
            scanf("%255s",secret_key);
            getchar();
        }
        else{
            strcpy(secret_key,cloud_sk);
        }
        file_p=fopen(secret_key,"r");
        if(file_p==NULL){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the key file %s. Exit now." RESET_DISPLAY "\n",secret_key);
            return -1;
        }
        if(find_multi_nkeys(secret_key,LINE_LENGTH_SHORT,"\"project_id\":","","","","")<1||find_multi_nkeys(secret_key,LINE_LENGTH_SHORT,"\"private_key\":","","","","")<1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The provided key file %s is invalid. Exit now." RESET_DISPLAY "\n",secret_key);
            fclose(file_p);
            return -1;
        }
        fclose(file_p);
        if(get_nmd5sum(crypto_keyfile,md5sum,33)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
            return -3;
        }
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s%s.secrets.key %s %s",NOW_CRYPTO_EXEC,secret_key,vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
        run_flag=system(cmdline);
        if(run_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to encrypt the key file. The key keeps unchanged." RESET_DISPLAY "\n");
            printf(cmdline,"%s %s %s",DELETE_FILE_CMD,secret_key,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return 5;
        }
        if(strcmp(echo_flag,"echo")==0){
            printf(GREY_LIGHT);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",CAT_FILE_CMD,secret_key);
            system(cmdline);
            printf(RESET_DISPLAY);
        }
        printf(cmdline,"%s %s %s",DELETE_FILE_CMD,secret_key,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The new secrets key pair has been encrypted and rotated locally.\n");
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
        return 0;
    }

#ifdef _WIN32
    strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
#elif __linux__
    strcpy(filename_temp,"/home/hpc-now/.secret.tmp.txt");
#elif __APPLE__
    strcpy(filename_temp,"/Users/hpc-now/.secret.tmp.txt");
#endif
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a temporary file in your system.\n");
        printf("|          Please check the available disk space. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(filename_temp2,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp2)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently there is no secrets keypair. This working directory may be\n");
        printf("|          corrputed, which is very unusual. Please contact us via:\n");
        printf("|          info@hpc-now.com for troubleshooting. Exit now." RESET_DISPLAY "\n");
        fclose(file_p);
        return -3;
    }
    get_ak_sk(filename_temp2,crypto_keyfile,access_key_prev,secret_key_prev,cloud_flag_prev);
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] AK and/or SK not specified. Use --ak AK --sk SK" RESET_DISPLAY "\n");
            fclose(file_p);
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input/paste your new secrets key pair:\n");
        keypair_temp=GETPASS_FUNC("[ INPUT: ] Access key ID  : ");
        strcpy(access_key,keypair_temp);
        reset_string(keypair_temp);
        keypair_temp=GETPASS_FUNC("[ INPUT: ] Access secrets : ");
        strcpy(secret_key,keypair_temp);
        reset_string(keypair_temp);
    }
    else{
        strcpy(access_key,cloud_ak);
        strcpy(secret_key,cloud_sk);
    }
    if(strcmp(echo_flag,"echo")==0){
        printf(GREY_LIGHT "\n|          Access key ID  : %s\n",access_key);
        printf("|          Access secrets : %s\n\n" RESET_DISPLAY,secret_key);
    }
    ak_length=strlen(access_key);
    sk_length=strlen(secret_key);
    if(ak_length==24&&sk_length==30){
        strcpy(cloud_flag,"CLOUD_A");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: AliCloud (HPC-NOW code: CLOUD_A).\n");
            printf("|          Please rotate a keypair from an AliCloud account.\n");
            printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
            return 3;
        }
        fprintf(file_p,"%s\n%s\n%s",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_B");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: TencentCloud (HPC-NOW code: CLOUD_B).\n");
            printf("|          Please rotate a keypair from the a TencentCloud account.\n");
            printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
            return 3;
        }
        fprintf(file_p,"%s\n%s\n%s",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        if(*(access_key+0)=='A'&&*(access_key+1)=='K'&&*(access_key+2)=='I'&&*(access_key+3)=='A'){
            strcpy(cloud_flag,"CLOUD_C");
        }
        else{
            strcpy(cloud_flag,"CLOUD_D");
        }
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            if(strcmp(cloud_flag_prev,"CLOUD_C")==0){
                printf("|          Current Vendor: Amazon Web Services (HPC-NOW code: CLOUD_C).\n");
                printf("|          Please rotate a keypair from an Amazon Web Services account.\n");
            }
            else{
                printf("|          Current Vendor: Huawei Cloud (HPC-NOW code: CLOUD_D).\n");
                printf("|          Please rotate a keypair from a Huawei Cloud account.\n");
            }
            printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
            return 3;
        }
        fprintf(file_p,"%s\n%s\n%s",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==32&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_E");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: BaiduCloud (HPC-NOW code: CLOUD_E).\n");
            printf("|          Please rotate a keypair from a BaiduCloud account.\n");
            printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
            return 3;
        }
        fprintf(file_p,"%s\n%s\n%s",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==40){
        strcpy(cloud_flag,"CLOUD_F");
        if(strcmp(cloud_flag,cloud_flag_prev)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: Azure (HPC-NOW code: CLOUD_F).\n");
            printf("|          Please rotate a keypair from the *SAME* subscription and tenant.\n");
            printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
            return 3;
        }
        printf("|       -> Current subscription ID: %s\n",az_subscription_id);
        printf("|       -> Current tenant ID      : %s\n",az_tenant_id);
        printf("[ -INFO- ] The new key pair MUST come from the subscription and tenant above.\n");
        fprintf(file_p,"%s\n%s\n%s",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now." RESET_DISPLAY "\n");
        fclose(file_p);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 3;
    }
    if(get_nmd5sum(crypto_keyfile,md5sum,33)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s%s.secrets.key %s %s",NOW_CRYPTO_EXEC,filename_temp,vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    create_and_get_stackdir(workdir,stackdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf.tmp",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        snprintf(filename_temp2,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s %s %s %s",NOW_CRYPTO_EXEC,filename_temp,filename_temp2,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        global_nreplace(filename_temp2,LINE_LENGTH_SMALL,access_key_prev,access_key);
        global_nreplace(filename_temp2,LINE_LENGTH_SMALL,secret_key_prev,secret_key);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s %s %s",NOW_CRYPTO_EXEC,filename_temp2,filename_temp,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The new secrets key pair has been encrypted and rotated locally.\n");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
    return 0;
}

int cluster_destroy(char* workdir, char* crypto_keyfile, char* force_flag, int batch_flag_local, tf_exec_config* tf_run){
    char cmdline[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[LINE_LENGTH_SHORT];
    char dot_terraform[FILENAME_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char curr_payment_method[16]="";
    int i;
    int compute_node_num=0;
    get_state_nvalue(workdir,"payment_method:",curr_payment_method,16);
    if(strcmp(curr_payment_method,"month")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please switch the payment method to " WARN_YELLO_BOLD "od" FATAL_RED_BOLD " first." RESET_DISPLAY "\n");
        return -3;
    }
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return -7;
    }
    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("|*         DELETING THE WHOLE CLUSTER - INCLUDING ALL THE NODES AND DATA!\n");
    printf("|*         THIS OPERATION IS UNRECOVERABLE!" RESET_DISPLAY "\n\n");
    if(strcmp(force_flag,"force")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Destroying the current cluster *WITHOUT* confirmation." RESET_DISPLAY "\n");
    }
    else{
        if(prompt_to_confirm("ARE YOU SURE ?",CONFIRM_STRING,batch_flag_local)==1){
            return 1;
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Cluster operation started ...\n");
    create_and_get_vaultdir(workdir,vaultdir);
    create_and_get_stackdir(workdir,stackdir);
    decrypt_files(workdir,crypto_keyfile);
    snprintf(dot_terraform,FILENAME_LENGTH-1,"%s%s.terraform",stackdir,PATH_SLASH);
    if(folder_exist_or_not(dot_terraform)==0){
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Some problems occoured. Retrying destroy now (1/2)..." RESET_DISPLAY "\n");
            sleep(2);
            if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Some problems occoured. Retrying destroy now (2/2)..." RESET_DISPLAY "\n");
                sleep(2);
                if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to destroy your cluster. This usually caused by either TF or\n");
                    printf("|          the providers developed and maintained by cloud service providers.\n");
                    printf("|          You *MUST* manually destroy the remaining cloud resources of this cluster.\n");
                    printf("|          Exit now." RESET_DISPLAY "\n");
                    delete_decrypted_files(workdir,crypto_keyfile);
                    return -1;
                }
            }
        }
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The whole cluster has been destroyed successfully.\n");
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Upating the usage records ...\n");
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
        compute_node_num=get_compute_node_num(filename_temp,"all");
        update_usage_summary(workdir,crypto_keyfile,"master","stop");
        update_usage_summary(workdir,crypto_keyfile,"database","stop");
        update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
        for(i=0;i<compute_node_num;i++){
            snprintf(string_temp,LINE_LENGTH_SHORT-1,"compute%d",i+1);
            update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
        }
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has " WARN_YELLO_BOLD "not been initialized" RESET_DISPLAY ". No need to destroy.\n");
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Deleting all the related local files and folders ...\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s*.tf %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scurrentstate %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scompute_template %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shostfile_latest %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s*.txt %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sconf%stf_prep.conf %s%sconf%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,workdir,PATH_SLASH,PATH_SLASH,workdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s",DELETE_FOLDER_CMD,SSHKEY_DIR,PATH_SLASH,cluster_name,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    delete_local_tf_config(stackdir);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been destroyed successfully.\n");
    return 0;
}

int delete_compute_node(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char* sshkey_dir=SSHKEY_DIR;
    int i,run_flag;
    int del_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%63s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing deleted. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    if(strcmp(param,"all")!=0){
        del_num=string_to_positive_num(param);
        if(del_num==0||del_num<0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now." RESET_DISPLAY "\n");
            return 1;
        }
        if(del_num>compute_node_num){
            run_flag=prompt_to_confirm("Did you mean deleting all the comput nodes?",CONFIRM_STRING,batch_flag_local);
            if(run_flag==1){
                return 1;
            }
        }
        else{
            snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to delete %d from %d compute node(s).",del_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                system(cmdline);
                snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute%d.tf* %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,i,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                system(cmdline);
            }
            if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){ 
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,i,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                }
                if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
                    return -127;
                }
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
            getstate(workdir,crypto_keyfile);
            graph(workdir,crypto_keyfile,0);
            get_latest_hosts(stackdir,filename_temp);
            printf("|\n");
            remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put","",0);
            sync_statefile(workdir,sshkey_dir);
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                snprintf(string_temp,127,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 0;
        }
    }
    snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to delete *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,i,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,i,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,sshkey_dir);
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    for(i=1;i<compute_node_num+1;i++){
        snprintf(string_temp,127,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int add_compute_node(char* workdir, char* crypto_keyfile, char* add_number_string, tf_exec_config* tf_run){
    char string_temp[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    int i;
    int add_number=0;
    int current_node_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    if(strlen(add_number_string)>2||strlen(add_number_string)<1){
        printf(FATAL_RED_BOLD "[ FATAL: ] The number of nodes to be added is invalid. A number (1-%d) is needed." RESET_DISPLAY "\n",MAXIMUM_ADD_NODE_NUMBER);
        return -1;
    }
    add_number=string_to_positive_num(add_number_string);
    if(add_number<MINIMUM_ADD_NODE_NUMBER||add_number>MAXIMUM_ADD_NODE_NUMBER){
        printf(FATAL_RED_BOLD "[ FATAL: ] The number of nodes to be added is out of range (1-%d).\n" RESET_DISPLAY,MAXIMUM_ADD_NODE_NUMBER);
        return -1;
    }
    snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to add %d compute node(s).",add_number);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster operation is in progress ...\n");
    create_and_get_stackdir(workdir,stackdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    current_node_num=get_compute_node_num(filename_temp,"all");
    for(i=0;i<add_number;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scompute_template %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1+current_node_num,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1+current_node_num);
        snprintf(string_temp,127,"compute%d",i+1+current_node_num);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"compute1",string_temp);
        snprintf(string_temp,127,"comp%d",i+1+current_node_num);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"comp1",string_temp);
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=0;i<add_number;i++){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute%d.tf",DELETE_FILE_CMD,stackdir,PATH_SLASH,i+1+current_node_num);
            system(cmdline);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,sshkey_dir);
    remote_exec(workdir,sshkey_dir,"connect",7);
    remote_exec(workdir,sshkey_dir,"all",8);
    for(i=0;i<add_number;i++){
        snprintf(string_temp,127,"compute%d",current_node_num+i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been added.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int shutdown_compute_nodes(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    int i;
    int down_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%63s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing to be shutdown. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    if(strcmp(param,"all")!=0){
        down_num=string_to_positive_num(param);
        if(down_num<0||down_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now." RESET_DISPLAY "\n");
            return 1;
        }
        if(down_num>compute_node_num){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified a number larger than the quantity of compute nodes.\n");
            printf("           Do you mean shutting down *ALL* the compute nodes?\n");
            if(batch_flag_local==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] RISKY! Cluster operation is auto-confirmed." RESET_DISPLAY "\n");
            }
            else{
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm:  ");
                fflush(stdin);
                scanf("%127s",string_temp);
                getchar();
                if(strcmp(string_temp,CONFIRM_STRING)!=0){
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You denied this operation. Exit now.\n");
                    return 1;
                }
            }
        }
        else{
            snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to shutdown %d from %d compute node(s).",down_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                snprintf(node_name,15,"compute%d",i);
                node_file_to_stop(stackdir,node_name,cloud_flag);
            }
            if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                    snprintf(node_name,15,"compute%d",i);
                    node_file_to_running(stackdir,node_name,cloud_flag);
                }
                if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
                    return -127;
                }
                delete_decrypted_files(workdir,crypto_keyfile);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
                return -1;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
            getstate(workdir,crypto_keyfile);
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            sync_statefile(workdir,SSHKEY_DIR);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                snprintf(string_temp,127,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
            return 0;
        }
    }
    snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to shutdown *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        snprintf(node_name,15,"compute%d",i);
        node_file_to_stop(stackdir,node_name,cloud_flag);
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            snprintf(node_name,15,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }        
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    for(i=1;i<compute_node_num+1;i++){
        snprintf(string_temp,127,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    sync_statefile(workdir,SSHKEY_DIR);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been shut down.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char node_name[16]="";
    char cloud_flag[16]="";
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    int on_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    int compute_node_num=0;
    int compute_node_num_on=0;
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%63s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_num_on=get_compute_node_num(filename_temp,"on");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing to be turned on. Exit now." RESET_DISPLAY "\n");
        return -1;
    }

    if(compute_node_num==compute_node_num_on){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, all the compute nodes are in the state of running.\n");
        printf("|          No compute node needs to be turned on. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    if(strcmp(param,"all")!=0){
        on_num=string_to_positive_num(param);
        if(on_num<0||on_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now." RESET_DISPLAY "\n");
            return 1;
        }
        if(on_num+compute_node_num_on>compute_node_num){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified a number larger than the number of currently down nodes.\n");
            printf("           Do you mean turning on *ALL* the compute nodes?\n");
            if(batch_flag_local==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] RISKY! Cluster operation is auto-confirmed." RESET_DISPLAY "\n");
            }
            else{
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm:  ");
                fflush(stdin);
                scanf("%127s",string_temp);
                getchar();
                if(strcmp(string_temp,CONFIRM_STRING)!=0){
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You denied this operation. Exit now.\n");
                    return 1;
                }
            }
        }
        else{
            snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to turn on *ALL* the %d compute node(s).",compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                snprintf(node_name,15,"compute%d",i);
                node_file_to_running(stackdir,node_name,cloud_flag);
            }
            if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                    snprintf(node_name,15,"compute%d",i);
                    node_file_to_stop(stackdir,node_name,cloud_flag);
                }
                if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
                    return -127;
                }
                delete_decrypted_files(workdir,crypto_keyfile);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
                return -1;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
            getstate(workdir,crypto_keyfile);
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            sync_statefile(workdir,sshkey_dir);
            remote_exec(workdir,sshkey_dir,"quick",1);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                snprintf(string_temp,127,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been turned on.\n");
            return 0;
        }
    }
    snprintf(string_temp,127,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to turn on *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        snprintf(node_name,15,"compute%d",i);
        node_file_to_running(stackdir,node_name,cloud_flag);
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
            snprintf(node_name,15,"compute%d",i);
            node_file_to_stop(stackdir,node_name,cloud_flag);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    sync_statefile(workdir,sshkey_dir);
    remote_exec(workdir,sshkey_dir,"quick",1);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        snprintf(string_temp,127,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been turned on.\n");
    return 0;
}

int reconfigure_compute_node(char* workdir, char* crypto_keyfile, char* new_config, char* htflag, tf_exec_config* tf_run){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    char prev_config[16]="";
    char prev_htflag[8]="";
    int config_diff_flag=0,ht_diff_flag=0;
    char cloud_flag[16]="";
    int compute_node_num=0;
    int compute_node_down_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    char node_name_temp[32]="";
    int cpu_core_num=0;
    int reinit_flag=0;
    char cmdline[CMDLINE_LENGTH]="";
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -5;
    }
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_down_num=get_compute_node_num(filename_temp,"down");
    if(compute_node_num==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Currently there is no compute nodes in your cluster. Exit now." RESET_DISPLAY "\n");
        return -3;
    }
    decrypt_files(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    snprintf(string_temp,63,"\"%s\"",new_config);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SMALL,string_temp,"","","","")==0||find_multi_nkeys(filename_temp,LINE_LENGTH_SMALL,string_temp,"","","","")<0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid compute configuration. Exit now." RESET_DISPLAY "\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scompute_template",stackdir,PATH_SLASH);
    if(strcmp(cloud_flag,"CLOUD_D")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"flavor_id = \"$","","",1,"flavor_id = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"instance_spec","","",1,"instance_spec","","",'.',3,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"size = \"$","","",1,"size = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"machine_type","","",1,"machine_type","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else{
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"instance_type","","",1,"instance_type","","",'.',3,prev_config,16);
    }

    config_diff_flag=strcmp(prev_config,new_config);

    if(strcmp(cloud_flag,"CLOUD_C")!=0){
        if(config_diff_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 1;
        }
        for(i=1;i<compute_node_num+1;i++){
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            global_nreplace(filename_temp,LINE_LENGTH_SMALL,prev_config,new_config);
        }
    }
    else{
        get_state_nvalue(workdir,"ht_flag:",prev_htflag,8);
        if(strcmp(htflag,"ON")!=0&&strcmp(htflag,"OFF")!=0){
            ht_diff_flag=0;
        }
        else{
            ht_diff_flag=strcmp(prev_htflag,htflag);
        }
        if(config_diff_flag==0&&ht_diff_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 1;
        }
        for(i=1;i<compute_node_num+1;i++){
            snprintf(filename_temp2,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp2,filename_temp2,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            if(config_diff_flag!=0){
                global_replace(filename_temp2,prev_config,new_config);
            }
            cpu_core_num=get_cpu_num(new_config)/2;
            if(ht_diff_flag!=0){
                delete_nlines_by_kwd(filename_temp2,LINE_LENGTH_SMALL,"cpu_core_count =",1);
                delete_nlines_by_kwd(filename_temp2,LINE_LENGTH_SMALL,"cpu_threads_per_core =",1);
                snprintf(string_temp2,63,"cpu_core_count = %d",cpu_core_num);
                insert_lines(filename_temp2,"#INSERT_HT_HERE",string_temp2);
                if(strcmp(prev_htflag,"ON")==0){
                    insert_lines(filename_temp2,"#INSERT_HT_HERE","cpu_threads_per_core = 1");
                }
                else{
                    insert_lines(filename_temp2,"#INSERT_HT_HERE","cpu_threads_per_core = 2");
                }
                reinit_flag=1;
            }
            else{
                delete_nlines_by_kwd(filename_temp2,LINE_LENGTH_SMALL,"cpu_core_count =",1);
                delete_nlines_by_kwd(filename_temp2,LINE_LENGTH_SMALL,"cpu_threads_per_core =",1);
                if(strcmp(prev_htflag,"OFF")==0){
                    insert_lines(filename_temp2,"#INSERT_HT_HERE","cpu_threads_per_core = 1");
                    snprintf(string_temp2,63,"cpu_core_count = %d",cpu_core_num);
                    insert_lines(filename_temp2,"#INSERT_HT_HERE",string_temp2);
                    reinit_flag=1;
                }
            }
            //printf("----%d\n",cpu_core_num);
        }
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return 3;
    }
    for(i=1;i<compute_node_num+1;i++){
        snprintf(node_name_temp,31,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
    }
    update_compute_template(stackdir,cloud_flag);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    sync_statefile(workdir,sshkey_dir);
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put","",0);
    if(compute_node_down_num!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Please turn on all the cluster nodes, log on to the master\n");
        printf("|          node, and run: " HIGH_GREEN_BOLD "sudo hpcmgr connect && sudo hpcmgr all" RESET_DISPLAY WARN_YELLO_BOLD "" RESET_DISPLAY "\n");
    }
    else{
        if(reinit_flag==0){
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
        }
        else{
            remote_exec(workdir,sshkey_dir,"connect",7);
            remote_exec(workdir,sshkey_dir,"all",8);
        }
    }
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        snprintf(node_name_temp,31,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The compute nodes have been reconfigured.\n");
    if(reinit_flag!=0){
        printf(GENERAL_BOLD "|         " RESET_DISPLAY " Recreating the compute node(s), the process may need 7 minutes.\n");
    }
    else{
        printf(GENERAL_BOLD "|         " RESET_DISPLAY " Changing the compute node(s), the process may need 1 minute.\n");
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s*bak %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int reconfigure_master_node(char* workdir, char* crypto_keyfile, char* new_config, tf_exec_config* tf_run){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char string_temp[64]="";
    char prev_config[16]="";
    char cloud_flag[16]="";
    char* sshkey_dir=SSHKEY_DIR;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        return -1;
    }
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -5;
    }

    decrypt_files(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    snprintf(string_temp,63,"\"%s\"",new_config);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SMALL,string_temp,"","","","")==0||find_multi_nkeys(filename_temp,LINE_LENGTH_SMALL,string_temp,"","","","")<0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid master node configuration. Exit now." RESET_DISPLAY "\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    if(strcmp(cloud_flag,"CLOUD_D")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"flavor_id = \"$","","",1,"flavor_id = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"instance_spec","","",1,"instance_spec","","",'.',3,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"size = \"$","","",1,"size = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"machine_type","","",1,"machine_type","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,prev_config,16);
    }
    else{
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"instance_type","","",1,"instance_type","","",'.',3,prev_config,16);
    }
    if(strcmp(prev_config,new_config)==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,prev_config,new_config);
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s.bak %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Wait %d seconds for remote execution ... \r",GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,sshkey_dir);
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    update_cluster_summary(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The master node has been reconfigured.\n");
    return 0;
}

int nfs_volume_up(char* workdir, char* crypto_keyfile, char* new_volume, tf_exec_config* tf_run){
    char stackdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char prev_volume[16]="";
    int prev_volume_num;
    int new_volume_num;
    char cloud_flag[16]="";
    char* sshkey_dir=SSHKEY_DIR;
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return 1;
    }
    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        return -1;
    }
    new_volume_num=string_to_positive_num(new_volume);
    if(new_volume_num<1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a positive number as the new volume." RESET_DISPLAY "\n");
        return 3;
    }
    get_state_nvalue(workdir,"shared_volume_gb:",prev_volume,16);
    prev_volume_num=string_to_positive_num(prev_volume);
    if(new_volume_num<prev_volume_num){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a new volume larger than the previous volume %d." RESET_DISPLAY "\n",prev_volume_num);
        return 3;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Volume from %d to %d GB, this is NOT reversible!\n",prev_volume_num,new_volume_num);
    create_and_get_stackdir(workdir,stackdir);
    decrypt_files(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    find_and_nreplace(filename_temp,LINE_LENGTH_SMALL,"#-#-#","","","","",prev_volume,new_volume);
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s.bak %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(strcmp(cloud_flag,"CLOUD_F")==0){
        strcpy(cmdline,"resize2fs /dev/sdc");
    }
    else{
        strcpy(cmdline,"resize2fs /dev/sdb");
    }
    remote_exec_general(workdir,sshkey_dir,"root",cmdline,"-n",0,0,"","");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    sync_statefile(workdir,sshkey_dir);
    delete_decrypted_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The shared volume has been expanded from %d to %d GB.\n",prev_volume_num,new_volume_num);
    return 0;
}

int cluster_sleep(char* workdir, char* crypto_keyfile, tf_exec_config* tf_run){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%63s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,"running","","","","")==0&&find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,"Running","","","","")==0&&find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,"RUNNING","","","","")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not running. Please wake up first.\n");
        printf("|          Command: hpcopr wakeup --all | --min . Exit now." RESET_DISPLAY "\n");
        return 1;
    }
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to shutdown *ALL* the nodes of the current cluster.\n");
    node_file_to_stop(stackdir,"master",cloud_flag);
    node_file_to_stop(stackdir,"database",cloud_flag);
    node_file_to_stop(stackdir,"natgw",cloud_flag);
    for(i=1;i<compute_node_num+1;i++){
        snprintf(node_name,15,"compute%d",i);
        node_file_to_stop(stackdir,node_name,cloud_flag);
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        node_file_to_running(stackdir,"master",cloud_flag);
        node_file_to_running(stackdir,"database",cloud_flag);
        node_file_to_running(stackdir,"natgw",cloud_flag);
        for(i=1;i<compute_node_num+1;i++){
            snprintf(node_name,15,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
    for(i=1;i<compute_node_num+1;i++){
        snprintf(string_temp,127,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    update_cluster_summary(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! All the nodes have been shutdown.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option, tf_exec_config* tf_run){
    if(strcmp(option,"all")!=0&&strcmp(option,"minimal")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'minimal' or 'all' as the second parameter.\n");
        printf("|          Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    char* sshkeydir=SSHKEY_DIR;
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%63s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        return -1;
    }
    if(strcmp(option,"minimal")==0&&cluster_asleep_or_not(workdir)!=0){
        return 3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(strcmp(option,"all")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY HIGH_CYAN_BOLD " ALL MODE:" RESET_DISPLAY " Turning on all the nodes of the current cluster.\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY HIGH_CYAN_BOLD " MINIMAL MODE:" RESET_DISPLAY " Turning on the management nodes of the current cluster.\n");
    }
    node_file_to_running(stackdir,"master",cloud_flag);
    node_file_to_running(stackdir,"database",cloud_flag);
    node_file_to_running(stackdir,"natgw",cloud_flag);
    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
            snprintf(node_name,15,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        node_file_to_stop(stackdir,"master",cloud_flag);
        node_file_to_stop(stackdir,"database",cloud_flag);
        node_file_to_stop(stackdir,"natgw",cloud_flag);
        if(strcmp(option,"all")==0){
            for(i=1;i<compute_node_num+1;i++){
                snprintf(node_name,15,"compute%d",i);
                node_file_to_stop(stackdir,node_name,cloud_flag);
            }
        }
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!" RESET_DISPLAY "\n");
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<5;i++){
            printf("[ -WAIT- ] Refreshing the cluster now %d ...\r",5-i);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
        if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,0)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            return -1;
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    update_usage_summary(workdir,crypto_keyfile,"database","start");
    update_usage_summary(workdir,crypto_keyfile,"natgw","start");
    if(strcmp(option,"all")==0){
        for(i=1;i<compute_node_num+1;i++){
            snprintf(string_temp,127,"compute%d",i);
            update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
        }
    }
    update_cluster_summary(workdir,crypto_keyfile);

    for(i=0;i<10;i++){
        printf("[ -WAIT- ] Still need to wait for %d sec(s) ...\r",10-i);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
    sync_statefile(workdir,sshkeydir);
    remote_exec(workdir,sshkeydir,"quick",1);
    if(strcmp(option,"all")==0){
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The cluster is in the state of " HIGH_CYAN_BOLD "full" RESET_DISPLAY " running.\n");
    }
    else{
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The cluster is in the state of " HIGH_CYAN_BOLD "minimal" RESET_DISPLAY " running.\n");
    }
    remote_exec_general(workdir,sshkeydir,"root","systemctl restart xrdp","-n",0,0,"","");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

/*
 * Return -3: failed to get workdir
 * Return -1: Cluster not empty
 * return  1: Failed to get conf
 * return  0: Get succeeded
 */
int get_default_conf(char* cluster_name, char* crypto_keyfile, char* edit_flag){
    char workdir[DIR_LENGTH]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -3;
    }
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char cloud_flag[32]="";
    char filename_temp[FILENAME_LENGTH]="";
    char url_aws_root[LOCATION_LENGTH_EXTENDED]="";
    char url_alicloud_root[LOCATION_LENGTH_EXTENDED]="";
    char url_qcloud_root[LOCATION_LENGTH_EXTENDED]="";
    char url_hwcloud_root[LOCATION_LENGTH_EXTENDED]="";
    char url_baiducloud_root[LOCATION_LENGTH_EXTENDED]="";
    char url_azure_root[LOCATION_LENGTH_EXTENDED]="";
    char url_gcp_root[LOCATION_LENGTH_EXTENDED]="";
    char confdir[DIR_LENGTH+16]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";

    create_and_get_vaultdir(workdir,vaultdir);
    if(code_loc_flag_var==1){
        snprintf(url_alicloud_root,LOCATION_LENGTH_EXTENDED-1,"%s%salicloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_qcloud_root,LOCATION_LENGTH_EXTENDED-1,"%s%sqcloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_aws_root,LOCATION_LENGTH_EXTENDED-1,"%s%saws%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_hwcloud_root,LOCATION_LENGTH_EXTENDED-1,"%s%shwcloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_baiducloud_root,LOCATION_LENGTH_EXTENDED-1,"%s%sbaidu%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_azure_root,LOCATION_LENGTH_EXTENDED-1,"%s%sazure%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        snprintf(url_gcp_root,LOCATION_LENGTH_EXTENDED-1,"%s%sgcp%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
    }
    else{
        snprintf(url_alicloud_root,LOCATION_LENGTH_EXTENDED-1,"%salicloud/",url_code_root_var);
        snprintf(url_qcloud_root,LOCATION_LENGTH_EXTENDED-1,"%sqcloud/",url_code_root_var);
        snprintf(url_aws_root,LOCATION_LENGTH_EXTENDED-1,"%saws/",url_code_root_var);
        snprintf(url_hwcloud_root,LOCATION_LENGTH_EXTENDED-1,"%shwcloud/",url_code_root_var);
        snprintf(url_baiducloud_root,LOCATION_LENGTH_EXTENDED-1,"%sbaidu/",url_code_root_var);
        snprintf(url_azure_root,LOCATION_LENGTH_EXTENDED-1,"%sazure/",url_code_root_var);
        snprintf(url_gcp_root,LOCATION_LENGTH_EXTENDED-1,"%sgcp/",url_code_root_var);
    }
    get_cloud_flag(workdir,cloud_flag);
    snprintf(confdir,DIR_LENGTH+15,"%s%sconf%s",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,confdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s.prev %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_alicloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_qcloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_aws_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_hwcloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_hwcloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_baiducloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_baiducloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_azure_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_azure_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        if(code_loc_flag_var==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s%stf_prep.conf %s",COPY_FILE_CMD,url_gcp_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s%stf_prep.conf",url_gcp_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else{
        return 1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    find_and_nreplace(filename_temp,LINE_LENGTH_SHORT,"CLUSTER_ID","","","","","hpcnow",cluster_name);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Default configuration file has been downloaded.\n");
    if(strcmp(edit_flag,"edit")!=0){
        return 0;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf",EDITOR_CMD,confdir,PATH_SLASH);
    system(cmdline);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The configuration file has been modified and saved.\n");
    return 0;
}

/*
 * Return -3: Failed to get workdir
 * Return -1: Cluster not empty
 * Return  1: User denied
 * Return  0: Normal exit
 */
int edit_configuration_file(char* cluster_name, char* crypto_keyfile, int batch_flag_local){
    char workdir[DIR_LENGTH]="";
    int run_flag;
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -3;
    }
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        run_flag=prompt_to_confirm("Cluster configuration file not found. Would you like to get one?",CONFIRM_STRING_QUICK,batch_flag_local);
        if(run_flag!=0){
            return 1;
        }
        get_default_conf(cluster_name,crypto_keyfile,"edit");
        return 0;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",EDITOR_CMD,filename_temp);
    system(cmdline);
    return 0;
}

/*
 * Return -3: Failed to get workdir
 * Return -1: Cluster not empty
 * Return  1: User denied
 * Return  0: Normal exit
 */
int remove_conf(char* cluster_name){
    char workdir[DIR_LENGTH]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -3;
    }
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return 1;
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 0;
    }
}

int rebuild_nodes(char* workdir, char* crypto_keyfile, char* option, int batch_flag_local, tf_exec_config* tf_run){
    if(strcmp(option,"mc")!=0&&strcmp(option,"mcdb")!=0&&strcmp(option,"all")!=0){
        return -5;
    }
    if(cluster_empty_or_not(workdir)==0){
        return -1;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char base_tf[FILENAME_LENGTH]="";
    char master_tf[FILENAME_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
    char* sshkey_folder=SSHKEY_DIR;
    char cloud_flag[16]="";
    char node_name[16]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char username_temp[64]="";
    char user_status_temp[32]="";
    char user_line_temp[256]="";
    FILE* file_p=NULL;
    int i;
    int compute_node_num=0;
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -7;
    }
    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("|*         YOU ARE REBUILDING THE CLUSTER NODES! YOUR CRITICAL NODES WILL\n");
    printf("|*         BE REMOVED AND RECREATED ! THIS OPERATION MAY FAIL DUE TO VAR-\n");
    printf("|*         -IOUS REASONS, YOU WILL HAVE TO DESTROY THE WHOLE CLUSTER AND\n");
    printf("|*         RE-INIT! THIS OPERATION IS NOT RECOMMENDED." RESET_DISPLAY "\n\n");

    if(prompt_to_confirm("ARE YOU SURE ?",CONFIRM_STRING,batch_flag_local)==1){
        return 1;
    }

    if(strcmp(option,"mc")==0){
        printf("|          * Will rebuild the " WARN_YELLO_BOLD "master" RESET_DISPLAY " and " WARN_YELLO_BOLD "compute" RESET_DISPLAY " nodes.\n");
    }
    else if(strcmp(option,"mcdb")==0){
        printf("|          * Will rebuild the " WARN_YELLO_BOLD "master" RESET_DISPLAY ", " WARN_YELLO_BOLD "compute" RESET_DISPLAY " and " WARN_YELLO_BOLD "mariadb" RESET_DISPLAY " nodes.\n");
    }
    else{
        printf("|          * Will try to rebuild " WARN_YELLO_BOLD "all" RESET_DISPLAY " the cluster nodes.\n");
    }
    create_and_get_stackdir(workdir,stackdir);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stmp %s && %s %s%stmp%s* %s",MKDIR_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT,DELETE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_master.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute*.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(strcmp(option,"mcdb")==0||strcmp(option,"all")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_database.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(strcmp(option,"all")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_natgw.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    remote_exec_general(workdir,sshkey_folder,"root","/usr/hpc-now/profile_bkup_rstr.sh backup","-n",0,0,"","");
    decrypt_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Removing previous nodes ...\n");
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the previous nodes. Rolling back now ..." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stmp%s* %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stmp %s",DELETE_FOLDER_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        delete_decrypted_files(workdir,crypto_keyfile);
        return 3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stmp%s* %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,stackdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stmp %s",DELETE_FOLDER_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    decrypt_files(workdir,crypto_keyfile);
    get_cloud_flag(workdir,cloud_flag);
    node_file_to_running(stackdir,"master",cloud_flag);
    node_file_to_running(stackdir,"natgw",cloud_flag);
    node_file_to_running(stackdir,"database",cloud_flag);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    for(i=1;i<compute_node_num+1;i++){
        snprintf(node_name,15,"compute%d",i);
        node_file_to_running(stackdir,node_name,cloud_flag);
    }

    create_and_get_vaultdir(workdir,vaultdir);
    decrypt_user_passwords(workdir,crypto_keyfile);
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    snprintf(base_tf,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    snprintf(master_tf,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    update_tf_passwords(base_tf,master_tf,user_passwords);

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully removed previous nodes. Rebuilding new nodes ...\n");
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to rebuild the nodes. Exit now." RESET_DISPLAY "\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 5;
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",ALI_SLEEP_TIME);
        for(i=0;i<ALI_SLEEP_TIME;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",ALI_SLEEP_TIME-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",QCLOUD_SLEEP_TIME);
        for(i=0;i<QCLOUD_SLEEP_TIME;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",QCLOUD_SLEEP_TIME-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_GLOBAL);
        for(i=0;i<AWS_SLEEP_TIME_GLOBAL;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_GLOBAL-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0||strcmp(cloud_flag,"CLOUD_E")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",GENERAL_SLEEP_TIME);
        for(i=0;i<GENERAL_SLEEP_TIME;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",GENERAL_SLEEP_TIME-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0||strcmp(cloud_flag,"CLOUD_G")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",2*GENERAL_SLEEP_TIME);
        for(i=0;i<2*GENERAL_SLEEP_TIME;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",2*GENERAL_SLEEP_TIME-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else{
        return -127;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,sshkey_folder);
    remote_exec_general(workdir,sshkey_folder,"root","/usr/hpc-now/profile_bkup_rstr.sh restore","-n",0,0,"","");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rebuilding the cluster users now ...\n");
    file_p=fopen(user_passwords,"r");
    get_user_sshkey(cluster_name,"root","ENABLED",sshkey_folder,crypto_keyfile);
    while(!feof(file_p)){
        fngetline(file_p,user_line_temp,256);
        if(strlen(user_line_temp)==0){
            continue;
        }
        get_seq_nstring(user_line_temp,' ',2,username_temp,64);
        get_seq_nstring(user_line_temp,' ',4,user_status_temp,32);
        get_user_sshkey(cluster_name,username_temp,user_status_temp,sshkey_folder,crypto_keyfile);
    }
    fclose(file_p);
    delete_decrypted_user_passwords(workdir);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    update_cluster_summary(workdir,crypto_keyfile);
    printf(WARN_YELLO_BOLD "[ -INFO- ] The rebuild process may need 7 minutes. Please do not operate\n");
    printf("|          this cluster during the period. Exit now." RESET_DISPLAY "\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int switch_cluster_payment(char* cluster_name, char* new_payment_method, char* crypto_keyfile, tf_exec_config* tf_run){
    char workdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cloud_flag[32]="";
    char curr_payment_method[8]="";
    char statefile[FILENAME_LENGTH]="";
    if(cluster_name_check(cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD " is not in the registry." RESET_DISPLAY "\n",cluster_name);
        return 1;
    }
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return 1;
    }
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] This operation is not valid for the current Cloud %s. Exit now." RESET_DISPLAY "\n",cloud_flag);
        return -5;
    }
    if(strcmp(new_payment_method,"od")!=0&&strcmp(new_payment_method,"month")!=0){
        return -3;
    }
    create_and_get_stackdir(workdir,stackdir);
    snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    get_state_nvalue(workdir,"payment_method:",curr_payment_method,8);
    if(strcmp(curr_payment_method,new_payment_method)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster payment has already been " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD "." RESET_DISPLAY "\n",curr_payment_method);
        return 3;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Switching the payment method from " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " to " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",curr_payment_method,new_payment_method);
    if(strcmp(new_payment_method,"month")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Please switch to " HIGH_CYAN_BOLD "od" WARN_YELLO_BOLD " if you'd like to destroy or remove this cluster.\n");
        printf("|          " HIGH_CYAN_BOLD "Automatic renewal" WARN_YELLO_BOLD " will be activated." RESET_DISPLAY "\n");
    }
    decrypt_files(workdir,crypto_keyfile);
    if(strcmp(new_payment_method,"month")==0){
        modify_payment_lines(stackdir,cloud_flag,"add");
    }
    else{
        modify_payment_lines(stackdir,cloud_flag,"del");
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to switch the payment method to " WARN_YELLO_BOLD "%s" RESET_DISPLAY " .\n",new_payment_method);
        if(strcmp(new_payment_method,"month")==0){
            modify_payment_lines(stackdir,cloud_flag,"del");
        }
        else{
            modify_payment_lines(stackdir,cloud_flag,"add");
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        return 5;
    }
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Congrats! The payment method is now " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",new_payment_method);
    return 0;
}

int view_run_log(char* workdir, char* stream, char* run_option, char* view_option, char* export_dest){
    char logfile[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char real_export_dest[FILENAME_LENGTH]="";
    char real_stream[16]="";
    if(strcmp(stream,"err")!=0&&strcmp(stream,"dbg")!=0){
        strcpy(real_stream,"std");
    }
    else{
        strcpy(real_stream,stream);
    }
    if(strcmp(run_option,"realtime")!=0&&strcmp(run_option,"archive")!=0){
        if(strcmp(real_stream,"std")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else if(strcmp(real_stream,"err")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_dbg.log",workdir,PATH_SLASH,PATH_SLASH);
        }
    }
    else if(strcmp(run_option,"realtime")==0){
        if(strcmp(real_stream,"std")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else if(strcmp(real_stream,"err")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_dbg.log",workdir,PATH_SLASH,PATH_SLASH);
        }
    }
    else{
        if(strcmp(real_stream,"std")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.log.archive",workdir,PATH_SLASH,PATH_SLASH);
        }
        else if(strcmp(real_stream,"err")==0){
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log.archive",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            snprintf(logfile,FILENAME_LENGTH-1,"%s%slog%stf_dbg.log.archive",workdir,PATH_SLASH,PATH_SLASH);
        }
    }
    if(file_exist_or_not(logfile)!=0){
        return -1;
    }
    if(strcmp(view_option,"print")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s",CAT_FILE_CMD,logfile);
        system(cmdline);
    }
    else{
#ifdef _WIN32
        if(tail_f_for_windows(logfile)==1){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Time is up. Please run this command again." RESET_DISPLAY "\n");
        }
#else
        snprintf(cmdline,CMDLINE_LENGTH-1,"tail -f %s",logfile);
        system(cmdline);
#endif
    }
    if(strlen(export_dest)!=0){
        local_path_parser(export_dest,real_export_dest);
        if(folder_exist_or_not(real_export_dest)==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s%s %s",COPY_FILE_CMD,logfile,real_export_dest,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported the logfile to the specified folder " HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",real_export_dest);
        }
        else if(file_creation_test(real_export_dest)==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s %s",COPY_FILE_CMD,logfile,real_export_dest,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported the logfile to the specified file " HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",real_export_dest);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified dest path %s doesn't work (either file already exists or folder doesn't exit).\n" RESET_DISPLAY,real_export_dest);
        }
    }
    return 0;
}