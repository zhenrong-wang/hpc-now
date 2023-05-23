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
#else
#include <conio.h> // This header is not standard! ONLY for mingw
#endif

#include "now_macros.h"
#include "general_funcs.h"
#include "cluster_general_funcs.h"
#include "general_print_info.h"
#include "cluster_operations.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern int code_loc_flag_var;

int exit_current_cluster(void){
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,CURRENT_CLUSTER_INDICATOR,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int switch_to_cluster(char* target_cluster_name){
    char* current_cluster=CURRENT_CLUSTER_INDICATOR;
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_workdir[DIR_LENGTH]="";
    FILE* file_p=NULL;
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name is not in the registry. Exit now.\n" RESET_DISPLAY);
        return 1;
    }
    if(show_current_cluster(temp_workdir,temp_cluster_name,0)==0){
        if(strcmp(temp_cluster_name,target_cluster_name)==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You are operating the cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " now. No need to switch.\n",target_cluster_name);
            return 3;
        }
    }
    file_p=fopen(current_cluster,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create current cluster indicator. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    fprintf(file_p,"%s",target_cluster_name);
    fclose(file_p);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully switched to the cluster " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",target_cluster_name);
    return 0;
}

int add_to_cluster_registry(char* new_cluster_name){
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    FILE* file_p=fopen(cluster_registry,"a+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry. Exit now." RESET_DISPLAY);
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
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the cluster %s from the registry.\n" RESET_DISPLAY,deleted_cluster_name);
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
    sprintf(cmdline,"%s %s %s %s",MOVE_FILE_CMD,filename_temp,cluster_registry,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int list_all_cluster_names(void){
    FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"r");
    char registry_line[LINE_LENGTH_SHORT]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
//    int getline_flag=0;
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the registry. Please repair the HPC-NOW services.\n" RESET_DISPLAY);
        return -1;
    }
    if(file_empty_or_not(ALL_CLUSTER_REGISTRY)==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The registry is empty. Please create one to operate.\n");
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " List of all the clusters:\n");
    while(fgetline(file_p,registry_line)!=1){
        if(strlen(registry_line)!=0){
            if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0){
                printf("|          %s\n",registry_line);
            }
            else{
                get_seq_string(registry_line,' ',4,temp_cluster_name);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf(HIGH_GREEN_BOLD "|  active: %s\n" RESET_DISPLAY,registry_line);
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
    char cloud_flag[32]="";
    int i=0;
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    if(strlen(target_cluster_name)==0){
        if(show_current_cluster(temp_cluster_workdir,temp_cluster_name,0)==1){
            fclose(file_p);
            return 1;
        }
        get_cloud_flag(temp_cluster_workdir,cloud_flag);
        if(check_pslock(temp_cluster_workdir)!=0){
            printf(HIGH_GREEN_BOLD "|  active: <> %s | %s | * OPERATION-IN-PROGRESS *\n" RESET_DISPLAY,temp_cluster_name,cloud_flag);
            fclose(file_p);
            return 0;
        }
        decrypt_files(temp_cluster_workdir,crypto_keyfile);
        printf(HIGH_GREEN_BOLD "|  active: <> %s | ",temp_cluster_name);
        if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
            printf("%s | * EMPTY CLUSTER *\n" RESET_DISPLAY,cloud_flag);
        }
        printf(RESET_DISPLAY);
        delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        fclose(file_p);
        return 0;
    }
    if(strcmp(target_cluster_name,"all")==0||strcmp(target_cluster_name,"ALL")==0||strcmp(target_cluster_name,"All")==0){
        while(fgetline(file_p,registry_line)==0){
            if(strlen(registry_line)!=0){
                get_seq_string(registry_line,' ',4,temp_cluster_name);
                get_workdir(temp_cluster_workdir,temp_cluster_name);
                get_cloud_flag(temp_cluster_workdir,cloud_flag);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf(HIGH_GREEN_BOLD "|  active: <> %s | ",temp_cluster_name);
                }
                else{
                    printf(GENERAL_BOLD "|          <> %s | ",temp_cluster_name);
                }
                if(check_pslock(temp_cluster_workdir)!=0){
                    printf("%s | * OPERATION-IN-PROGRESS *\n" RESET_DISPLAY,cloud_flag);
                    i++;
                    continue;
                }
                decrypt_files(temp_cluster_workdir,crypto_keyfile);
                i++;
                if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
                    printf(GENERAL_BOLD "%s | * EMPTY CLUSTER *\n" RESET_DISPLAY,cloud_flag);
                }
                printf(RESET_DISPLAY);
                delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            }
        }
        fclose(file_p);
        if(i==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The registry is empty. Have you created any clusters?\n" RESET_DISPLAY);
            return 0;
        }
        return 0;
    }
    fclose(file_p);
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        return 3;
    }
    else{
        get_workdir(temp_cluster_workdir,target_cluster_name);
        get_cloud_flag(temp_cluster_workdir,cloud_flag);
        if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
            printf(HIGH_GREEN_BOLD "|  active: <> %s | ",temp_cluster_name);
        }
        else{
            printf(GENERAL_BOLD "|          <> %s | ",temp_cluster_name);
        }
        if(check_pslock(temp_cluster_workdir)!=0){
            printf("%s | * OPERATION-IN-PROGRESS * \n" RESET_DISPLAY,cloud_flag);
            return 0;
        }
        decrypt_files(temp_cluster_workdir,crypto_keyfile);
        if(graph(temp_cluster_workdir,crypto_keyfile,1)!=0){
            printf("%s | * EMPTY CLUSTER *\n" RESET_DISPLAY,cloud_flag);
        }
        printf(RESET_DISPLAY);
        delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        return 0;
    }
}

