/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "now_macros.h"
#include "general_funcs.h"
#include "cluster_general_funcs.h"
#include "opr_crypto.h"

//return 1: option incorrect
//return -1: Registry empty
//return -3: FILE I/O error
//return 3: User dened.
//return 5: cluster name invalid of a single cluster.
//return 7: Failed to decrypt a single cluster
//return 20+: Failed to decrypt all
//return -5: PS locked!
int encrypt_decrypt_clusters(char* cluster_list, char* option, int batch_flag_local){
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify an option: encrypt or decrypt." RESET_DISPLAY "\n");
        return 1;
    }
    if(file_empty_or_not(ALL_CLUSTER_REGISTRY)<1){
        printf(FATAL_RED_BOLD "[ FATAL: ] The registry is empty. Have you created any clusters?" RESET_DISPLAY "\n");
        return -1;
    }
    char cluster_name_temp[LINE_LENGTH_SHORT]=""; //Here we have to use a wider array.
    char cluster_workdir_temp[DIR_LENGTH]="";
    char registry_line_buffer[LINE_LENGTH_SHORT]="";
    char registry_copy[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    int flag,final_flag=0;
    int i=1;
    if(strcmp(option,"decrypt")==0){
        if(strcmp(cluster_list,"all")==0){
            printf(FATAL_RED_BOLD "|* VERY RISKY! Decrypting files of " RESET_DISPLAY WARN_YELLO_BOLD "ALL" RESET_DISPLAY FATAL_RED_BOLD " the clusters!" RESET_DISPLAY "\n");
        }
        else{
            printf(FATAL_RED_BOLD "|* VERY RISKY! Decrypting files of cluster(s) " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " !" RESET_DISPLAY "\n",cluster_list);
        }
    }
    else{
        printf(FATAL_RED_BOLD "|* Encrypting the cluster's sensitive files with now-crypto." RESET_DISPLAY "\n");
    }
    flag=prompt_to_confirm("ARE YOUR SURE TO CONTINUE?",CONFIRM_STRING,batch_flag_local);
    if(flag==1){
        return 3;
    }
    if(strcmp(cluster_list,"all")==0){
        if(check_pslock_all()!=0){      
            printf(FATAL_RED_BOLD "[ FATAL: ] Locked (operation-in-progress) cluster(s) found, exit." RESET_DISPLAY "\n");
            return -5;
        }
        sprintf(cmdline,"%s %s %s.copy %s",COPY_FILE_CMD,ALL_CLUSTER_REGISTRY,ALL_CLUSTER_REGISTRY,SYSTEM_CMD_REDIRECT);
        if(system(cmdline)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] FILE I/O error when copying registry." RESET_DISPLAY "\n");
            return -3;
        }
        sprintf(registry_copy,"%s.copy",ALL_CLUSTER_REGISTRY);
        FILE* file_p=fopen(registry_copy,"r");
        if(file_p==NULL){
            printf(FATAL_RED_BOLD "[ FATAL: ] FILE I/O error when opening copied registry." RESET_DISPLAY "\n");
            return -3;
        }
        while(fngetline(file_p,registry_line_buffer,LINE_LENGTH_SHORT)==0){
            get_seq_string(registry_line_buffer,' ',4,cluster_name_temp);
            if(cluster_name_check(cluster_name_temp)!=-127){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster name %s is not valid. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
                final_flag++;
                continue;
            }
            get_workdir(cluster_workdir_temp,cluster_name_temp);
            if(strcmp(option,"decrypt")==0){
                flag=decrypt_single_cluster(cluster_name_temp,NOW_CRYPTO_EXEC,CRYPTO_KEY_FILE);
                if(flag!=0){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to decrypt files of the cluster %s. Error code: %d." RESET_DISPLAY "\n",cluster_name_temp,flag);
                    encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
                    delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
                    final_flag++;
                }
                else{
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Decrypted" RESET_DISPLAY " sensitive files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
                }
            }
            else{
                encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
                delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Encrypted" RESET_DISPLAY " sensitive files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
            }
        }
        fclose(file_p);
        snprintf(cmdline,2047,"%s %s %s",DELETE_FILE_CMD,registry_copy,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        if(final_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster(s) %sion finished with %d failed cluster(s)." RESET_DISPLAY "\n",option,final_flag);
        }
        else{
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY WARN_YELLO_BOLD " Cluster(s) %sion" RESET_DISPLAY " finished successfully.\n",option);
        }
        // Now decrypt/encrypt the operator's private SSH key. That is SSHKEY_DIR/now-cluster-login
        if(strcmp(option,"decrypt")==0){
            if(decrypt_opr_privkey(SSHKEY_DIR,CRYPTO_KEY_FILE)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to decrypt the operator's private SSH key." RESET_DISPLAY "\n");
                final_flag++;
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Decrypted" RESET_DISPLAY " the operator's private SSH key.\n");
            }
        }
        else{
            if(encrypt_opr_privkey(SSHKEY_DIR,CRYPTO_KEY_FILE)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to encrypt the operator's private SSH key." RESET_DISPLAY "\n");
                final_flag++;
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Encrypted" RESET_DISPLAY " the operator's private SSH key.\n");
            }
        }
        if(final_flag!=0){
            return 20+final_flag;
        }
        else{
            return 0;
        }
    }
    if(contain_or_not(cluster_list,":")!=0){
        if(cluster_name_check(cluster_list)!=-127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is not valid." RESET_DISPLAY "\n",cluster_list);
            return 5;
        }
        get_workdir(cluster_workdir_temp,cluster_list);
        if(check_pslock(cluster_workdir_temp,decryption_status(cluster_workdir_temp))!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster %s is currently locked (operation-in-progress)." RESET_DISPLAY "\n",cluster_list);
            return -5;
        }
        if(strcmp(option,"decrypt")==0){
            flag=decrypt_single_cluster(cluster_list,NOW_CRYPTO_EXEC,CRYPTO_KEY_FILE);
            if(flag!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to decrypt files of the cluster %s. Error code: %d." RESET_DISPLAY "\n",cluster_list,flag);
                encrypt_decrypt_all_user_ssh_privkeys(cluster_list,"encrypt",CRYPTO_KEY_FILE);
                delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
                return 7;
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Decrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_list);
                return 0;
            }
        }
        else{
            encrypt_decrypt_all_user_ssh_privkeys(cluster_list,"encrypt",CRYPTO_KEY_FILE);
            delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Encrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_list);
            return 0;
        }
    }
    while(get_seq_string(cluster_list,':',i,cluster_name_temp)==0){
        if(cluster_name_check(cluster_name_temp)!=-127){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster name %s is not valid. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
            final_flag++;
            i++;
            continue;
        }
        get_workdir(cluster_workdir_temp,cluster_name_temp);
        if(check_pslock(cluster_workdir_temp,decryption_status(cluster_workdir_temp))!=0){
            printf(WARN_YELLO_BOLD "[ FATAL: ] The cluster %s is locked (operation-in-progress). Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
            final_flag++;
            i++;
            continue;
        }
        if(strcmp(option,"decrypt")==0){
            flag=decrypt_single_cluster(cluster_name_temp,NOW_CRYPTO_EXEC,CRYPTO_KEY_FILE);
            if(flag!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to decrypt files of the cluster %s. Error code: %d." RESET_DISPLAY "\n",cluster_name_temp,flag);
                encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
                delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
                final_flag++;
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Decrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
            }
            i++;
        }
        else{
            encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
            delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY WARN_YELLO_BOLD " Encrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
        }
    }
    if(final_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster(s) %sion finished with %d failed cluster(s)." RESET_DISPLAY "\n",option,final_flag);
    }
    else{
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY WARN_YELLO_BOLD " Cluster(s) %sion" RESET_DISPLAY " finished successfully.\n",option);
    }
    if(final_flag!=0){
        return 20+final_flag;
    }
    else{
        return 0;
    }
}

