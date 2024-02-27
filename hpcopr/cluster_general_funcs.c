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
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#include "now_macros.h"
#include "general_funcs.h"
#include "time_process.h"
#include "cluster_general_funcs.h"
#include "general_print_info.h"

/*
 * return  0: valid cluster roles
 * return  1: invalid cluster roles
 * return -1: the given maxlenth is too small
 * 
 * cluster_role: is the raw format of role, opr/admin/user
 * cluster_role_ext: is the aligned format or role, opr  /admin/user 
 */
int cluster_role_detect(char* workdir, char cluster_role[], char cluster_role_ext[], unsigned int maxlen){
    char vaultdir[DIR_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char cloud_secret_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    strncpy(cluster_role,"inval",maxlen-1);
    strncpy(cluster_role_ext,"inval",maxlen-1);
    if(maxlen<6){
        return -3;
    }
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -1;
    }
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return 1;
    }
    snprintf(cloud_secret_file,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(file_exist_or_not(cloud_secret_file)==0){
        strncpy(cluster_role,"opr",maxlen-1);
        strncpy(cluster_role_ext,"opr  ",maxlen-1);
        return 0;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.%s%sroot.key.tmp",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        strncpy(cluster_role,"admin",maxlen-1);
        strncpy(cluster_role_ext,"admin",maxlen-1);
        return 0;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        strncpy(cluster_role,"user",maxlen-1);
        strncpy(cluster_role_ext,"user ",maxlen-1);
        return 0;
    }
    strncpy(cluster_role,"inval",maxlen-1);
    strncpy(cluster_role_ext,"inval",maxlen-1);
    return 1;
}

int add_to_cluster_registry(char* new_cluster_name, char* import_flag){
    char randstr[7]="";
    char filename_temp[FILENAME_LENGTH]="";
    generate_random_nstring(randstr,7,0);
    file_convert(ALL_CLUSTER_REGISTRY,randstr,"decrypt");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,randstr);
    FILE* file_p=fopen(filename_temp,"a+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry." RESET_DISPLAY);
        return -1;
    }
    if(strcmp(import_flag,"imported")==0){
        fprintf(file_p,"< cluster name: %s > < imported >\n",new_cluster_name);
    }
    else{
        fprintf(file_p,"< cluster name: %s >\n",new_cluster_name);
    }
    fclose(file_p);
    if(file_convert(ALL_CLUSTER_REGISTRY,randstr,"encrypt")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to encrypt the cluster registry." RESET_DISPLAY);
        return -1;
    }
    registry_dec_backup(); /* Update the decrypted backup */
    return 0;
}

/*
 * return  0: successfully get the subdir
 * return -1: dir_maxlen, subdir_name_length, or the workdir length is incorrect is incorrect
 * return  3: Failed to create the subdir
 */
int create_and_get_subdir(char* workdir, char* subdir_name, char subdir_path[], unsigned int dir_maxlen){
    int base_length=strlen(HPC_NOW_ROOT_DIR)+8;
    int run_flag;
    if(strlen(workdir)<base_length||folder_exist_or_not(workdir)!=0||dir_maxlen<DIR_LENGTH_SHORT||strlen(subdir_name)<1){
        memset(subdir_path,'\0',dir_maxlen);
        return -1;
    }
    snprintf(subdir_path,dir_maxlen-1,"%s%s%s",workdir,PATH_SLASH,subdir_name);
    if(folder_exist_or_not(subdir_path)!=0){
        run_flag=mk_pdir(subdir_path);
        if(run_flag!=0){
            memset(subdir_path,'\0',dir_maxlen);
            return 3;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}

/* 
 * This function is deprecated
 * One should use create_and_get_subdir()
 * return non-zero: failed
 * return 0: succeeded
 */
int create_and_get_stackdir(char* workdir, char* stackdir){
    int base_length=strlen(HPC_NOW_ROOT_DIR)+7;
    if(strlen(workdir)<base_length){
        strcpy(stackdir,"");
        return 1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    int run_flag;
    sprintf(stackdir,"%s%sstack",workdir,PATH_SLASH);
    if(folder_exist_or_not(stackdir)!=0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,stackdir,SYSTEM_CMD_REDIRECT);
        run_flag=system(cmdline);
        if(run_flag!=0){
            strcpy(stackdir,"");
            return 3;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}

int get_cloud_flag(char* workdir, char* crypto_keyfile, char cloud_flag[], unsigned int maxlen){
    char hash_key[64]="";
    char vaultdir[DIR_LENGTH]="";
    char cluster_vaults_encrypted[FILENAME_LENGTH]="";
    char cluster_vaults_decrypted[FILENAME_LENGTH]="";
    char cloud_secrets_encrypted[FILENAME_LENGTH]="";
    char cloud_secrets_decrypted[FILENAME_LENGTH]="";
    char old_flag_file[FILENAME_LENGTH]="";
    if(maxlen<8){
        memset(cloud_flag,'\0',maxlen);
        return -5;
    }
    memset(cloud_flag,'\0',maxlen);
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(cloud_secrets_encrypted,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    snprintf(old_flag_file,FILENAME_LENGTH-1,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
    snprintf(cluster_vaults_encrypted,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",vaultdir,PATH_SLASH);
    snprintf(cluster_vaults_decrypted,FILENAME_LENGTH-1,"%s%sget_cloud_flag.temp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(cluster_vaults_encrypted)==0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,cluster_vaults_encrypted,cluster_vaults_decrypted,hash_key);
        get_key_nvalue(cluster_vaults_decrypted,LINE_LENGTH_SHORT,"cloud_flag_code:",' ',cloud_flag,maxlen);
        rm_file_or_dir(cluster_vaults_decrypted);
        goto final_check;
    }
    if(file_exist_or_not(cloud_secrets_encrypted)==0){ /* Only valid for operators */
        snprintf(cloud_secrets_decrypted,FILENAME_LENGTH-1,"%s%sget_cloud_flag.temp",vaultdir,PATH_SLASH);
        decrypt_single_file_general(NOW_CRYPTO_EXEC,cloud_secrets_encrypted,cloud_secrets_decrypted,hash_key);
        if(find_multi_nkeys(cloud_secrets_decrypted,LINE_LENGTH_SHORT,"\"googleapis.com\"","","","","")>0){
            strncpy(cloud_flag,"CLOUD_G",maxlen-1);
            rm_file_or_dir(cloud_secrets_decrypted);
            return 0;
        }
        find_and_nget(cloud_secrets_decrypted,LINE_LENGTH_SMALL,"CLOUD_","","",1,"CLOUD_","","",' ',1,cloud_flag,maxlen);
        rm_file_or_dir(cloud_secrets_decrypted);
        goto final_check;
    }
    if(file_exist_or_not(old_flag_file)==0){ /* For compatibility. The cloud_flag.flg file will be deprecated. */
        snprintf(old_flag_file,FILENAME_LENGTH-1,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
        FILE* file_p=fopen(old_flag_file,"r");
        fngetline(file_p,cloud_flag,maxlen);
        fclose(file_p);
    }
    /* 
     * Here we guarantee that the cloud_flag equals to CLOUD_A ~ CLOUD_G. In the future if new vendor included, this line needs to be rewritten 
     */
final_check:
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        memset(cloud_flag,'\0',maxlen);
        return 1;
    }
    return 0;
}

int decrypt_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_info){
    char vaultdir[DIR_LENGTH]="";
    char hash_key[64]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(bucket_info,"%s%sbucket_info.txt.tmp",vaultdir,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s%sbucket_info.txt.tmp %s%sbucket_info.txt %s %s",NOW_CRYPTO_EXEC,vaultdir,PATH_SLASH,vaultdir,PATH_SLASH,hash_key,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int remote_copy(char* workdir, char* crypto_keyfile ,char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option, char* recursive_flag, int silent_flag){
    char real_recursive_flag[4]="";
    char privkey_base[FILENAME_LENGTH]="";
    char privkey_decrypted[FILENAME_LENGTH_EXT]="";
    char remote_address[32]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char cluster_role[16]="";
    char cluster_role_ext[16]="";
    char randstr[7]="";
    int run_flag;
    if(strcmp(option,"put")!=0&&strcmp(option,"get")!=0){
        return -1;
    }
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -7;
    }
    if(strcmp(recursive_flag,"-r")!=0){
        strcpy(real_recursive_flag,"");
    }
    else{
        strcpy(real_recursive_flag,"-r");
    }
    get_state_nvalue(workdir,crypto_keyfile,"master_public_ip:",remote_address,32);
    cluster_role_detect(workdir,cluster_role,cluster_role_ext,16);
    if(strcmp(username,"root")==0&&strcmp(cluster_role,"opr")==0){
        snprintf(privkey_base,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_dir,PATH_SLASH);
    }
    else{
        snprintf(privkey_base,FILENAME_LENGTH-1,"%s%s.%s%s%s.key",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,username);
    }
    generate_random_nstring(randstr,7,1);
    if(file_convert(privkey_base,randstr,"decrypt")!=0){
        return -3;
    }
    snprintf(privkey_decrypted,FILENAME_LENGTH_EXT-1,"%s.%s",privkey_base,randstr);
    if(chmod_ssh_privkey(privkey_decrypted)!=0){
        rm_file_or_dir(privkey_decrypted);
        return -3;
    }
    if(strcmp(option,"put")==0){
        if(silent_flag==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"scp %s -o StrictHostKeyChecking=no -i %s %s %s@%s:%s %s",real_recursive_flag,privkey_decrypted,local_path,username,remote_address,remote_path,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"scp %s -o StrictHostKeyChecking=no -i %s %s %s@%s:%s",real_recursive_flag,privkey_decrypted,local_path,username,remote_address,remote_path);
        }
    }
    else{
        if(silent_flag==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"scp %s -o StrictHostKeyChecking=no -i %s %s@%s:%s %s %s",real_recursive_flag,privkey_decrypted,username,remote_address,remote_path,local_path,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"scp %s -o StrictHostKeyChecking=no -i %s %s@%s:%s %s",real_recursive_flag,privkey_decrypted,username,remote_address,remote_path,local_path);
        }
    }
    run_flag=system(cmdline);
    rm_file_or_dir(privkey_decrypted);
    if(run_flag!=0){
        return 1;
    }
    else{
        return 0;
    }
}

int encrypt_user_privkey(char* ssh_privkey, char* crypto_keyfile){
    char hash_key[64]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -1;
    }
    if(encrypt_and_delete(NOW_CRYPTO_EXEC,ssh_privkey,hash_key)!=0){
        return 1;
    }
    return 0;
}

int decrypt_user_privkey(char* ssh_privkey_encrypted, char* crypto_keyfile){
    char hash_key[64]="";
    char ssh_privkey[FILENAME_LENGTH]="";
    int i;
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -1;
    }
    if(decrypt_single_file(NOW_CRYPTO_EXEC,ssh_privkey_encrypted,hash_key)!=0){
        return -3;
    }
    for(i=0;i<strlen(ssh_privkey_encrypted)-4;i++){
        *(ssh_privkey+i)=*(ssh_privkey_encrypted+i);
    }
    if(chmod_ssh_privkey(ssh_privkey)!=0){
        rm_file_or_dir(ssh_privkey);
        return 1;
    }
    return 0;
}

/* 
 * Change the permision of a *decrypted* ssh private key file
 * return -1: keyfile not exist
 * return -3: FILE I/O Error
 * return -5: Failed to change the ownership
 * return -7: VERY Abnormal
 * return 0: Normal exit
 */
int chmod_ssh_privkey(char* ssh_privkey){
    if(file_exist_or_not(ssh_privkey)!=0){
        return -1;
    }
#ifdef _WIN32
    FILE* file_p=NULL;
    char group_and_user[64]="";
    char line_seq_buffer[256]="";
    char line_seq_buffer2[128]="";
    char line_buffer[512]="";
    char randstr[7]="";
    char get_perm_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    generate_random_nstring(randstr,7,1);
    snprintf(cmdline,CMDLINE_LENGTH-1,"takeown /f %s %s",ssh_privkey,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /c /t /inheritance:d %s",ssh_privkey,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /c /t /remove:g Users %s",ssh_privkey,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /c /t /remove:g \"Authenticated Users\" %s",ssh_privkey,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /c /t /remove:g Administrators %s",ssh_privkey,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(get_perm_temp,FILENAME_LENGTH-1,"%s%s.tmp%skey_perm.%s.txt",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,randstr);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s > %s",ssh_privkey,get_perm_temp);
    system(cmdline);
    file_p=fopen(get_perm_temp,"r");
    if(file_p==NULL){
        return -3;
    }
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,511);
        get_seq_nstring(line_buffer,' ',2,line_seq_buffer,256);
        get_seq_nstring(line_seq_buffer,'\\',2,line_seq_buffer2,128);
        get_seq_nstring(line_seq_buffer2,':',1,group_and_user,64);
        if(strcmp(group_and_user,"hpc-now")!=0&&strlen(group_and_user)!=0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /c /t /remove %s %s",ssh_privkey,group_and_user,SYSTEM_CMD_REDIRECT_NULL);
            if(system(cmdline)!=0){
                fclose(file_p);
                rm_file_or_dir(get_perm_temp);
                return -5;
            }
        }
    }
    fclose(file_p);
    rm_file_or_dir(get_perm_temp);
#else
    struct passwd* pwd=getpwnam("hpc-now");
    if(pwd==NULL){
        return -7;
    }
    chown(ssh_privkey,pwd->pw_uid,pwd->pw_gid);
    if(chmod(ssh_privkey,0600)!=0){
        return 1;
    }
#endif
    return 0;
}

/*
 * remote copy the private key to local folder
 * and encrypt the private kay file
 */
int get_user_sshkey(char* cluster_name, char* user_name, char* user_status, char* sshkey_dir, char* crypto_keyfile){
    char sshkey_subdir[DIR_LENGTH]="";
    char ssh_privkey[FILENAME_LENGTH]="";
    char ssh_privkey_remote[FILENAME_LENGTH]="";
    char workdir[DIR_LENGTH];
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)){
        return -1;
    }
    snprintf(sshkey_subdir,DIR_LENGTH-1,"%s%s.%s",sshkey_dir,PATH_SLASH,cluster_name);
    if(folder_exist_or_not(sshkey_subdir)!=0){
        if(mk_pdir(sshkey_subdir)!=0){
            return -3;
        }
    }
    snprintf(ssh_privkey,FILENAME_LENGTH-1,"%s%s%s.key",sshkey_subdir,PATH_SLASH,user_name);
    if(strcmp(user_name,"root")==0){
        snprintf(ssh_privkey_remote,FILENAME_LENGTH-1,"/root/.ssh/id_rsa");
    }
    else{
        if(strcmp(user_status,"DISABLED")==0){
            snprintf(ssh_privkey_remote,FILENAME_LENGTH-1,"/root/.sshkey_deleted/id_rsa.%s",user_name);
        }
        else{
            snprintf(ssh_privkey_remote,FILENAME_LENGTH-1,"/home/%s/.ssh/id_rsa",user_name);
        }
    }
    if(remote_copy(workdir,crypto_keyfile,sshkey_dir,ssh_privkey,ssh_privkey_remote,"root","get","",0)!=0){
        return 1;
    }
    else{
        encrypt_user_privkey(ssh_privkey,crypto_keyfile);
        return 0;
    }
}

int delete_user_sshkey(char* cluster_name, char* user_name, char* sshkey_dir){
    char user_privkey[FILENAME_LENGTH]="";
    snprintf(user_privkey,FILENAME_LENGTH-1,"%s%s.%s%s%s.key*",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,cluster_name);
    return rm_file_or_dir(user_privkey);
}

/*
 * return 0: succeeded
 * return non-zero: failed
 */
int create_and_get_vaultdir(char* workdir, char* vaultdir){
    int base_length=strlen(HPC_NOW_ROOT_DIR)+7;
    if(strlen(workdir)<base_length){
        strcpy(vaultdir,"");
        return 1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    int run_flag;
    sprintf(vaultdir,"%s%svault",workdir,PATH_SLASH);
    if(folder_exist_or_not(vaultdir)!=0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,vaultdir,SYSTEM_CMD_REDIRECT);
        run_flag=system(cmdline);
        if(run_flag!=0){
            strcpy(vaultdir,"");
            return 3;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}

int remote_exec(char* workdir, char* crypto_keyfile, char* sshkey_folder, char* exec_type, int delay_minutes){
    char cmdline[CMDLINE_LENGTH]="";
    char opr_privkey_base[FILENAME_LENGTH]="";
    char opr_privkey_decrypted[FILENAME_LENGTH_EXT]="";
    char remote_address[32]="";
    int run_flag;
    char randstr[7]="";
    if(strcmp(exec_type,"connect")!=0&&strcmp(exec_type,"all")!=0&&strcmp(exec_type,"clear")!=0&&strcmp(exec_type,"quick")!=0){
        return -3;
    }
    if(delay_minutes<0){
        return -1;
    }
    if(get_state_nvalue(workdir,crypto_keyfile,"master_public_ip:",remote_address,32)!=0){
        return -7;
    }
    generate_random_nstring(randstr,7,1);
    snprintf(opr_privkey_base,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(file_convert(opr_privkey_base,randstr,"decrypt")!=0){
        return -5;
    }
    snprintf(opr_privkey_decrypted,FILENAME_LENGTH_EXT-1,"%s.%s",opr_privkey_base,randstr);
    if(chmod_ssh_privkey(opr_privkey_decrypted)!=0){
        rm_file_or_dir(opr_privkey_decrypted);/* Delete the decrypted opr ssh private key. */
        return -5;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr %s\" | at now + %d minutes\" %s",opr_privkey_decrypted,remote_address,exec_type,delay_minutes,SYSTEM_CMD_REDIRECT);
    run_flag=system(cmdline);
    rm_file_or_dir(opr_privkey_decrypted);/* Delete the decrypted opr ssh private key. */
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

int remote_exec_general(char* workdir, char* crypto_keyfile, char* sshkey_folder, char* username, char* commands, char* extra_options, int delay_minutes, int silent_flag, char* std_redirect, char* err_redirect){
    int run;
    char cmdline[CMDLINE_LENGTH]="";
    char privkey_base[FILENAME_LENGTH]="";
    char privkey_decrypted[FILENAME_LENGTH_EXT]="";
    char remote_address[32]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char cluster_role[16]="";
    char cluster_role_ext[16]="";
    char randstr[7]="";
    if(delay_minutes<0){
        return -1;
    }
    if(get_state_nvalue(workdir,crypto_keyfile,"master_public_ip:",remote_address,32)!=0){
        return -5;
    }
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -7;
    }
    cluster_role_detect(workdir,cluster_role,cluster_role_ext,16);
    if(strcmp(username,"root")==0&&strcmp(cluster_role,"opr")==0){
        snprintf(privkey_base,FILENAME_LENGTH,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    }
    else{
        snprintf(privkey_base,FILENAME_LENGTH,"%s%s.%s%s%s.key",sshkey_folder,PATH_SLASH,cluster_name,PATH_SLASH,username);
    }
    generate_random_nstring(randstr,7,1);
    if(file_convert(privkey_base,randstr,"decrypt")!=0){
        return -3;
    }
    snprintf(privkey_decrypted,FILENAME_LENGTH_EXT-1,"%s.%s",privkey_base,randstr);
    if(chmod_ssh_privkey(privkey_decrypted)!=0){
        rm_file_or_dir(privkey_decrypted); /* Delete the decrypted opr ssh private key. */
        return -3;
    }
    if(delay_minutes==0){
        if(silent_flag==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" %s",extra_options,privkey_decrypted,username,remote_address,commands,SYSTEM_CMD_REDIRECT);
        }
        else if(silent_flag==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\"",extra_options,privkey_decrypted,username,remote_address,commands);
        }
        else if(silent_flag==2){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" %s",extra_options,privkey_decrypted,username,remote_address,commands,SYSTEM_CMD_ERR_REDIRECT_NULL);
        }
        else{
            if(strcmp(std_redirect,err_redirect)==0){
                if(strlen(std_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\"",extra_options,privkey_decrypted,username,remote_address,commands);
                }
                else{
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" >%s 2>&1",extra_options,privkey_decrypted,username,remote_address,commands,std_redirect);
                }
            }
            else{
                if(strlen(std_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" 2>%s",extra_options,privkey_decrypted,username,remote_address,commands,err_redirect);
                }
                else if(strlen(err_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" >%s 2>&1",extra_options,privkey_decrypted,username,remote_address,commands,std_redirect);
                }
                else{
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" >%s 2>%s",extra_options,privkey_decrypted,username,remote_address,commands,std_redirect,err_redirect);
                }
            }
        }
    }
    else{
        if(silent_flag==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" %s",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,SYSTEM_CMD_REDIRECT);
        }
        else if(silent_flag==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\"",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes);
        }
        else if(silent_flag==2){
            snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" %s",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,SYSTEM_CMD_ERR_REDIRECT_NULL);
        }
        else{
            if(strcmp(std_redirect,err_redirect)==0){
                if(strlen(std_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\"",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes);
                }
                else{
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" >%s 2>&1",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,std_redirect);
                }
            }
            else{
                if(strlen(std_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" 2>%s",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,err_redirect);
                }
                else if(strlen(err_redirect)==0){
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" >%s 2>&1",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,std_redirect);
                }
                else{
                    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh %s -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" >%s 2>%s",extra_options,privkey_decrypted,username,remote_address,commands,delay_minutes,std_redirect,err_redirect);
                }
            }
        }
    }
    /*printf("#%s\n",cmdline);*/
    run=system(cmdline);
    rm_file_or_dir(privkey_decrypted);
    if(run!=0){
        return 1;
    }
    return 0;
}

