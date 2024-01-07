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

#include "now_macros.h"
#include "general_funcs.h"
#include "cluster_general_funcs.h"
#include "cluster_operations.h"
#include "userman.h"
#include "transfer.h"

int get_import_info(char cluster_name_output[], unsigned int name_len_max, char tmp_top_output[], unsigned int dir_len_max, char unique_id[], unsigned int id_len_max, char* tmp_import_root, char* md5sum){
    if(name_len_max<CLUSTER_ID_LENGTH_MAX_PLUS||dir_len_max<DIR_LENGTH_SHORT||id_len_max<RANDSTR_LENGTH_PLUS){
        return -5;
    }
    char cluster_name_flag[FILENAME_LENGTH]="";
    char cluster_name_flag_tmp[FILENAME_LENGTH]="";
    char dir_win[DIR_LENGTH]="";
    char dir_lin[DIR_LENGTH]="";
    char dir_dwn[DIR_LENGTH]="";
    char dir_top[DIR_LENGTH_EXT]="";

    snprintf(dir_win,DIR_LENGTH-1,"%s%sprogramdata",tmp_import_root,PATH_SLASH);
    snprintf(dir_lin,DIR_LENGTH-1,"%s%susr",tmp_import_root,PATH_SLASH);
    snprintf(dir_dwn,DIR_LENGTH-1,"%s%sApplications",tmp_import_root,PATH_SLASH);
    if(folder_exist_or_not(dir_win)==0){
        snprintf(dir_top,DIR_LENGTH_EXT-1,"%s%shpc-now",dir_win,PATH_SLASH);
    }
    else if(folder_exist_or_not(dir_lin)==0){
        snprintf(dir_top,DIR_LENGTH_EXT-1,"%s%s.hpc-now",dir_lin,PATH_SLASH);
    }
    else if(folder_exist_or_not(dir_dwn)==0){
        snprintf(dir_top,DIR_LENGTH_EXT-1,"%s%s.hpc-now",dir_dwn,PATH_SLASH);
    }
    else{
        return -3;
    }
    snprintf(cluster_name_flag_tmp,FILENAME_LENGTH-1,"%s%scluster_name_flag.txt.tmp",dir_top,PATH_SLASH);
    snprintf(cluster_name_flag,FILENAME_LENGTH-1,"%s%scluster_name_flag.txt",dir_top,PATH_SLASH);
    if(decrypt_single_file(NOW_CRYPTO_EXEC,cluster_name_flag_tmp,md5sum)!=0){
        strcpy(cluster_name_output,"");
        strcpy(tmp_top_output,"");
        return -5;
    }
    file_cr_clean(cluster_name_flag);
    if(find_multi_nkeys(cluster_name_flag,LINE_LENGTH_SHORT,TRANSFER_HEADER,"","","","")>0){
        get_key_nvalue(cluster_name_flag,LINE_LENGTH_TINY,"cluster_name:",' ',cluster_name_output,name_len_max);
        get_key_nvalue(cluster_name_flag,LINE_LENGTH_TINY,"unique_id:",' ',unique_id,id_len_max);
        if(strlen(cluster_name_output)>0&&strlen(unique_id)>0){
            strncpy(tmp_top_output,dir_top,dir_len_max-1);
            return 0;
        }
    }
    strcpy(cluster_name_output,"");
    strcpy(tmp_top_output,"");
    strcpy(unique_id,"");
    return 1;
}

int user_list_check(char* cluster_name, char* user_list_read, char* user_list_final, int* user1_flag){
    int i=1;
    int j;
    int k=0;
    int valid_user_num=0;
    char user_list_buffer[1024]="";
    char username_temp[32]="";
    do{
        get_seq_nstring(user_list_read,':',i,username_temp,32);
        if(strcmp(username_temp,"user1")==0){
            *user1_flag=1;
        }
        i++;
        if(strcmp(username_temp,"root")==0){
            continue; //root user cannot be exported.
        }
        if(user_name_quick_check(cluster_name,username_temp,SSHKEY_DIR)==0){
            valid_user_num++;
            for(j=0;j<strlen(username_temp);j++){
                *(user_list_buffer+k)=*(username_temp+j);
                k++;
            }
            *(user_list_buffer+k)=':';
            k++;
        }
    }while(strlen(username_temp)!=0);
    *(user_list_buffer+k)='\0';
    strcpy(user_list_final,user_list_buffer);
    return valid_user_num;
}