int refresh_cluster(char* target_cluster_name, char* crypto_keyfile, char* force_flag){
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Cannot open the registry. the HPC-NOW service cannot work properly. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_cluster_workdir[DIR_LENGTH]="";
    if(strlen(target_cluster_name)==0){
        if(show_current_cluster(temp_cluster_workdir,temp_cluster_name,0)==1){
            return 1;
        }
        else{
            if(strcmp(force_flag,"force")==0){
                printf(GENERAL_BOLD "\n");
                printf("|*                                C A U T I O N !                                  \n");
                printf("|*                                                                                 \n");
                printf("|*   YOU ARE REFRESHING THE CLUSTER *WITHOUT* CHECKING OPERATION LOCK STATUS !     \n");
                printf("|*   PLEASE MAKE SURE CURRENTLY THE CLUSTER IS *NOT* IN A OPERATION PROGRESS !     \n");
                printf("|*                                                                                 \n");
                printf("|*                                C A U T I O N !                                  \n\n");
            }
            else{
                if(check_pslock(temp_cluster_workdir)!=0){
                    return -3;
                }
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Refreshing the current cluster now ...\n");
            decrypt_files(temp_cluster_workdir,crypto_keyfile);
            if(terraform_execution(TERRAFORM_EXEC,"apply",temp_cluster_workdir,crypto_keyfile,OPERATION_ERROR_LOG,1)!=0){
                delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
                return -5;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
            getstate(temp_cluster_workdir,crypto_keyfile);
            graph(temp_cluster_workdir,crypto_keyfile,0);
            printf("|\n");
            update_cluster_summary(temp_cluster_workdir,crypto_keyfile);
            delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            return 0;
        }
    }
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not in the registry. Exit now.\n" RESET_DISPLAY);
        return 7;
    }
    else{
        get_workdir(temp_cluster_workdir,target_cluster_name);
        if(strcmp(force_flag,"force")==0){
            printf(GENERAL_BOLD "\n");
            printf("|*                                C A U T I O N !                                  \n");
            printf("|*                                                                                 \n");
            printf("|*   YOU ARE REFRESHING THE CLUSTER *WITHOUT* CHECKING OPERATION LOCK STATUS !     \n");
            printf("|*   PLEASE MAKE SURE CURRENTLY THE CLUSTER IS *NOT* IN A OPERATION PROGRESS !     \n");
            printf("|*                                                                                 \n");
            printf("|*                                C A U T I O N !                                  \n\n");
        }
         else{
            if(check_pslock(temp_cluster_workdir)!=0){
                return 3;
            }
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Refreshing the target cluster %s now ...\n",temp_cluster_name);
        decrypt_files(temp_cluster_workdir,crypto_keyfile);
        if(terraform_execution(TERRAFORM_EXEC,"apply",temp_cluster_workdir,crypto_keyfile,OPERATION_ERROR_LOG,1)!=0){
            delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
            return 5;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
        getstate(temp_cluster_workdir,crypto_keyfile);
        graph(temp_cluster_workdir,crypto_keyfile,0);
        printf("|\n");
        update_cluster_summary(temp_cluster_workdir,crypto_keyfile);
        delete_decrypted_files(temp_cluster_workdir,crypto_keyfile);
        return 2;
    }
}

int remove_cluster(char* target_cluster_name, char*crypto_keyfile, char* force_flag){
    if(strlen(target_cluster_name)<CLUSTER_ID_LENGTH_MIN||strlen(target_cluster_name)>CLUSTER_ID_LENGTH_MAX){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified name %s is invalid.\n" RESET_DISPLAY,target_cluster_name);
        return 1;
    }
    char cluster_workdir[DIR_LENGTH]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char log_trash[FILENAME_LENGTH]="";
    char tf_realtime_log[FILENAME_LENGTH]="";
    char tf_archive_log[FILENAME_LENGTH]="";
    sprintf(log_trash,"%s%slog_trashbin.txt",HPC_NOW_ROOT_DIR,PATH_SLASH);
    if(cluster_name_check_and_fix(target_cluster_name,temp_cluster_name)!=-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,target_cluster_name);
        list_all_cluster_names();
        return 3;
    }
    get_workdir(cluster_workdir,target_cluster_name);
    sprintf(tf_realtime_log,"%s%slog%stf_prep.log",cluster_workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_archive_log,"%s%slog%stf_prep.log.archive",cluster_workdir,PATH_SLASH,PATH_SLASH);
    if(strcmp(force_flag,"force")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Removing the specified cluster *WITHOUT* state or resource check.\n" RESET_DISPLAY);
    }
    else{
        if(cluster_empty_or_not(cluster_workdir)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster is *NOT* empty!\n" RESET_DISPLAY);
            glance_clusters(target_cluster_name,crypto_keyfile);
            printf(WARN_YELLO_BOLD "[ -WARN- ] Would you like to remove it anyway? This operation is *NOT* recoverable!\n" RESET_DISPLAY);
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to continuie: " );
            fflush(stdin);
            scanf("%s",doubleconfirm);
            getchar();
            if(strcmp(doubleconfirm,"y-e-s")==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Please type the cluster name %s to confirm. This opeartion is\n",target_cluster_name);
                printf("|          absolutely *NOT* recoverable!\n" RESET_DISPLAY);
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
                fflush(stdin);
                scanf("%s",doubleconfirm);
                getchar();
                if(strcmp(doubleconfirm,target_cluster_name)!=0){
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only %s is accepted to confirm. You chose to deny this operation.\n",target_cluster_name);
                    printf("|          Nothing changed.\n");
                    return 5;
                }
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
                printf("|          Nothing changed.\n");
                return 5;
            }
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified cluster is empty. This operation will remove all the related files\n");
            printf("|          from your system and registry. Would you like to continue?\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to continuie: ");
            fflush(stdin);
            scanf("%s",doubleconfirm);
            getchar();
            if(strcmp(doubleconfirm,"y-e-s")!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
                printf("|          Nothing changed.\n");
                return 5;
            }
        }
    }
    if(cluster_destroy(cluster_workdir,crypto_keyfile,"force")!=0){
        delete_decrypted_files(cluster_workdir,crypto_keyfile);
        return 7;
    }
    archive_log(log_trash,tf_archive_log);
    archive_log(log_trash,tf_realtime_log);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Removing all the related files ...\n");
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,cluster_workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Deleting the cluster from the registry ...\n");
    delete_from_cluster_registry(target_cluster_name);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The cluster %s has been removed completely.\n",target_cluster_name);
    return 0;
}

int create_new_cluster(char* crypto_keyfile, char* cluster_name, char* cloud_ak, char* cloud_sk, char* echo_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char real_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char input_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char filename_temp[FILENAME_LENGTH]="";
    int cluster_name_check_flag=0;
    char cloud_flag[16]="";
    FILE* file_p=NULL;
    char new_workdir[DIR_LENGTH]="";
    char new_vaultdir[DIR_LENGTH]="";
    char* keypair_temp=NULL;
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
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    fclose(file_p);
    file_p=fopen(current_cluster,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create the current cluster indicator. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    fclose(file_p);
    if(strlen(cluster_name)==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input the cluster name (A-Z | a-z | 0-9 | - , maximum length %d):\n",CLUSTER_ID_LENGTH_MAX);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        scanf("%s",input_cluster_name);
        getchar();
    }
    else{
        strcpy(input_cluster_name,cluster_name);
    }
    cluster_name_check_flag=cluster_name_check_and_fix(input_cluster_name,real_cluster_name);
    if(cluster_name_check_flag==127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster name only accepts English letters 'A-Z', 'a-z', '0-9' and '-'.\n");
        printf("|          The specified name %s contains illegal characters.\n",input_cluster_name);
        printf("|          Please check and retry. Exit now.\n" RESET_DISPLAY);
        return 1;
    }
    else if(cluster_name_check_flag==-127){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s already exists in the registry.\n",input_cluster_name);
        printf("|          Please check and retry. Exit now.\n" RESET_DISPLAY);
        return 1;
    }
    else if(cluster_name_check_flag==1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster name length <%d, add '-hpcnow'.\n" RESET_DISPLAY,CLUSTER_ID_LENGTH_MIN);
    }
    else if(cluster_name_check_flag==2){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster name length > %d, cut to %d.\n" RESET_DISPLAY,CLUSTER_ID_LENGTH_MAX,CLUSTER_ID_LENGTH_MAX);
    }
    else if(cluster_name_check_flag==-1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Would you like to use the random string %s as a cluster name? \n",real_cluster_name);
        printf("|          Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted as a confirmation. \n" RESET_DISPLAY);
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
            printf("|          Nothing changed.\n");
            return 3;
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the cluster name %s.\n",real_cluster_name);
#ifdef _WIN32
    strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
#elif __linux__
    strcpy(filename_temp,"/home/hpc-now/.secret.tmp.txt");
#elif __APPLE__
    strcpy(filename_temp,"/Users/hpc-now/.secret.tmp.txt");
#endif
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return -1;
    }
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
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
        strcpy(cloud_flag,"CLOUD_C");
        fprintf(file_p,"%s\n%s\n%s\n",access_key,secret_key,cloud_flag);
        fclose(file_p);
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.\n" RESET_DISPLAY);
        fclose(file_p);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 5;
    }
    sprintf(new_workdir,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,real_cluster_name,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,new_workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    create_and_get_vaultdir(new_workdir,new_vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s%s.secrets.key %s %s",now_crypto_exec,filename_temp,new_vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(filename_temp,"%s%scloud_flag.flg",new_vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    if(file_p!=NULL){
        fprintf(file_p,"%s\n",cloud_flag);
        fclose(file_p);
    }
    add_to_cluster_registry(real_cluster_name);
    switch_to_cluster(real_cluster_name);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The secrets key pair has been encrypted and stored locally. You can either:\n");
    printf("|          1. run 'hpcopr init' to create a default cluster. OR\n");
    printf("|          2. run 'hpcopr get-conf' to get the default cluster configuration, and run\n");
    printf("|              'hpcopr init' to create a customized cluster.\n");
    printf("|          You can also switch to this cluster name and operate this cluster later.\n");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
    return 0;
}

int rotate_new_keypair(char* workdir, char* cloud_ak, char* cloud_sk, char* crypto_keyfile, char* echo_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char *now_crypto_exec=NOW_CRYPTO_EXEC;
    int ak_length,sk_length;
    char* keypair_temp=NULL;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[32]="";
    char doubleconfirm[32]="";
    char access_key_prev[AKSK_LENGTH]="";
    char secret_key_prev[AKSK_LENGTH]="";
    char cloud_flag_prev[32]="";
    char md5sum[33]="";

#ifdef _WIN32
    strcpy(filename_temp,"c:\\programdata\\hpc-now\\secret.tmp.txt");
#elif __linux__
    strcpy(filename_temp,"/home/hpc-now/.secret.tmp.txt");
#elif __APPLE__
    strcpy(filename_temp,"/Users/hpc-now/.secret.tmp.txt");
#endif
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a temporary file in your system.\n");
        printf("|          Please check the available disk space. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp2,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp2)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently there is no secrets keypair. This working directory may be\n");
        printf("|          corrputed, which is very unusual. Please contact us via:\n");
        printf("|          info@hpc-now.com for troubleshooting. Exit now.\n" RESET_DISPLAY);
        return -3;
    }
    printf(GENERAL_BOLD "\n");
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
    printf("| ARE YOU SURE? Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY GENERAL_BOLD " is accepted to double confirm this operation:\n" RESET_DISPLAY);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    scanf("%s",doubleconfirm);
    getchar();
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    get_ak_sk(filename_temp2,crypto_keyfile,access_key_prev,secret_key_prev,cloud_flag_prev);
    if(strlen(cloud_ak)==0||strlen(cloud_sk)==0){
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
            printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
            return 3;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_A",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==36&&sk_length==32){
        strcpy(cloud_flag,"CLOUD_B");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: TencentCloud (HPC-NOW code: CLOUD_B).\n");
            printf("|          Please rotate a keypair from an TencentCloud account.\n");
            printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
            return 3;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_B",access_key,secret_key);
        fclose(file_p);
    }
    else if(ak_length==20&&sk_length==40){
        strcpy(cloud_flag,"CLOUD_C");
        if(strcmp(cloud_flag_prev,cloud_flag)!=0){
            fclose(file_p);
            printf(FATAL_RED_BOLD "[ FATAL: ] The new keypair comes from a different Cloud Service Vendor.\n");
            printf("|          Switching cloud vendors for a working directory is not permitted.\n");
            printf("|          Current Vendor: Amazon Web Services (HPC-NOW code: CLOUD_C).\n");
            printf("|          Please rotate a keypair from an Amazon Web Services account.\n");
            printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
            return 3;
        }
        fprintf(file_p,"%s\n%s\nCLOUD_C",access_key,secret_key);
        fclose(file_p);
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid key pair. Please double check your inputs. Exit now.\n" RESET_DISPLAY);
        fclose(file_p);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 3;
    }
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(cmdline,"%s encrypt %s %s%s.secrets.key %s %s",now_crypto_exec,filename_temp,vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%shpc_stack_base.tf.tmp",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(filename_temp2,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
        sprintf(cmdline,"%s decrypt %s %s %s %s",now_crypto_exec,filename_temp,filename_temp2,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        global_replace(filename_temp2,access_key_prev,access_key);
        global_replace(filename_temp2,secret_key_prev,secret_key);
        sprintf(cmdline,"%s encrypt %s %s %s %s",now_crypto_exec,filename_temp2,filename_temp,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The new secrets key pair has been encrypted and rotated locally.\n");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Exit now.\n");
    return 0;
}

int cluster_destroy(char* workdir, char* crypto_keyfile, char* force_flag){
    char doubleconfirm[32]="";
    char cmdline[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[LINE_LENGTH];
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    int i;
    int compute_node_num=0;
    printf(GENERAL_BOLD "\n");
    printf("|*                                C A U T I O N !                                  \n");
    printf("|*                                                                                 \n");
    printf("|*   YOU ARE DELETING THE WHOLE CLUSTER - INCLUDING ALL THE NODES AND *DATA*!      \n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          \n");
    printf("|*                                                                                 \n");
    printf("|*                                C A U T I O N !                                  \n");
    if(strcmp(force_flag,"force")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Destroying the current cluster *WITHOUT* confirmation.\n" RESET_DISPLAY);
    }
    else{
        printf("| ARE YOU SURE? Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to double confirm this operation:\n" RESET_DISPLAY);
        fflush(stdin);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        scanf("%s",doubleconfirm);
        getchar();
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
            printf("|          Nothing changed.\n");
            return 1;
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Cluster operation started ...\n");
    create_and_get_vaultdir(workdir,vaultdir);
    decrypt_files(workdir,crypto_keyfile);
    create_and_get_stackdir(workdir,stackdir);
    if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Some problems occoured. Retrying destroy now (1/2)...\n" RESET_DISPLAY);
        sleep(2);
        if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Some problems occoured. Retrying destroy now (2/2)...\n" RESET_DISPLAY);
            sleep(2);
            if(terraform_execution(tf_exec,"destroy",workdir,crypto_keyfile,error_log,1)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to destroy your cluster. This usually caused by either Terraform or\n");
                printf("|          the providers developed and maintained by cloud service providers.\n");
                printf("|          You *MUST* manually destroy the remaining cloud resources of this cluster.\n");
                printf("|          Exit now.\n" RESET_DISPLAY);
                delete_decrypted_files(workdir,crypto_keyfile);
                return -1;
            }
        }
    }
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
    for(i=0;i<compute_node_num;i++){
        sprintf(string_temp,"compute%d",i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    delete_decrypted_files(workdir,crypto_keyfile);
    sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s*.tf %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%scurrentstate %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%scompute_template %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shostfile_latest %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s*.tmp %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s*.txt %s%s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%sconf%stf_prep.conf %s%sconf%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,workdir,PATH_SLASH,PATH_SLASH,workdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The whole cluster has been destroyed successfully.\n");
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
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing deleted. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
                return -1;
            }
            del_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(del_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
            return 1;
        }
        if(del_num>compute_node_num){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified a number larger than the quantity of compute nodes.\n");
            printf("           Do you mean deleting *ALL* the compute nodes?\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            getchar();
            if(strcmp(string_temp,"y-e-s")!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to delete %d from %d compute node(s).",del_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                system(cmdline);
                sprintf(cmdline,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,i,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                system(cmdline);
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){ 
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                    sprintf(cmdline,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,i,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                }
                if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
            remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            for(i=compute_node_num-del_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 0;
        }
    }
    sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to delete *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(cmdline,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,i,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            sprintf(cmdline,"%s %s%shpc_stack_compute%d.tf %s%s %s",MOVE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,i,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
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
        printf(FATAL_RED_BOLD "[ FATAL: ] The number of nodes to be added is invalid. A number (1-%d) is needed.\n",MAXIMUM_ADD_NODE_NUMBER);
        printf("           Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    for(i=0;i<strlen(add_number_string);i++){
        if(*(add_number_string+i)<'0'||*(add_number_string+i)>'9'){
            printf(FATAL_RED_BOLD "[ FATAL: ] The number of nodes to be added is invalid. A number (1-%d) is needed.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("           Exit now.\n" RESET_DISPLAY);
            return -1;
        }
        else{
            add_number+=(*(add_number_string+i)-'0')*pow(10,strlen(add_number_string)-i-1);
        }
    }

    if(add_number>MAXIMUM_ADD_NODE_NUMBER||add_number<1){
        printf(FATAL_RED_BOLD "[ FATAL: ] The number of nodes to be added is out of range (1-%d). Exit now.\n" RESET_DISPLAY,MAXIMUM_ADD_NODE_NUMBER);
        return -1;
    }
    sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified to add %d compute node(s).",add_number);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster operation is in progress ...\n");
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    current_node_num=get_compute_node_num(filename_temp,"all");
    for(i=0;i<add_number;i++){
        sprintf(cmdline,"%s %s%scompute_template %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1+current_node_num,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1+current_node_num);
        sprintf(string_temp,"compute%d",i+1+current_node_num);
        global_replace(filename_temp,"compute1",string_temp);
        sprintf(string_temp,"comp%d",i+1+current_node_num);
        global_replace(filename_temp,"comp1",string_temp);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=0;i<add_number;i++){
            sprintf(cmdline,"%s %s%shpc_stack_compute%d.tf",DELETE_FILE_CMD,stackdir,PATH_SLASH,i+1+current_node_num);
            system(cmdline);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
    remote_exec(workdir,sshkey_dir,"connect",7);
    remote_exec(workdir,sshkey_dir,"all",8);
    for(i=0;i<add_number;i++){
        sprintf(string_temp,"compute%d",current_node_num+i+1);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been added.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int shutdown_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    int down_num=0;
    char filename_temp[FILENAME_LENGTH]="";
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing to be shutdown. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
                return -1;
            }
            down_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(down_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
            return 1;
        }
        if(down_num>compute_node_num){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified a number larger than the quantity of compute nodes.\n");
            printf("           Do you mean shutting down *ALL* the compute nodes?\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            getchar();
            if(strcmp(string_temp,"y-e-s")!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to shutdown %d from %d compute node(s).",down_num,compute_node_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                sprintf(node_name,"compute%d",i);
                node_file_to_stop(stackdir,node_name,cloud_flag);
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                    sprintf(node_name,"compute%d",i);
                    node_file_to_running(stackdir,node_name,cloud_flag);
                }
                if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num-down_num+1;i<compute_node_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been deleted.\n");
            return 0;
        }
    }
    sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to shutdown *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name,"compute%d",i);
        node_file_to_stop(stackdir,node_name,cloud_flag);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            sprintf(node_name,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been shut down.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char node_name[16]="";
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
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    decrypt_files(workdir,crypto_keyfile);
    getstate(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_num_on=get_compute_node_num(filename_temp,"on");
    if(compute_node_num==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, there is no compute nodes, nothing to be turned on. Exit now.\n" RESET_DISPLAY);
        return -1;
    }

    if(compute_node_num==compute_node_num_on){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently, all the compute nodes are in the state of running.\n");
        printf("|          No compute node needs to be turned on. Exit now.\n" RESET_DISPLAY);
        return -1;
    }

    if(strcmp(param,"all")!=0){
        for(i=0;i<strlen(param);i++){
            if(*(param+i)<'0'||*(param+i)>'9'){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
                return -1;
            }
            on_num+=(*(param+i)-'0')*pow(10,strlen(param)-1-i);
        }
        if(on_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'all' or a positive number. Exit now.\n" RESET_DISPLAY);
            return 1;
        }
        if(on_num+compute_node_num_on>compute_node_num){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You specified a number larger than the number of currently down nodes.\n");
            printf("           Do you mean turning on *ALL* the compute nodes?\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm:  ");
            fflush(stdin);
            scanf("%s",string_temp);
            getchar();
            if(strcmp(string_temp,"y-e-s")!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You chose to deny this operation. Exit now.\n");
                return 1;
            }
        }
        else{
            sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to turn on %d compute node(s).",on_num);
            printf("%s\n",string_temp);
            decrypt_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                sprintf(node_name,"compute%d",i);
                node_file_to_running(stackdir,node_name,cloud_flag);
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                    sprintf(node_name,"compute%d",i);
                    node_file_to_stop(stackdir,node_name,cloud_flag);
                }
                if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
            remote_exec(workdir,sshkey_dir,"quick",1);
            delete_decrypted_files(workdir,crypto_keyfile);
            for(i=compute_node_num_on+1;i<compute_node_num_on+on_num+1;i++){
                sprintf(string_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been turned on.\n");
            return 0;
        }
    }
    sprintf(string_temp,GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You planned to turn on *ALL* the %d compute node(s).",compute_node_num);
    printf("%s\n",string_temp);
    decrypt_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        sprintf(node_name,"compute%d",i);
        node_file_to_running(stackdir,node_name,cloud_flag);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
            sprintf(node_name,"compute%d",i);
            node_file_to_stop(stackdir,node_name,cloud_flag);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
    remote_exec(workdir,sshkey_dir,"quick",1);
    delete_decrypted_files(workdir,crypto_keyfile);
    for(i=compute_node_num_on+1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The specified compute nodes have been turned on.\n");
    return 0;
}

int check_reconfigure_list(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char single_line[64]="";
    char reconf_list[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(reconf_list,"%s%sreconf.list",stackdir,PATH_SLASH);
    if((file_p=fopen(reconf_list,"r"))==NULL){
        return -1;
    }
    while(fgetline(file_p,single_line)==0){
        printf("|          %s\n",single_line);
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
    char cloud_flag[16]="";
    int compute_node_num=0;
    int compute_node_down_num=0;
    char* sshkey_dir=SSHKEY_DIR;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char node_name_temp[32]="";
    char* tf_exec=TERRAFORM_EXEC;
    int cpu_core_num=0;
    int reinit_flag=0;
    char cmdline[CMDLINE_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    compute_node_down_num=get_compute_node_num(filename_temp,"down");
    if(compute_node_num==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Currently there is no compute nodes in your cluster. Exit now.\n" RESET_DISPLAY);
        return -1;
    }

    decrypt_files(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    sprintf(string_temp,"\"%s\"",new_config);
    if(find_multi_keys(filename_temp,string_temp,"","","","")==0||find_multi_keys(filename_temp,string_temp,"","","","")<0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid compute configuration. Exit now.\n" RESET_DISPLAY);
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    get_cloud_flag(workdir,cloud_flag);
    sprintf(filename_temp,"%s%scompute_template",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    if(strcmp(prev_config,new_config)==0){
        if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
            printf("|          Nothing changed. Exit now.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 1;
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            if(strlen(htflag)==0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"hton")!=0&&strcmp(htflag,"htoff")!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"hton")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            else if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
                printf("|          Nothing changed. Exit now.\n");
                delete_decrypted_files(workdir,crypto_keyfile);
                return 1;
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
                    sprintf(filename_temp2,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
                    sprintf(cmdline,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp2,filename_temp2,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                    global_replace(filename_temp2,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
                }
                reinit_flag=1;
            }
            if(find_multi_keys(filename_temp,"cpu_threads_per_core = 1","","","","")>0){
                for(i=1;i<compute_node_num+1;i++){
                    sprintf(filename_temp2,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
                    sprintf(cmdline,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp2,filename_temp2,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                    global_replace(filename_temp2,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
                }
                reinit_flag=1;
            }
            if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
                for(i=1;i<compute_node_num+1;i++){
                    sprintf(cmdline,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp2,filename_temp2,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                }
                if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
                    delete_decrypted_files(workdir,crypto_keyfile);
                    printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
                    return -127;
                }
                delete_decrypted_files(workdir,crypto_keyfile);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
                return -1;
            }
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
            }
            update_compute_template(stackdir,cloud_flag);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
            getstate(workdir,crypto_keyfile);
            graph(workdir,crypto_keyfile,0);
            printf("|\n");
            get_latest_hosts(stackdir,filename_temp);
            remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
            remote_exec(workdir,sshkey_dir,"connect",1);
            remote_exec(workdir,sshkey_dir,"all",2);
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name_temp,"compute%d",i);
                update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
            }
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The compute nodes have been reconfigured.\n");
            delete_decrypted_files(workdir,crypto_keyfile);
            return 0;
        }
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            sprintf(cmdline,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            global_replace(filename_temp,prev_config,new_config);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            sprintf(cmdline,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
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
                printf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
                global_replace(filename_temp,"cpu_threads_per_core = 1","cpu_threads_per_core = 2");
            }
            reinit_flag=1;
        }
        if(strcmp(htflag,"htoff")==0&&find_multi_keys(filename_temp,"cpu_threads_per_core = 2","","","","")>0){
            for(i=1;i<compute_node_num+1;i++){
                sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
                global_replace(filename_temp,"cpu_threads_per_core = 2","cpu_threads_per_core = 1");
            }
            reinit_flag=2;
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        for(i=1;i<compute_node_num+1;i++){
            sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
            sprintf(cmdline,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"stop");
    }
    update_compute_template(stackdir,cloud_flag);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
    if(compute_node_down_num!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Please turn on all the cluster nodes, log on to the master\n");
        printf("|          node, and run: " HIGH_GREEN_BOLD "sudo hpcmgr connect && sudo hpcmgr all" RESET_DISPLAY WARN_YELLO_BOLD "\n" RESET_DISPLAY);
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
        sprintf(node_name_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,node_name_temp,"start");
    }
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The compute nodes have been reconfigured.\n");
    sprintf(cmdline,"%s %s%s*bak %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
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
    sprintf(filename_temp,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    sprintf(string_temp,"\"%s\"",new_config);
    if(find_multi_keys(filename_temp,string_temp,"","","","")==0||find_multi_keys(filename_temp,string_temp,"","","","")<0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid master node configuration. Exit now.\n" RESET_DISPLAY);
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    get_cloud_flag(workdir,cloud_flag);
    sprintf(filename_temp,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,prev_config);
    if(strcmp(prev_config,new_config)==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified configuration is the same as previous configuration.\n");
        printf("|          Nothing changed. Exit now.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 1;
    }
    sprintf(filename_temp,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    sprintf(cmdline,"%s %s %s.bak %s",COPY_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    global_replace(filename_temp,prev_config,new_config);
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ... \n");
        sprintf(cmdline,"%s %s.bak %s %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -3;
    }
    sprintf(cmdline,"%s %s.bak %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds for remote execution ... \r",GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_dir,filename_temp,"/root/hostfile","root","put");
    remote_exec(workdir,sshkey_dir,"connect",1);
    remote_exec(workdir,sshkey_dir,"all",2);
    update_cluster_summary(workdir,crypto_keyfile);
    delete_decrypted_files(workdir,crypto_keyfile);
    update_usage_summary(workdir,crypto_keyfile,"master","start");
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The master node has been reconfigured.\n");
    return 0;
}

int cluster_sleep(char* workdir, char* crypto_keyfile){
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(find_multi_keys(filename_temp,"running","","","","")==0&&find_multi_keys(filename_temp,"Running","","","","")==0&&find_multi_keys(filename_temp,"RUNNING","","","","")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not running. Please wake up first.\n");
        printf("|          Command: hpcopr wakeup minimal|all. Exit now.\n" RESET_DISPLAY);
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
        sprintf(node_name,"compute%d",i);
        node_file_to_stop(stackdir,node_name,cloud_flag);
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        node_file_to_running(stackdir,"master",cloud_flag);
        node_file_to_running(stackdir,"database",cloud_flag);
        node_file_to_running(stackdir,"natgw",cloud_flag);
        for(i=1;i<compute_node_num+1;i++){
            sprintf(node_name,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
            return -127;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster has been successfully rolled back.\n");
        return -1;
    }
    /*if(strcmp(cloud_flag,"CLOUD_C")==0){
        for(i=0;i<10;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",10-i);
            fflush(stdout);
            sleep(1);
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,0)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            return -1;
        }
    }*/
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    update_usage_summary(workdir,crypto_keyfile,"master","stop");
    update_usage_summary(workdir,crypto_keyfile,"database","stop");
    update_usage_summary(workdir,crypto_keyfile,"natgw","stop");
    for(i=1;i<compute_node_num+1;i++){
        sprintf(string_temp,"compute%d",i);
        update_usage_summary(workdir,crypto_keyfile,string_temp,"stop");
    }
    update_cluster_summary(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! All the nodes of have been shutdown.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option){
    if(strcmp(option,"all")!=0&&strcmp(option,"minimal")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either 'minimal' or 'all' as the second parameter.\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    char string_temp[128]="";
    char unique_cluster_id[64]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    char* tf_exec=TERRAFORM_EXEC;
    char* error_log=OPERATION_ERROR_LOG;
    int i;
    char filename_temp[FILENAME_LENGTH]="";
    char* sshkeydir=SSHKEY_DIR;
    char node_name[16]="";
    int compute_node_num=0;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fscanf(file_p,"%s",unique_cluster_id);
    fclose(file_p);
    create_and_get_stackdir(workdir,stackdir);
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -1;
    }
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
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
            sprintf(node_name,"compute%d",i);
            node_file_to_running(stackdir,node_name,cloud_flag);
        }
    }
    if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back now ...\n");
        node_file_to_stop(stackdir,"master",cloud_flag);
        node_file_to_stop(stackdir,"database",cloud_flag);
        node_file_to_stop(stackdir,"natgw",cloud_flag);
        if(strcmp(option,"all")==0){
            for(i=1;i<compute_node_num+1;i++){
                sprintf(node_name,"compute%d",i);
                node_file_to_stop(stackdir,node_name,cloud_flag);
            }
        }
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,1)!=0){
            delete_decrypted_files(workdir,crypto_keyfile);
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to roll back. The cluster may be corrupted!\n" RESET_DISPLAY);
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
        if(terraform_execution(tf_exec,"apply",workdir,crypto_keyfile,error_log,0)!=0){
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
            sprintf(string_temp,"compute%d",i);
            update_usage_summary(workdir,crypto_keyfile,string_temp,"start");
        }
    }
    update_cluster_summary(workdir,crypto_keyfile);
    if(strcmp(option,"all")==0){
        remote_exec(workdir,sshkeydir,"quick",1);
    }
    if(strcmp(option,"all")==0){
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The cluster is in the state of " HIGH_CYAN_BOLD "full" RESET_DISPLAY " running.\n");
    }
    else{
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Congrats! The cluster is in the state of " HIGH_CYAN_BOLD "minimal" RESET_DISPLAY " running.\n");
    }
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int get_default_conf(char* workdir, char* crypto_keyfile, int edit_flag){
    if(cluster_empty_or_not(workdir)!=0){
        return -1;
    }
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char temp_workdir[DIR_LENGTH]="";
    if(show_current_cluster(temp_workdir,temp_cluster_name,0)!=0){
        return 127;
    }
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
        sprintf(url_aws_root,"%s%saws%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        sprintf(url_qcloud_root,"%s%sqcloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
        sprintf(url_alicloud_root,"%s%salicloud%s",url_code_root_var,PATH_SLASH,PATH_SLASH);
    }
    else{
        sprintf(url_aws_root,"%saws/",url_code_root_var);
        sprintf(url_qcloud_root,"%sqcloud/",url_code_root_var);
        sprintf(url_alicloud_root,"%salicloud/",url_code_root_var);
    }
    get_cloud_flag(workdir,cloud_flag);
    sprintf(confdir,"%s%sconf%s",workdir,PATH_SLASH,PATH_SLASH);
    if(folder_exist_or_not(confdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,confdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    sprintf(filename_temp,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        sprintf(cmdline,"%s %s %s.prev %s",MOVE_FILE_CMD,filename_temp,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf %s",COPY_FILE_CMD,url_alicloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s%stf_prep.conf",url_alicloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf %s",COPY_FILE_CMD,url_qcloud_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s%stf_prep.conf",url_qcloud_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(code_loc_flag_var==1){
            sprintf(cmdline,"%s %s%stf_prep.conf %s%stf_prep.conf %s",COPY_FILE_CMD,url_aws_root,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"curl %stf_prep.conf -s -o %s%stf_prep.conf",url_aws_root,confdir,PATH_SLASH);
        }
        if(system(cmdline)!=0){
            return 1;
        }
    }
    else{
        return 1;
    }
    sprintf(filename_temp,"%s%stf_prep.conf",confdir,PATH_SLASH);
    find_and_replace(filename_temp,"CLUSTER_ID","","","","","hpcnow",temp_cluster_name);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Default configuration file has been downloaded.\n");
    if(edit_flag!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Would you like to edit it now? Input " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " to confirm:\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        getchar();
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
            return 3;
        }
    }
    sprintf(cmdline,"%s %s%stf_prep.conf",EDITOR_CMD,confdir,PATH_SLASH);
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
    sprintf(filename_temp,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Cluster configuration file not found. Would you like to get one?\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm: ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        getchar();
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            return 1;
        }
        get_default_conf(workdir,crypto_keyfile,0);
        return 0;
    }
    sprintf(cmdline,"%s %s",EDITOR_CMD,filename_temp);
    system(cmdline);
    return 0;
}

int rebuild_nodes(char* workdir, char* crypto_keyfile, char* option){
    if(strcmp(option,"mc")!=0&&strcmp(option,"mcdb")!=0&&strcmp(option,"all")!=0){
        return -5;
    }
    if(cluster_empty_or_not(workdir)==0){
        return -1;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char remote_commands[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* sshkey_folder=SSHKEY_DIR;
    char cloud_flag[16]="";
    char node_name[16]="";
    char doubleconfirm[64]="";
    char bucket_conf[FILENAME_LENGTH]="";
    char bucket_id[32]="";
    int i;
    int compute_node_num=0;
    printf(GENERAL_BOLD "\n");
    printf("|*                                C A U T I O N !                            \n");
    printf("|*                                                                           \n");
    printf("|*   YOU ARE REBUILDING THE CLUSTER NODES! YOUR CRITICAL NODES WILL BE       \n");
    printf("|*   REMOVED AND RECREATED ! THIS OPERATION MAY FAIL DUE TO VARIOUS REASONS. \n");
    printf("|*   IF ANYTHING GOES WRONG, YOU WILL HAVE TO DESTROY THE WHOLE CLUSTER      \n");
    printf("|*   AND RE-INIT ! Usually we do not recommend users to do this operation.   \n");
    printf("|*                                                                           \n");
    printf("|*                                C A U T I O N !                            \n");
    printf("| ARE YOU SURE? Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY GENERAL_BOLD " is accepted to double confirm this operation:\n\n" RESET_DISPLAY);
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    scanf("%s",doubleconfirm);
    getchar();
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
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
    sprintf(cmdline,"%s %s%stmp %s && %s %s%stmp%s* %s",MKDIR_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT,DELETE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack_master.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%shpc_stack_compute*.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(strcmp(option,"mcdb")==0||strcmp(option,"all")==0){
        sprintf(cmdline,"%s %s%shpc_stack_database.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    if(strcmp(option,"all")==0){
        sprintf(cmdline,"%s %s%shpc_stack_natgw.tf.tmp %s%stmp%s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    decrypt_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Removing previous nodes ...\n");
    if(terraform_execution(TERRAFORM_EXEC,"apply",workdir,crypto_keyfile,OPERATION_ERROR_LOG,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the previous nodes. Rolling back now ...\n" RESET_DISPLAY);
        sprintf(cmdline,"%s %s%stmp%s* %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%stmp %s",DELETE_FOLDER_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        delete_decrypted_files(workdir,crypto_keyfile);
        return 3;
    }
    sprintf(cmdline,"%s %s%stmp%s* %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,PATH_SLASH,stackdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%stmp %s",DELETE_FOLDER_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    decrypt_files(workdir,crypto_keyfile);
    get_cloud_flag(workdir,cloud_flag);
    node_file_to_running(stackdir,"master",cloud_flag);
    node_file_to_running(stackdir,"natgw",cloud_flag);
    node_file_to_running(stackdir,"database",cloud_flag);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    compute_node_num=get_compute_node_num(filename_temp,"all");
    for(i=1;i<compute_node_num+1;i++){
        sprintf(node_name,"compute%d",i);
        node_file_to_running(stackdir,node_name,cloud_flag);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully removed previous nodes. Rebuilding new nodes ...\n");
    if(terraform_execution(TERRAFORM_EXEC,"apply",workdir,crypto_keyfile,OPERATION_ERROR_LOG,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to rebuild the nodes. Exit now.\n" RESET_DISPLAY);
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
    else{
        return -127;
    }
    get_cluster_bucket_id(workdir,crypto_keyfile,bucket_id);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the cluster operation:\n|\n");
    getstate(workdir,crypto_keyfile);
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    decrypt_get_bucket_conf(workdir,crypto_keyfile,bucket_conf);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        remote_copy(workdir,sshkey_folder,bucket_conf,"/root/.ossutilconfig","root","put");
        sprintf(remote_commands,"echo -e \"export BUCKET=oss://%s\" >> /etc/profile",bucket_id);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        remote_copy(workdir,sshkey_folder,bucket_conf,"/root/.cos.conf","root","put");
        sprintf(remote_commands,"echo -e \"export BUCKET=cos://%s\" >> /etc/profile",bucket_id);
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        remote_copy(workdir,sshkey_folder,bucket_conf,"/root/.s3cfg","root","put");
        sprintf(remote_commands,"echo -e \"export BUCKET=s3://%s\" >> /etc/profile",bucket_id);
    }
    remote_exec_general(workdir,sshkey_folder,"root",remote_commands,0);
    get_latest_hosts(stackdir,filename_temp);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/hostfile","root","put");
    create_and_get_vaultdir(workdir,vaultdir);
    decrypt_user_passwords(workdir,crypto_keyfile);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    remote_copy(workdir,sshkey_folder,filename_temp,"/root/.cluster_secrets/user_secrets.txt","root","put");
    delete_decrypted_user_passwords(workdir);
    remote_exec(workdir,sshkey_folder,"connect",7);
    remote_exec(workdir,sshkey_folder,"all",8);
    update_cluster_summary(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The rebuild process may need 7 minutes. Please do not operate\n");
    printf("|          this cluster during the period. Exit now.\n");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int view_run_log(char* workdir, char* stream, char* run_option, char* view_option){
    char logfile[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char real_stream[16]="";
    if(strcmp(stream,"std")!=0&&strcmp(stream,"err")!=0){
        strcpy(real_stream,"std");
    }
    else{
        strcpy(real_stream,stream);
    }
    if(strcmp(run_option,"realtime")!=0&&strcmp(run_option,"archive")!=0){
        if(strcmp(real_stream,"std")==0){
            sprintf(logfile,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            strcpy(logfile,OPERATION_ERROR_LOG);
        }
    }
    else if(strcmp(run_option,"realtime")==0){
        if(strcmp(real_stream,"std")==0){
            sprintf(logfile,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            strcpy(logfile,OPERATION_ERROR_LOG);
        }
    }
    else{
        if(strcmp(real_stream,"std")==0){
            sprintf(logfile,"%s%slog%stf_prep.log.archive",workdir,PATH_SLASH,PATH_SLASH);
        }
        else{
            sprintf(logfile,"%s.archive",OPERATION_ERROR_LOG);
        }
    }
    if(file_exist_or_not(logfile)!=0){
        return -1;
    }
    if(strcmp(view_option,"print")==0){
        sprintf(cmdline,"%s %s",CAT_FILE_CMD,logfile);
        system(cmdline);
        return 0;
    }
    else{
#ifdef _WIN32
        if(tail_f_for_windows(logfile)==1){
            printf(WARN_YELLO_BOLD "[ -INFO- ] Time is up. Please run this command again.\n" RESET_DISPLAY);
        }
#else
        sprintf(cmdline,"tail -f %s",logfile);
        system(cmdline);
#endif
        return 0;
    }
}