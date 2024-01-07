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
#include "cluster_general_funcs.h"
#include "userman.h"

int usrmgr_prereq_check(char* workdir, char* ucmd, int batch_mode_flag){
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -7;
    }
    if(cluster_asleep_or_not(workdir)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The cluster %s is not running. Please wake up first." RESET_DISPLAY "\n",cluster_name);
        return -1;
    }
    int i=check_down_nodes(workdir);
    int confirm_flag=0;
    if(i!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] There are %d down nodes." RESET_DISPLAY "\n",i);
        if(strcmp(ucmd,"delete")==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] We recommend to wake up all nodes before this operation." RESET_DISPLAY "\n");
            confirm_flag=prompt_to_confirm("Continue *WITHOUT* waking up all the nodes?",CONFIRM_STRING,batch_mode_flag);
            if(confirm_flag==1){
                return 3;
            }
            else{
                return 5;
            }
        }
    }
    return 0;
}

void usrmgr_remote_exec(char* workdir, char* sshkey_folder, int prereq_check_flag){
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote executing now ...\n");
    remote_exec(workdir,sshkey_folder,"connect",1);
    remote_exec(workdir,sshkey_folder,"all",2);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    if(prereq_check_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The cluster user has been updated with down nodes." RESET_DISPLAY "\n");
    }
}

int hpc_user_list(char* workdir, char* crypto_keyfile, int decrypt_flag){
    if(decrypt_flag==0){
        if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
            return -1;
        }
    }
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char single_line[LINE_LENGTH_SHORT]="";
    char username[32]="";
    char enable_flag[16]="";
    FILE* file_p=NULL;
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    while(fngetline(file_p,single_line,LINE_LENGTH_SHORT)!=1){
        get_seq_nstring(single_line,' ',2,username,32);
        get_seq_nstring(single_line,' ',4,enable_flag,16);
        if(strcmp(enable_flag,"STATUS:ENABLED")==0){
            printf(HIGH_GREEN_BOLD "|          +- username: %s %s" RESET_DISPLAY "\n",username,enable_flag);
        }
        else{
            printf(WARN_YELLO_BOLD "|          +- username: %s %s"  RESET_DISPLAY  "\n",username,enable_flag);
        }
    }
    fclose(file_p);
    if(decrypt_flag==0){
        delete_decrypted_user_passwords(workdir);
    }
    return 0;
}