int export_cluster(char* cluster_name, char* user_list, char* admin_flag, char* crypto_keyfile, char* password, char* export_target_file, int batch_flag_local){
    char workdir[DIR_LENGTH]="";
    char current_stackdir[DIR_LENGTH]="";
    char current_vaultdir[DIR_LENGTH]="";
    char current_sshdir[DIR_LENGTH]="";
    char tmp_root[DIR_LENGTH]="";
    char tmp_stackdir[DIR_LENGTH_EXT]="";
    char tmp_vaultdir[DIR_LENGTH_EXT]="";
    char tmp_sshdir[DIR_LENGTH_EXT]="";
    char cmdline[CMDLINE_LENGTH]="";
    char source_file[FILENAME_LENGTH]="";
    char target_file[FILENAME_LENGTH]="";
    char cluster_name_flag[FILENAME_LENGTH]="";
    char cluster_name_flag_tmp[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp_2[FILENAME_LENGTH]="";
    char filename_temp_3[FILENAME_LENGTH]="";
    char filename_temp_4[FILENAME_LENGTH]="";
    char username_temp[32]="";
    char username_temp_2[32]="";
    char unique_id[16]="";
    char* password_temp;
    char real_password[128]="";
    char real_export_file[FILENAME_LENGTH_EXT]="";
    char real_export_folder[FILENAME_LENGTH_EXT]="";
    char export_filename[1024]="";
    char user_line_buffer[LINE_LENGTH_SHORT]="";
    char real_user_list[1024]="";
    char md5sum_current[64]="";
    char md5sum_trans[64]="";
    char user_list_buffer[1024]="";
    char real_admin_flag[8]="";
    int i=0;
    int user1_flag=0;
    int export_user_num=0;
    FILE* file_p_tmp=NULL;
    FILE* file_p=NULL;
    time_t current_time_long;
    char current_date[64]="";
    char current_time[64]="";
    struct tm* time_p=NULL;

    time(&current_time_long);
    time_p=localtime(&current_time_long);
    snprintf(current_date,63,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,63,"%d-%d-%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);

    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return -7;
    }
    if(get_nucid(workdir,unique_id,16)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the unique cluster id." RESET_DISPLAY "\n");
        return -7;
    }
    if(strlen(user_list)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] User list specified. Use --ul USER_LIST." RESET_DISPLAY "\n");
            return 17;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input the username(s) to be exported. i.e. " HIGH_CYAN_BOLD "user1:user2." RESET_DISPLAY "\n");
            hpc_user_list(workdir,crypto_keyfile,0,1);
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%1023s",user_list_buffer);
            getchar();
        }
    }
    else{
        strcpy(user_list_buffer,user_list);
    }
    if(strcmp(user_list_buffer,"ALL")==0||strcmp(user_list_buffer,"all")==0||strcmp(user_list_buffer,"All")==0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Exporting *ALL* users of cluster %s . NOT RECOMMENDED!\n" RESET_DISPLAY,cluster_name);
        strcpy(real_user_list,"all");
        strcpy(real_admin_flag,"admin");
        user1_flag=1;
    }
    else{
        export_user_num=user_list_check(cluster_name,user_list_buffer,real_user_list,&user1_flag);
        if(export_user_num==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified user list is invalid. Exit now." RESET_DISPLAY "\n");
            return -1;
        }
        if(user1_flag==1&&strcmp(admin_flag,"admin")==0){
            strcpy(real_admin_flag,"admin");
        }
        else{
            strcpy(real_admin_flag,"");
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Cluster: %s . Exporting users below:\n",cluster_name);
        printf(GENERAL_BOLD "|          %s\n" RESET_DISPLAY,real_user_list);
    }
    if(strlen(password)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Password not specified. Use -p PASSWORD ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a *complex* password to encrypt and export.\n");
        password_temp=GETPASS_FUNC("[ INPUT: ] *without echo* ");
        strcpy(real_password,password_temp);
    }
    else{
        strcpy(real_password,password);
    }
    password_hash(real_password,md5sum_trans,64);
    local_path_parser(export_target_file,filename_temp);
    if(strlen(filename_temp)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Export path not specified. Use -d EXPORT_DEST." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a path (folder or file) to export. i.e. " HIGH_CYAN_BOLD "/home/hpc-now/" RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
        fflush(stdin);
        scanf("%511s",filename_temp_2);
        getchar();
        local_path_parser(filename_temp_2,filename_temp);
    }
    if(folder_exist_or_not(filename_temp)==0){
        strcpy(real_export_folder,filename_temp);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exporting to the folder " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",real_export_folder);
    }
    else{
        if(file_creation_test(filename_temp)==0){
            strcpy(real_export_file,filename_temp);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exporting to the file " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " .\n",real_export_file);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The specified export path %s is invalid. Using the default one.\n" RESET_DISPLAY,filename_temp);
        }
    }

    create_and_get_subdir(workdir,"stack",current_stackdir,DIR_LENGTH);
    create_and_get_subdir(workdir,"vault",current_vaultdir,DIR_LENGTH);
    snprintf(current_sshdir,DIR_LENGTH-1,"%s%s.%s",SSHKEY_DIR,PATH_SLASH,cluster_name);

    snprintf(tmp_root,DIR_LENGTH-1,"%s%sexport",HPC_NOW_ROOT_DIR,PATH_SLASH);
    snprintf(tmp_stackdir,DIR_LENGTH_EXT-1,"%s%s%s%sstack",tmp_root,PATH_SLASH,cluster_name,PATH_SLASH);
    snprintf(tmp_vaultdir,DIR_LENGTH_EXT-1,"%s%s%s%svault",tmp_root,PATH_SLASH,cluster_name,PATH_SLASH);
    snprintf(tmp_sshdir,DIR_LENGTH_EXT-1,"%s%s.%s",tmp_root,PATH_SLASH,cluster_name);

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating temperory directories ...\n");
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cluster_name_flag,FILENAME_LENGTH-1,"%s%scluster_name_flag.txt",HPC_NOW_ROOT_DIR,PATH_SLASH);
    snprintf(cluster_name_flag_tmp,FILENAME_LENGTH-1,"%s%scluster_name_flag.txt.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    file_p=fopen(cluster_name_flag,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create cluster name flag file. Exit now." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -5;
    }
    fprintf(file_p,"%s\ncluster_name: %s\nunique_id: %s\n",TRANSFER_HEADER,cluster_name,unique_id);
    fclose(file_p);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s%s%slog %s",MKDIR_CMD,tmp_root,PATH_SLASH,cluster_name,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s%s%sconf %s",MKDIR_CMD,tmp_root,PATH_SLASH,cluster_name,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.terraform %s",MKDIR_CMD,tmp_stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,tmp_vaultdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,tmp_sshdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exporting related files ...\n");
    if(get_nmd5sum(crypto_keyfile,md5sum_current,64)!=0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -5;
    }
    snprintf(source_file,FILENAME_LENGTH-1,"%s%sbucket_info.txt.tmp",current_vaultdir,PATH_SLASH);
    snprintf(target_file,FILENAME_LENGTH-1,"%s%sbucket_info.txt",tmp_vaultdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,source_file,target_file,md5sum_current);
    snprintf(source_file,FILENAME_LENGTH-1,"%s%sbucket_key.txt.tmp",current_vaultdir,PATH_SLASH);
    snprintf(target_file,FILENAME_LENGTH-1,"%s%sbucket_key.txt",tmp_vaultdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,source_file,target_file,md5sum_current);
    
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt.tmp",current_vaultdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_current);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",current_vaultdir,PATH_SLASH);
    if(strcmp(real_user_list,"all")==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s %s",COPY_FILE_CMD,filename_temp,tmp_vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s%s %s",COPY_FILE_CMD,current_sshdir,PATH_SLASH,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    else{
        snprintf(filename_temp_2,FILENAME_LENGTH-1,"%s%suser_passwords.txt",tmp_vaultdir,PATH_SLASH);
        file_p_tmp=fopen(filename_temp_2,"w+");
        do{
            i++;
            get_seq_nstring(real_user_list,':',i,username_temp,32);
            file_p=fopen(filename_temp,"r");
            while(fngetline(file_p,user_line_buffer,LINE_LENGTH_SHORT)==0){
                get_seq_nstring(user_line_buffer,' ',2,username_temp_2,32);
                if(strcmp(username_temp,username_temp_2)==0){
                    fprintf(file_p_tmp,"%s\n",user_line_buffer);
                    goto next_user;
                }
            }
next_user:
            fclose(file_p);
            snprintf(filename_temp_4,FILENAME_LENGTH-1,"%s%s%s.key.tmp",current_sshdir,PATH_SLASH,username_temp);
            snprintf(filename_temp_3,FILENAME_LENGTH-1,"%s%s%s.key",current_sshdir,PATH_SLASH,username_temp);
            if(decrypt_user_privkey(filename_temp_4,crypto_keyfile)!=0){
                continue;
            }
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s%s %s",COPY_FILE_CMD,filename_temp_3,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            snprintf(filename_temp_3,FILENAME_LENGTH-1,"%s%s%s.key",tmp_sshdir,PATH_SLASH,username_temp);
//            printf("%s ----\n%s .......\n",filename_temp,md5sum_trans);
//            printf("%s .......\n",md5sum_trans);
            encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp_3,md5sum_trans);
        }while(strlen(username_temp)!=0);
        fclose(file_p_tmp);
    }
    if(strcmp(real_admin_flag,"admin")==0){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt.tmp",current_vaultdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_current)==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sCLUSTER_SUMMARY.txt %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The admin file is missing. Root/Admin privilege is disabled." RESET_DISPLAY "\n");
        }
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sroot.key.tmp",current_sshdir,PATH_SLASH);
        decrypt_user_privkey(filename_temp,crypto_keyfile);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sroot.key %s%s %s",COPY_FILE_CMD,current_sshdir,PATH_SLASH,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sroot.key",tmp_sshdir,PATH_SLASH);
        encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    }
    else{
        printf(WARN_YELLO_BOLD "[ -WARN- ] Not exporting Root/Admin privilege." RESET_DISPLAY "\n");
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scloud_flag.flg %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.az_extra.info %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sUCID_LATEST.txt %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%scurrentstate %s%s %s",COPY_FILE_CMD,current_stackdir,PATH_SLASH,tmp_stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",tmp_stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create cluster state flag file. Exit now." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        delete_decrypted_files(workdir,crypto_keyfile);
        return -5;
    }
    fprintf(file_p,"This is not a genuine terraform-related file. Only for users of cluster %s. DO NOT DELETE THIS FILE.\n",cluster_name);
    fclose(file_p);

    encrypt_and_delete(NOW_CRYPTO_EXEC,cluster_name_flag,md5sum_trans);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sCLUSTER_SUMMARY.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_key.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",tmp_stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    delete_decrypted_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Generating a now-cluster file ...\n");

    if(strlen(real_export_folder)>0){
        snprintf(export_filename,1023,"%s%s%s-%s-%s.now",real_export_folder,PATH_SLASH,cluster_name,current_date,current_time);
    }
    else if(strlen(real_export_file)>0){
        strncpy(export_filename,real_export_file,1023);
    }
    else{
#ifdef _WIN32
        snprintf(export_filename,1023,"C:\\hpc-now\\%s-%s-%s.now",cluster_name,current_date,current_time);
#elif __linux__
        snprintf(export_filename,1023,"/home/hpc-now/%s-%s-%s.now",cluster_name,current_date,current_time);
#elif __APPLE__
        snprintf(export_filename,1023,"/Users/hpc-now/%s-%s-%s.now",cluster_name,current_date,current_time);
#else
        snprintf(export_filename,1023,"%s-%s-%s.now",cluster_name,current_date,current_time); 
#endif        
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"tar -zcf %s %s %s %s",export_filename,tmp_root,cluster_name_flag_tmp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,cluster_name_flag_tmp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(file_exist_or_not(export_filename)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to export the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " .\n" RESET_DISPLAY,cluster_name);
        return 1;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exported the cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY "\n\n",cluster_name);
        printf(GENERAL_BOLD "|       +- Password" RESET_DISPLAY "       : " GREY_LIGHT "%s" RESET_DISPLAY "\n",real_password);
        printf(GENERAL_BOLD "|       +- Exported File" RESET_DISPLAY "  : " HIGH_CYAN_BOLD "%s" RESET_DISPLAY "\n",export_filename);
        printf(GENERAL_BOLD "|       +- User List" RESET_DISPLAY "      : " HIGH_CYAN_BOLD "%s" RESET_DISPLAY "\n",user_list_buffer);
        if(strcmp(real_admin_flag,"admin")==0){
            printf(GENERAL_BOLD "|       +- Admin Privilege" RESET_DISPLAY ": " HIGH_CYAN_BOLD "YES" RESET_DISPLAY "\n");
        }
        else{
            printf(GENERAL_BOLD "|       +- Admin Privilege" RESET_DISPLAY ": " HIGH_CYAN_BOLD "NO" RESET_DISPLAY "\n");
        }
        return 0;
    }
}