//return -1: cluster_registry not found
//return -3: cluster_name incorrect
//return -5: cloud_flag error or vaultdir error
//return -7: failed to get the key md5
//return 1: already decrypted
//return 0: decryption finished
int decrypt_single_cluster(char* target_cluster_name, char* now_crypto_exec, char* crypto_keyfile){
    char target_cluster_workdir[DIR_LENGTH]="";
    char target_cluster_vaultdir[DIR_LENGTH]="";
    char cloud_flag[32]="";
    char md5sum[64]="";
    int run_flag;
    char filename_temp[FILENAME_LENGTH]="";
    if(file_empty_or_not(ALL_CLUSTER_REGISTRY)<1){
        return -1;
    }
    if(cluster_name_check(target_cluster_name)!=-127){
        return -3;
    }
    get_workdir(target_cluster_workdir,target_cluster_name);
    if(get_cloud_flag(target_cluster_workdir,cloud_flag)!=0||create_and_get_vaultdir(target_cluster_workdir,target_cluster_vaultdir)!=0){
        return -5;
    }
    if(decryption_status(target_cluster_workdir)!=0){
        return -9;
    }
    if(get_nmd5sum(crypto_keyfile,md5sum,64)!=0){
        return -7;
    }
    run_flag=encrypt_decrypt_all_user_ssh_privkeys(target_cluster_name,"decrypt",crypto_keyfile);
    if(run_flag!=0&&run_flag!=-5){
        return 1;
    }
    decrypt_files(target_cluster_workdir,crypto_keyfile); //Delete the /stack files.
    // Now, decrypt the /vault files.
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sbucket_info.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    if(strcmp(cloud_flag,"CLOUD_G")==0){ //Decrypt the special bucket secrets
        sprintf(filename_temp,"%s%sbucket_key.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    }
    if(strcmp(cloud_flag,"CLOUD_E")==0){ //Decrypt the special bucket secrets
        sprintf(filename_temp,"%s%scredentials",target_cluster_vaultdir,PATH_SLASH);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    }
    decrypt_cloud_secrets(now_crypto_exec,target_cluster_workdir,md5sum);
    return 0;
}