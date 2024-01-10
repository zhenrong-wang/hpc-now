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

/*
 * return 1: option error
 * 
 * When specifying 'all', return values: 
 *   0 - normal
 *  -7 - Empty registry and failed to decrypt/encrypt ssh key
 *  -5 - Locked clusters found
 *  -3 - Failed to copy/open registry, not quite possible
 * 20+ - Failed to decrypt/encrypt some clusters
 * 
 * When cluster_list is a single cluster name, return values:
 *   0 - normal
 *  -1 - Empty registry
 *   5 - Invalide cluster name
 *  -5 - Locked cluster
 *   7 - Failed to decrypt
 * 
 * When cluster_list is a list, return values:
 *  0  - normal
 * 20+ - Failed to decrypt/encrypt some clusters
 */
int encrypt_decrypt_clusters(char* cluster_list, char* option, int batch_flag_local){
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify an option: encrypt or decrypt." RESET_DISPLAY "\n");
        return 1;
    }
    int final_flag=0;
    char filename_temp[FILENAME_LENGTH]="";
    char cluster_name_temp[32]=""; //Here we have to use a wider array.
    char cluster_workdir_temp[DIR_LENGTH]="";
    char registry_line_buffer[LINE_LENGTH_SHORT]="";
    int flag=0;
    int i=1;
    if(strcmp(option,"decrypt")==0){
        if(strcmp(cluster_list,"all")==0){
            printf(WARN_YELLO_BOLD "[  ****  ] VERY RISKY! Decrypting files of " RESET_DISPLAY GENERAL_BOLD "ALL" RESET_DISPLAY WARN_YELLO_BOLD " the clusters!" RESET_DISPLAY "\n");
        }
        else{
            printf(WARN_YELLO_BOLD "[  ****  ] VERY RISKY! Decrypting files of cluster(s) " RESET_DISPLAY GENERAL_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD " !" RESET_DISPLAY "\n",cluster_list);
        }
        flag=prompt_to_confirm("ARE YOUR SURE TO CONTINUE?",CONFIRM_STRING,batch_flag_local);
    }
    else{
        printf(WARN_YELLO_BOLD "[  ****  ] Encrypting the cluster's sensitive files with now-crypto-aes." RESET_DISPLAY "\n");
        flag=prompt_to_confirm("ARE YOUR SURE TO CONTINUE?",CONFIRM_STRING_QUICK,batch_flag_local);
    }
    /* If user denied, exit*/
    if(flag==1){
        return 3;
    }
    /*
     * If the option is "all":
     * return  -5: Some clusters locked
     * return  -7: Opr ssh private key failed 
     * return  -3: FILE I/O
     * return 20+: Some clusters failed
     */
    if(strcmp(cluster_list,"all")==0){
        if(check_pslock_all()!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Locked (operation-in-progress) cluster(s) found, exit." RESET_DISPLAY "\n");
            return -5;
        }
        if(encrypt_decrypt_opr_privkey(SSHKEY_DIR,option,CRYPTO_KEY_FILE)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to %s the operator's private SSH key." RESET_DISPLAY "\n",option);
            return -7;
        }
        /* Caution: The cluster registry decrypted and NOT encrypted! */
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s.dec",ALL_CLUSTER_REGISTRY);
        if(file_exist_or_not(filename_temp)!=0){
            encrypt_decrypt_cluster_registry("decrypt");
        }
        FILE* file_p=fopen(filename_temp,"r");
        if(file_p==NULL){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to decrypt the cluster registry." RESET_DISPLAY "\n");
            return -3;
        }
        while(fngetline(file_p,registry_line_buffer,LINE_LENGTH_SHORT)!=1){
            if(line_check_by_keyword(registry_line_buffer,"< cluster name",':',1)!=0){
                continue;
            }
            get_seq_nstring(registry_line_buffer,' ',4,cluster_name_temp,32);
            if(get_nworkdir(cluster_workdir_temp,DIR_LENGTH,cluster_name_temp)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get workdir of %s. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
                final_flag++;
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
                    printf(GENERAL_BOLD "[ -INFO- ] Decrypted" RESET_DISPLAY " sensitive files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
                }
            }
            else{
                encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
                delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
                printf(GENERAL_BOLD "[ -INFO- ] Encrypted" RESET_DISPLAY " sensitive files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
            }
        }
        fclose(file_p);
        if(final_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster(s) %sion finished with %d failed cluster(s)." RESET_DISPLAY "\n",option,final_flag);
        }
        else{
            printf(GENERAL_BOLD "[ -DONE- ] Cluster(s) %sion" RESET_DISPLAY " finished successfully.\n",option);
        }
        if(strcmp(option,"encrypt")==0){
            encrypt_decrypt_cluster_registry("encrypt"); /* For encrypt option, will encrypt the CLUSTER_REGISTRY */
        }
        if(final_flag!=0){
            return 20+final_flag;
        }
        else{
            return 0;
        }
    }
    if(contain_or_not(cluster_list,":")!=0){
        if(cluster_name_check(cluster_list)!=-7){
            printf(FATAL_RED_BOLD "[ FATAL: ] Cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is not valid." RESET_DISPLAY "\n",cluster_list);
            return 5;
        }
        if(get_nworkdir(cluster_workdir_temp,DIR_LENGTH,cluster_list)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get workdir of %s. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
            return 5;
        }
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
                printf(GENERAL_BOLD "[ -INFO- ] Decrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_list);
                return 0;
            }
        }
        else{
            encrypt_decrypt_all_user_ssh_privkeys(cluster_list,"encrypt",CRYPTO_KEY_FILE);
            delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
            printf(GENERAL_BOLD "[ -INFO- ] Encrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_list);
            return 0;
        }
    }
    while(get_seq_nstring(cluster_list,':',i,cluster_name_temp,32)==0){
        if(cluster_name_check(cluster_name_temp)!=-7){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster name %s is not valid. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
            final_flag++;
            i++;
            continue;
        }
        if(get_nworkdir(cluster_workdir_temp,DIR_LENGTH,cluster_name_temp)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get workdir of %s. Skipped it." RESET_DISPLAY "\n",cluster_name_temp);
            final_flag++;
            i++;
            continue;
        }
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
                printf(GENERAL_BOLD "[ -INFO- ] Decrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
            }
            i++;
        }
        else{
            encrypt_decrypt_all_user_ssh_privkeys(cluster_name_temp,"encrypt",CRYPTO_KEY_FILE);
            delete_decrypted_files(cluster_workdir_temp,CRYPTO_KEY_FILE);
            printf(GENERAL_BOLD "[ -INFO- ] Encrypted" RESET_DISPLAY " files of the cluster " GENERAL_BOLD "%s" RESET_DISPLAY ".\n",cluster_name_temp);
        }
    }
    if(final_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Cluster(s) %sion finished with %d failed cluster(s)." RESET_DISPLAY "\n",option,final_flag);
    }
    else{
        printf(GENERAL_BOLD "[ -DONE- ] Cluster(s) %sion" RESET_DISPLAY " finished successfully.\n",option);
    }
    if(final_flag!=0){
        return 20+final_flag;
    }
    else{
        return 0;
    }
}