int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag){
    if(file_exist_or_not(secret_file)!=0){
        return 1;
    }
    if(file_exist_or_not(crypto_key_file)!=0){
        return 1;
    }
    char hash_key[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char get_ak[128]="";
    char get_sk[128]="";
    char cloud_flag_get[32]="";
    FILE* file_p=NULL;
    if(get_file_sha_hash(crypto_key_file,hash_key,64)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s %s.dat %s %s",NOW_CRYPTO_EXEC,secret_file,secret_file,hash_key,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.dat",secret_file);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fngetline(file_p,get_ak,128);
    fngetline(file_p,get_sk,128);
    fngetline(file_p,cloud_flag_get,32);
    fclose(file_p);
    rm_file_or_dir(filename_temp);
    if(strlen(get_ak)==0||strlen(get_sk)==0||strlen(cloud_flag_get)==0){
        strcpy(ak,"");
        strcpy(sk,"");
        strcpy(cloud_flag,"");
        return 1;
    }
    else{
        strcpy(ak,get_ak);
        strcpy(sk,get_sk);
        strcpy(cloud_flag,cloud_flag_get);
        return 0;
    }
}

int display_cloud_info(char* workdir, char* crypto_keyfile){
    char cloud_flag[16]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cloud_ak[128]="";
    char cloud_sk_buffer[128]="";
    char cloud_flag_buffer[32]="";
    char az_subscription_id[128]="";
    char az_tenant_id[128]="";
    char gcp_project_id[128]="";
    char gcp_client_email[128]="";
    char gcp_client_id[128]="";
    
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0||get_cloud_flag(workdir,crypto_keyfile,cloud_flag,16)!=0){
        return -3;
    }
    if(strcmp(cloud_flag,"CLOUD_G")!=0){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
        get_ak_sk(filename_temp,crypto_keyfile,cloud_ak,cloud_sk_buffer,cloud_flag_buffer);
        if(strcmp(cloud_flag,"CLOUD_F")==0){  
            get_azure_ninfo(workdir,LINE_LENGTH_SHORT,crypto_keyfile,az_subscription_id,az_tenant_id,128);
        }
    }
    else{
        gcp_credential_convert(workdir,"decrypt",0);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.key.json",vaultdir,PATH_SLASH);
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"project_id\":","","",1,"\"project_id\":","","",'\"',4,gcp_project_id,128);
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"client_email\":","","",1,"\"client_email\":","","",'\"',4,gcp_client_email,128);
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"client_id\":","","",1,"\"client_id\":","","",'\"',4,gcp_client_id,128);
        find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"private_key_id\":","","",1,"\"private_key_id\":","","",'\"',4,cloud_ak,128);
        gcp_credential_convert(workdir,"delete",0);
    }
    printf("\nCloud Vendor    : %s ",cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        printf("| Alibaba Cloud | https://www.alibabacloud.com\n");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        printf("| Tencent Cloud | https://www.tencentcloud.com\n");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        printf("| Amazon Web Services | https://aws.amazon.com\n");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        printf("| Huawei Cloud | https://www.huaweicloud.com\n");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        printf("| Baidu BCE Cloud | https://cloud.baidu.com\n");
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        printf("| Microsoft Azure Cloud | https://azure.microsoft.com\n");
    }
    else{
        printf("| Google Cloud Platform | https://cloud.google.com\n");
    }
    if(strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        printf("Access Key ID   : %s\n",cloud_ak);
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        printf("Subscription ID : %s\n",az_subscription_id);
        printf("Tenant ID       : %s\n",az_tenant_id);
        printf("Access Key ID   : %s\n",cloud_ak);
    }
    else{
        printf("Project ID      : %s\n",gcp_project_id);
        printf("Client Email    : %s\n",gcp_client_email);
        printf("Client ID       : %s\n",gcp_client_id);
        printf("Private Key ID  : %s\n",cloud_ak);
    }
    return 0;
}

/* 
 * This function has been deprecated. 
 */
int get_azure_info(char* workdir, char* az_subscription_id, char* az_tenant_id){
    char vaultdir[DIR_LENGTH]="";
    char az_extra_info_file[FILENAME_LENGTH]="";
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(az_extra_info_file,FILENAME_LENGTH-1,"%s%s.az_extra.info",vaultdir,PATH_SLASH);
    if(file_exist_or_not(az_extra_info_file)!=0){
        return -1;
    }
    get_key_value(az_extra_info_file,"azure_subscription_id:",' ',az_subscription_id);
    get_key_value(az_extra_info_file,"azure_tenant_id:",' ',az_tenant_id);
    if(strlen(az_subscription_id)==0||strlen(az_tenant_id)==0){
        return 1;
    }
    return 0;
}

int get_azure_ninfo(char* workdir, unsigned int linelen_max, char* crypto_keyfile, char* az_subscription_id, char* az_tenant_id, unsigned int id_len_max){
    char vaultdir[DIR_LENGTH]="";
    char cloud_secrets[FILENAME_LENGTH]="";
    char cluster_vaults[FILENAME_LENGTH]="";
    char az_info_old[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char hash_key[64]="";
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -5;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sget_az_info.temp",vaultdir,PATH_SLASH);
    snprintf(cloud_secrets,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    snprintf(cluster_vaults,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",vaultdir,PATH_SLASH);
    snprintf(az_info_old,FILENAME_LENGTH-1,"%s%s.az_extra.info",vaultdir,PATH_SLASH);

    if(file_exist_or_not(cluster_vaults)==0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,cluster_vaults,filename_temp,hash_key);
    }
    else if(file_exist_or_not(cloud_secrets)==0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,cloud_secrets,filename_temp,hash_key);
        if(find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,"azure_subscription_id:","","","","")<1){
            rm_file_or_dir(filename_temp);
            strcpy(filename_temp,az_info_old);
        }
    }
    get_key_nvalue(filename_temp,LINE_LENGTH_SMALL,"azure_subscription_id:",' ',az_subscription_id,id_len_max);
    get_key_nvalue(filename_temp,LINE_LENGTH_SMALL,"azure_tenant_id:",' ',az_tenant_id,id_len_max);
    if(strcmp(filename_temp,az_info_old)!=0){
        rm_file_or_dir(filename_temp);
    }
    if(strlen(az_subscription_id)>0&&strlen(az_tenant_id)>0){
        return 0;
    }
    return 1;
}

int get_cpu_num(const char* vm_model){
    int length=strlen(vm_model);
    int i,c_index=0;
    int cpu_num=0;
    if(length<5||length>9){
        return -1;
    }
    if(*(vm_model)!='a'&&*(vm_model)!='i'&&*(vm_model)!='t'&&*(vm_model)!='e'){
        return -1;
    }
    if(*(vm_model+length-1)!='g'){
        return -1;
    }
    if(*(vm_model+2)!='c'&&*(vm_model+3)!='c'&&*(vm_model+4)!='c'){
        return -1;
    }
    for(i=0;i<length;i++){
        if(*(vm_model+i)=='c'){
            c_index=i;
            break;
        }
    }
    for(i=1;i<c_index;i++){
        cpu_num=cpu_num*10+(*(vm_model+i)-'0');
    }
    return cpu_num;
}

/* 
 * return 1: Locked: (terraform.tfstate was found and the cluster is not decrypted)
 * return 0: Unlocked: (terraform.tfstate was not found; or, terraform.tfstate was found bug the cluster is decrypted)
 */
int check_pslock(char* workdir, int decrypt_flag){
    char stackdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        if(decrypt_flag!=0){ /* If also decrypted, return 0 */
            return 0;
        }
        else{
            return 1;
        }
    }
    else{
        return 0;
    }
}

/* 
 * return -1: REGISTRY is missing
 * return 1: one or more clusters are locked
 * return 0: all clusters are not locked
 */
int check_pslock_all(void){
    char randstr[7]="";
    char filename_temp[FILENAME_LENGTH]="";
    char line_buffer[LINE_LENGTH_SHORT]="";
    char cluster_name_temp[32]="";
    char cluster_workdir_temp[DIR_LENGTH]="";
    int decrypt_flag=0;
    /* Will detect the decrypted backup file by default
     * If the decrypted backup file is absent, then decrypt a new one */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.dec.bak",ALL_CLUSTER_REGISTRY);
    if(file_exist_or_not(filename_temp)!=0){
        generate_random_nstring(randstr,7,0);
        file_convert(ALL_CLUSTER_REGISTRY,randstr,"decrypt");
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,randstr);
        decrypt_flag=1; /* Mark the decryption */
    }
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    while(fngetline(file_p,line_buffer,LINE_LENGTH_SHORT)!=1){
        if(line_check_by_keyword(line_buffer,"< cluster name",':',1)!=0){
            continue;
        }
        if(get_seq_nstring(line_buffer,' ',4,cluster_name_temp,32)!=0){
            continue;
        }
        if(get_nworkdir(cluster_workdir_temp,DIR_LENGTH,cluster_name_temp)!=0){
            continue;
        }
        if(check_pslock(cluster_workdir_temp,decryption_status(cluster_workdir_temp))!=0){
            fclose(file_p);
            file_convert(ALL_CLUSTER_REGISTRY,randstr,"delete_decrypted");
            return 1;
        }
    }
    fclose(file_p);
    /* If a decryption happened, then delete the decrypted file */
    if(decrypt_flag==1){
        file_convert(ALL_CLUSTER_REGISTRY,randstr,"delete_decrypted");
    }
    return 0;
}

int create_local_tf_config(tf_exec_config* tf_run, char* stackdir){
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tf_running.conf.local",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        return 1; /* Normally, this should not happen. */
    }
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return 1;
    }
    fprintf(file_p,"tf_execution:  %s\n",tf_run->tf_runner);
    fprintf(file_p,"tf_dbg_level:  %s\n",tf_run->dbg_level);
    fprintf(file_p,"max_wait_sec:  %d\n",tf_run->max_wait_time);
    fclose(file_p);
    return 0;
}

/* 
 * return 0: local_tf_config file exists
 * return 1: local_tf_config file doesn't exits
 */
int check_local_tf_config(char* workdir, char tf_running_config_local[], unsigned int filename_maxlen){
    if(filename_maxlen<FILENAME_LENGTH){
        memset(tf_running_config_local,'\0',filename_maxlen);
        return -1;
    }
    char stackdir[DIR_LENGTH_EXT]="";
    char filename_temp[FILENAME_LENGTH]="";
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        memset(tf_running_config_local,'\0',filename_maxlen);
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tf_running.conf.local",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        strncpy(tf_running_config_local,filename_temp,filename_maxlen-1);
        return 0;
    }
    else{
        memset(tf_running_config_local,'\0',filename_maxlen);
        return 1;
    }
}

int delete_local_tf_config(char* stackdir){
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tf_running.conf.local",stackdir,PATH_SLASH);
    if(rm_file_or_dir(filename_temp)!=0){
        return 1;
    }
    return 0;
}

/* 
 * return 0, valid
 * return non-0, invalid
 */
int valid_vm_config_or_not(char* workdir, char* vm_config){
    char config_list_file[FILENAME_LENGTH]="";
    char vm_config_ext[64]="";
    snprintf(config_list_file,FILENAME_LENGTH-1,"%s%sconf%sreconf.list",workdir,PATH_SLASH,PATH_SLASH);
    if(file_empty_or_not(config_list_file)<1){
        return -1;
    }
    if(strlen(vm_config)==0||strlen(vm_config)>16){
        return 1;
    }
    snprintf(vm_config_ext,63," %s ",vm_config);
    if(find_multi_nkeys(config_list_file,LINE_LENGTH_TINY,vm_config_ext,"","","","")<1){
        return 1;
    }
    return 0;
}

int get_compute_node_num(char* stackdir, char* crypto_keyfile, char* option){
    char statefile[FILENAME_LENGTH]="";
    char statefile_decrypted[FILENAME_LENGTH]="";
    char statefile_temp[FILENAME_LENGTH]="";
    char hash_key[64]="";
    char get_num[8]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    snprintf(statefile_decrypted,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(statefile_decrypted)!=0){
        snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate.tmp",stackdir,PATH_SLASH);
        snprintf(statefile_temp,FILENAME_LENGTH-1,"%s%sget_node_num.temp",stackdir,PATH_SLASH);
        decrypt_single_file_general(NOW_CRYPTO_EXEC,statefile,statefile_temp,hash_key);
        strcpy(statefile_decrypted,statefile_temp);
    }
    if(strcmp(option,"all")==0){
        get_key_nvalue(statefile_decrypted,LINE_LENGTH_SHORT,"total_compute_nodes:",' ',get_num,8); 
    }
    else if(strcmp(option,"on")==0){
        get_key_nvalue(statefile_decrypted,LINE_LENGTH_SHORT,"running_compute_nodes:",' ',get_num,8); 
    }
    else{
        get_key_nvalue(statefile_decrypted,LINE_LENGTH_SHORT,"down_compute_nodes:",' ',get_num,8); 
    }
    rm_file_or_dir(statefile_temp);
    return string_to_positive_num(get_num);
}

/* 
 * return -1: source file not exist
 * return 0: normal exit
 * Decrypt a file with suffix to a file without .tmp suffix. e.g. text.txt.tmp to text.txt
 */
int decrypt_single_file(char* now_crypto_exec, char* filename, char* hash_key){
    char filename_new[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    int i;
    for(i=0;i<strlen(filename)-4;i++){
        *(filename_new+i)=*(filename+i);
    }
    if(file_exist_or_not(filename)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s %s %s %s",now_crypto_exec,filename,filename_new,hash_key,SYSTEM_CMD_REDIRECT);
        return system(cmdline);
    }
    else{
        return -1;
    }
}

int decrypt_single_file_general(char* now_crypto_exec, char* source_file, char* target_file, char* hash_key){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(source_file)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s %s %s %s",now_crypto_exec,source_file,target_file,hash_key,SYSTEM_CMD_REDIRECT);
        return system(cmdline);
    }
    else{
        return -1;
    }
}

/* decrypt ALL the files in /stack */
int decrypt_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char hash_key[33]="";
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -1;
    }
    if(get_file_sha_hash(crypto_key_filename,hash_key,33)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scompute_template.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate.backup.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    compute_node_num=get_compute_node_num(stackdir,crypto_key_filename,"all");
    for(i=1;i<compute_node_num+1;i++){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf.tmp",stackdir,PATH_SLASH,i);
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    }
    return 0;
}

/* 
 * Forcely overwrite the original encrypted file!
 *
 * return -1: source file not exist
 * return  1: encrypt failed
 * return  3: delete failed
 * return  0: normal exit 
 */
int encrypt_and_delete(char* now_crypto_exec, char* filename, char* hash_key){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(filename)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s.tmp %s %s",now_crypto_exec,filename,filename,hash_key,SYSTEM_CMD_REDIRECT);
        if(system(cmdline)!=0){
            return 1;
        }
        if(rm_file_or_dir(filename)!=0){
            return 3;
        }
        else{
            return 0;
        }
    }
    else{
        return -1;
    }
}

/* 
 * Forcely overwrite the original encrypted file!
 *
 * return -1: source file not exist
 * return  1: encrypt failed
 * return  3: delete failed
 * return  0: normal exit 
 */
int encrypt_and_delete_general(char* now_crypto_exec, char* source_file, char* target_file, char* hash_key){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(source_file)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s %s %s",now_crypto_exec,source_file,target_file,hash_key,SYSTEM_CMD_REDIRECT);
        if(system(cmdline)!=0){
            return 1;
        }
        if(rm_file_or_dir(source_file)!=0){
            return 3;
        }
        else{
            return 0;
        }
    }
    else{
        return -1;
    }
}

/*
 * Encrypt and delete all the *potentially* decrypted sensitive files.
 * Including those in /stack, and /vaultdir
 * CAUTION: USER SSH PRIVATE KEYS and OPR SSH PRIVATE KEY ARE NOT INCLUDED
 * return -1: Folder Error
 * return -3: Failed to get the crypto hash string
 * return  0: deleted done
 */
int delete_decrypted_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char hash_key[33]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    if(get_file_sha_hash(crypto_key_filename,hash_key,33)!=0){
        return -3;
    }
    /* This is very important. AND ALSO RISKY! */
    encrypt_cloud_secrets(NOW_CRYPTO_EXEC,workdir,hash_key); 
    /* For compatibility. The file CLUSTER_SUMMARY.txt has been deprecated since 0.3.1.0027 */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);

    /* cluster_vaults to replace the UCID_LATEST.txt and CLUSTER_SUMMARY.txt, bce_credentials, bce_config, and gcp_bucket_key */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scluster_vaults.txt",vaultdir,PATH_SLASH); 
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);

    /* ONLY for BaiduBCECloud, deprecated */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scredentials",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconfig",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);

    /* ONLY for GCP, deprecated */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_key.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);

    /* The /stack files */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scompute_template",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate.backup",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    compute_node_num=get_compute_node_num(stackdir,crypto_key_filename,"all");
    for(i=1;i<compute_node_num+1;i++){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
        encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    }
    /* User registry */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    return 0;
}

/* 
 * return  1: option error
 * return -7: Failed to get a valid working directory
 * return -5: user_registry_failed to decrypt
 * return -3: failed to get the crypto key
 * return -1: failed to get the vaultdir
 * return  0: normal exit
 */
int encrypt_decrypt_all_user_ssh_privkeys(char* cluster_name, char* option, char* crypto_keyfile){
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return 1;
    }
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
    char user_passwords_decrypted[FILENAME_LENGTH]="";
    char user_line[LINE_LENGTH_TINY];
    char user_name_temp[32]="";
    char user_ssh_privkey_encrypted[FILENAME_LENGTH]="";
    char user_ssh_privkey_decrypted[FILENAME_LENGTH]="";
    char hash_key[64]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -7;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    snprintf(user_passwords_decrypted,FILENAME_LENGTH-1,"%s%suser_passwords.txt.dec",vaultdir,PATH_SLASH);
    if(file_exist_or_not(user_passwords_decrypted)!=0){
        snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
        decrypt_single_file_general(NOW_CRYPTO_EXEC,user_passwords,user_passwords_decrypted,hash_key);
    }
    FILE* file_p=fopen(user_passwords_decrypted,"r");
    if(file_p==NULL){
        return -5;
    }
    while(!feof(file_p)){
        fngetline(file_p,user_line,127);
        get_seq_nstring(user_line,' ',2,user_name_temp,32);
        snprintf(user_ssh_privkey_encrypted,FILENAME_LENGTH-1,"%s%s.%s%s%s.key.tmp",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH,user_name_temp);
        snprintf(user_ssh_privkey_decrypted,FILENAME_LENGTH-1,"%s%s.%s%s%s.key.dec",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH,user_name_temp);
        if(strcmp(option,"encrypt")==0){
            encrypt_and_delete_general(NOW_CRYPTO_EXEC,user_ssh_privkey_decrypted,user_ssh_privkey_encrypted,hash_key);
        }
        else{
            decrypt_single_file_general(NOW_CRYPTO_EXEC,user_ssh_privkey_encrypted,user_ssh_privkey_decrypted,hash_key);
        }
    }
    fclose(file_p);
    rm_file_or_dir(user_passwords_decrypted);
    snprintf(user_ssh_privkey_encrypted,FILENAME_LENGTH-1,"%s%s.%s%sroot.key.tmp",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH);
    snprintf(user_ssh_privkey_decrypted,FILENAME_LENGTH-1,"%s%s.%s%sroot.key.dec",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH);
    if(strcmp(option,"encrypt")==0){
        encrypt_and_delete_general(NOW_CRYPTO_EXEC,user_ssh_privkey_decrypted,user_ssh_privkey_encrypted,hash_key);
    }
    else{
        decrypt_single_file_general(NOW_CRYPTO_EXEC,user_ssh_privkey_encrypted,user_ssh_privkey_decrypted,hash_key);
    }
    return 0;
}