int hpc_user_delete(char* workdir, char* crypto_keyfile, char* sshkey_dir, char* username){
    if(strlen(username)==0){
        return 17;
    }
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    if(username_check_select(workdir,username,"delete")!=0){
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        delete_decrypted_user_passwords(workdir);
        return -7;
    }
    snprintf(user_registry_file,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(strcmp(username,"root")==0||strcmp(username,"user1")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user and user1 are protected and cannot be deleted." RESET_DISPLAY "\n");
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    snprintf(remote_commands,CMDLINE_LENGTH-1,"echo y-e-s | hpcmgr users delete %s os",username);
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,"-n",0,0,"","")==0){
        snprintf(remote_commands,CMDLINE_LENGTH-1,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s",username);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,"-n",0,0,"","")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the user %s from your cluster. Exit now." RESET_DISPLAY "\n",username);
            delete_decrypted_user_passwords(workdir);
            return 1;
        }
        else{
            delete_user_from_registry(user_registry_file,username);
            encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
            delete_user_sshkey(cluster_name,username,sshkey_dir);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully deleted user %s.\n",username);
            return 0;
        }
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status." RESET_DISPLAY "\n");
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int hpc_user_enable_disable(char* workdir, char* sshkey_dir, char* username, char* crypto_keyfile, char* option){
    if(strlen(username)==0){
        return 17;
    }
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    if(username_check_select(workdir,username,option)!=0){
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    char prev_keywords[16]="";
    char prev_keywords_ext[16]="";
    char new_keywords[16]="";
    char new_keywords_ext[16]="";
    if(strcmp(option,"enable")==0){
        strcpy(prev_keywords,"DISABLED");
        strcpy(prev_keywords_ext,"STATUS:DISABLED");
        strcpy(new_keywords,"ENABLED");
        strcpy(new_keywords_ext,"STATUS:ENABLED");
    }
    else if(strcmp(option,"disable")==0){
        strcpy(prev_keywords,"ENABLED");
        strcpy(prev_keywords_ext,"STATUS:ENABLED");
        strcpy(new_keywords,"DISABLED");
        strcpy(new_keywords_ext,"STATUS:DISABLED");
    }
    else{
        return -7;
    }
    char username_ext[128]="";
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(user_registry_file,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    snprintf(username_ext,127,"username: %s ",username);

    if(find_multi_nkeys(user_registry_file,LINE_LENGTH_SHORT,username_ext,new_keywords,"","","")>0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The user " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is already " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY,username,new_keywords);
        delete_decrypted_user_passwords(workdir);
        return -5;
    }
    if(strcmp(option,"enable")==0){
        snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr users add %s",username);
    }
    else{
        snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr users delete %s",username);
    }
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,"-n",0,0,"","")==0){
        find_and_nreplace(user_registry_file,LINE_LENGTH_SHORT,username_ext,"","","","",prev_keywords_ext,new_keywords_ext);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully %s the user %s.\n",new_keywords,username);
        encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
        return 0;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status." RESET_DISPLAY "\n");
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int hpc_user_setpasswd(char* workdir, char* ssheky_dir, char* crypto_keyfile, char* username, char* password){
    if(strlen(username)==0||strlen(password)==0){
        return 17;
    }
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    if(username_check_select(workdir,username,"passwd")!=0){
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char password_prev[USER_PASSWORD_LENGTH_MAX]="";
    char username_ext[128]="";
    char password_prev_ext[128]="";
    char password_ext[128]="";
    char remote_commands[CMDLINE_LENGTH]="";

    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(user_registry_file,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);

    if(user_password_complexity_check(password,SPECIAL_PASSWORD_CHARS)!=0){
        delete_decrypted_user_passwords(workdir);
        return -5;
    }
    snprintf(remote_commands,CMDLINE_LENGTH-1,"echo '%s' | passwd %s --stdin",password,username); /* Added '' to enclose the password string*/
    if(remote_exec_general(workdir,ssheky_dir,"root",remote_commands,"-n",0,0,"","")==0){
        snprintf(username_ext,127,"username: %s ",username);
        snprintf(password_ext,127," %s ",password);
        find_and_nget(user_registry_file,LINE_LENGTH_SHORT,username_ext,"","",1,username_ext,"","",' ',3,password_prev,21);
        snprintf(password_prev_ext,127," %s ",password_prev);
        find_and_nreplace(user_registry_file,LINE_LENGTH_SHORT,username_ext,"","","","",password_prev_ext,password_ext);
        sync_user_passwords(workdir,ssheky_dir);
        encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully updated the password for user %s.\n",username);
        return 0;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status." RESET_DISPLAY "\n");
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int hpc_user_add(char* workdir, char* sshkey_dir, char* crypto_keyfile, char* username, char* password){
    if(strlen(username)==0||strlen(password)==0){
        return 17;
    }
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    FILE* file_p=NULL;
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        delete_decrypted_user_passwords(workdir);
        return -7;
    }
    snprintf(user_registry_file,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(username_check_add(workdir,username)!=0){
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    if(user_password_complexity_check(password,SPECIAL_PASSWORD_CHARS)!=0){
        delete_decrypted_user_passwords(workdir);
        return -5;
    }
    snprintf(remote_commands,CMDLINE_LENGTH-1,"hpcmgr users add %s '%s'",username,password); /* Added '' to enclose the password string*/
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,"-n",0,0,"","")==0){
        snprintf(remote_commands,CMDLINE_LENGTH-1,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s | grep 'STATUS:ENABLED'",username);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,"-n",0,0,"","")==0){
            printf("[ -INFO- ] Updating the local user-info registry ...\n");
            file_p=fopen(user_registry_file,"a");
            fprintf(file_p,"username: %s %s STATUS:ENABLED\n",username,password);
            fclose(file_p);
            get_user_sshkey(cluster_name,username,"ENABLED",sshkey_dir,crypto_keyfile);
            printf("[ -DONE- ] The user %s has been added to your cluster successfully.\n",username);
            encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to add the user %s to your cluster. Exit now.\n",username);
            delete_decrypted_user_passwords(workdir);
            return 1;
        }
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status." RESET_DISPLAY "\n");
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}