/*
 * return -1: cluster_registry not found
 * return -3: cluster_name incorrect or failed to get workdir
 * return -5: cloud_flag error or vaultdir error
 * return -7: failed to get the key md5
 * return 1: already decrypted
 * return 0: decryption finished
 */
int decrypt_single_cluster(char* target_cluster_name, char* now_crypto_exec, char* crypto_keyfile){
    char target_cluster_workdir[DIR_LENGTH]="";
    char target_cluster_vaultdir[DIR_LENGTH]="";
    char cloud_flag[32]="";
    char md5sum[64]="";
    int run_flag;
    char filename_temp[FILENAME_LENGTH]="";
    if(cluster_name_check(target_cluster_name)!=-7){
        return -3;
    }
    if(get_nworkdir(target_cluster_workdir,DIR_LENGTH,target_cluster_name)!=0){
        return -3;
    }
    if(get_cloud_flag(target_cluster_workdir,crypto_keyfile,cloud_flag,32)!=0||create_and_get_subdir(target_cluster_workdir,"vault",target_cluster_vaultdir,DIR_LENGTH)!=0){
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
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    if(strcmp(cloud_flag,"CLOUD_G")==0){ //Decrypt the special bucket secrets
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_key.txt.tmp",target_cluster_vaultdir,PATH_SLASH);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    }
    if(strcmp(cloud_flag,"CLOUD_E")==0){ //Decrypt the special bucket secrets
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scredentials",target_cluster_vaultdir,PATH_SLASH);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconfig",target_cluster_vaultdir,PATH_SLASH);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    }
    decrypt_cloud_secrets(now_crypto_exec,target_cluster_workdir,md5sum);
    return 0;
}