/*
 * Only for decrypt --all or encrypt --all command
 * The return value of 
 */
int encrypt_decrypt_opr_privkey(char* sshkey_folder, char* option, char* crypto_keyfile){
    char hash_key[64]="";
    char opr_privkey_encrypted[FILENAME_LENGTH]="";
    char opr_privkey_decrypted[FILENAME_LENGTH]="";
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return -5;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    snprintf(opr_privkey_encrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login.tmp",sshkey_folder,PATH_SLASH);
    /* The encrypted file is supposed to be there. If the encrypted file does not exist, return 0 directly. */
    if(file_exist_or_not(opr_privkey_encrypted)!=0){
        return 0; 
    }
    snprintf(opr_privkey_decrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login.dec",sshkey_folder,PATH_SLASH);
    if(strcmp(option,"encrypt")==0){
        if(file_exist_or_not(opr_privkey_decrypted)!=0){
            return 0; /* If the decrypted file does not exist, return 0 directly. */
        }
        encrypt_and_delete_general(NOW_CRYPTO_EXEC,opr_privkey_decrypted,opr_privkey_encrypted,hash_key);
    }
    else{
        decrypt_single_file_general(NOW_CRYPTO_EXEC,opr_privkey_encrypted,opr_privkey_decrypted,hash_key);
    }
    return 0;
}

/* decrypt the cloud_secrets to a file named cloud_secrets_VERY_RISKY.txt */
int decrypt_cloud_secrets(char* now_crypto_exec, char* workdir, char* hash_key){
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH];
    char key_file[FILENAME_LENGTH]="";
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(key_file,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(file_exist_or_not(key_file)!=0){
        return -1;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s decrypt %s %s%scloud_secrets_VERY_RISKY.txt %s %s",now_crypto_exec,key_file,vaultdir,PATH_SLASH,hash_key,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

/* 
 * return 1: decrypted
 * return 0: encrypted
 */
int decryption_status(char* workdir){
    char vaultdir[DIR_LENGTH]="";
    char secret_file[FILENAME_LENGTH]="";
    char secret_file_decrypted[FILENAME_LENGTH]="";
    char user_passwords_decrypted[FILENAME_LENGTH]="";
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(secret_file,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    snprintf(user_passwords_decrypted,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    snprintf(secret_file_decrypted,FILENAME_LENGTH-1,"%s%scloud_secrets_VERY_RISKY.txt",vaultdir,PATH_SLASH);
    /* printf("#%s\n#%s\n#%s\n",secret_file,user_passwords_decrypted,secret_file_decrypted); */
    if(file_exist_or_not(secret_file)==0){
        if(file_exist_or_not(secret_file_decrypted)==0){
            return 1;
        }
        return 0;
    }
    if(file_exist_or_not(user_passwords_decrypted)==0){
        return 1;
    }
    return 0;
}

/* return -7: SOMETHING FATAL happened. */
int encrypt_cloud_secrets(char* now_crypto_exec, char* workdir, char* hash_key){
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH];
    char key_file[FILENAME_LENGTH]="";
    int flag=0;
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(key_file,FILENAME_LENGTH-1,"%s%scloud_secrets_VERY_RISKY.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(key_file)!=0){
        return 0; /* If the decrypted file is absent, skip. */
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s encrypt %s %s%s.secrets.key %s %s",now_crypto_exec,key_file,vaultdir,PATH_SLASH,hash_key,SYSTEM_CMD_REDIRECT);
    flag=system(cmdline);
    if(flag==0){ /* If Encrypted successfully, then delete the decrypted one. */
        return rm_file_or_dir(key_file);
    }
    return -7;
}

/*
 * This function generate/update 2 *PLAIN_TEXT* files:
 * stackdir/currentstate
 * stackdir/hostfile_latest
 * These 2 files needs to be encrypted !
 * 
 * Key words for currentstate:
 * master_config:
 * compute_confit:
 * ht_flag:
 * master_public_ip:
 * master_private_ip:
 * master_status:
 * database_status:
 * compute%d_private_ip:
 * compute%d_status:
 * total_compute_nodes:
 * running_compute_nodes:
 * down_compute_nodes:
 * payment_method:
 */
int getstate(char* workdir, char* crypto_filename){
    char cloud_flag[16]="";
    char stackdir[DIR_LENGTH]="";
    char hash_key[64]="";
    char tfstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char master_tf[FILENAME_LENGTH]="";
    char statefile[FILENAME_LENGTH]="";
    char hostfile[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char master_config[16]="";
    char compute_config[16]="";
    int compute_cores=0;
    char ht_flag[16]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    char pay_method[8]="";
    int node_num_gs;
    int node_num_on_gs=0;
    int i;
    FILE* file_p_tfstate=NULL;
    FILE* file_p_statefile=NULL;
    FILE* file_p_hostfile=NULL;
    if(get_cloud_flag(workdir,crypto_filename,cloud_flag,16)!=0||create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -3;
    }
    if(get_file_sha_hash(crypto_filename,hash_key,64)!=0){
        return -5;
    }
    snprintf(tfstate,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(tfstate)!=0){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate.tmp",stackdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key)!=0){
            return -1;
        }
        if(file_exist_or_not(tfstate)!=0){
            return -1;
        }
    }
    snprintf(master_tf,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    if(file_exist_or_not(master_tf)!=0){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf.tmp",stackdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key)!=0){
            return -1;
        }
        if(file_exist_or_not(master_tf)!=0){
            return -1;
        }
    }
    snprintf(compute_template,FILENAME_LENGTH-1,"%s%scompute_template",stackdir,PATH_SLASH);
    if(file_exist_or_not(compute_template)!=0){
        return -1;
    }
    file_p_tfstate=fopen(tfstate,"r");
    snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p_statefile=fopen(statefile,"w+");
    if(file_p_statefile==NULL){
        fclose(file_p_tfstate);
        return -1;
    }
    snprintf(hostfile,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    file_p_hostfile=fopen(hostfile,"w+");
    if(file_p_hostfile==NULL){
        fclose(file_p_tfstate);
        fclose(file_p_statefile);
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_D")==0){
        find_and_nget(master_tf,LINE_LENGTH_SMALL,"flavor_id = \"$","","",1,"flavor_id = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,master_config,16);
        find_and_nget(compute_template,LINE_LENGTH_SHORT,"flavor_id = \"$","","",1,"flavor_id = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,compute_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        find_and_nget(master_tf,LINE_LENGTH_SMALL,"instance_spec","","",1,"instance_spec","","",'.',3,master_config,16);
        find_and_nget(compute_template,LINE_LENGTH_SMALL,"instance_spec","","",1,"instance_spec","","",'.',3,compute_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        find_and_nget(master_tf,LINE_LENGTH_SMALL,"size = \"$","","",1,"size = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,master_config,16);
        find_and_nget(compute_template,LINE_LENGTH_SMALL,"size = \"$","","",1,"size = \"$","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,compute_config,16);
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        find_and_nget(master_tf,LINE_LENGTH_SMALL,"machine_type","","",1,"machine_type","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,master_config,16);
        find_and_nget(compute_template,LINE_LENGTH_SMALL,"machine_type","","",1,"machine_type","","",'.',2,string_temp,64);
        get_seq_nstring(string_temp,'}',1,compute_config,16);
    }
    else{
        find_and_nget(master_tf,LINE_LENGTH_SMALL,"instance_type","","",1,"instance_type","","",'.',3,master_config,16);
        find_and_nget(compute_template,LINE_LENGTH_SMALL,"instance_type","","",1,"instance_type","","",'.',3,compute_config,16);
    }
    if(find_multi_nkeys(compute_template,LINE_LENGTH_SMALL,"cpu_threads_per_core = 1","","","","")!=0){
        strcpy(ht_flag,"OFF");
    }
    else{
        strcpy(ht_flag,"ON");
    }
    if(find_multi_nkeys(compute_template,LINE_LENGTH_SMALL,"instance_charge_type = \"PrePaid\"","","","","")>0||find_multi_nkeys(compute_template,LINE_LENGTH_SMALL,"instance_charge_type = \"PREPAID\"","","","","")>0||find_multi_nkeys(compute_template,LINE_LENGTH_SMALL,"charging_mode = \"prePaid\"","","","","")>0||find_multi_nkeys(compute_template,LINE_LENGTH_SMALL,"payment_timing = \"Prepaid\"","","","","")>0){
        strcpy(pay_method,"month");
    }
    else{
        strcpy(pay_method,"od");
    }
    compute_cores=get_cpu_num(compute_config);
    fprintf(file_p_statefile,"%s\n",INTERNAL_FILE_HEADER);
    fprintf(file_p_statefile,"master_config: %s\ncompute_config: %s\nht_flag: %s\ncompute_node_cores: %d\n",master_config,compute_config,ht_flag,compute_cores);
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"compute","","","","");
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"master","","",50,"public_ip","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"master","","",50,"private_ip","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"master","","",50,"instance_status","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"master_status: %s\n",string_temp);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"database","","",50,"instance_status","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        }
        else{
            find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"master","","",90,"\"status\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"master_status: %s\n",string_temp);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"instance_name\": \"database","","",90,"\"status\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        }
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"instance_name\": \"compute%d",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",50,"private_ip","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",30,"instance_status","","",'\"',4,string_temp,64);
                fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            }
            else{
                find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",90,"\"status\":","","",'\"',4,string_temp,64);
                fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            }
            if(strcmp(string_temp,"RUNNING")==0||strcmp(string_temp,"running")==0||strcmp(string_temp,"Running")==0){
                node_num_on_gs++;
            }
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"name\": \"compute","","","","");
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master","","",90,"\"public_ip\"","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master","","",90,"\"private_ip\"","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"m_state","","",30,"\"state\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_status: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"db_state","","",30,"\"state\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"name\": \"compute%d",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",90,"private_ip","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            snprintf(string_temp2,63,"\"name\": \"comp%d",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",30,"\"state\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            if(strcmp(string_temp,"RUNNING")==0||strcmp(string_temp,"running")==0||strcmp(string_temp,"Running")==0){
                node_num_on_gs++;
            }
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"type\": \"huaweicloud_compute_instance\",","","","","")-3;
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master_eip\",","","",20,"\"address\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",20,"\"access_ip_v4\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",50,"\"power_action\":","","",'\"',4,string_temp,64);
        if(strcmp(string_temp,"ON")==0){
            fprintf(file_p_statefile,"master_status: Running\n");
        }
        else{
            fprintf(file_p_statefile,"master_status: Stopped\n");
        } 
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"database\",","","",50,"\"power_action\":","","",'\"',4,string_temp,64);
        if(strcmp(string_temp,"ON")==0){
            fprintf(file_p_statefile,"database_status: Running\n");
        }
        else{
            fprintf(file_p_statefile,"database_status: Stopped\n");
        }
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"name\": \"compute%d\",",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",50,"\"access_ip_v4\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",50,"\"power_action\":","","",'\"',4,string_temp,64);
            if(strcmp(string_temp,"ON")==0){
                fprintf(file_p_statefile,"compute%d_status: Running\n",i+1);
                node_num_on_gs++;
            }
            else{
                fprintf(file_p_statefile,"compute%d_status: Stopped\n",i+1);
            }
        }
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"type\": \"huaweicloud_evs_volume\",","","",50,"\"size\":","","",'\"',3,string_temp,64);
        get_seq_nstring(string_temp,' ',2,string_temp2,64);
        get_seq_nstring(string_temp2,',',1,string_temp,64);
        fprintf(file_p_statefile,"shared_volume_gb: %s\n",string_temp);
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"type\": \"baiducloud_instance\",","","","","")-3;
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master_eip\",","","",20,"\"eip\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",50,"\"internal_ip\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",100,"\"status\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_status: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"database\",","","",100,"\"status\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"name\": \"compute%d\",",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",50,"\"internal_ip\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",100,"\"status\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            if(strcmp(string_temp,"Running")==0){
                node_num_on_gs++;
            }
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"azurerm_linux_virtual_machine\"","","","","")-3;
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",80,"\"public_ip_address\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",80,"\"private_ip_address\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        fprintf(file_p_statefile,"master_status: Running\ndatabase_status: Running\n");
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"name\": \"compute%d\",",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",80,"\"private_ip_address\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            fprintf(file_p_statefile,"compute%d_status: Running\n",i+1);
            node_num_on_gs++;
        }
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"shared_volume\",","","",50,"\"disk_size_gb\":","","",'\"',3,string_temp,64);
        get_seq_nstring(string_temp,' ',2,string_temp2,64);
        get_seq_nstring(string_temp2,',',1,string_temp,64);
        fprintf(file_p_statefile,"shared_volume_gb: %s\n",string_temp);
    }
    else{
        node_num_gs=find_multi_nkeys(tfstate,LINE_LENGTH_SHORT,"\"google_compute_instance\"","","","","")-3;
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",80,"\"nat_ip\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",80,"\"network_ip\":","","",'\"',4,string_temp,64);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"master\",","","",80,"\"current_status\":","","",'\"',4,string_temp,64);
        if(strcmp(string_temp,"TERMINATED")==0){
            fprintf(file_p_statefile,"master_status: STOPPED\n");
        }
        else{
            fprintf(file_p_statefile,"master_status: RUNNING\n");
        }
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"database\",","","",80,"\"current_status\":","","",'\"',4,string_temp,64);
        if(strcmp(string_temp,"TERMINATED")==0){
            fprintf(file_p_statefile,"database_status: STOPPED\n");
        }
        else{
            fprintf(file_p_statefile,"database_status: RUNNING\n");
        }
        for(i=0;i<node_num_gs;i++){
            snprintf(string_temp2,63,"\"name\": \"compute%d\",",i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",80,"\"network_ip\":","","",'\"',4,string_temp,64);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            find_and_nget(tfstate,LINE_LENGTH_SHORT,string_temp2,"","",80,"\"current_status\":","","",'\"',4,string_temp,64);
            if(strcmp(string_temp,"TERMINATED")==0){
                fprintf(file_p_statefile,"compute%d_status: STOPPED\n",i+1);
            }
            else{
                fprintf(file_p_statefile,"compute%d_status: RUNNING\n",i+1);
                node_num_on_gs++;
            }
        }
        find_and_nget(tfstate,LINE_LENGTH_SHORT,"\"name\": \"shared_volume\",","","",30,"\"size\":","","",'\"',3,string_temp,64);
        get_seq_nstring(string_temp,' ',2,string_temp2,64);
        get_seq_nstring(string_temp2,',',1,string_temp,64);
        fprintf(file_p_statefile,"shared_volume_gb: %s\n",string_temp);
    }

    fprintf(file_p_statefile,"total_compute_nodes: %d\n",node_num_gs);
    fprintf(file_p_statefile,"running_compute_nodes: %d\n",node_num_on_gs);
    fprintf(file_p_statefile,"down_compute_nodes: %d\n",node_num_gs-node_num_on_gs);
    fprintf(file_p_statefile,"payment_method: %s\n",pay_method);
    fclose(file_p_statefile);
    fclose(file_p_hostfile);
    fclose(file_p_tfstate);
    return 0;
}

/*
 * This function is replaced by get_state_nvalue()
 */