int import_cluster(char* zip_file, char* password, char* crypto_keyfile, int batch_flag_local){
    char real_zipfile[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp_2[FILENAME_LENGTH]="";
    char cluster_name_buffer[32]="";
    char cluster_name_final[32]="";
    
    char cluster_name_temp[32]="";
    char cluster_workdir_temp[DIR_LENGTH]="";
    char unique_id_temp[16]="";
    char cluster_role_temp[16]="";
    char cluster_role_ext_temp[16]="";
    int comp_flag1,comp_flag2;
    int duplicate_flag=0;
    char rand_str_suffix[8]="";
    int cluster_name_buffer_length=0;

    char tmp_top_dir[DIR_LENGTH]="";
    char tmp_workdir[DIR_LENGTH_EXT]="";
    char username_temp[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char tmp_import_root[DIR_LENGTH]="";
    char tmp_unique_id[16]="";

    char imported_workdir[DIR_LENGTH]="";
    char imported_ssh_dir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char registry_line[LINE_LENGTH_SHORT]="";
    char* password_temp;
    char real_password[128]="";
    char md5sum_password[64]="";
    char md5sum_local[64]="";
    int update_flag=0;
    char user_line_buffer[256]="";
    int admin_flag=0;

    local_path_parser(zip_file,filename_temp);
    if(strlen(filename_temp)==0||file_empty_or_not(filename_temp)<1){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Import file not specified or invalid. Use -s SOURCE_PATH ." RESET_DISPLAY "\n");
            return 17;
        }
        printf("[ -INFO- ] Please input the path of the now-cluster file. i.e. ~/import.now, d:\\import.now\n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%511s",filename_temp);
        getchar();
        local_path_parser(filename_temp,filename_temp_2);
        if(strlen(filename_temp_2)==0||file_empty_or_not(filename_temp_2)<1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the now-cluster file " RESET_DISPLAY WARN_YELLO_BOLD "%s " RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY ,filename_temp_2);
            return -3;
        }
        else{
            strcpy(real_zipfile,filename_temp_2);
        }
    }
    else{
        strcpy(real_zipfile,filename_temp);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Using the specified now-cluster file: " HIGH_CYAN_BOLD "%s" RESET_DISPLAY "\n",real_zipfile);
    if(strlen(password)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Password not specified. Use -p PASSWORD ." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input the password to decrypt and import.\n");
        password_temp=GETPASS_FUNC("[ INPUT: ] *without echo* ");
        strcpy(real_password,password_temp);
    }
    else{
        strcpy(real_password,password);
    }
    password_hash(real_password,md5sum_password,64);
    if(get_nmd5sum(crypto_keyfile,md5sum_local,64)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(tmp_import_root,DIR_LENGTH-1,"%s%simport",HPC_NOW_ROOT_DIR,PATH_SLASH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",MKDIR_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"tar -zxf %s -C %s%s",real_zipfile,tmp_import_root,PATH_SLASH);
    if(system(cmdline)!=0||get_import_info(cluster_name_buffer,32,tmp_top_dir,DIR_LENGTH,tmp_unique_id,16,tmp_import_root,md5sum_password)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to extract and get the import information.\n" RESET_DISPLAY);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -5;
    }
    FILE* file_p=fopen(ALL_CLUSTER_REGISTRY,"r"); /* file opened */
    if(file_p==NULL){ /* file_open failed, exit immediately */
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the cluster registry, please run " RESET_DISPLAY WARN_YELLO_BOLD "hpcopr envcheck" RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -7;
    }
    while(fngetline(file_p,registry_line,LINE_LENGTH_SHORT)!=1){
        if(contain_or_nnot(registry_line,"< cluster name:")<1){
            continue;
        }
        get_seq_nstring(registry_line,' ',4,cluster_name_temp,32);
        get_nworkdir(cluster_workdir_temp,DIR_LENGTH,cluster_name_temp);
        get_nucid(cluster_workdir_temp,unique_id_temp,16);
        comp_flag1=strcmp(tmp_unique_id,unique_id_temp);
        comp_flag2=strcmp(cluster_name_buffer,cluster_name_temp);
        if(comp_flag1==0){
            cluster_role_detect(cluster_workdir_temp,cluster_role_temp,cluster_role_ext_temp,16);
            if(strcmp(cluster_role_temp,"opr")==0){
                duplicate_flag=5; /* Duplicate unique id and operating the cluster, exit.*/
                break;
            }
            if(comp_flag2==0){
                duplicate_flag=3; /* Duplicate unique id and cluster name, and the cluster is imported */
                break;
            }
            duplicate_flag=1; /* Duplicate unique id, but not the cluster name*/
            break;
        }
        if(comp_flag2==0){
            duplicate_flag=2; /* Duplicate cluster name, but uniquie id*/
        }
    }
    fclose(file_p); /* file closed */
    if(duplicate_flag==5){
        printf(FATAL_RED_BOLD "[ FATAL: ] You are operating the identical cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD ", import abort." RESET_DISPLAY "\n",cluster_name_temp);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -7;
    }
    else if(duplicate_flag==3||duplicate_flag==1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " has already been imported.\n",cluster_name_buffer);
        if(prompt_to_confirm("Continue to update it? CAUTION: he cluster name will keep unchanged.",CONFIRM_STRING,batch_flag_local)==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return -9;
        }
        strncpy(cluster_name_final,cluster_name_temp,31);
        update_flag=1;
    }
    else if(duplicate_flag==2){
        printf(GENERAL_BOLD "[ -INFO- ] Duplicate name found. Generating a unique cluster name ..." RESET_DISPLAY "\n");
        generate_random_nstring(rand_str_suffix,8,0); /* Generate a random string with actual length 7 and a '\0' */
        strncpy(cluster_name_final,cluster_name_buffer,31);
        cluster_name_buffer_length=strlen(cluster_name_buffer);
        if(cluster_name_buffer_length<18){
            memcpy(cluster_name_final+cluster_name_buffer_length,rand_str_suffix,8); /* Add 7 chars */
        }
        else{
            memcpy(cluster_name_final+17,rand_str_suffix,8); /* Otherwise replace the last 7 chars */
        }
        /* If still duplicate, then exit. */
        if(cluster_name_check(cluster_name_final)==-7){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to generate a unique cluster name, import abort." RESET_DISPLAY "\n");
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return -7;
        }
        printf(WARN_YELLO_BOLD "[ -WARN- ] Generated a unique cluster name %s." RESET_DISPLAY "\n",cluster_name_final);
    }
    /* After the process above, the cluster_name_final should be unique and ready to import */
    if(get_nworkdir(imported_workdir,DIR_LENGTH,cluster_name_final)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -7;
    }
    /* Delete the folder to be imported (if exists. Not quite possible) */
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,imported_workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(imported_ssh_dir,DIR_LENGTH,"%s%s.%s",SSHKEY_DIR,PATH_SLASH,cluster_name_final);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,imported_ssh_dir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    /* Guarantee that the target dirs don't exist */
    if(folder_exist_or_not(imported_workdir)==0||folder_exist_or_not(imported_ssh_dir)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a workdir and/or sshkey dir." RESET_DISPLAY "\n");
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -7;
    }
    /* Start moving files and folders*/
    /* Moving sshkeys to the SSHKEY_DIR */
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sexport%s.%s %s%s%s %s",MOVE_FILE_CMD,tmp_top_dir,PATH_SLASH,PATH_SLASH,cluster_name_buffer,SSHKEY_DIR,PATH_SLASH,cluster_name_final,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    /* Decrypt current files */
    snprintf(tmp_workdir,DIR_LENGTH_EXT-1,"%s%sexport%s%s",tmp_top_dir,PATH_SLASH,PATH_SLASH,cluster_name_buffer);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_info.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sbucket_key.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%suser_passwords.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%svault%sCLUSTER_SUMMARY.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sstack%sterraform.tfstate.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
    
    /* Move the working directory */
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s%sworkdir%s%s %s",MOVE_FILE_CMD,tmp_workdir,HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name_final,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(update_flag==0){
        add_to_cluster_registry(cluster_name_final,"imported");
    }
    /* Decrypt and re-encrypt the root ssh key */
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sroot.key.tmp",imported_ssh_dir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_password);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sroot.key",imported_ssh_dir,PATH_SLASH);
        encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_local);
        admin_flag=1;
    }
    create_and_get_subdir(imported_workdir,"vault",vaultdir,DIR_LENGTH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    /* Decrypt and re-encrypt the users' ssh keys */
    file_p=fopen(filename_temp,"r"); /* file opened */
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to import the specified cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ." RESET_DISPLAY "\n",cluster_name_buffer);
        return 1;
    }
    while(!feof(file_p)){
        fngetline(file_p,user_line_buffer,255);
        get_seq_nstring(user_line_buffer,' ',2,username_temp,64);
        snprintf(filename_temp_2,FILENAME_LENGTH-1,"%s%s%s.key.tmp",imported_ssh_dir,PATH_SLASH,username_temp);
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp_2,md5sum_password);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,filename_temp_2,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp_2,FILENAME_LENGTH-1,"%s%s%s.key",imported_ssh_dir,PATH_SLASH,username_temp);
        encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp_2,md5sum_local);
    }
    fclose(file_p); /* file closed */
    /* Now, encrypt and delete all the decrypted files */
    delete_decrypted_files(imported_workdir,crypto_keyfile);
    create_and_get_subdir(imported_workdir,"stack",stackdir,DIR_LENGTH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_cr_clean(filename_temp);
    /* Now, print the import summary */
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified cluster %s has been imported.\n\n",cluster_name_final);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Import Summary :\n");
    printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Cluster Name   : %s\n",cluster_name_final);
    printf(GENERAL_BOLD "|         " RESET_DISPLAY " User List      : \n");
    hpc_user_list(imported_workdir,crypto_keyfile,0,0);
    if(admin_flag==1){
        printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Admin Privilege : YES \n");
    }
    else{
        printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Admin Privilege : NO \n");
    }
    printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Node Topology   : \n");
    graph(imported_workdir,crypto_keyfile,0);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    switch_to_cluster(cluster_name_final);
    return 0;
}

int update_cluster_status(char* cluster_name, char* currentstate){
    char workdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        return -1;
    }
    create_and_get_subdir(workdir,"stack",stackdir,DIR_LENGTH);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s%scurrentstate %s",COPY_FILE_CMD,currentstate,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    if(system(cmdline)==0){
        return 0;
    }
    return 1;
}