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

int get_cluster_name_import(char* cluster_name_output, char* tmp_top_output, char* tmp_import_root, char* md5sum){
    char cluster_name_flag[FILENAME_LENGTH]="";
    char cluster_name_flag_tmp[FILENAME_LENGTH]="";
    char dir_win[DIR_LENGTH]="";
    char dir_lin[DIR_LENGTH]="";
    char dir_dwn[DIR_LENGTH]="";
    char dir_top[DIR_LENGTH_EXT]="";
    char cluster_name_buffer[512]="";
    char cluster_name_flag_header[1024]="";
    FILE* file_p=NULL;
    sprintf(dir_win,"%s%sprogramdata",tmp_import_root,PATH_SLASH);
    sprintf(dir_lin,"%s%susr",tmp_import_root,PATH_SLASH);
    sprintf(dir_dwn,"%s%sApplications",tmp_import_root,PATH_SLASH);
    if(folder_exist_or_not(dir_win)==0){
        sprintf(dir_top,"%s%shpc-now",dir_win,PATH_SLASH);
    }
    else if(folder_exist_or_not(dir_lin)==0){
        sprintf(dir_top,"%s%s.hpc-now",dir_lin,PATH_SLASH);
    }
    else if(folder_exist_or_not(dir_dwn)==0){
        sprintf(dir_top,"%s%s.hpc-now",dir_dwn,PATH_SLASH);
    }
    else{
        return -3;
    }
    sprintf(cluster_name_flag_tmp,"%s%scluster_name_flag.txt.tmp",dir_top,PATH_SLASH);
    sprintf(cluster_name_flag,"%s%scluster_name_flag.txt",dir_top,PATH_SLASH);
    if(decrypt_single_file(NOW_CRYPTO_EXEC,cluster_name_flag_tmp,md5sum)!=0){
        strcpy(cluster_name_output,"");
        strcpy(tmp_top_output,"");
        return -5;
    }
    file_p=fopen(cluster_name_flag,"r");
    if(file_p==NULL){
        strcpy(cluster_name_output,"");
        strcpy(tmp_top_output,"");
        return -5;
    }
    fgetline(file_p,cluster_name_flag_header);
    if(strcmp(cluster_name_flag_header,TRANSFER_HEADER)!=0){
        strcpy(cluster_name_output,"");
        strcpy(tmp_top_output,"");
        fclose(file_p);
        return -7;
    }
    fgetline(file_p,cluster_name_buffer);
    fclose(file_p);
    if(strlen(cluster_name_buffer)==0){
        strcpy(cluster_name_output,"");
        strcpy(tmp_top_output,"");
        return 1;
    }
    else{
        strcpy(cluster_name_output,cluster_name_buffer);
        strcpy(tmp_top_output,dir_top);
        return 0;
    }
}