int get_state_value(char* workdir, char* key, char* value){
    char stackdir[DIR_LENGTH]="";
    char statefile[FILENAME_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    return get_key_value(statefile,key,' ',value);
}

int get_state_nvalue(char* workdir, char* crypto_keyfile, char* key, char* value, unsigned int valen_max){
    char stackdir[DIR_LENGTH]="";
    char statefile_decrypted[FILENAME_LENGTH]="";
    char statefile[FILENAME_LENGTH]="";
    char statefile_temp[FILENAME_LENGTH]="";
    char hash_key[64]="";
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(statefile_decrypted,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(check_statefile(statefile_decrypted)!=0){
        if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
            return -3;
        }
        snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate.tmp",stackdir,PATH_SLASH);
        snprintf(statefile_temp,FILENAME_LENGTH-1,"%s%sget_state_value.temp",stackdir,PATH_SLASH);
        decrypt_single_file_general(NOW_CRYPTO_EXEC,statefile,statefile_temp,hash_key);
        strcpy(statefile_decrypted,statefile_temp);
    }
    get_key_nvalue(statefile_decrypted,LINE_LENGTH_SHORT,key,' ',value,valen_max);
    rm_file_or_dir(statefile_temp);
    if(strlen(value)<1){
        return 1;
    }
    return 0;
}

/* 
 * Generate Operator SSHKEY pair and ecnrypt the private key.
 */
int generate_encrypt_opr_sshkey(char* sshkey_folder, char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    char privkey_file_encrypted[FILENAME_LENGTH]="";
    char privkey_file_decrypted[FILENAME_LENGTH]="";
    char privkey_file_decrypted2[FILENAME_LENGTH]="";
    char pubkey_file[FILENAME_LENGTH]="";
    char hash_key[64]="";
    int run_flag;
    if(folder_exist_or_not(sshkey_folder)!=0){
        if(mk_pdir(sshkey_folder)!=0){
            return -1;
        }
    }
    snprintf(privkey_file_encrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login.tmp",sshkey_folder,PATH_SLASH);
    snprintf(privkey_file_decrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    snprintf(privkey_file_decrypted2,FILENAME_LENGTH-1,"%s%snow-cluster-login.dec",sshkey_folder,PATH_SLASH);
    snprintf(pubkey_file,FILENAME_LENGTH-1,"%s%snow-cluster-login.pub",sshkey_folder,PATH_SLASH);
    if(file_exist_or_not(pubkey_file)==0&&file_exist_or_not(privkey_file_encrypted)==0){
        rm_file_or_dir(privkey_file_decrypted);
        rm_file_or_dir(privkey_file_decrypted2);
        return 0; /* The SSH key pair exists, force delete the decrypted one(if exists) */
    }
    /* Generate a new key pair */
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3; /* Failed to get the crypto key */
    }
    batch_file_operation(sshkey_folder,"now-cluster-login*","","rm",0);
    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh-keygen -t rsa -N \"\" -f %s%snow-cluster-login -q",sshkey_folder,PATH_SLASH);
    if(system(cmdline)!=0){
        batch_file_operation(sshkey_folder,"now-cluster-login*","","rm",0);
        return 1; /* Failed to generate a ssh keypair, delete any files generated and return 1 */
    }
    snprintf(privkey_file_decrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    run_flag=encrypt_and_delete(NOW_CRYPTO_EXEC,privkey_file_decrypted,hash_key); /* Encrypt the private key. */
    if(run_flag>0){
        batch_file_operation(sshkey_folder,"now-cluster-login*","","rm",0);
        return 3; /* Failed to encrypt. delete any files generated and return 3; */
    }
    return 0;
}

/*
 * Get the pubkey
 * return -1: pubkey doesn't exist
 * return 1: FILE I/O error
 * return 2: maxlength 
 * return 3: read nothing
 * return 0: normal exit
 */
int get_opr_pubkey(char* sshkey_folder, char* pubkey, unsigned int length){
    char pubkey_file[FILENAME_LENGTH]="";
    int run_flag;
    snprintf(pubkey_file,FILENAME_LENGTH-1,"%s%snow-cluster-login.pub",sshkey_folder,PATH_SLASH);
    FILE* file_p=fopen(pubkey_file,"r");
    if(file_p==NULL){
        return -3;
    }
    run_flag=fngetline(file_p,pubkey,length-1);
    fclose(file_p);
    if(run_flag<0){
        return 1;
    }
    else if(run_flag==127){
        return 2;
    }
    else if(run_flag==1){
        return 3;
    }
    return 0;
}

/* 
 * chmod_flag =0: chmod (for hpcopr remote exec)
 * chmod_flag!=0: not chmod (for plain decrypt/encrypt) 
 */
int decrypt_opr_privkey(char* sshkey_folder, char* crypto_keyfile){
    char privkey_file_encrypted[FILENAME_LENGTH]="";
    char privkey_file[FILENAME_LENGTH]="";
    char hash_key[64]="";
    int run_flag;
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -1; /* Failed to get the hash_key, which is not quite possible */
    }
    snprintf(privkey_file_encrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login.tmp",sshkey_folder,PATH_SLASH);
    if(file_exist_or_not(privkey_file_encrypted)!=0){
        return 2;
    }
    snprintf(privkey_file,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    run_flag=decrypt_single_file(NOW_CRYPTO_EXEC,privkey_file_encrypted,hash_key);
    if(run_flag!=0){
        return 1; /* Failed to decrypt the file, exit immediately. */
    }
    if(chmod_ssh_privkey(privkey_file)!=0){
        rm_file_or_dir(privkey_file);
        return 3; /* Failed to activate the SSH private key. */
    }
    return 0;
}

int encrypt_opr_privkey(char* sshkey_folder, char* crypto_keyfile){
    char privkey_file_decrypted[FILENAME_LENGTH]="";
    char privkey_file_encrypted[FILENAME_LENGTH]="";
    char hash_key[64]="";
    int run_flag;
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -1; /* Failed to get the hash_key, which is not quite possible */
    }
    snprintf(privkey_file_decrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(file_exist_or_not(privkey_file_decrypted)!=0){
        return 2;
    }
    snprintf(privkey_file_encrypted,FILENAME_LENGTH-1,"%s%snow-cluster-login.tmp",sshkey_folder,PATH_SLASH);
    run_flag=rm_file_or_dir(privkey_file_encrypted);
    if(run_flag!=0){
        return -3; /* Failed to delete the encrypted file */
    }
    run_flag=encrypt_and_delete(NOW_CRYPTO_EXEC,privkey_file_decrypted,hash_key);
    if(run_flag>0){
        return 1; /* Failed to encrypt and delete */
    }
    return 0;
}

/*
 * This function is deprecated. Use generate_encrypt_opr_sshkey
 */
int generate_sshkey(char* sshkey_folder, char* pubkey){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    FILE* file_p=NULL;

    if(folder_exist_or_not(sshkey_folder)!=0){
        if(mk_pdir(sshkey_folder)!=0){
            return -1;
        }
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    snprintf(filename_temp2,FILENAME_LENGTH-1,"%s%snow-cluster-login.pub",sshkey_folder,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0&&file_exist_or_not(filename_temp2)==0){
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
    else{
        batch_file_operation(sshkey_folder,"now-cluster-login*","","rm",0);
        snprintf(cmdline,CMDLINE_LENGTH-1,"ssh-keygen -t rsa -N \"\" -f %s%snow-cluster-login -q",sshkey_folder,PATH_SLASH);
        system(cmdline);
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
}

/* Should write a real C function, instead of calling system commands. But it is totally OK.*/
int archive_log(char* logarchive, char* logfile){
    char line_buffer[LINE_LENGTH_SMALL]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    if(file_exist_or_not(logfile)!=0){
        return -1;
    }
    FILE* file_p=fopen(logarchive,"a+");
    if(file_p==NULL){
        return -1;
    }
    FILE* file_p_2=fopen(logfile,"r");
    fprintf(file_p,"\n\n# TIMESTAMP OF THIS ARCHIVE: %d-%d-%d %d:%d:%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    while(fngetline(file_p_2,line_buffer,LINE_LENGTH_SMALL)!=1){
        fprintf(file_p,"%s\n",line_buffer);
    }
    fclose(file_p_2);
    fclose(file_p);
    file_p_2=fopen(logfile,"w+");
    fclose(file_p_2);
    return 0;
}

void single_file_to_running(char* filename_temp, char* cloud_flag){
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_nreplace(filename_temp,LINE_LENGTH_SMALL,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"stopped","running");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"OFF\"","\"ON\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"stop\"","\"start\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"TERMINATED","RUNNING");
    }
}

int update_compute_template(char* stackdir, char* cloud_flag){
    char prev_template[FILENAME_LENGTH]="";
    char new_template[FILENAME_LENGTH]="";
    snprintf(prev_template,FILENAME_LENGTH-1,"%s%scompute_template",stackdir,PATH_SLASH);
    snprintf(new_template,FILENAME_LENGTH-1,"%s%shpc_stack_compute1.tf",stackdir,PATH_SLASH);
    if(cp_file(new_template,prev_template,0)!=0){
        return 1;
    }
    single_file_to_running(new_template,cloud_flag);
    return 0;
}

int wait_for_complete(char* tf_realtime_log, char* option, int max_time, char* errorlog, char* errlog_archive, int silent_flag){
    int i=0;
    int total_minutes=0;
    char* annimation="\\|/-";
    char findkey[32]="";
    if(strcmp(option,"init")==0){
        strcpy(findkey,"successfully initialized!");
        total_minutes=1;
    }
    else if(strcmp(option,"apply")==0){
        strcpy(findkey,"Apply complete!");
        total_minutes=3;
    }
    else if(strcmp(option,"destroy")==0){
        strcpy(findkey,"Destroy complete!");
        total_minutes=3;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] TF_OPTION_NOT_SUPPORTED." RESET_DISPLAY "\n");
        return -7;
    }
    while(find_multi_nkeys(tf_realtime_log,LINE_LENGTH_SMALL,findkey,"","","","")<1&&i<max_time){
        if(silent_flag!=0){
            fflush(stdin);
            printf(GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " This may need %d min(s). %d sec(s) passed ... (%c)\r",total_minutes,i,*(annimation+i%4));
            fflush(stdout);
        }
        i++;
        SLEEP_FUNC(1);
        if(file_empty_or_not(errorlog)>0){
            if(find_multi_nkeys(errorlog,LINE_LENGTH_SMALL,"Warning:","","","","")>0){
                archive_log(errlog_archive,errorlog);
            }
            else{
                if(silent_flag!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] TF_EXEC_ERROR." RESET_DISPLAY "\n");
                }
                return 7;
            }
        }
    }
    if(i==max_time){
        if(silent_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] TF_EXEC_TIMEOUT." RESET_DISPLAY "\n");
        }
        return 1;
    }
    else{
        if(silent_flag!=0){
            printf("\n");
        }
        return 0;
    }
}

int graph(char* workdir, char* crypto_keyfile, int graph_level){
    char hash_key[64]="";
    char cluster_name[32]="";
    char master_address[32]="";
    char master_status[16]="";
    char master_config[16]="";
    char db_status[16]="";
    char cloud_flag[16]="";
    char cluster_role[16]="";
    char cluster_role_ext[16]="";
    char shared_volume[16]="";
    char string_temp[32]="";
    char compute_address[32]="";
    char compute_status[16]="";
    char compute_config[16]="";
    char payment_method[16]="";
    char payment_method_long[64]="";
    char statefile[FILENAME_LENGTH]="";
    char statefile_encrypted[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cluster_name_column[LINE_LENGTH_SHORT]="";
    char ht_status[16]="";
    char ht_status_ext[32]="";
    int node_num=0;
    char node_num_string[8]="";
    int running_node_num=0;
    char running_node_num_string[8]="";
    int max_cluster_name_length=get_max_cluster_name_length();
    int current_cluster_name_length=0;
    int i,j,state_flag=0;
    int decrypt_flag=0;
    char decrypt_prompt[32]="";
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0||get_cluster_nname(cluster_name,32,workdir)!=0){
        return -3;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -7;
    }
    snprintf(statefile_encrypted,FILENAME_LENGTH-1,"%s%scurrentstate.tmp",stackdir,PATH_SLASH);
    snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(statefile)!=0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,statefile_encrypted,statefile,hash_key);
        state_flag=1;
    }
    decrypt_flag=decryption_status(workdir);
    if(decrypt_flag!=0){
        strcpy(decrypt_prompt,"* !DECRYPTED! *");
    }
    if(file_empty_or_not(statefile)<1||get_cloud_flag(workdir,crypto_keyfile,cloud_flag,16)!=0){
        return 1;
    }
    cluster_role_detect(workdir,cluster_role,cluster_role_ext,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"master_public_ip:",' ',master_address,32);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"master_status:",' ',master_status,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"database_status:",' ',db_status,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"master_config:",' ',master_config,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"compute_config:",' ',compute_config,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"ht_flag:",' ',ht_status,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"total_compute_nodes:",' ',node_num_string,8);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"payment_method:",' ',payment_method,16);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"shared_volume_gb:",' ',shared_volume,16);
    if(strlen(ht_status)!=0){
        snprintf(ht_status_ext,31,"HT-%s",ht_status);
    }
    node_num=string_to_positive_num(node_num_string);
    get_key_nvalue(statefile,LINE_LENGTH_SHORT,"running_compute_nodes:",' ',running_node_num_string,8);
    running_node_num=string_to_positive_num(running_node_num_string);
    if(strcmp(payment_method,"month")==0){
        strcpy(payment_method_long,"Monthly PrePaid & Automatic Renewal");
    }
    else{
        strcpy(payment_method_long,"On-Demand PostPaid");
    }
    if(graph_level==0){
        printf(GREY_LIGHT "[  ****  ] " RESET_DISPLAY GENERAL_BOLD "+-Cluster name: " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY GENERAL_BOLD " +-Cluster role: " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY "\n",cluster_name,cluster_role);
        printf(GREY_LIGHT "[  ****  ] " RESET_DISPLAY GENERAL_BOLD "+-Payment method: " HIGH_CYAN_BOLD "%s, %s" RESET_DISPLAY GENERAL_BOLD " +-Cloud: " HIGH_CYAN_BOLD "%s" RESET_DISPLAY"\n",payment_method,payment_method_long,cloud_flag);
        printf(GREY_LIGHT "[  ****  ] +-" RESET_DISPLAY "+-master(%s,%s,%s)" RESET_DISPLAY "\n",master_address,master_status,master_config);
        printf(GREY_LIGHT "[  ****  ] +-+-" RESET_DISPLAY "+-db(%s)\n",db_status);
        for(i=0;i<node_num;i++){
            snprintf(string_temp,31,"compute%d_private_ip:",i+1);
            get_key_nvalue(statefile,LINE_LENGTH_SHORT,string_temp,' ',compute_address,32);
            snprintf(string_temp,31,"compute%d_status:",i+1);
            get_key_nvalue(statefile,LINE_LENGTH_SHORT,string_temp,' ',compute_status,16);
            if(strlen(ht_status_ext)!=0){
                printf(GREY_LIGHT "[  ****  ] +-+-+-" RESET_DISPLAY "+-compute%d(%s,%s,%s,%s)\n",i+1,compute_address,compute_status,compute_config,ht_status_ext);
            }
            else{
                printf(GREY_LIGHT "[  ****  ] +-+-+-" RESET_DISPLAY "+-compute%d(%s,%s,%s)\n",i+1,compute_address,compute_status,compute_config);
            }
        }
        if(strcmp(cloud_flag,"CLOUD_D")==0||strcmp(cloud_flag,"CLOUD_F")==0){
            printf(GREY_LIGHT "[  ****  ] +-" RESET_DISPLAY "+-shared_storage(%s GB)\n",shared_volume);
        }
        if(decrypt_flag!=0){
            printf(FATAL_RED_BOLD "[ -WARN- ] VERY RISKY!!! The cluster is decrypted and NOT protected!" RESET_DISPLAY "\n");
        }
    }
    else if(graph_level==1){
        if(strlen(shared_volume)!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s | %s  %s" RESET_DISPLAY "\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method,decrypt_prompt);
            }
            else{
                printf("%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s | %s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method);
            }
        }
        else{
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s  %s" RESET_DISPLAY "\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method,decrypt_prompt);
            }
            else{
                printf("%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method);
            }
        }
    }
    else if(graph_level==2){
        if(strlen(shared_volume)!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s,%s,%s,%s,%s,%s,%d,%d,%s,%s,%s,%s,%s" RESET_DISPLAY "\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method,decrypt_prompt);
            }
            else{
                printf("%s,%s,%s,%s,%s,%s,%d,%d,%s,%s,%s,%s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method);
            }
        }
        else{
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s,%s,%s,%s,%s,%s,%d,%d,%s,%s,%s,%s" RESET_DISPLAY "\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method,decrypt_prompt);
            }
            else{
                printf("%s,%s,%s,%s,%s,%s,%d,%d,%s,%s,%s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method);
            }
        }
    }
    else{
        current_cluster_name_length=strlen(cluster_name);
        if(current_cluster_name_length<max_cluster_name_length){
            for(j=0;j<current_cluster_name_length;j++){
                *(cluster_name_column+j)=*(cluster_name+j);
            }
            for(j=current_cluster_name_length;j<max_cluster_name_length;j++){
                *(cluster_name_column+j)=' ';
            }
        }
        else{
            strcpy(cluster_name_column,cluster_name);
        }
        if(strlen(shared_volume)!=0){
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s | %s  %s" RESET_DISPLAY "\n",cluster_name_column,cluster_role_ext,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method,decrypt_prompt);
            }
            else{
                printf("%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s | %s\n",cluster_name_column,cluster_role_ext,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,shared_volume,payment_method);
            }
        }
        else{
            if(decrypt_flag!=0){
                printf(FATAL_RED_BOLD "%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s  %s" RESET_DISPLAY "\n",cluster_name_column,cluster_role_ext,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method,decrypt_prompt);
            }
            else{
                printf("%s | %s | %s | %s %s %s | %d/%d | %s | %s | %s\n",cluster_name_column,cluster_role_ext,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status_ext,payment_method);
            }
        }
    }
    if(state_flag==1){ /* Only if this function decrypted the statefile, then encrypt back. */
        encrypt_and_delete(NOW_CRYPTO_EXEC,statefile,hash_key);
    }
    printf(RESET_DISPLAY);
    return 0;
}

/* 
 * Check whether the state file is valid
 * return 1: invalid 
 * return 0: valid
 */
int check_statefile(char* statefile){
    char value_temp[32]="";
    if(find_multi_nkeys(statefile,LINE_LENGTH_TINY,INTERNAL_FILE_HEADER,"","","","")<1){
        return 1;
    }
    if(file_empty_or_not(statefile)<15){
        return 1;
    }
    if(get_key_nvalue(statefile,LINE_LENGTH_TINY,"master_config:",' ',value_temp,32)!=0){
        return 1;
    }
    if(get_key_nvalue(statefile,LINE_LENGTH_TINY,"compute_config:",' ',value_temp,32)!=0){
        return 1;
    }
    return 0;
}

int secure_encrypt_and_delete(char* filename, char* crypto_keyfile){
    char filename_enc[FILENAME_LENGTH]="";
    char hash_key[64]="";
    snprintf(filename_enc,FILENAME_LENGTH-1,"%s.tmp",filename);
    if(file_exist_or_not(filename_enc)==0){
        return rm_file_or_dir(filename);
    }
    get_file_sha_hash(crypto_keyfile,hash_key,64);
    if(encrypt_and_delete(NOW_CRYPTO_EXEC,filename,hash_key)!=0){
        return 1;
    }
    return 0;
}

/*
 * return 0: cluster is empty
 * return 1: not empty
 * This function will not delete the standard statefile, aka stackdir/currentstate 
 */
int cluster_empty_or_not(char* workdir,char* crypto_keyfile){
    char statefile[FILENAME_LENGTH]="";
    char statefile_decrypted[FILENAME_LENGTH]="";
    char statefile_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char dot_terraform[DIR_LENGTH_EXT]="";
    char hash_key[64]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3; /* Abnormal */
    }
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return 0;
    }
    snprintf(dot_terraform,DIR_LENGTH_EXT-1,"%s%s.terraform",stackdir,PATH_SLASH);
    /* If .terraform doesn't exist, the cluster is empty */
    if(folder_exist_or_not(dot_terraform)!=0){
        return 0;
    }
    /* Now check the statefile */
    snprintf(statefile_decrypted,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    /* If the standard statefile exists, the cluster is not empty */
    if(file_exist_or_not(statefile_decrypted)==0){
        if(check_statefile(statefile_decrypted)==0){
            return 1;
        }
        /* The state file seems to illegal. Delete it to avoid potential corruption. */
        rm_file_or_dir(statefile_decrypted);
    }
    snprintf(statefile,FILENAME_LENGTH-1,"%s%scurrentstate.tmp",stackdir,PATH_SLASH);
    snprintf(statefile_temp,FILENAME_LENGTH-1,"%s%scheck_empty.temp",stackdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,statefile,statefile_temp,hash_key);
    if(check_statefile(statefile_temp)==0){
        rm_file_or_dir(statefile_temp);
        return 1;
    }
    else{
        rm_file_or_dir(statefile_temp);
        return 0;
    }
}

/* 
 * return 0: asleep
 * return non-zero: not asleep
 */
int cluster_asleep_or_not(char* workdir, char* crypto_keyfile){
    char master_state[32]="";
    char running_compute_nodes[8]="";
    get_state_nvalue(workdir,crypto_keyfile,"master_status:",master_state,32);
    if(strcmp(master_state,"running")!=0&&strcmp(master_state,"Running")!=0&&strcmp(master_state,"RUNNING")!=0){
        return 0;
    }
    else{
        get_state_nvalue(workdir,crypto_keyfile,"running_compute_nodes:",running_compute_nodes,8);
        if(strcmp(running_compute_nodes,"0")==0){
            return 1;
        }
        else{
            return 2;
        }
    }
}

int cluster_full_running_or_not(char* workdir, char* crypto_keyfile){
    if(cluster_asleep_or_not(workdir,crypto_keyfile)==0){
        return 1;
    }
    char down_compute_nodes[8]="";
    get_state_nvalue(workdir,crypto_keyfile,"down_compute_nodes:",down_compute_nodes,8);
    if(strcmp(down_compute_nodes,"0")==0){
        return 0;
    }
    return 1;
}

int tf_exec_config_validation(tf_exec_config* tf_run){
    if(strcmp(tf_run->tf_runner,TERRAFORM_EXEC)!=0&&strcmp(tf_run->tf_runner,TOFU_EXEC)!=0){
        return 1;
    }
    if(strcmp(tf_run->dbg_level,"trace")!=0&&strcmp(tf_run->dbg_level,"debug")!=0&&strcmp(tf_run->dbg_level,"info")!=0&&strcmp(tf_run->dbg_level,"warn")!=0&&strcmp(tf_run->dbg_level,"error")!=0&&strcmp(tf_run->dbg_level,"off")!=0&&strcmp(tf_run->dbg_level,"TRACE")!=0&&strcmp(tf_run->dbg_level,"DEBUG")!=0&&strcmp(tf_run->dbg_level,"INFO")!=0&&strcmp(tf_run->dbg_level,"WARN")!=0&&strcmp(tf_run->dbg_level,"ERROR")!=0&&strcmp(tf_run->dbg_level,"OFF")!=0){
        return 1;
    }
    if(tf_run->max_wait_time<MAXIMUM_WAIT_TIME||tf_run->max_wait_time>MAXIMUM_WAIT_TIME_EXT){
        return 1;
    }
    return 0;
}

int tf_execution(tf_exec_config* tf_run, char* execution_name, char* workdir, char* crypto_keyfile, int silent_flag){
    if(tf_exec_config_validation(tf_run)!=0){
        printf("[ FATAL:] The tf execution config is invalid or empty. Please report this bug.\n");
        return 1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char tf_realtime_log[FILENAME_LENGTH]="";
    char tf_realtime_log_archive[FILENAME_LENGTH]="";
    char tf_error_log[FILENAME_LENGTH]="";
    char tf_error_log_archive[FILENAME_LENGTH]="";
    char tf_dbg_log[FILENAME_LENGTH]="";
    char tf_dbg_log_archive[FILENAME_LENGTH]="";
    char cloud_flag[16]="";

    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0||get_cloud_flag(workdir,crypto_keyfile,cloud_flag,16)!=0){
        return -3;
    }
    if(strcmp(cloud_flag,"CLOUD_G")==0){
        gcp_credential_convert(workdir,"decrypt",0);
    }
    snprintf(tf_realtime_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_realtime_log_archive,FILENAME_LENGTH-1,"%s%slog%stf_prep.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_error_log,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_error_log_archive,FILENAME_LENGTH-1,"%s%slog%stf_prep.err.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_dbg_log,FILENAME_LENGTH-1,"%s%slog%stf_dbg.log",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(tf_dbg_log_archive,FILENAME_LENGTH-1,"%s%slog%stf_dbg.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    archive_log(tf_realtime_log_archive,tf_realtime_log);
    archive_log(tf_error_log_archive,tf_error_log);
    archive_log(tf_dbg_log_archive,tf_dbg_log);

    if(strcmp(execution_name,"init")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"cd %s%s && %s TF_LOG=%s&&%s TF_LOG_PATH=%s%slog%stf_dbg.log && echo yes | %s %s %s -upgrade -lock=false > %s 2>%s &",stackdir,PATH_SLASH,SET_ENV_CMD,tf_run->dbg_level,SET_ENV_CMD,workdir,PATH_SLASH,PATH_SLASH,START_BG_JOB,tf_run->tf_runner,execution_name,tf_realtime_log,tf_error_log);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"cd %s%s && %s TF_LOG=%s&&%s TF_LOG_PATH=%s%slog%stf_dbg.log && echo yes | %s %s %s -lock=false -parallelism=1000 > %s 2>%s &",stackdir,PATH_SLASH,SET_ENV_CMD,tf_run->dbg_level,SET_ENV_CMD,workdir,PATH_SLASH,PATH_SLASH,START_BG_JOB,tf_run->tf_runner,execution_name,tf_realtime_log,tf_error_log);
    }
    /*signal(SIGINT,SIG_IGN);*/
    if(system(cmdline)!=0){
        /*signal(SIGINT,SIG_DFL);*/
        return -7;
    }
    if(silent_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Do not terminate this process. TF: %s. Tmax: %d secs.\n",tf_run->tf_runner_type,tf_run->max_wait_time);
        printf("[  ****  ] CMD: %s. DBG: %s. LOG: hpcopr -b viewlog" RESET_DISPLAY "\n",execution_name,tf_run->dbg_level);
    }
    if(wait_for_complete(tf_realtime_log,execution_name,tf_run->max_wait_time,tf_error_log,tf_error_log_archive,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to operate the cluster. Operation command: %s.\n" RESET_DISPLAY,execution_name);
        archive_log(tf_error_log_archive,tf_error_log);
        archive_log(tf_dbg_log_archive,tf_dbg_log);
        if(strcmp(cloud_flag,"CLOUD_G")==0){
            gcp_credential_convert(workdir,"delete",0);
        }
        /*signal(SIGINT,SIG_DFL);*/
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_G")==0){
        gcp_credential_convert(workdir,"delete",0);
    }
    archive_log(tf_dbg_log_archive,tf_dbg_log);
    /*signal(SIGINT,SIG_DFL);*/
    return 0;
}

int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option){
    char* usage_file=USAGE_LOG_FILE;
    char ucid_short[16]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cluster_id[30]="";
    char cloud_region[16]="";
    char cloud_vendor[16]="";
    char unique_cluster_id[64]="";
    char current_date[32]="";
    char current_time[32]="";
    char prev_date[32]="";
    char prev_time[32]="";
    char master_config[16]="";
    char compute_config[16]="";
    char cpu_vendor[8]="";
    FILE* file_p=NULL;
    time_t current_time_long;
    struct tm* time_p=NULL;
    int vcpu=0;
    double running_hours=0;
    char running_hours_string[16]="";
    double cpu_hours=0;
    char cpu_hours_string[16]="";
    if(get_nucid(workdir,crypto_keyfile,ucid_short,16)!=0||get_cloud_flag(workdir,crypto_keyfile,cloud_vendor,16)!=0){
        return -3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"cluster_id","","",1,"cluster_id","","",' ',3,cluster_id,30);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"region_id","","",1,"region_id","","",' ',3,cloud_region,16);
    snprintf(unique_cluster_id,63,"%s-%s",cluster_id,ucid_short);
    get_state_nvalue(workdir,crypto_keyfile,"master_config:",master_config,16);
    get_state_nvalue(workdir,crypto_keyfile,"compute_config:",compute_config,16);
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,31,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,31,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    if(strcmp(option,"start")==0){
        file_p=fopen(usage_file,"a+");
        if(contain_or_not(node_name,"compute")==0){
            vcpu=get_cpu_num(compute_config);
            if(*(compute_config+0)!='a'){
                strcpy(cpu_vendor,"intel64");
            }
            else{
                strcpy(cpu_vendor,"amd64");
            }
            fprintf(file_p,"%s,%s,%s,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,node_name,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        if(strcmp(node_name,"master")==0){
            vcpu=get_cpu_num(master_config);
            if(*(master_config+0)!='a'){
                strcpy(cpu_vendor,"intel64");
            }
            else{
                strcpy(cpu_vendor,"amd64");
            }
            fprintf(file_p,"%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        if(strcmp(node_name,"natgw")==0||strcmp(node_name,"database")==0){
            vcpu=2;
            strcpy(cpu_vendor,"intel64");
            fprintf(file_p,"%s,%s,%s,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",unique_cluster_id,cloud_vendor,node_name,vcpu,current_date,current_time,cpu_vendor,cloud_region);
            fclose(file_p);
            return 0;
        }
        fclose(file_p);
        return -1;
    }
    else if(strcmp(option,"stop")==0){
        find_and_nget(usage_file,LINE_LENGTH_SMALL,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',5,prev_date,32);
        find_and_nget(usage_file,LINE_LENGTH_SMALL,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',6,prev_time,32);
        find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","RUNNING_DATE",current_date);
        find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","RUNNING_TIME",current_time);
        running_hours=calc_running_hours(prev_date,prev_time,current_date,current_time);
        snprintf(running_hours_string,15,"%.4lf",running_hours);
        find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","NULL1",running_hours_string);
        if(contain_or_not(node_name,"compute")==0){
            vcpu=get_cpu_num(compute_config);
            cpu_hours=vcpu*running_hours;
            snprintf(cpu_hours_string,15,"%.4lf",cpu_hours);
            find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"master")==0){
            vcpu=get_cpu_num(master_config);
            cpu_hours=vcpu*running_hours;
            snprintf(cpu_hours_string,15,"%.4lf",cpu_hours);
            find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"database")==0||strcmp(node_name,"natgw")==0){
            vcpu=2;
            cpu_hours=vcpu*running_hours;
            snprintf(cpu_hours_string,15,"%.4lf",cpu_hours);
            find_and_nreplace(usage_file,LINE_LENGTH_SHORT,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        return -1;
    }
    return -1;
}

int get_vault_info(char* workdir, char* crypto_keyfile, char* username, char* bucket_flag, char* root_flag){
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char real_username[32]="";
    char hash_key[64]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char single_line[LINE_LENGTH_SHORT]="";
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    char cluster_vaults_decrypted[FILENAME_LENGTH]="";
    char user_passwords_decrypted[FILENAME_LENGTH]="";
    char unique_cluster_id[32]="";
    char username_temp[32]="";
    char password[32]="";
    char enable_flag[32]="";
    char master_address[32]="";
    char az_tenant_id[128]="";
    char az_subscription_id[128]="";
    bucket_info bucketinfo;
    char cloud_flag[16]="";
    if(cluster_empty_or_not(workdir,crypto_keyfile)==0){
        print_empty_cluster_info();
        return 1;
    }
    if(strlen(username)>0){
        if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
            return -7;
        }
        if(user_name_quick_check(cluster_name,username,SSHKEY_DIR)!=0){
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] The specified username '%s' is invalid or unauthorized.\n" RESET_DISPLAY,username);
            strcpy(real_username,"");
        }
        else{
            strncpy(real_username,username,31);
        }
    }
    else{
        strcpy(real_username,"");
    }
    int rootflag, bucketflag;
    if(strcmp(bucket_flag,"bucket")==0){
        bucketflag=1;
    }
    if(strcmp(root_flag,"root")==0){
        rootflag=1;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cryoto key." RESET_DISPLAY "\n");
        return -3;
    }
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)||get_cloud_flag(workdir,crypto_keyfile,cloud_flag,16)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the subdirs and/or cloud_flag." RESET_DISPLAY "\n");
        return -3;
    }
    if(get_nucid(workdir,crypto_keyfile,unique_cluster_id,32)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the UNIQUE cluster ID." RESET_DISPLAY "\n");
        return -1;
    }
    if(get_bucket_ninfo(workdir,crypto_keyfile,LINE_LENGTH_SHORT,&bucketinfo)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the the information." RESET_DISPLAY "\n");
        return -3;
    }
    get_azure_ninfo(workdir,LINE_LENGTH_SHORT,crypto_keyfile,az_subscription_id,az_tenant_id,128);
    get_state_nvalue(workdir,crypto_keyfile,"master_public_ip:",master_address,32);
    printf(WARN_YELLO_BOLD "\n+------------ HPC-NOW CLUSTER SENSITIVE INFORMATION: ------------+" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD "| Unique Cluster ID: " RESET_DISPLAY "%s\n",unique_cluster_id);
    printf(WARN_YELLO_BOLD "+-------------- CLUSTER PORTAL AND *CREDENTIALS* ----------------+" RESET_DISPLAY "\n");
    if(strlen(master_address)<7){
        printf(GENERAL_BOLD "| Cluster IP Address: " RESET_DISPLAY WARN_YELLO_BOLD "NOT_RUNNING" RESET_DISPLAY "\n");
    }
    else{
        printf(GENERAL_BOLD "| Cluster IP Address: " RESET_DISPLAY "%s\n",master_address);
    }
    printf(GENERAL_BOLD "| Cloud Region: " RESET_DISPLAY "%s\n",bucketinfo.region_id);
    if(strlen(az_tenant_id)>0){
        printf(GENERAL_BOLD "| Azure Tenant ID: " RESET_DISPLAY "%s\n",az_tenant_id);
    }
    printf(GENERAL_BOLD "| Bucket Address:" RESET_DISPLAY " %s\n",bucketinfo.bucket_address);
    if(strcmp(cloud_flag,"CLOUD_G")==0){
        printf(GENERAL_BOLD "| Bucket URL Link:" RESET_DISPLAY " %s\n",bucketinfo.bucket_ak); 
    }
    if(bucketflag==1){
        if(strcmp(cloud_flag,"CLOUD_G")!=0){
            printf(GENERAL_BOLD "| Bucket AccessKey: " RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,bucketinfo.bucket_ak);
            printf(GENERAL_BOLD "| Bucket SecretKey: " RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,bucketinfo.bucket_sk);
        }
        else{
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_key.txt.tmp",vaultdir,PATH_SLASH);
            decrypt_single_file_general(NOW_CRYPTO_EXEC,filename_temp,"/home/hpc-now/gcloud-bucket-key.json",hash_key);
            printf(GENERAL_BOLD "| Bucket JSON-Format Key: /home/hpc-now/gcloud-bucket-key.json" RESET_DISPLAY "\n");
            printf(WARN_YELLO_BOLD "| CAUTION! The file contains sensitive private key in plain text!\n");
            printf("| We *strongly* recommend you to delete this file after using it!\n");
            printf("| Please use the gcloud client and authenticate with the key file\n");
            printf("| to manage your cloud storage.\n");
        }
    }
    printf(WARN_YELLO_BOLD "+---------------- CLUSTER USERS AND *PASSWORDS* -----------------+" RESET_DISPLAY "\n");
    if(rootflag==1){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",vaultdir,PATH_SLASH);
        if(file_exist_or_not(filename_temp)==0){
            snprintf(cluster_vaults_decrypted,FILENAME_LENGTH-1,"%s%sget_rootpass.temp",vaultdir,PATH_SLASH);
            decrypt_single_file_general(NOW_CRYPTO_EXEC,filename_temp,cluster_vaults_decrypted,hash_key);
            get_key_nvalue(cluster_vaults_decrypted,LINE_LENGTH_SHORT,"mast_root_password:",' ',password,32);
            rm_file_or_dir(cluster_vaults_decrypted); /* Deleted the decrypted vaults */
        }
        /* For compatibility. The file CLUSTER_SUMMARY.txt has been deprecated since 0.3.1.0027 */
        else{
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
            decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
            if(file_exist_or_not(filename_temp)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] You are not an operator or administrator." RESET_DISPLAY "\n");
            }
            else{
                find_and_nget(filename_temp,LINE_LENGTH_SHORT,"Master Node Root Password:","","",1,"Master Node Root Password:","","",' ',5,password,32);
            }
        }
        if(strlen(password)<1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the root password." RESET_DISPLAY "\n");
        }
        else{
            printf(FATAL_RED_BOLD "| Root Password:" RESET_DISPLAY GREY_LIGHT " %s " RESET_DISPLAY FATAL_RED_BOLD "! DO NOT DISCLOSE TO ANYONE !\n" RESET_DISPLAY,password);
        }
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    snprintf(user_passwords_decrypted,FILENAME_LENGTH-1,"%s%sget_userpass.temp",vaultdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,filename_temp,user_passwords_decrypted,hash_key);
    file_p=fopen(user_passwords_decrypted,"r");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to decrypt and display the user registry." RESET_DISPLAY "\n");
        return -7;
    }
    while(fngetline(file_p,single_line,LINE_LENGTH_SHORT)!=1){
        if(line_check_by_keyword(single_line,"username:",' ',1)==0){
            get_seq_nstring(single_line,' ',2,username_temp,32);
            get_seq_nstring(single_line,' ',3,password,32);
            get_seq_nstring(single_line,' ',4,enable_flag,32);
            if(strlen(real_username)==0||strcmp(real_username,username_temp)==0){
                if(strcmp(enable_flag,"STATUS:DISABLED")==0){
                    printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY WARN_YELLO_BOLD "%s\n" RESET_DISPLAY,username_temp,password,enable_flag);
                }
                else{
                    printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY GENERAL_BOLD "%s\n" RESET_DISPLAY,username_temp,password,enable_flag);
                }
            }
        }
    }
    fclose(file_p);
    rm_file_or_dir(user_passwords_decrypted); /* Deleted the decrypted vaults */
    printf(WARN_YELLO_BOLD "+---------- DO NOT DISCLOSE THE INFORMATION TO OTHERS -----------+" RESET_DISPLAY "\n");
    /* For compatibility. The file CLUSTER_SUMMARY.txt has been deprecated since 0.3.1.0027 */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    secure_encrypt_and_delete(filename_temp,crypto_keyfile);
    return 0;
}

/* 
 * return 0: batch mode and skipped, or user explicitly accepted
 * return 1: user explicitly denied
 */
int confirm_to_operate_cluster(char* current_cluster_name, int batch_flag_local){
    if(batch_flag_local==0){
        return 0;
    }
    char confirm[64]="";
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You are operating the cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY ", which may affect\n",current_cluster_name);
    printf("[  ****  ] the " GENERAL_BOLD "resources, data, or jobs" RESET_DISPLAY ". Input " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " to confirm.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%63s",confirm);
    getchar();
    if(strcmp(confirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. Denied.\n");
        return 1;
    }
    return 0;
}

/* 
 * Return 0: continue
 *        1: stop
 */
int confirm_to_init_cluster(char* current_cluster_name, int batch_flag_local){
    if(batch_flag_local==0){
        return 0;
    }
    char confirm[64]="";
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please check the summary above and input " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " to continue.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%63s",confirm);
    getchar();
    if(strcmp(confirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. Denied.\n");
        return 1;
    }
    return 0;
}

/* 
 * return -1: skipped due to batch mode
 * return 1 : user explicitly denied
 * return 0 : user explicitly confirmed
 */
int prompt_to_confirm(const char* prompt_string, const char* confirm_string, int batch_flag_local){
    if(batch_flag_local==0){
        return -1;
    }
    char confirm[256]="";
    printf(GENERAL_BOLD "[ -INFO- ] " RESET_DISPLAY "%s\n",prompt_string);
    printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY "Input " WARN_YELLO_BOLD "%s" RESET_DISPLAY " to confirm: ",confirm_string);
    fflush(stdin);
    scanf("%255s",confirm);
    getchar();
    if(strcmp(confirm,confirm_string)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD "%s" RESET_DISPLAY " is accepted to confirm. Denied.\n",confirm_string);
        return 1;
    }
    return 0;
}

/* 
 * return 2 : cmd_flag found
 * return -1: cmd_flag not found, but skipped due to batch mode
 * return 1 : user explicitly denied
 * return 0 : user- explicitly confirmed
 */
int prompt_to_confirm_args(const char* prompt_string, const char* confirm_string, int batch_flag_local, int argc, char** argv, char* cmd_flag){
    if(cmd_flag_check(argc,argv,cmd_flag)==0){
        return 2;
    }
    return prompt_to_confirm(prompt_string,confirm_string,batch_flag_local);
}

/* 
 * CAUTION !
 * When using this function, please make sure the pointer input_string has enough length!
 * Otherwise stack overflow may occur! 
 * 
 * return 1 : skipped due to batch mode
 * return 0 : user input
 */
int prompt_to_input(const char* prompt_string, char reply_string[], unsigned int reply_len_max, int batch_flag_local){
    int i;
    if(batch_flag_local==0){
        return 1;
    }
    if(strlen(prompt_string)!=0){
        printf(GENERAL_BOLD "[ -INFO- ] " RESET_DISPLAY "%s\n",prompt_string);
    }
    printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY "");
    fflush(stdin);
    fgets(reply_string,reply_len_max,stdin);
    for(i=0;i<reply_len_max;i++){
        if(reply_string[i]=='\n'||reply_string[i]=='\r'){
            reply_string[i]='\0';
        }
    }
    return 0;
}

/* 
 * CAUTION !
 * When using this function, please make sure the pointer input_string has enough length!
 * Otherwise stack overflow may occur! 
 * force_input=0: force input. force_input=1: optional
 * return 2 : cmd_keyword found
 * return 1 : skipped due to batch mode
 * return 0 : user input
 */
int prompt_to_input_required_args(const char* prompt_string, char reply_string[], unsigned int reply_len_max, int batch_flag_local,int argc, char** argv, char* cmd_keyword){
    if(cmd_keyword_ncheck(argc,argv,cmd_keyword,reply_string,reply_len_max)==0){
        return 2;
    }
    return prompt_to_input(prompt_string,reply_string,reply_len_max,batch_flag_local);
}

int prompt_to_input_optional_args(const char* prompt_confirm, const char* confirm_string, const char* prompt_string, char reply_string[], unsigned int reply_len_max, int batch_flag_local,int argc, char** argv, char* cmd_keyword){
    if(cmd_keyword_ncheck(argc,argv,cmd_keyword,reply_string,reply_len_max)==0){
        return 2;
    }
    if(batch_flag_local==0){
        strcpy(reply_string,"");
        return 4;
    }
    if(prompt_to_confirm(prompt_confirm,confirm_string,batch_flag_local)==1){
        strcpy(reply_string,"");
        return 6;
    }
    return prompt_to_input(prompt_string,reply_string,reply_len_max,batch_flag_local);
}

int check_down_nodes(char* workdir, char* crypto_keyfile){
    char stackdir[DIR_LENGTH];
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -1;
    }
    return get_compute_node_num(stackdir,crypto_keyfile,"down");
}