int user_list_check(char* cluster_name, char* user_list_read, char* user_list_final, int* user1_flag){
    int i=1;
    int j;
    int k=0;
    int valid_user_num=0;
    char user_list_buffer[1024]="";
    char username_temp[USERNAME_LENGTH_MAX]="";
    do{
        get_seq_string(user_list_read,':',i,username_temp);
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
    char username_temp[USERNAME_LENGTH_MAX]="";
    char username_temp_2[USERNAME_LENGTH_MAX]="";
    char* password_temp;
    char real_password[128]="";
    char real_export_file[FILENAME_LENGTH_EXT]="";
    char real_export_folder[FILENAME_LENGTH_EXT]="";
    char export_filename[1024]="";
    char user_line_buffer[256]="";
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
    char current_date[12]="";
    char current_time[12]="";
    struct tm* time_p=NULL;

    time(&current_time_long);
    time_p=localtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d-%d-%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);

    get_workdir(workdir,cluster_name);
    if(strlen(user_list)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] User list specified. Use --ul USER_LIST." RESET_DISPLAY "\n");
            return 17;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input the username(s) to be exported. i.e. " HIGH_CYAN_BOLD "user1:user2." RESET_DISPLAY "\n");
            hpc_user_list(workdir,crypto_keyfile,0);
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%1024s",user_list_buffer);
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
    password_hash(real_password,md5sum_trans);
    local_path_parser(export_target_file,filename_temp);
    if(strlen(filename_temp)==0){
        if(batch_flag_local==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Export path not specified. Use -d EXPORT_DEST." RESET_DISPLAY "\n");
            return 17;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a path (folder or file) to export. i.e. " HIGH_CYAN_BOLD "/home/hpc-now/" RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
        fflush(stdin);
        scanf("%512s",filename_temp_2);
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

    create_and_get_stackdir(workdir,current_stackdir);
    create_and_get_vaultdir(workdir,current_vaultdir);
    sprintf(current_sshdir,"%s%s.%s",SSHKEY_DIR,PATH_SLASH,cluster_name);

    sprintf(tmp_root,"%s%sexport",HPC_NOW_ROOT_DIR,PATH_SLASH);
    sprintf(tmp_stackdir,"%s%s%s%sstack",tmp_root,PATH_SLASH,cluster_name,PATH_SLASH);
    sprintf(tmp_vaultdir,"%s%s%s%svault",tmp_root,PATH_SLASH,cluster_name,PATH_SLASH);
    sprintf(tmp_sshdir,"%s%s.%s",tmp_root,PATH_SLASH,cluster_name);

    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating temperory directories ...\n");
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cluster_name_flag,"%s%scluster_name_flag.txt",HPC_NOW_ROOT_DIR,PATH_SLASH);
    sprintf(cluster_name_flag_tmp,"%s%scluster_name_flag.txt.tmp",HPC_NOW_ROOT_DIR,PATH_SLASH);
    file_p=fopen(cluster_name_flag,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create cluster name flag file. Exit now." RESET_DISPLAY "\n");
        sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -5;
    }
    fprintf(file_p,"%s\n%s\n",TRANSFER_HEADER,cluster_name);
    fclose(file_p);
    sprintf(cmdline,"%s %s%s%s%slog %s",MKDIR_CMD,tmp_root,PATH_SLASH,cluster_name,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s%s%sconf %s",MKDIR_CMD,tmp_root,PATH_SLASH,cluster_name,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s.terraform %s",MKDIR_CMD,tmp_stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,tmp_vaultdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,tmp_sshdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exporting related files ...\n");
    get_crypto_key(crypto_keyfile,md5sum_current);
    sprintf(source_file,"%s%sbucket_info.txt.tmp",current_vaultdir,PATH_SLASH);
    sprintf(target_file,"%s%sbucket_info.txt",tmp_vaultdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,source_file,target_file,md5sum_current);
    sprintf(source_file,"%s%sbucket_key.txt.tmp",current_vaultdir,PATH_SLASH);
    sprintf(target_file,"%s%sbucket_key.txt",tmp_vaultdir,PATH_SLASH);
    decrypt_single_file_general(NOW_CRYPTO_EXEC,source_file,target_file,md5sum_current);
    
    sprintf(filename_temp,"%s%suser_passwords.txt.tmp",current_vaultdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_current);
    sprintf(filename_temp,"%s%suser_passwords.txt",current_vaultdir,PATH_SLASH);
    if(strcmp(real_user_list,"all")==0){
        sprintf(cmdline,"%s %s %s %s",COPY_FILE_CMD,filename_temp,tmp_vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s%s* %s%s %s",COPY_FILE_CMD,current_sshdir,PATH_SLASH,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    else{
        sprintf(filename_temp_2,"%s%suser_passwords.txt",tmp_vaultdir,PATH_SLASH);
        file_p_tmp=fopen(filename_temp_2,"w+");
        do{
            i++;
            get_seq_string(real_user_list,':',i,username_temp);
            file_p=fopen(filename_temp,"r");
            while(fgetline(file_p,user_line_buffer)==0){
                get_seq_string(user_line_buffer,' ',2,username_temp_2);
                if(strcmp(username_temp,username_temp_2)==0){
                    fprintf(file_p_tmp,"%s\n",user_line_buffer);
                    goto next_user;
                }
            }
next_user:
            fclose(file_p);
            sprintf(filename_temp_3,"%s%s%s.key",current_sshdir,PATH_SLASH,username_temp);
            sprintf(cmdline,"%s %s %s%s %s",COPY_FILE_CMD,filename_temp_3,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            sprintf(filename_temp_3,"%s%s%s.key",tmp_sshdir,PATH_SLASH,username_temp);
//            printf("%s ----\n%s .......\n",filename_temp,md5sum_trans);
//            printf("%s .......\n",md5sum_trans);
            encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp_3,md5sum_trans);
        }while(strlen(username_temp)!=0);
        fclose(file_p_tmp);
    }
    if(strcmp(real_admin_flag,"admin")==0){
        sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",current_vaultdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum_current)==0){
            sprintf(cmdline,"%s %s%sCLUSTER_SUMMARY.txt %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] The admin file is missing. Root/Admin privilege is disabled." RESET_DISPLAY "\n");
        }
        sprintf(cmdline,"%s %s%sroot.key %s%s %s",COPY_FILE_CMD,current_sshdir,PATH_SLASH,tmp_sshdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%sroot.key",tmp_sshdir,PATH_SLASH);
        encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    }
    else{
        printf(WARN_YELLO_BOLD "[ -WARN- ] Not exporting Root/Admin privilege." RESET_DISPLAY "\n");
    }
    sprintf(cmdline,"%s %s%scloud_flag.flg %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s.az_extra.info %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT_NULL);
    system(cmdline);
    sprintf(cmdline,"%s %s%sUCID_LATEST.txt %s%s %s",COPY_FILE_CMD,current_vaultdir,PATH_SLASH,tmp_vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%scurrentstate %s%s %s",COPY_FILE_CMD,current_stackdir,PATH_SLASH,tmp_stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(filename_temp,"%s%sterraform.tfstate",tmp_stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create cluster state flag file. Exit now." RESET_DISPLAY "\n");
        sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        delete_decrypted_files(workdir,crypto_keyfile);
        return -5;
    }
    fprintf(file_p,"This is not a genuine terraform-related file. Only for users of cluster %s. DO NOT DELETE THIS FILE.\n",cluster_name);
    fclose(file_p);

    encrypt_and_delete(NOW_CRYPTO_EXEC,cluster_name_flag,md5sum_trans);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    sprintf(filename_temp,"%s%suser_passwords.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    sprintf(filename_temp,"%s%sbucket_info.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    sprintf(filename_temp,"%s%sbucket_key.txt",tmp_vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    sprintf(filename_temp,"%s%sterraform.tfstate",tmp_stackdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum_trans);
    delete_decrypted_files(workdir,crypto_keyfile);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Generating a now-cluster file ...\n");

    if(strlen(real_export_folder)>0){
        sprintf(export_filename,"%s%s%s-%s-%s.now",real_export_folder,PATH_SLASH,cluster_name,current_date,current_time);
    }
    else if(strlen(real_export_file)>0){
        strcpy(export_filename,real_export_file);
    }
    else{
#ifdef _WIN32
        sprintf(export_filename,"C:\\hpc-now\\%s-%s-%s.now",cluster_name,current_date,current_time);
#elif __linux__
        sprintf(export_filename,"/home/hpc-now/%s-%s-%s.now",cluster_name,current_date,current_time);
#elif __APPLE__
        sprintf(export_filename,"/Users/hpc-now/%s-%s-%s.now",cluster_name,current_date,current_time);
#else
        sprintf(export_filename,"%s-%s-%s.now",cluster_name,current_date,current_time); 
#endif        
    }
    sprintf(cmdline,"tar -zcf %s %s %s %s",export_filename,tmp_root,cluster_name_flag_tmp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,cluster_name_flag_tmp,SYSTEM_CMD_REDIRECT);
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
    char cluster_name_buffer[128]="";
    char tmp_top_dir[DIR_LENGTH_SHORT]="";
    char tmp_workdir[DIR_LENGTH_EXT]="";
    char username_temp[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char tmp_import_root[DIR_LENGTH]="";
    char cluster_sshkey_dir[DIR_LENGTH];
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char doubleconfirm[64]="";
    char* password_temp;
    char real_password[128]="";
    char md5sum[64]="";
    int update_flag=0;
    FILE* file_p=NULL;
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
        scanf("%512s",filename_temp);
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
    password_hash(real_password,md5sum);
    sprintf(tmp_import_root,"%s%simport",HPC_NOW_ROOT_DIR,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"tar -zxf %s -C %s%s",real_zipfile,tmp_import_root,PATH_SLASH);
    if(system(cmdline)!=0||get_cluster_name_import(cluster_name_buffer,tmp_top_dir,tmp_import_root,md5sum)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified password " WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " may be incorrect. Please double check.\n" RESET_DISPLAY,real_password);
        sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return -5;
    }
    get_workdir(workdir,cluster_name_buffer);
    if(cluster_name_check(cluster_name_buffer)==-127){
        create_and_get_vaultdir(workdir,vaultdir);
        sprintf(filename_temp,"%s%s.secrets.key",vaultdir,PATH_SLASH);
        if(file_exist_or_not(filename_temp)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You are operating the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " . No need to import.\n" RESET_DISPLAY ,cluster_name_buffer);
            sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
            system(cmdline);
            return -7;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY " has already been imported to this environment.\n",cluster_name_buffer);
            printf("|          Would you like to replace it? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continue.\n");
            if(batch_flag_local==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] RISKY! Cluster operation is auto-confirmed." RESET_DISPLAY "\n");
                update_flag=1;
            }
            else{
                printf("[ INPUT: ] ");
                fflush(stdin);
                scanf("%64s",doubleconfirm);
                getchar();
                if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
                    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. You chose to deny this operation.\n");
                    printf("|          Nothing changed.\n");
                    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
                    system(cmdline);
                    return -9;
                }
                else{
                    update_flag=1;
                }
            }
        }
    }
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,workdir,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%s.%s %s",DELETE_FOLDER_CMD,SSHKEY_DIR,PATH_SLASH,cluster_name_buffer,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"%s %s%sexport%s.%s %s%s %s",MOVE_FILE_CMD,tmp_top_dir,PATH_SLASH,PATH_SLASH,cluster_name_buffer,SSHKEY_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(tmp_workdir,"%s%sexport%s%s",tmp_top_dir,PATH_SLASH,PATH_SLASH,cluster_name_buffer);
    sprintf(filename_temp,"%s%svault%sbucket_info.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%svault%sbucket_key.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%svault%suser_passwords.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%svault%sCLUSTER_SUMMARY.txt.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sstack%sterraform.tfstate.tmp",tmp_workdir,PATH_SLASH,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(cmdline,"%s %s %s%sworkdir%s %s",MOVE_FILE_CMD,tmp_workdir,HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    if(update_flag==0){
        add_to_cluster_registry(cluster_name_buffer,"imported");
    }
    
    sprintf(cluster_sshkey_dir,"%s%s.%s",SSHKEY_DIR,PATH_SLASH,cluster_name_buffer);
    sprintf(filename_temp,"%s%sroot.key.tmp",cluster_sshkey_dir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp,"%s%sroot.key",cluster_sshkey_dir,PATH_SLASH);
        activate_sshkey(filename_temp);
        admin_flag=1;
    }
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to import the specified cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ." RESET_DISPLAY "\n",cluster_name_buffer);
        return 1;
    }
    file_p=fopen(filename_temp,"r");
    while(!feof(file_p)){
        fgetline(file_p,user_line_buffer);
        get_seq_string(user_line_buffer,' ',2,username_temp);
        sprintf(filename_temp_2,"%s%s%s.key.tmp",cluster_sshkey_dir,PATH_SLASH,username_temp);
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp_2,md5sum);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp_2,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(filename_temp_2,"%s%s%s.key",cluster_sshkey_dir,PATH_SLASH,username_temp);
        activate_sshkey(filename_temp_2);
    }
    fclose(file_p);
    delete_decrypted_files(workdir,crypto_keyfile);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_cr_clean(filename_temp);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The specified cluster %s has been imported.\n\n",cluster_name_buffer);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Import Summary :\n");
    printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Cluster Name   : %s\n",cluster_name_buffer);
    printf(GENERAL_BOLD "|         " RESET_DISPLAY " User List      : \n");
    hpc_user_list(workdir,crypto_keyfile,0);
    if(admin_flag==1){
        printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Admin Privilege : YES \n");
    }
    else{
        printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Admin Privilege : NO \n");
    }
    printf(GENERAL_BOLD "|       +-" RESET_DISPLAY " Node Topology   : \n");
    graph(workdir,crypto_keyfile,0);
    sprintf(cmdline,"%s %s %s",DELETE_FOLDER_CMD,tmp_import_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    switch_to_cluster(cluster_name_buffer);
    return 0;
}

int update_cluster_status(char* cluster_name, char* currentstate){
    char workdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    get_workdir(workdir,cluster_name);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(cmdline,"%s %s %s%scurrentstate %s",COPY_FILE_CMD,currentstate,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}