int cluster_ssh(char* workdir, char* crypto_keyfile, char* username, char* role_flag, char* sshkey_dir){
    char master_address[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char privkey_base[FILENAME_LENGTH]="";
    char privkey_decrypted[FILENAME_LENGTH_EXT]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char randstr[7]="";
    int run_flag;
    get_state_nvalue(workdir,crypto_keyfile,"master_public_ip:",master_address,64);
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -7;
    }
    if(strcmp(role_flag,"opr")==0){
        snprintf(privkey_base,FILENAME_LENGTH-1,"%s%snow-cluster-login",sshkey_dir,PATH_SLASH);
    }
    else{
        snprintf(privkey_base,FILENAME_LENGTH-1,"%s%s.%s%s%s.key",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,username);
    }
    generate_random_nstring(randstr,7,1);
    if(file_convert(privkey_base,randstr,"decrypt")!=0){
        return -5;
    }
    snprintf(privkey_decrypted,FILENAME_LENGTH_EXT-1,"%s.%s",privkey_base,randstr);
    if(chmod_ssh_privkey(privkey_decrypted)!=0){
        rm_file_or_dir(privkey_decrypted);
        return -3;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"ssh -i %s -o StrictHostKeyChecking=no %s@%s",privkey_decrypted,username,master_address);
    run_flag=system(cmdline);
    rm_file_or_dir(privkey_decrypted);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

int node_file_to_running(char* stackdir, char* node_name, char* cloud_flag){
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_%s.tf",stackdir,PATH_SLASH,node_name);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_nreplace(filename_temp,LINE_LENGTH_SMALL,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"stopped","running");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"OFF\"","\"ON\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"stop\"","\"start\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"TERMINATED\"","\"RUNNING\"");
    }
    else{
        return 1;
    }
    return 0;
}

int node_file_to_stop(char* stackdir, char* node_name, char* cloud_flag){
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_%s.tf",stackdir,PATH_SLASH,node_name);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_nreplace(filename_temp,LINE_LENGTH_SMALL,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"running","stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"ON\"","\"OFF\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"start\"","\"stop\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"\"RUNNING\"","\"TERMINATED\"");
    }
    else{
        return 1;
    }
    return 0;
}

/* 
 * This function is deprecated
 * Please use get_bucket_ninfo() for security
 */
int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk){
    char filename_temp[FILENAME_LENGTH]="";
    char hash_key[64]="";
    char line_buffer[128]="";
    char header[16]="";
    char tail[128]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_info.txt.tmp",workdir,PATH_SLASH,PATH_SLASH);
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_info.txt",workdir,PATH_SLASH,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    int i=0;
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,128);
        get_seq_nstring(line_buffer,' ',1,header,16);
        get_seq_nstring(line_buffer,' ',2,tail,128);
        if(strcmp(header,"BUCKET:")==0){
            strcpy(bucket_address,tail);
            i++;
        }
        else if(strcmp(header,"REGION:")==0){
            get_seq_string(line_buffer,'\"',2,region_id);
            i++;
        }
        else if(strcmp(header,"BUCKET_AK:")==0){
            strcpy(bucket_ak,tail);
            i++;
        }
        else if(strcmp(header,"BUCKET_SK:")==0){
            strcpy(bucket_sk,tail);
            i++;
        }
        else if(strcmp(header,"BUCKET_LINK:")==0){
            strcpy(bucket_ak,tail);
            i++;
        }
        else{
            continue;
        }
    }
    fclose(file_p);
    secure_encrypt_and_delete(filename_temp,crypto_keyfile);
    if(contain_or_not(bucket_address,"gs://")==0){
        if(i!=3){
            strcpy(bucket_address,"");
            strcpy(region_id,"");
            strcpy(bucket_ak,"");
            strcpy(bucket_sk,"");
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        if(i!=4){
            strcpy(bucket_address,"");
            strcpy(region_id,"");
            strcpy(bucket_ak,"");
            strcpy(bucket_sk,"");
            return 1;
        }
        else{
            return 0;
        }
    }
}

/*
 * This function is more secure than get_bucket_info()
 */
int get_bucket_ninfo(char* workdir, char* crypto_keyfile, unsigned int linelen_max, bucket_info* bucketinfo){
    char filename_temp[FILENAME_LENGTH]="";
    char hash_key[64]="";
    int i=0;
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_info.txt.tmp",workdir,PATH_SLASH,PATH_SLASH);
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key)!=0){
        return 3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_info.txt",workdir,PATH_SLASH,PATH_SLASH);
    if(get_key_nvalue(filename_temp,linelen_max,"BUCKET:",' ',bucketinfo->bucket_address,128)==0){
        i++;
    }
    if(get_key_nvalue(filename_temp,linelen_max,"REGION: ",'\"',bucketinfo->region_id,32)==0){ /* There is a blank */
        i++;
    }
    if(get_key_nvalue(filename_temp,linelen_max,"BUCKET_AK:",' ',bucketinfo->bucket_ak,128)==0){
        i++;
    }
    if(get_key_nvalue(filename_temp,linelen_max,"BUCKET_SK:",' ',bucketinfo->bucket_sk,128)==0){
        i++;
    }
    if(strlen(bucketinfo->bucket_ak)==0){
        get_key_nvalue(filename_temp,linelen_max,"BUCKET_LINK:",' ',bucketinfo->bucket_ak,128);
        i++;
    }
    secure_encrypt_and_delete(filename_temp,crypto_keyfile);
    if(contain_or_not(bucketinfo->bucket_address,"gs://")==0){
        if(i!=3){
            strcpy(bucketinfo->bucket_address,"");
            strcpy(bucketinfo->region_id,"");
            strcpy(bucketinfo->bucket_ak,"");
            strcpy(bucketinfo->bucket_sk,"");
            return 1;
        }
        else{
            return 0;
        }
    }
    if(i!=4){
        strcpy(bucketinfo->bucket_address,"");
        strcpy(bucketinfo->region_id,"");
        strcpy(bucketinfo->bucket_ak,"");
        strcpy(bucketinfo->bucket_sk,"");
        return 1;
    }
    else{
        return 0;
    }
}

int tail_f_for_windows(char* filename){
    FILE* file_p=fopen(filename,"r");
    int ch='\0';
    time_t start_time;
    time_t current_time;
    time(&start_time);
    if(file_p==NULL){
        return -1;
    }
    fseek(file_p,-1,SEEK_END);
    printf(WARN_YELLO_BOLD "[ -INFO- ] MAXIMUM DURATION: 30s." RESET_DISPLAY "\n");
    while(1){
        time(&current_time);
        if((ch=fgetc(file_p))!=EOF){
            putchar(ch);
        }
        if((current_time-start_time)>30){
            fclose(file_p);
            return 1;
        }
    }
    fclose(file_p);
    return 0;
}

/*
 * This function is deprecated. Please use get_nucid
 */
int get_ucid(char* workdir, char* ucid_string){
    char vaultdir[DIR_LENGTH]="";
    char filename_ucid[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(filename_ucid,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_ucid,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,ucid_string);
    fclose(file_p);
    return 0;
}

int get_nucid(char* workdir, char* crypto_keyfile, char* ucid_string, unsigned int ucid_strlen_max){
    char hash_key[64]="";
    char vaultdir[DIR_LENGTH]="";
    char ucid_old_file[FILENAME_LENGTH]="";
    char cluster_vaults_encrypted[FILENAME_LENGTH]="";
    char cluster_vaults_decrypted[FILENAME_LENGTH]="";
    strcpy(ucid_string,"");
    if(ucid_strlen_max<11){
        return -3;
    }
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -1;
    }
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(cluster_vaults_encrypted,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",vaultdir,PATH_SLASH);
    snprintf(cluster_vaults_decrypted,FILENAME_LENGTH-1,"%s%sget_ucid.temp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(cluster_vaults_encrypted)==0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,cluster_vaults_encrypted,cluster_vaults_decrypted,hash_key);
        get_key_nvalue(cluster_vaults_decrypted,LINE_LENGTH_SHORT,"short_unique_id:",' ',ucid_string,ucid_strlen_max);
        rm_file_or_dir(cluster_vaults_decrypted);
        if(strlen(ucid_string)>0){
            return 0;
        }
    }
    snprintf(ucid_old_file,FILENAME_LENGTH-1,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(ucid_old_file,"r");
    if(file_p==NULL){
        return 1;
    }
    fngetline(file_p,ucid_string,ucid_strlen_max);
    fclose(file_p);
    if(strlen(ucid_string)<1){
        return 1;
    }
    return 0;
}

int decrypt_user_passwords(char* workdir, char* crypto_keyfile){
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* crypto_exec=NOW_CRYPTO_EXEC;
    char hash_key[64]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return -1;
    }
    decrypt_single_file(crypto_exec,filename_temp,hash_key);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return 1;
    }
    return 0;
}

int delete_decrypted_user_passwords(char* workdir){
    char vaultdir[DIR_LENGTH]="";
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    return rm_file_or_dir(filename_temp);
}

int encrypt_and_delete_user_passwords(char* workdir, char* crypto_keyfile){
    char vaultdir[DIR_LENGTH]="";
    char hash_key[64]="";
    char filename_temp[FILENAME_LENGTH]="";
    if(get_file_sha_hash(crypto_keyfile,hash_key,64)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    int run_flag=encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,hash_key);
    if(run_flag>0){
        return 1;
    }
    return 0;
}

int sync_user_passwords(char* workdir, char* crypto_keyfile, char* sshkey_dir){
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    int run_flag;
    if(create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    run_flag=remote_copy(workdir,crypto_keyfile,sshkey_dir,filename_temp,"/root/.cluster_secrets/user_secrets.txt","root","put","",0);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

int sync_statefile(char* workdir, char* crypto_keyfile, char* sshkey_dir){
    char stackdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    if(create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    return remote_copy(workdir,crypto_keyfile,sshkey_dir,filename_temp,"/usr/hpc-now/currentstate","root","put","",0);
}

int user_password_complexity_check(char* password, char* special_chars){
    if(strlen(password)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Empty password. Length must be in the range %d - %d." RESET_DISPLAY "\n",USER_PASSWORD_LENGTH_MIN,USER_PASSWORD_LENGTH_MAX);
        return -1;
    }
    else if(strlen(password)<USER_PASSWORD_LENGTH_MIN||strlen(password)>USER_PASSWORD_LENGTH_MAX){
        printf(FATAL_RED_BOLD "[ FATAL: ] The password " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY FATAL_RED_BOLD " length is out of range [%d - %d]." RESET_DISPLAY "\n",password,USER_PASSWORD_LENGTH_MIN,USER_PASSWORD_LENGTH_MAX);
        return -1;
    }
    if(password_complexity_check(password,special_chars)==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] The password " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid." RESET_DISPLAY "\n",password);
        printf("[  ****  ] Must include at least 3 of 4 different types: \n");
        printf("[  ****  ] " HIGH_GREEN_BOLD "A-Z  a-z  0-9  %s" RESET_DISPLAY "\n",special_chars);
        return 1;
    }
    return 0;
}

int input_user_passwd(char* password_string, int batch_flag_local){
    char password_temp[32];
    char password_input[32]="";
    char password_confirm[USER_PASSWORD_LENGTH_MAX]="";
    if(batch_flag_local==0){
        return -1;
    }
    printf("[ -INFO- ] Length: %d-%d. Must include at least 3 of 4 different types: \n",USER_PASSWORD_LENGTH_MIN,USER_PASSWORD_LENGTH_MAX);
    printf("[  ****  ] " HIGH_GREEN_BOLD "A-Z  a-z  0-9  %s" RESET_DISPLAY "\n",SPECIAL_PASSWORD_CHARS);
    getpass_stdin("[ INPUT: ] Type a password : ",password_temp,32);
    if(user_password_complexity_check(password_temp,SPECIAL_PASSWORD_CHARS)!=0){
        return 1;
    }
    strcpy(password_input,password_temp);
    getpass_stdin("[  ****  ] Re-type password: ",password_temp,32);                         
    if(strlen(password_temp)>USER_PASSWORD_LENGTH_MAX){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password." RESET_DISPLAY "\n");
        printf(FATAL_RED_BOLD "[  ****  ] " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY WARN_YELLO_BOLD " != " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",password_input,password_temp);
        return 1;
    }
    strcpy(password_confirm,password_temp);
    strcpy(password_temp,"");
    if(strcmp(password_input,password_confirm)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password." RESET_DISPLAY "\n");
        printf(FATAL_RED_BOLD "[  ****  ] " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY WARN_YELLO_BOLD " != " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",password_input,password_confirm);
        return 1;
    }
    strcpy(password_string,password_input);
    return 0;
}

/* 
 * Return  0: User exists
 * Return -1: Failed to get a valid working directory
 * Return 1 and 2: User doesn't exist
 */
int user_name_quick_check(char* cluster_name, char* user_name, char* sshkey_dir){
    char user_sshkey_encrypted[FILENAME_LENGTH]="";
    char user_sshkey_decrypted[FILENAME_LENGTH]="";
    snprintf(user_sshkey_encrypted,FILENAME_LENGTH-1,"%s%s.%s%s%s.key.tmp",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,user_name);
    snprintf(user_sshkey_decrypted,FILENAME_LENGTH-1,"%s%s.%s%s%s.key",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,user_name);
    if(file_exist_or_not(user_sshkey_encrypted)!=0&&file_exist_or_not(user_sshkey_decrypted)!=0){
        return 1;
    }
    else{
        return 0;
    }
}

/*
 * return -5: user registry not exist
 * return -3: username length invalid
 * return  3: start with -, illegal
 * return  5: invalid chars found
 * return  7: found in the registry
 * return  0: valid but not found
 */
int username_check(char* user_registry, char* username_input){
    if(file_exist_or_not(user_registry)!=0){
        return -5;
    }
    if(strlen(username_input)<USERNAME_LENGTH_MIN||strlen(username_input)>USERNAME_LENGTH_MAX){
        return -3;
    }
    int i;
    char username_ext[128]="";
    if(*(username_input)=='-'){
        return 3;
    }
    for(i=0;i<strlen(username_input);i++){
        if(*(username_input+i)=='A'||*(username_input+i)=='Z'||*(username_input+i)=='a'||*(username_input+i)=='z'||*(username_input+i)=='0'||*(username_input+i)=='9'||*(username_input+i)=='-'){
            continue;
        }
        else if(*(username_input+i)>'A'&&*(username_input+i)<'Z'){
            continue;
        }
        else if(*(username_input+i)>'a'&&*(username_input+i)<'z'){
            continue;
        }
        else if(*(username_input+i)>'0'&&*(username_input+i)<'9'){
            continue;
        }
        else{
            return 5;
        }
    }
    snprintf(username_ext,127,"username: %s ",username_input);
    if(find_multi_nkeys(user_registry,LINE_LENGTH_SHORT,username_ext,"","","","")>0){
        return 7;
    }
    return 0;
}


/*
 * Return     0: Username can be added
 * Return Non-0: User name cannot be added, and print out the reason
 */
int username_check_add(char* workdir, char* username_input){
    char vaultdir[DIR_LENGTH]="";
    char user_registry[FILENAME_LENGTH]="";
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(user_registry,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    int check_flag=username_check(user_registry,username_input);
    if(check_flag!=0){
        if(check_flag==-3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The length is out of range (%d - %d).\n" RESET_DISPLAY,USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
            return -3;
        }
        else if(check_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] Do *NOT* begin with '-' ." RESET_DISPLAY "\n");
            return -3;
        }
        else if(check_flag==5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Illegal character(s) found, only A-Z | a-z | - are valid." RESET_DISPLAY "\n");
            return -3;
        }
        else if(check_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the user registry." RESET_DISPLAY "\n");
            return -3;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Username " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " duplicated." RESET_DISPLAY "\n",username_input);
            return -3;
        }
    }
    else{
        printf(GENERAL_BOLD "[  ****  ] Using username: %s" RESET_DISPLAY "\n",username_input);
        return 0;
    }
}

int username_check_select(char* workdir, char* username_input, const char* option){
    if(strcmp(username_input,"root")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user is protected." RESET_DISPLAY "\n");
        return -3;
    }
    if(strcmp(username_input,"user1")==0&&strcmp(option,"passwd")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The administator user1 is protected." RESET_DISPLAY "\n");
        return -3;
    }
    
    char vaultdir[DIR_LENGTH]="";
    char user_registry[FILENAME_LENGTH]="";
    create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(user_registry,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);

    if(username_check(user_registry,username_input)!=7){
        printf(FATAL_RED_BOLD "[ FATAL: ] Username " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is not valid." RESET_DISPLAY "\n",username_input);
        return -3;
    }
    else{
        return 0;
    }
}

int delete_user_from_registry(char* user_registry_file, char* username){
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char single_line[LINE_LENGTH_SHORT]="";
    char filename_temp[FILENAME_LENGTH]="";
    char randstr[8];
    char username_temp[32]="";
    generate_random_nstring(randstr,8,1);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s.temp",user_registry_file,randstr);
    file_p_2=fopen(filename_temp,"w+");
    if(file_p_2==NULL){
        return -3;
    }
    file_p=fopen(user_registry_file,"r");
    if(file_p==NULL){
        fclose(file_p_2);
        return -1;
    }
    while(fngetline(file_p,single_line,LINE_LENGTH_SHORT)!=1){
        get_seq_nstring(single_line,' ',2,username_temp,32);
        if(strcmp(username_temp,username)==0){
            continue;
        }
        fprintf(file_p_2,"%s\n",single_line);
    }
    fclose(file_p);
    fclose(file_p_2);
    rm_file_or_dir(user_registry_file);
    if(rename(filename_temp,user_registry_file)!=0){
        return 1;
    }
    return 0;
}

void get_workdir(char* cluster_workdir, char* cluster_name){
    sprintf(cluster_workdir,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,PATH_SLASH);
}

/*
 * IMPORTANT: You must guarantee the dirlen_max is larger than 280
 * This function is better than get_workdir because it is more secure and added return value
 */
int get_nworkdir(char* cluster_workdir, unsigned int dirlen_max, char* cluster_name){
    if(strlen(cluster_name)<CLUSTER_ID_LENGTH_MIN||dirlen_max<strlen(cluster_name)+DIR_LENGTH_SHORT){
        strcpy(cluster_workdir,"");
        return -1;
    }
    snprintf(cluster_workdir,dirlen_max-1,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,PATH_SLASH);
    return 0;
}

int get_nworkdir_without_last_slash(char* cluster_workdir, unsigned int dirlen_max, char* cluster_name){
    if(strlen(cluster_name)<CLUSTER_ID_LENGTH_MIN||dirlen_max<strlen(cluster_name)+DIR_LENGTH_SHORT){
        strcpy(cluster_workdir,"");
        return -1;
    }
    snprintf(cluster_workdir,dirlen_max-1,"%s%sworkdir%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name);
    return 0;
}

/* Please make sure the cluster_name[] with width 25 */
int get_cluster_name(char* cluster_name, char* cluster_workdir){
    char path_seprator=PATH_SLASH[0];
    int i=0;
    char dir_buffer[128]="";
    char dir_buffer2[128]="";
    while(i<16){
        i++;
        get_seq_nstring(cluster_workdir,path_seprator,i,dir_buffer,128);
        if(strlen(dir_buffer)==0){
            if(cluster_name_check(dir_buffer2)==0||cluster_name_check(dir_buffer2)==-7){
                strcpy(cluster_name,dir_buffer2);
                return 0;
            }
        }
        else{
            strcpy(dir_buffer2,dir_buffer);
        }
    }
    return 1;
}

int get_cluster_nname(char* cluster_name, unsigned int cluster_name_len_max, char* cluster_workdir){
    char path_seprator=PATH_SLASH[0];
    int i=0;
    char dir_buffer[128]="";
    char dir_buffer2[128]="";  
    /* Max directory depth: 16 */
    while(i<16){
        i++;
        get_seq_nstring(cluster_workdir,path_seprator,i,dir_buffer,128);
        if(strlen(dir_buffer)==0){
            if(cluster_name_check(dir_buffer2)==0||cluster_name_check(dir_buffer2)==-7){
                strncpy(cluster_name,dir_buffer2,cluster_name_len_max-1);
                return 0;
            }
        }
        else{
            strcpy(dir_buffer2,dir_buffer);
        }
    }
    return 1;
}

/* 
 * CAUTION: This function may overwrite the cluster registry
 * If the encrypted file exists, then return 0 and do nothing
 * Otherwise encrypt current one (if the decrypted one exists) or create a blank one
 * return  0: OK (either exists or create successfully)
 * return -3: Failed to get the crypto_key
 * return -1: Failed to create a new one
 * return  1: Failed to encrypt and delete
 */
int check_cluster_registry(void){
    char registry_encrypted[FILENAME_LENGTH]="";
    char registry_decbackup[FILENAME_LENGTH]="";
    char hash_key[64]="";
    snprintf(registry_encrypted,FILENAME_LENGTH-1,"%s.tmp",ALL_CLUSTER_REGISTRY);
    snprintf(registry_decbackup,FILENAME_LENGTH-1,"%s.dec.bak",ALL_CLUSTER_REGISTRY);
    /* If the encrypted file exists, then exit an do nothing*/
    if(file_exist_or_not(registry_encrypted)==0){
        if(file_exist_or_not(registry_decbackup)!=0){
            return registry_dec_backup();
        }
        return 0;
    }
    if(get_file_sha_hash(CRYPTO_KEY_FILE,hash_key,64)!=0){
        return -3;
    }
    /* 
     * If REGISTRY.tmp is absent and the REGISTRY is also absent
     * Then create a blank one.
     */
    if(file_exist_or_not(ALL_CLUSTER_REGISTRY)!=0){
        FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"w+");
        if(file_p==NULL){
            return -1;
        }
        fprintf(file_p,"%s\n",INTERNAL_FILE_HEADER);
        fclose(file_p);
    }
    /* Encrypt the REGISTRY */
    if(encrypt_and_delete(NOW_CRYPTO_EXEC,ALL_CLUSTER_REGISTRY,hash_key)!=0){
        return 1;
    }
    /* Update the decrypted backup. */
    return registry_dec_backup();
}

/* 
 * Supported options:
 * filename_base -- filename_base.tmp -- filename_base.extra_str -- filename_base.extra_str.backup (Other commands)
 *            `-- filename_base.extra_str.dbackup (backup,restore,delete_backup)
 * 
 * 1. decrypt: decrypt filename_base.tmp to filename_base.extra_str
 * 2. encrypt: encrypt filename_base.extra_str to filename_base.tmp, and delete filename_base.extra_str
 * 
 * 3. backup: directly copy filename_base to filename_base.extra_str.dbackup
 * 4. restore: directly move filename_base.extra_str.dbackup to filename_base
 * 5. delete_backup: delete filename_base.extra_str.dbackup
 * 
 * 6. backup_encrypt: backup filename_base.extra_str to filename_base.extra_str.backup and encrypt to filename_base.tmp
 * 7. restore_encrypt: encrypt filename_base.extra_str.backup to filename_base.tmp and delete filename_base.extra_str
 * 
 * 8. delete_decrypted: delete filename_base.extra_str
 * 9. delete_decrypted_backup: delete filename_base.extra_str.backup
 * 10. delete_decrypted_all: delete filename_base.extra_str.backup filename_base.extra_str
 * Others are illegal, do nothing and return -3;
 */
int file_convert(char* filename_base, char* extra_str, char* option){
    char file_encrypted[FILENAME_LENGTH]="";
    char file_decrypted[FILENAME_LENGTH]="";
    char file_dec_back[FILENAME_LENGTH]="";
    char file_backup[FILENAME_LENGTH]="";
    char hash_key[64]="";
    if(strlen(filename_base)<1||strlen(extra_str)<1){
        return -5;
    }
    if(strcmp(option,"backup")==0||strcmp(option,"restore")==0||strcmp(option,"delete_backup")==0){
        snprintf(file_backup,FILENAME_LENGTH-1,"%s.%s.backup",filename_base,extra_str);
        if(strcmp(option,"backup")==0){
            if(cp_file(filename_base,file_backup,0)!=0){
                return 1;
            }
            return 0;
        }
        else if(strcmp(option,"restore")==0){
            rm_file_or_dir(filename_base);
            return rename(file_backup,filename_base);
        }
        else{
            return rm_file_or_dir(file_backup);
        }
    }
    snprintf(file_decrypted,FILENAME_LENGTH-1,"%s.%s",filename_base,extra_str);
    snprintf(file_dec_back,FILENAME_LENGTH-1,"%s.%s.dbackup",filename_base,extra_str);
    if(strcmp(option,"delete_decrypted")==0){
        return rm_file_or_dir(file_decrypted);
    }
    if(strcmp(option,"delete_decrypted_backup")==0){
        return rm_file_or_dir(file_dec_back);
    }
    if(strcmp(option,"delete_decrypted_all")==0){
        rm_file_or_dir(file_dec_back);
        rm_file_or_dir(file_decrypted);
        return (file_exist_or_not(file_decrypted)|file_exist_or_not(file_dec_back));
    }
    snprintf(file_encrypted,FILENAME_LENGTH-1,"%s.tmp",filename_base);
    if(get_file_sha_hash(CRYPTO_KEY_FILE,hash_key,64)!=0){
        return -1;
    }
    if(strcmp(option,"decrypt")==0){
        decrypt_single_file_general(NOW_CRYPTO_EXEC,file_encrypted,file_decrypted,hash_key);
        return file_exist_or_not(file_decrypted);
    }
    if(strcmp(option,"encrypt")==0){
        encrypt_and_delete_general(NOW_CRYPTO_EXEC,file_decrypted,file_encrypted,hash_key);
        return file_exist_or_not(file_encrypted);
    }
    if(strcmp(option,"backup_encrypt")==0){
        cp_file(file_decrypted,file_dec_back,0);
        if(file_exist_or_not(file_dec_back)!=0){
            return 1;
        }
        encrypt_and_delete_general(NOW_CRYPTO_EXEC,file_decrypted,file_encrypted,hash_key);
        return file_exist_or_not(file_encrypted);
    }
    if(strcmp(option,"restore_encrypt")==0){
        if(file_exist_or_not(file_dec_back)!=0){
            return 1;
        }
        encrypt_and_delete_general(NOW_CRYPTO_EXEC,file_dec_back,file_encrypted,hash_key);
        return file_exist_or_not(file_encrypted);
    }
    return -3;
}

int registry_dec_backup(void){
    char registry_encrypted[FILENAME_LENGTH]="";
    char registry_decbackup[FILENAME_LENGTH]="";
    char hash_key[64]="";
    if(get_file_sha_hash(CRYPTO_KEY_FILE,hash_key,64)!=0){
        return -3;
    }
    snprintf(registry_decbackup,FILENAME_LENGTH-1,"%s.dec.bak",ALL_CLUSTER_REGISTRY);
    snprintf(registry_encrypted,FILENAME_LENGTH-1,"%s.tmp",ALL_CLUSTER_REGISTRY);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,registry_encrypted,registry_decbackup,hash_key);
    return file_exist_or_not(registry_decbackup);
}

int line_check_by_keyword(char* line, char* keyword, char split_ch, int seq_num){
    if(strlen(line)<1||strlen(keyword)<1||split_ch=='\0'||seq_num<1){
        return -3;
    }
    int keyword_temp_len=strlen(keyword)+1;
    char* keyword_temp=(char*)malloc(sizeof(char)*keyword_temp_len);
    if(keyword_temp==NULL){
        return -1;
    }
    reset_nstring(keyword_temp,keyword_temp_len);
    get_seq_nstring(line,split_ch,seq_num,keyword_temp,keyword_temp_len);
    return strcmp(keyword_temp,keyword);
}

/* 
 * This function is only for opr_crypto module
 * For general purpose, please use cluster_registry_convert() function!
 */
int encrypt_decrypt_cluster_registry(char* option){
    char hash_key[64]="";
    char registry_encrypted[FILENAME_LENGTH]="";
    char registry_decrypted[FILENAME_LENGTH]="";
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return -1;
    }
    if(get_file_sha_hash(CRYPTO_KEY_FILE,hash_key,64)!=0){
        return -3;
    }
    snprintf(registry_decrypted,FILENAME_LENGTH-1,"%s.dec",ALL_CLUSTER_REGISTRY);
    snprintf(registry_encrypted,FILENAME_LENGTH-1,"%s.tmp",ALL_CLUSTER_REGISTRY);
    if(strcmp(option,"encrypt")==0){
        if(encrypt_and_delete_general(NOW_CRYPTO_EXEC,registry_decrypted,registry_encrypted,hash_key)!=0){
            return 1;
        }
        return 0;
    }
    decrypt_single_file_general(NOW_CRYPTO_EXEC,registry_encrypted,registry_decrypted,hash_key);
    return file_exist_or_not(registry_decrypted);
}

int update_tf_passwords(char* base_tf, char* master_tf, char* user_passwords){
    if(file_exist_or_not(base_tf)!=0||file_exist_or_not(master_tf)!=0||file_exist_or_not(user_passwords)!=0){
        return -1;
    }
    FILE* file_p=NULL;
    FILE* file_p_base=NULL;
    char user_line_buffer[256]="";
    char line_temp[LINE_LENGTH_SHORT]="";
    char user_name_temp[64]="";
    char user_passwd_temp[64]="";
    char user_status_temp[16]="";
    file_ntrunc_by_kwds(base_tf,LINE_LENGTH_SMALL,"","user1_passwd",1);
    delete_nlines_by_kwd(master_tf,LINE_LENGTH_SMALL,"username:",1);
    file_p=fopen(user_passwords,"r");
    file_p_base=fopen(base_tf,"a");
    while(!feof(file_p)){
        fngetline(file_p,user_line_buffer,256);
        if(strlen(user_line_buffer)==0){
            continue;
        }
        get_seq_nstring(user_line_buffer,' ',2,user_name_temp,64);
        get_seq_nstring(user_line_buffer,' ',3,user_passwd_temp,64);
        get_seq_nstring(user_line_buffer,' ',4,user_status_temp,16);
        fprintf(file_p_base,"variable \"%s_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",user_name_temp,user_passwd_temp);
        snprintf(line_temp,LINE_LENGTH_SHORT-1,"echo -e \"username: %s ${var.%s_passwd} %s\" >> /root/user_secrets.txt",user_name_temp,user_name_temp,user_status_temp);
        insert_lines(master_tf,"master_private_ip",line_temp);
    }
    fclose(file_p);
    fclose(file_p_base);
    return 0;
}

int check_reconfigure_list(char* workdir, int print_flag){
    char single_line[LINE_LENGTH_TINY]="";
    char reconf_list[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    snprintf(reconf_list,FILENAME_LENGTH-1,"%s%sconf%sreconf.list",workdir,PATH_SLASH,PATH_SLASH);
    if((file_p=fopen(reconf_list,"r"))==NULL){
        return -1;
    }
    if(print_flag==0){
        fclose(file_p);
        return 0;
    }
    while(fngetline(file_p,single_line,LINE_LENGTH_TINY)!=1){
        if(*(single_line+0)=='+'||*(single_line+0)=='|'){
            printf("|         %s\n",single_line);
        }
    }
    return 0;
}

/* 
 * VAUTION! This function is deprecated!
 * If silent_flag=1, verbose. Will tell the user which cluster is active
 * If silent_flag=0, silent. Will print nothing
 * If silent_flag= other_number, Will only show the warning
 * Return 1: No current cluster
 * Return 0: Get current cluster and workdir
 */
int show_current_cluster(char* cluster_workdir, char* current_cluster_name, int silent_flag){
    FILE* file_p=NULL;
    if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0||file_empty_or_not(CURRENT_CLUSTER_INDICATOR)==0){
        if(silent_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently you are not operating any cluster." RESET_DISPLAY "\n");
        }
        return 1;
    }
    else{
        file_p=fopen(CURRENT_CLUSTER_INDICATOR,"r");
        fscanf(file_p,"%31s",current_cluster_name);
        if(silent_flag==1){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Current cluster:" HIGH_GREEN_BOLD " %s" RESET_DISPLAY ".\n",current_cluster_name);
        }
        fclose(file_p);
        get_workdir(cluster_workdir,current_cluster_name);
        return 0;
    }
}

/*  
 * If silent_flag=1, verbose. Will tell the user which cluster is active
 * If silent_flag=0, silent. Will print nothing
 * If silent_flag= other_number, Will only show the warning
 * 
 * Return 1: No current cluster or get cluster_workdir failed
 * Return 0: Get current cluster and workdir
 */
int show_current_ncluster(char* cluster_workdir, unsigned int dirlen_max, char* current_cluster_name, unsigned int cluster_name_len_max, int silent_flag){
    if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0||file_empty_or_not(CURRENT_CLUSTER_INDICATOR)<1){
        if(silent_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently you are not operating any cluster." RESET_DISPLAY "\n");
        }
        return 1;
    }
    char line_buffer[LINE_LENGTH_SHORT]="";
    char header[32]="";
    int i=0;
    FILE* file_p=fopen(CURRENT_CLUSTER_INDICATOR,"r");
    while(fngetline(file_p,line_buffer,LINE_LENGTH_SHORT)!=1&&i<8){
        i++;
        get_seq_nstring(line_buffer,':',1,header,32);
        if(strcmp(header,"current_cluster")==0){
            get_seq_nstring(line_buffer,' ',5,current_cluster_name,cluster_name_len_max);
            break;
        }
    }
    fclose(file_p);
    if(get_nworkdir(cluster_workdir,dirlen_max,current_cluster_name)!=0){
        if(silent_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently you are not operating any cluster." RESET_DISPLAY "\n");
        }
        exit_current_cluster();
        return 1;
    }
    if(silent_flag==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Current cluster:" HIGH_GREEN_BOLD " %s" RESET_DISPLAY ".\n",current_cluster_name);
    }
    return 0;
}

int current_cluster_or_not(char* current_indicator, char* cluster_name){
    char line_buffer[LINE_LENGTH_SHORT]="";
    char header[32]="";
    char current_cluster_name[32]="";
    int i=0;
    FILE* file_p=fopen(current_indicator,"r");
    if(file_p==NULL){
        return 1;
    }
    while(fngetline(file_p,line_buffer,LINE_LENGTH_SHORT)!=1&&i<8){
        i++;
        get_seq_nstring(line_buffer,':',1,header,32);
        if(strcmp(header,"current_cluster")==0){
            get_seq_nstring(line_buffer,' ',5,current_cluster_name,32);
            break;
        }
    }
    fclose(file_p);
    if(strcmp(current_cluster_name,cluster_name)!=0){
        return -1;
    }
    return 0;
}

/*
 * return -1: start with -, illegal
 * return -3: length not in the range, illegal
 * return -5: illegal char found
 * return -7: OK and found
 * return  0: legal but not found.
 * return  1: ABNORMAL failed to decrypt or encrypt the cluster registry
 */
int cluster_name_check(char* cluster_name){
    char cluster_name_ext[64]="";
    char filename_temp[FILENAME_LENGTH]="";
    int i;
    if(*(cluster_name+0)=='-'){
        return -1;
    }
    if(strlen(cluster_name)<CLUSTER_ID_LENGTH_MIN||strlen(cluster_name)>CLUSTER_ID_LENGTH_MAX){
        return -3;
    }
    for(i=0;i<strlen(cluster_name);i++){
        if(*(cluster_name+i)=='-'||*(cluster_name+i)=='0'||*(cluster_name+i)=='9'){
            continue;
        }
        if(*(cluster_name+i)>'0'&&*(cluster_name+i)<'9'){
            continue;
        }
        if(*(cluster_name+i)<'A'||*(cluster_name+i)>'z'){
            return -5;
        }
        else if(*(cluster_name+i)>'Z'&&*(cluster_name+i)<'a'){
            return -5;
        }
        else{
            continue;
        }
    }
    if(file_convert(ALL_CLUSTER_REGISTRY,"general","decrypt")!=0){
        return 1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,"general");
    snprintf(cluster_name_ext,64,"< cluster name: %s >",cluster_name);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,cluster_name_ext,"","","","")>0){
        file_convert(ALL_CLUSTER_REGISTRY,"general","delete_decrypted");
        return -7;
    }
    if(file_convert(ALL_CLUSTER_REGISTRY,"general","delete_decrypted")!=0){
        return 1;
    }
    return 0;
}

int check_and_cleanup(char* prev_workdir){
    char current_workdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char current_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
#ifdef _WIN32
    char appdata_dir[DIR_LENGTH]="";
    char dirname_temp[DIR_LENGTH_EXT]="";
#endif
    if(strlen(prev_workdir)!=0){
        if(show_current_ncluster(current_workdir,DIR_LENGTH,current_cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,0)==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently there is no switched cluster." RESET_DISPLAY "\n");
        }
        else{
            if(strcmp(current_workdir,prev_workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The switched cluster is" RESET_DISPLAY HIGH_GREEN_BOLD " %s" RESET_DISPLAY WARN_YELLO_BOLD ".\n" RESET_DISPLAY,current_cluster_name);
            }
        }
    }
#ifdef _WIN32
    if(get_win_appdata_dir(appdata_dir,DIR_LENGTH)!=0){
        return -5;
    }
    system("icacls C:\\programdata\\hpc-now\\workdir /remove Administrators /T > nul 2>&1");
    system("icacls C:\\programdata\\hpc-now\\etc /remove Administrators /T > nul 2>&1");
    system("icacls C:\\programdata\\hpc-now\\workdir /deny Administrators:F /T > nul 2>&1");
    system("icacls C:\\programdata\\hpc-now\\etc /deny Administrators:F /T > nul 2>&1");
    snprintf(dirname_temp,DIR_LENGTH_EXT-1,"%s\\Microsoft\\Windows\\Recent",appdata_dir);
    rm_file_or_dir(dirname_temp);
    batch_file_operation(NOW_TMP_DIR,"*.rdp","","rm",0);
#elif __linux__
    batch_file_operation(NOW_TMP_DIR,"*remmina*","","rm",0);
    batch_file_operation(NOW_TMP_DIR,".*remmina*","","rm",0);
#else
    batch_file_operation("/tmp/",".hpc-now*.rdp","","rm",0);
#endif
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%smd5values.conf",GENERAL_CONF_DIR,PATH_SLASH);
    rm_file_or_dir(filename_temp);
    print_tail();
    return 0;
}

/*
 * Options:
 *   verbosity_level = 0: fully silent
 *   verbosity_level = others : print out everything
 * Return values: 
 *   -1: No registry file
 *    1: Empty registry
 *    0: Normal exit
 */
int list_all_cluster_names(int verbosity_level){
    char filename_temp[FILENAME_LENGTH]="";
    char randstr[7]="";
    generate_random_nstring(randstr,7,0);
    file_convert(ALL_CLUSTER_REGISTRY,randstr,"decrypt");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,randstr);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        if(verbosity_level!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the registry. Please run hpcopr repair." RESET_DISPLAY "\n");
        }
        return -1;
    }
    char registry_line[LINE_LENGTH_SHORT]="";
    char temp_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    int i=0;
    while(fngetline(file_p,registry_line,LINE_LENGTH_SHORT)!=1){
        if(line_check_by_keyword(registry_line,"< cluster name",':',1)!=0){
            continue;
        }
        i++;
        if(verbosity_level==0){
            continue;
        }
        get_seq_nstring(registry_line,' ',4,temp_cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS);
        if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
            printf(GENERAL_BOLD "| switch : (%d) %s" RESET_DISPLAY "\n",i,temp_cluster_name);
        }
        else{
            printf(RESET_DISPLAY "| :::::: : (%d) %s" RESET_DISPLAY "\n",i,temp_cluster_name);
        }
    }
    fclose(file_p);
    file_convert(ALL_CLUSTER_REGISTRY,randstr,"delete_decrypted");
    if(i==0){
        if(verbosity_level!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The local cluster registry is empty." RESET_DISPLAY "\n");
        }
        return 1; /* Empty cluster registry. */
    }
    return 0;
}

int exit_current_cluster(void){
    return rm_file_or_dir(CURRENT_CLUSTER_INDICATOR);
}

int delete_from_cluster_registry(char* deleted_cluster_name){
    char randstr[7]="";
    char filename_temp[FILENAME_LENGTH]="";
    char registry_line[LINE_LENGTH_SHORT]="";
    generate_random_nstring(randstr,7,0);
    file_convert(ALL_CLUSTER_REGISTRY,randstr,"decrypt");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,randstr);
    snprintf(registry_line,LINE_LENGTH_SHORT-1,"< cluster name: %s >",deleted_cluster_name);
    if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,deleted_cluster_name)==0){
        exit_current_cluster();
    }
    delete_nlines_by_kwd(filename_temp,LINE_LENGTH_SMALL,registry_line,1);
    if(file_convert(ALL_CLUSTER_REGISTRY,randstr,"encrypt")!=0){
        return 1;
    }
    registry_dec_backup(); /* Update the decrypted backup */
    return 0;
}

int modify_payment_single_line(char* filename_temp, char* modify_flag, char* line_buffer){
    if(strlen(line_buffer)==0){
        return 1;
    }
    if(strcmp(modify_flag,"add")==0){
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"user_data",line_buffer);
    }
    else{
        delete_nlines_by_kwd(filename_temp,LINE_LENGTH_SHORT,line_buffer,1);
    }
    return 0;
}

int modify_payment_lines(char* stackdir, char* crypto_keyfile, char* cloud_flag, char* modify_flag){
    if(strcmp(modify_flag,"add")!=0&&strcmp(modify_flag,"del")!=0){
        return -1;
    }
    char line_buffer1[128]="";
    char line_buffer2[128]="";
    char line_buffer3[128]="";
    char line_buffer4[128]="";
    char filename_temp[FILENAME_LENGTH]="";
    int i;
    int compute_nodes=get_compute_node_num(stackdir,crypto_keyfile,"all");
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        strcpy(line_buffer1,"  instance_charge_type = \"PrePaid\"");
        strcpy(line_buffer2,"  period_unit = \"Month\"");
        strcpy(line_buffer3,"  period = 1");
        strcpy(line_buffer4,"renewal_status = \"AutoRenewal\"");
        
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        strcpy(line_buffer1,"  instance_charge_type = \"PREPAID\"");
        strcpy(line_buffer2,"  instance_charge_type_prepaid_period = 1");
        strcpy(line_buffer3,"  force_delete = true");
        strcpy(line_buffer4,"instance_charge_type_prepaid_renew_flag = \"NOTIFY_AND_AUTO_RENEW\"");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        strcpy(line_buffer1,"  charging_mode = \"prePaid\"");
        strcpy(line_buffer2,"  period_unit = \"month\"");
        strcpy(line_buffer3,"  period = 1");
        strcpy(line_buffer4,"  auto_renew = true");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        strcpy(line_buffer1,"  payment_timing = \"Prepaid\"");
        strcpy(line_buffer2,"");
        strcpy(line_buffer3,"");
        strcpy(line_buffer4,"");
    }
    else{
        return -3;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer1);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer2);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer3);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer4);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer1);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer2);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer3);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer4);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer1);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer2);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer3);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer4);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scompute_template",stackdir,PATH_SLASH);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer1);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer2);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer3);
    modify_payment_single_line(filename_temp,modify_flag,line_buffer4);
    for(i=0;i<compute_nodes;i++){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        modify_payment_single_line(filename_temp,modify_flag,line_buffer1);
        modify_payment_single_line(filename_temp,modify_flag,line_buffer2);
        modify_payment_single_line(filename_temp,modify_flag,line_buffer3);
        modify_payment_single_line(filename_temp,modify_flag,line_buffer4);
    }
    return 0;
}

int bceconfig_convert(char* vaultdir, char* option, char* region_id, char* bucket_ak, char* bucket_sk){
    char config[FILENAME_LENGTH]="";
    char credentials[FILENAME_LENGTH]="";
    snprintf(config,FILENAME_LENGTH-1,"%s%sconfig",vaultdir,PATH_SLASH);
    snprintf(credentials,FILENAME_LENGTH-1,"%s%scredentials",vaultdir,PATH_SLASH);
    if(strcmp(option,"generate")!=0){
        rm_file_or_dir(config);
        rm_file_or_dir(credentials);
        return 0;
    }
    FILE* file_p1=fopen(config,"w+");
    if(file_p1==NULL){
        return -1;
    }
    FILE* file_p2=fopen(credentials,"w+");
    if(file_p2==NULL){
        fclose(file_p1);
        return -1;
    }
    fprintf(file_p1,"[Defaults]\nDomain = %s.bcebos.com\nRegion = %s\nAutoSwitchDomain = \nBreakpointFileExpiration = 10000\nHttps = yes\nMultiUploadThreadNum = \nSyncProcessingNum = \nMultiUploadPartSize = \nProxyHost =\n",region_id,region_id);
    fclose(file_p1);
    fprintf(file_p2,"[Defaults]\nAk = %s\nSk = %s\nSts = ",bucket_ak,bucket_sk);
    fclose(file_p2);
    return 0;
}

/* 
 * key_flag=0, gcp_secrets; key_flag!=0, bucket_secrets;
 * operation="decrypt", decrypt; others, encrypt
 */
int gcp_credential_convert(char* workdir, const char* operation, int key_flag){
    char hash_key[64]="";
    char vaultdir[DIR_LENGTH]="";
    char keyfile_encrypted[FILENAME_LENGTH]="";
    char keyfile_decrypted[FILENAME_LENGTH]="";
    char cluster_vaults[FILENAME_LENGTH]="";
    if(get_file_sha_hash(CRYPTO_KEY_FILE,hash_key,64)!=0||create_and_get_subdir(workdir,"vault",vaultdir,DIR_LENGTH)!=0){
        return -3;
    }
    if(key_flag==0){
        snprintf(keyfile_encrypted,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
        snprintf(keyfile_decrypted,FILENAME_LENGTH-1,"%s%s.key.json",vaultdir,PATH_SLASH);
        if(strcmp(operation,"decrypt")==0){
            decrypt_single_file_general(NOW_CRYPTO_EXEC,keyfile_encrypted,keyfile_decrypted,hash_key);
            delete_nlines_by_kwd(keyfile_decrypted,LINE_LENGTH_MID,"CLOUD_G",1);
            return file_exist_or_not(keyfile_decrypted);
        }
        else{
            if(rm_file_or_dir(keyfile_decrypted)!=0){
                return 1;
            }
            return 0;
        }
    }
    else{
        /* Use cluster_vaults.txt.tmp instead of bucket_key.txt.tmp */
        snprintf(cluster_vaults,FILENAME_LENGTH-1,"%s%scluster_vaults.txt.tmp",vaultdir,PATH_SLASH);
        /* bucket_key.txt.tmp is deprecated. */
        snprintf(keyfile_encrypted,FILENAME_LENGTH-1,"%s%sbucket_key.txt.tmp",vaultdir,PATH_SLASH);
        /* The decrypted key file. */
        snprintf(keyfile_decrypted,FILENAME_LENGTH-1,"%s%s.bucket_key.json",vaultdir,PATH_SLASH);
        if(strcmp(operation,"decrypt")==0){
            if(file_exist_or_not(cluster_vaults)==0){
                decrypt_single_file_general(NOW_CRYPTO_EXEC,cluster_vaults,keyfile_decrypted,hash_key);
                file_cr_clean(keyfile_decrypted);
                file_ntrunc_by_kwds(keyfile_decrypted,LINE_LENGTH_MID,"{\n","",1);
                return file_exist_or_not(keyfile_decrypted);
            }
            decrypt_single_file_general(NOW_CRYPTO_EXEC,keyfile_encrypted,keyfile_decrypted,hash_key);
            return file_exist_or_not(keyfile_decrypted);
        }
        else{
            if(rm_file_or_dir(keyfile_decrypted)!=0){
                return 1;
            }
            return 0;
        }
    }
}

int get_max_cluster_name_length(void){
    char randstr[7]="";
    char filename_temp[FILENAME_LENGTH]="";
    generate_random_nstring(randstr,7,0);
    file_convert(ALL_CLUSTER_REGISTRY,randstr,"decrypt");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.%s",ALL_CLUSTER_REGISTRY,randstr);
    FILE* file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    char registry_single_line[LINE_LENGTH_SHORT]="";
    char cluster_name_temp[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    int max_length=0;
    int temp_length=0;
    while(fngetline(file_p,registry_single_line,LINE_LENGTH_SHORT)!=1){
        if(line_check_by_keyword(registry_single_line,"< cluster name",':',1)!=0){
            continue;
        }
        get_seq_nstring(registry_single_line,' ',4,cluster_name_temp,CLUSTER_ID_LENGTH_MAX_PLUS);
        temp_length=strlen(cluster_name_temp);
        if(temp_length>max_length){
            max_length=temp_length;
        }
    }
    fclose(file_p);
    if(file_convert(ALL_CLUSTER_REGISTRY,randstr,"delete_decrypted")!=0){
        return -1;
    }
    return max_length;
}

int password_to_clipboard(char* cluster_workdir, char*crypto_keyfile, char* username, char* randstr){
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_vaults[FILENAME_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH_EXT]="";
    char vaultdir[DIR_LENGTH]="";
    char password_string[64]="";
    char username_ext[64]="";
    char hash_key[64]="";
    int run_flag;
    FILE* file_p=NULL;
    if(create_and_get_subdir(cluster_workdir,"vault",vaultdir,DIR_LENGTH)!=0||get_file_sha_hash(crypto_keyfile,hash_key,64)!=0){
        return -3;
    }
    if(strcmp(username,"root")==0){
        snprintf(cluster_vaults,FILENAME_LENGTH-1,"%s%scluster_vaults.txt",vaultdir,PATH_SLASH);
        file_convert(cluster_vaults,randstr,"decrypt");
        snprintf(filename_temp,FILENAME_LENGTH_EXT-1,"%s.%s",cluster_vaults,randstr);
        if(get_key_nvalue(filename_temp,LINE_LENGTH_SHORT,"mast_root_password:",' ',password_string,64)==0){
            file_convert(cluster_vaults,randstr,"delete_decrypted");
        }
        /* The step below is only for compatibility */
        else{
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
            decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,hash_key);
            snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
            find_and_nget(filename_temp,LINE_LENGTH_SHORT,"Master Node Root Password:","","",1,"Master Node Root Password:","","",' ',5,password_string,64);
            rm_file_or_dir(filename_temp);
        }
    }
    else{
        snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
        file_convert(user_passwords,randstr,"decrypt");
        snprintf(filename_temp,FILENAME_LENGTH_EXT-1,"%s.%s",user_passwords,randstr);
        snprintf(username_ext,63,"username: %s ",username);
        find_and_nget(filename_temp,LINE_LENGTH_SHORT,username_ext,"","",1,username_ext,"","",' ',3,password_string,64);
        file_convert(user_passwords,randstr,"delete_decrypted");
    }
    if(strlen(password_string)==0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tmp%spassword_for_rdp.%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,randstr);
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return 1;
    }
    fprintf(file_p,"%s",password_string);
    fclose(file_p);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s %s",CAT_FILE_CMD,filename_temp,PIPE_TO_CLIPBOARD_CMD,SYSTEM_CMD_REDIRECT_NULL);
    run_flag=system(cmdline);
    rm_file_or_dir(filename_temp);
    if(run_flag!=0){
        return 3;
    }
    else{
        return 0;
    }
}

int generate_rdp_file(char* cluster_name, char* master_address, char* username, char* randstr){
    char filename_rdp[FILENAME_LENGTH]="";
#ifdef __linux__
    snprintf(filename_rdp,FILENAME_LENGTH-1,"%s%s.tmp%s%s-%s-%s.remmina",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,username,randstr);
    /*snprintf(filename_rdp,FILENAME_LENGTH-1,"/tmp/.hpc-now-%s-%s-%s.remmina",cluster_name,username,randstr);*/
#elif __APPLE__
    snprintf(filename_rdp,FILENAME_LENGTH-1,"/tmp/.hpc-now-%s-%s-%s.rdp",cluster_name,username,randstr);
#else
    snprintf(filename_rdp,FILENAME_LENGTH-1,"%s%s.tmp%s%s-%s-%s.rdp",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,username,randstr);
#endif
    FILE* file_p=fopen(filename_rdp,"w+");
    if(file_p==NULL){
        return -1;
    }
#ifdef __linux__
    fprintf(file_p,"[remmina]\n");
    fprintf(file_p,"name=%s-%s\n",master_address,username);
    fprintf(file_p,"server=%s\n",master_address);
    fprintf(file_p,"colordepth=32\n");
    fprintf(file_p,"username=%s\n",username);
    fprintf(file_p,"protocal=RDP\n");
    fprintf(file_p,"disableclipboard=0\n");
    fclose(file_p);
    return 0;
#else
    fprintf(file_p,"full address:s:%s\n",master_address);
    fprintf(file_p,"redirectclipboard:i:1\n");
    fprintf(file_p,"autoreconnection enabled:i:1\n");
    fprintf(file_p,"authentication level:i:2\n");
    fprintf(file_p,"prompt for credentials on client:i:1\n");
    fprintf(file_p,"username:s:%s\n",username);
    fprintf(file_p,"negotiate security layer:i:1\n");
    fclose(file_p);
    return 0;
#endif
}

int start_rdp_connection(char* cluster_workdir, char* crypto_keyfile, char* username, int password_flag){
    char master_address[32]="";
    char filename_rdp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char randstr[8]="";
    int run_flag;
    generate_random_nstring(randstr,8,1);
    if(password_flag==0){
        if(password_to_clipboard(cluster_workdir,crypto_keyfile,username,randstr)!=0){
            return 1;
        }
        else{
            printf(WARN_YELLO_BOLD "|\n[ -WARN- ] VERY RISKY! The user's password has been copied to the clipboard!\n");
            printf("[  ****  ] Please empty your clipboard after pasting the password!" RESET_DISPLAY "\n");
        }
    }
    if(get_cluster_nname(cluster_name,CLUSTER_ID_LENGTH_MAX_PLUS,cluster_workdir)!=0){
        return 3;
    }
    if(get_state_nvalue(cluster_workdir,crypto_keyfile,"master_public_ip:",master_address,32)!=0){
        return 5;
    }
    if(generate_rdp_file(cluster_name,master_address,username,randstr)!=0){
        return 7;
    }
#ifdef __linux__
    snprintf(filename_rdp,FILENAME_LENGTH-1,"%s%s.tmp%s%s-%s-%s.remmina",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,username,randstr);
    /*snprintf(filename_rdp,FILENAME_LENGTH-1,"/tmp/.hpc-now-%s-%s-%s.remmina",cluster_name,username,randstr);*/
#elif __APPLE__
    snprintf(filename_rdp,FILENAME_LENGTH-1,"/tmp/.hpc-now-%s-%s-%s.rdp",cluster_name,username,randstr);
#else
    snprintf(filename_rdp,FILENAME_LENGTH-1,"%s%s.tmp%s%s-%s-%s.rdp",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,username,randstr);
#endif
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",RDP_EDIT_CMD,filename_rdp,SYSTEM_CMD_REDIRECT_NULL);
    run_flag=system(cmdline);
    if(run_flag!=0){
        return 9;
    }
    return 0;
}

int cluster_rdp(char* cluster_workdir, char* crypto_keyfile, char* username, char* cluster_role, int password_flag){
    if(strcmp(cluster_role,"opr")!=0&&strcmp(cluster_role,"admin")!=0&&strcmp(username,"root")==0){
        return -3;
    }
    return start_rdp_connection(cluster_workdir,crypto_keyfile,username,password_flag);
}

/* 
 * If file exists, return the file pointer
 * Otherwise, return NULL
 */
FILE* check_regions_list_file(char* cluster_name){
    char workdir[DIR_LENGTH]="";
    char region_list[FILENAME_LENGTH]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return NULL;
    }
    snprintf(region_list,FILENAME_LENGTH-1,"%s%sconf%sregions.list",workdir,PATH_SLASH,PATH_SLASH);
    FILE* file_p=fopen(region_list,"r");
    return file_p;
}

int list_cloud_regions(char* cluster_name, int format_flag){
    int i=0;
    char line_buffer[LINE_LENGTH_TINY]="";
    FILE* file_p=check_regions_list_file(cluster_name);
    if(file_p==NULL){
        return -1;
    }
    if(format_flag!=0){
        putchar('\n');
    }
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,LINE_LENGTH_TINY-1);
        if(*(line_buffer+0)!='['){
            continue;
        }
        if(contain_or_not(line_buffer,"[Region:")==0){
            if(format_flag!=0){
                printf("|       +- %s\n",line_buffer);
            }
            else{
                printf("%s\n",line_buffer);
            }
            i++;
        }
    }
    fclose(file_p);
    if(format_flag!=0){
        putchar('\n');
    }
    if(i==0){
        return 1;
    }
    return 0;
}

int list_cloud_zones(char* cluster_name, char* region, int format_flag){
    int i=0;
    char line_buffer[LINE_LENGTH_TINY]="";
    char region_ext[64]="";
    FILE* file_p=check_regions_list_file(cluster_name);
    if(file_p==NULL){
        return -1;
    }
    snprintf(region_ext,64,"[%s]",region);
    if(format_flag!=0){
        putchar('\n');
    }
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,LINE_LENGTH_TINY-1);
        if(*(line_buffer+0)!='['){
            continue;
        }
        if(contain_or_not(line_buffer,region_ext)==0){
            if(format_flag!=0){
                printf("|       +- %s\n",line_buffer);
            }
            else{
                printf("%s\n",line_buffer);
            }
            i++;
        }
    }
    fclose(file_p);
    if(format_flag!=0){
        putchar('\n');
    }
    if(i==0){
        return 1;
    }
    return 0;
}

/* 
 * return 0: valid
 * return 1: invalid
 */
int valid_region_or_not(char* cluster_name, char* region){
    char line_buffer[LINE_LENGTH_TINY]="";
    char region_ext[64]="";
    FILE* file_p=check_regions_list_file(cluster_name);
    if(file_p==NULL){
        return -1;
    }
    snprintf(region_ext,64,"[Region:%s]",region);
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,LINE_LENGTH_TINY-1);
        if(contain_or_not(line_buffer,region_ext)==0){
            fclose(file_p);
            return 0;
        }
    }
    fclose(file_p);
    return 1;
}

int valid_region_zone_or_not(char* cluster_name, char* region, char* zone){
    char line_buffer[LINE_LENGTH_TINY]="";
    char zone_ext[64]="";
    FILE* file_p=check_regions_list_file(cluster_name);
    if(file_p==NULL){
        return -1;
    }
    snprintf(zone_ext,63,"[%s] %s",region,zone);
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,LINE_LENGTH_TINY-1);
        if(contain_or_not(line_buffer,zone_ext)==0){
            fclose(file_p);
            return 0;
        }
    }
    fclose(file_p);
    return 1;
}

int valid_zone_or_not(char* cluster_name, char* zone){
    char line_buffer[LINE_LENGTH_TINY]="";
    FILE* file_p=check_regions_list_file(cluster_name);
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fngetline(file_p,line_buffer,LINE_LENGTH_TINY-1);
        if(contain_or_not(line_buffer,zone)==0){
            fclose(file_p);
            return 0;
        }
    }
    fclose(file_p);
    return 1;
}

int get_default_zone(char* cluster_name, char* region, char* default_zone){
    char workdir[DIR_LENGTH]="";
    char region_list[FILENAME_LENGTH]="";
    char region_ext1[64]="";
    char region_ext2[64]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -1;
    }
    snprintf(region_list,FILENAME_LENGTH-1,"%s%sconf%sregions.list",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(region_ext1,63,"[Region:%s]",region);
    snprintf(region_ext2,63,"[%s]",region);
    if(find_and_get(region_list,region_ext1,"","",16,region_ext2,"[Default]","",' ',2,default_zone)!=0){
        return 1;
    }
    return 0;
}

int get_default_nzone(char* cluster_name, char* region, char* default_zone, unsigned int zone_len_max){
    char workdir[DIR_LENGTH]="";
    char region_list[FILENAME_LENGTH]="";
    char region_ext1[64]="";
    char region_ext2[64]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -1;
    }
    snprintf(region_list,FILENAME_LENGTH-1,"%s%sconf%sregions.list",workdir,PATH_SLASH,PATH_SLASH);
    snprintf(region_ext1,63,"[Region:%s]",region);
    snprintf(region_ext2,63,"[%s]",region);
    if(find_and_nget(region_list,LINE_LENGTH_SHORT,region_ext1,"","",16,region_ext2,"[Default]","",' ',2,default_zone,zone_len_max)!=0){
        return 1;
    }
    return 0;
}