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
#include "time_process.h"
#include "cluster_general_funcs.h"

int get_crypto_key(char* crypto_key_filename, char* md5sum){
    char cmdline[CMDLINE_LENGTH]="";
    FILE* md5_tmp=NULL;
#ifdef _WIN32
    char buffer[256]="";
#endif
#ifdef __APPLE__
    sprintf(cmdline,"md5 %s | awk '{print $4}' > /tmp/md5.txt.tmp",crypto_key_filename);
#elif __linux__
    sprintf(cmdline,"md5sum %s | awk '{print $1}' > /tmp/md5.txt.tmp",crypto_key_filename);
#elif _WIN32
    sprintf(cmdline,"certutil -hashfile %s md5 > md5.txt.tmp",crypto_key_filename);
#endif
    system(cmdline);
#ifdef _WIN32
    md5_tmp=fopen("md5.txt.tmp","r");
#else
    md5_tmp=fopen("/tmp/md5.txt.tmp","r");
#endif
    if(md5_tmp==NULL){
        return -1;
    }
#ifdef _WIN32
    fgetline(md5_tmp,buffer);
#endif
    fgetline(md5_tmp,md5sum);
    fclose(md5_tmp);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q md5.txt.tmp %s",SYSTEM_CMD_REDIRECT);
#else
    sprintf(cmdline,"rm -rf /tmp/md5.txt.tmp %s",SYSTEM_CMD_REDIRECT);
#endif
    system(cmdline);
    return 0;
}

void create_and_get_stackdir(char* workdir, char* stackdir){
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(stackdir,"%s%sstack",workdir,PATH_SLASH);
    if(folder_exist_or_not(stackdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,stackdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
}

void get_latest_hosts(char* stackdir, char* hostfile_latest){
    sprintf(hostfile_latest,"%s%shostfile_latest",stackdir,PATH_SLASH);
}

int get_cloud_flag(char* workdir, char* cloud_flag){
    char vaultdir[DIR_LENGTH]="";
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%scloud_flag.flg",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        strcpy(cloud_flag,"");
        return -1;
    }
    file_p=fopen(filename_temp,"r");
    fscanf(file_p,"%s",cloud_flag);
    fclose(file_p);
    return 0;
}

int decrypt_get_bucket_conf(char* workdir, char* crypto_keyfile, char* bucket_conf){
    char vaultdir[DIR_LENGTH]="";
    char md5sum[64]="";
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(bucket_conf,"%s%sbucket.conf",vaultdir,PATH_SLASH);
    sprintf(cmdline,"%s decrypt %s%sbucket.conf.tmp %s%sbucket.conf %s %s",NOW_CRYPTO_EXEC,vaultdir,PATH_SLASH,vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int remote_copy(char* workdir, char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option){
    if(strcmp(option,"put")!=0&&strcmp(option,"get")!=0){
        return 1;
    }
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    char currentstate[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p=fopen(currentstate,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    sprintf(private_key,"%s%snow-cluster-login",sshkey_dir,PATH_SLASH);
    if(strcmp(option,"put")==0){
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s %s@%s:%s %s",private_key,local_path,username,remote_address,remote_path,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s@%s:%s %s %s",private_key,username,remote_address,remote_path,local_path,SYSTEM_CMD_REDIRECT);
    }
    system(cmdline);
    return 0;
}

void create_and_get_vaultdir(char* workdir, char* vaultdir){
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(vaultdir,"%s%svault",workdir,PATH_SLASH);
    if(folder_exist_or_not(vaultdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,vaultdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
}

int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes){
    if(strcmp(exec_type,"connect")!=0&&strcmp(exec_type,"all")!=0&&strcmp(exec_type,"clear")!=0&&strcmp(exec_type,"quick")!=0){
        return -1;
    }
    if(delay_minutes<0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char remote_address[32]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    sprintf(private_key,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr %s\" | at now + %d minutes\" %s",private_key,remote_address,exec_type,delay_minutes,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int remote_exec_general(char* workdir, char* sshkey_folder, char* remote_user, char* commands, int delay_minutes){
    if(delay_minutes<0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char remote_address[32]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    sprintf(private_key,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    if(delay_minutes==0){
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" %s",private_key,remote_user,remote_address,commands,SYSTEM_CMD_REDIRECT);
    }
    else{
        sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" %s",private_key,remote_user,remote_address,commands,delay_minutes,SYSTEM_CMD_REDIRECT);
    }
    return system(cmdline);
}

int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag){
    if(file_exist_or_not(secret_file)!=0){
        return 1;
    }
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    if(file_exist_or_not(crypto_key_file)!=0){
        return 1;
    }
    char md5[33]="";
    char cmdline[CMDLINE_LENGTH]="";
    char decrypted_file_name[FILENAME_LENGTH]="";
    FILE* decrypted_file=NULL;
    if(get_crypto_key(crypto_key_file,md5)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the crypto key. Exit now.\n" RESET_DISPLAY);
        return -1;
    }
    sprintf(cmdline,"%s decrypt %s %s.dat %s %s", now_crypto_exec, secret_file, secret_file, md5,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(decrypted_file_name,"%s.dat",secret_file);
    decrypted_file=fopen(decrypted_file_name,"r");
    if(decrypted_file==NULL){
        return -1;
    }
    fscanf(decrypted_file,"%s\n%s\n%s",ak,sk,cloud_flag);
    fclose(decrypted_file);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,decrypted_file_name,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int get_cpu_num(const char* vm_model){
    int length=strlen(vm_model);
    int i,c_index=0;
    int cpu_num=0;
    if(length<5||length>9){
        return -1;
    }
    if(*(vm_model)!='a'&&*(vm_model)!='i'&&*(vm_model)!='t'){
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
        cpu_num+=(*(vm_model+i)-'0')*pow(10,c_index-i-1);
    }
    return cpu_num;
}

int check_pslock(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        return 1;
    }
    else{
        return 0;
    }
}

int get_compute_node_num(char* currentstate_file, char* option){
    if(strcmp(option,"all")!=0&&strcmp(option,"on")!=0&&strcmp(option,"down")!=0){
        return -1;
    }
    FILE* file_p=fopen(currentstate_file,"r");
    if(file_p==NULL){
        return -1;
    }
    char buffer[64]="";
    int i,node_num=0;
    int node_num_on=0;
    for(i=0;i<4;i++){
        if(fgetline(file_p,buffer)!=0){
            fclose(file_p);
            return 0;
        }
    }
    while(fgetline(file_p,buffer)==0&&strlen(buffer)!=0){
        fgetline(file_p,buffer);
        node_num++;
        if(strcmp(buffer,"running")==0||strcmp(buffer,"Running")==0||strcmp(buffer,"RUNNING")==0){
            node_num_on++;
        }
    }
    fclose(file_p);
    if(strcmp(option,"all")==0){
        return node_num;
    }
    else if(strcmp(option,"on")==0){
        return node_num_on;
    }
    else{
        return node_num-node_num_on;
    }
}

int decrypt_single_file(char* now_crypto_exec, char* filename, char* md5sum){
    char filename_new[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    int i;
    for(i=0;i<strlen(filename)-4;i++){
        *(filename_new+i)=*(filename+i);
    }
    if(file_exist_or_not(filename)==0){
        sprintf(cmdline,"%s decrypt %s %s %s %s",now_crypto_exec,filename,filename_new,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        return 0;
    }
    else{
        return -1;
    }
}

int decrypt_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    get_crypto_key(crypto_key_filename,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_base.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sterraform.tfstate.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sterraform.tfstate.backup.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_master.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_database.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_natgw.tf.tmp",stackdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        compute_node_num=get_compute_node_num(filename_temp,"all");
    }
    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf.tmp",stackdir,PATH_SLASH,i);
        decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    }
    return 0;
}

void encrypt_and_delete(char* now_crypto_exec, char* filename, char* md5sum){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(filename)==0){
        sprintf(cmdline,"%s encrypt %s %s.tmp %s %s",now_crypto_exec,filename,filename,md5sum,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
}

int delete_decrypted_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    get_crypto_key(crypto_key_filename,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_base.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sterraform.tfstate.backup",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        compute_node_num=get_compute_node_num(filename_temp,"all");
    }
    for(i=1;i<compute_node_num+1;i++){
        sprintf(filename_temp,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i);
        encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    }
    return 0;
}

int getstate(char* workdir, char* crypto_filename){
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[16]="";
    char filename_tfstate[FILENAME_LENGTH]="";
    char filename_currentstate[FILENAME_LENGTH]="";
    char filename_hostfile[FILENAME_LENGTH]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    int node_num_gs;
    int i;
    FILE* file_p_tfstate=NULL;
    FILE* file_p_currentstate=NULL;
    FILE* file_p_hostfile=NULL;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_tfstate,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    file_p_tfstate=fopen(filename_tfstate,"r");
    if(file_p_tfstate==NULL){
        return -1;
    }
    sprintf(filename_currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p_currentstate=fopen(filename_currentstate,"w+");
    if(file_p_currentstate==NULL){
        fclose(file_p_tfstate);
        return -1;
    }
    get_cloud_flag(workdir,cloud_flag);
    sprintf(filename_hostfile,"%s%shostfile_latest",stackdir,PATH_SLASH);
    file_p_hostfile=fopen(filename_hostfile,"w+");
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        node_num_gs=find_multi_keys(filename_tfstate,"\"instance_name\": \"compute","","","","");
        find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"public_ip","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"private_ip","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        reset_string(string_temp);
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_get(filename_tfstate,"\"instance_name\": \"master","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
            find_and_get(filename_tfstate,"\"instance_name\": \"database","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
        }
        else if(strcmp(cloud_flag,"CLOUD_A")==0){
            find_and_get(filename_tfstate,"\"instance_name\": \"master","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
            find_and_get(filename_tfstate,"\"instance_name\": \"database","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            reset_string(string_temp);
        }
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"instance_name\": \"compute%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",50,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            reset_string(string_temp);
            if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_get(filename_tfstate,string_temp2,"","",30,"instance_status","","",'\"',4,string_temp);
                if(i!=node_num_gs-1){
                    fprintf(file_p_currentstate,"%s\n",string_temp);
                }
                else{
                    fprintf(file_p_currentstate,"%s",string_temp);
                }
                reset_string(string_temp);
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                find_and_get(filename_tfstate,string_temp2,"","",90,"\"status\":","","",'\"',4,string_temp);
                if(i!=node_num_gs-1){
                    fprintf(file_p_currentstate,"%s\n",string_temp);
                }
                else{
                    fprintf(file_p_currentstate,"%s",string_temp);
                }
                reset_string(string_temp);
            }
            reset_string(string_temp2);
        }
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        node_num_gs=find_multi_keys(filename_tfstate,"\"name\": \"compute","","","","");
        find_and_get(filename_tfstate,"\"name\": \"master","","",90,"\"public_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"master","","",90,"\"private_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"m_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        find_and_get(filename_tfstate,"\"name\": \"db_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_currentstate,"%s\n",string_temp);
        reset_string(string_temp);
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"name\": \"compute%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",90,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_currentstate,"%s\n",string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            reset_string(string_temp);
            reset_string(string_temp2);
            sprintf(string_temp2,"\"name\": \"comp%d",i+1);
            find_and_get(filename_tfstate,string_temp2,"","",30,"\"state\":","","",'\"',4,string_temp);
            if(i!=node_num_gs-1){
                fprintf(file_p_currentstate,"%s\n",string_temp);
            }
            else{
                fprintf(file_p_currentstate,"%s",string_temp);
            }
            reset_string(string_temp);
            reset_string(string_temp2);
        }
    }
    fclose(file_p_currentstate);
    fclose(file_p_hostfile);
    fclose(file_p_tfstate);
    return 0;
}

int generate_sshkey(char* sshkey_folder, char* pubkey){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
    FILE* file_p=NULL;

    if(folder_exist_or_not(sshkey_folder)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,sshkey_folder,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(cmdline,"attrib +h +s +r %s",sshkey_folder);
    system(cmdline);
#endif
    sprintf(filename_temp,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    sprintf(filename_temp2,"%s%snow-cluster-login.pub",sshkey_folder,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0&&file_exist_or_not(filename_temp2)==0){
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
    else{
        sprintf(cmdline,"%s %s%snow-cluster-login* %s",DELETE_FILE_CMD,sshkey_folder,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline); 
        sprintf(cmdline,"ssh-keygen -t rsa -N \"\" -f %s%snow-cluster-login -q",sshkey_folder,PATH_SLASH);
        system(cmdline);
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
}

int update_cluster_summary(char* workdir, char* crypto_keyfile){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char md5sum[33]="";
    char master_address[32]="";
    char master_address_prev[16]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    FILE* file_p=NULL;
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    find_and_get(filename_temp,"Master","Node","IP:",1,"Master","Node","IP:",' ',4,master_address_prev);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    if(strcmp(master_address,master_address_prev)!=0){
        sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
        global_replace(filename_temp,master_address_prev,master_address);
        encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    }
    else{
        sprintf(cmdline,"%s %s%sCLUSTER_SUMMARY.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    system(cmdline);
    return 0;
}

/* Should write a real C function, instead of calling system commands. But it is totally OK.*/
int archive_log(char* logarchive, char* logfile){
    char line_buffer[LINE_LENGTH]="";
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
    while(fgetline(file_p_2,line_buffer)==0){
        fprintf(file_p,"%s\n",line_buffer);
    }
    fclose(file_p_2);
    fclose(file_p);
    return 0;
}

void single_file_to_running(char* filename_temp, char* cloud_flag){
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

void update_compute_template(char* stackdir, char* cloud_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%scompute_template",stackdir,PATH_SLASH);
    sprintf(cmdline,"%s %s%shpc_stack_compute1.tf %s %s",COPY_FILE_CMD,stackdir,PATH_SLASH,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    single_file_to_running(filename_temp,cloud_flag);
}

int wait_for_complete(char* workdir, char* option, char* errorlog, int silent_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    int i=0;
    int total_minutes=0;
    char* annimation="\\|/-";

    create_and_get_stackdir(workdir,stackdir);
    sprintf(logdir,"%s%slog%s",workdir,PATH_SLASH,PATH_SLASH);
    if(strcmp(option,"init")==0){
        sprintf(cmdline,"%s %s%stf_prep.log | %s successfully | %s initialized! %s",CAT_FILE_CMD,logdir,PATH_SLASH,GREP_CMD,GREP_CMD,SYSTEM_CMD_REDIRECT_NULL);
        total_minutes=1;
    }
    else{
        sprintf(cmdline,"%s %s%stf_prep.log | %s complete! %s",CAT_FILE_CMD,logdir,PATH_SLASH,GREP_CMD,SYSTEM_CMD_REDIRECT_NULL);
        total_minutes=3;
    }
    while(system(cmdline)!=0&&i<MAXIMUM_WAIT_TIME){
        if(silent_flag!=0){
            fflush(stdin);
            printf("[ -WAIT- ] This may need %d min(s). %d sec(s) passed ... (%c)\r",total_minutes,i,*(annimation+i%4));
            fflush(stdout);
        }
        i++;
        sleep(1);
        if(file_empty_or_not(errorlog)>0){
            if(silent_flag!=0){
                printf("\n");
            }
            return 127;
        }
    }
    if(i==MAXIMUM_WAIT_TIME){
        if(silent_flag!=0){
            printf("\n");
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
/*    if(getstate(workdir,crypto_keyfile)!=0){
        return -1;
    }*/
    char master_address[32]="";
    char master_status[16]="";
    char master_config[16]="";
    char db_status[16]="";
    char cloud_flag[16]="";
    char compute_address[32]="";
    char compute_status[16]="";
    char compute_config[16]="";
    char line_buffer[32]="";
    char currentstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char master_tf[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char ht_status[16]="";
    int node_num=0;
    int running_node_num=0;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(currentstate,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(compute_template,"%s%scompute_template",stackdir,PATH_SLASH);
    if(file_exist_or_not(compute_template)!=0||file_exist_or_not(currentstate)!=0||get_cloud_flag(workdir,cloud_flag)==-1){
        return 1;
    }
    FILE* file_p=fopen(currentstate,"r");
    fgetline(file_p,master_address);
    fgetline(file_p,line_buffer);
    fgetline(file_p,master_status);
    fgetline(file_p,db_status);
    find_and_get(compute_template,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    if(find_multi_keys(compute_template,"cpu_threads_per_core = 1","","","","")!=0){
        strcpy(ht_status,"*HT-OFF*");
    }
    sprintf(master_tf,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    find_and_get(master_tf,"instance_type","","",1,"instance_type","","",'.',3,master_config);
    if(graph_level==0){
        printf(HIGH_GREEN_BOLD "|          +-master(%s,%s,%s)\n",master_address,master_status,master_config);
        printf("|            +-db(%s)\n" RESET_DISPLAY,db_status);
    }
    while(fgetline(file_p,compute_address)==0){
        fgetline(file_p,compute_status);
        node_num++;
        if(strcmp(compute_status,"running")==0||strcmp(compute_status,"Running")==0||strcmp(compute_status,"RUNNING")==0){
            running_node_num++;
        }
        if(graph_level==0){
            if(strlen(ht_status)!=0){
                printf(HIGH_GREEN_BOLD "|              +-compute%d(%s,%s,%s,%s)\n" RESET_DISPLAY,node_num,compute_address,compute_status,compute_config,ht_status);
            }
            else{
                printf(HIGH_GREEN_BOLD "|              +-compute%d(%s,%s,%s)\n" RESET_DISPLAY,node_num,compute_address,compute_status,compute_config);
            }
        }
    }
    fclose(file_p);
    if(graph_level==1){
        if(strlen(ht_status)!=0){
            printf("%s | %s %s %s | %d/%d | %s | %s\n",cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status);
        }
        else{
            printf("%s | %s %s %s | %d/%d | %s \n",cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config);
        }
    }
    else if(graph_level==2){
        if(strlen(ht_status)!=0){
            printf("%s,%s,%s,%s,%d,%d,%s,%s\n",cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status);
        }
        else{
            printf("%s,%s,%s,%s,%d,%d,%s\n",cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config);
        }
    }
    return 0;
}

int cluster_empty_or_not(char* workdir){
    char statefile[FILENAME_LENGTH]="";
    char templatefile[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(templatefile,"%s%scompute_template",stackdir,PATH_SLASH);
    if(file_exist_or_not(statefile)!=0&&file_exist_or_not(templatefile)!=0){
        return 0;
    }
    else if(file_empty_or_not(statefile)==0&&file_empty_or_not(templatefile)==0){
        return 0;
    }
    else{
        return 1;
    }
}

int cluster_asleep_or_not(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char master_state[32]="";
    char buffer[32]="";
    int i;
    create_and_get_stackdir(workdir,stackdir);
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    for(i=0;i<2;i++){
        fgetline(file_p,buffer);
    }
    fgetline(file_p,master_state);
    fclose(file_p);
    if(strcmp(master_state,"running")!=0&&strcmp(master_state,"Running")!=0&&strcmp(master_state,"RUNNING")!=0){
        return 0;
    }
    else{
        return 1;
    }
}

int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, char* error_log, int silent_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char tf_realtime_log[FILENAME_LENGTH];
    char tf_realtime_log_archive[FILENAME_LENGTH];
    char tf_error_log_archive[FILENAME_LENGTH];
    int run_flag=0;
    int wait_flag=0;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(tf_realtime_log,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_realtime_log_archive,"%s%slog%stf_prep.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_error_log_archive,"%s.archive",error_log);
    archive_log(tf_realtime_log_archive,tf_realtime_log);
    archive_log(tf_error_log_archive,error_log);
    sprintf(cmdline,"cd %s%s && %s TF_LOG=DEBUG&&%s TF_LOG_PATH=%s%slog%sterraform.log && echo yes | %s %s %s > %s 2>%s &",stackdir,PATH_SLASH,SET_ENV_CMD,SET_ENV_CMD,workdir,PATH_SLASH,PATH_SLASH,START_BG_JOB,tf_exec,execution_name,tf_realtime_log,error_log);
    run_flag=system(cmdline);
    if(silent_flag!=0){
        printf(WARN_YELLO_BOLD "[ -INFO- ] Do not terminate this process manually. Max Exec Time: %d s\n",MAXIMUM_WAIT_TIME);
        printf("|          Command: %s. Error log: %s\n" RESET_DISPLAY,execution_name,error_log);
    }
    wait_flag=wait_for_complete(workdir,execution_name,error_log,1);
    if(run_flag!=0||wait_flag!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to operate the cluster. Operation command: %s.\n" RESET_DISPLAY,execution_name);
        archive_log(tf_error_log_archive,error_log);
        return -1;
    }
    return 0;
}

int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option){
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char* usage_file=USAGE_LOG_FILE;
    char randstr[32]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cluster_id[32]="";
    char cloud_region[16]="";
    char buffer1[128]="";
    char buffer2[128]="";
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
    create_and_get_vaultdir(workdir,vaultdir);
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,randstr);
    fclose(file_p);
    sprintf(filename_temp,"%s%sconf%stf_prep.conf",workdir,PATH_SLASH,PATH_SLASH);
    find_and_get(filename_temp,"CLUSTER_ID","","",1,"CLUSTER_ID","","",' ',3,cluster_id);
    sprintf(unique_cluster_id,"%s-%s",cluster_id,randstr);
    find_and_get(filename_temp,"master_inst","","",1,"master_inst","","",' ',3,master_config);
    find_and_get(filename_temp,"REGION_ID","","",1,"REGION_ID","","",' ',3,cloud_region);
    sprintf(filename_temp,"%s%scompute_template",stackdir,PATH_SLASH);
    find_and_get(filename_temp,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    sprintf(filename_temp,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_vendor);
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    sprintf(current_date,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    sprintf(current_time,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
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
        find_and_get(usage_file,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',5,prev_date);
        find_and_get(usage_file,unique_cluster_id,node_name,"NULL",1,unique_cluster_id,node_name,"NULL",',',6,prev_time);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","RUNNING_DATE",current_date);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","RUNNING_TIME",current_time);
        running_hours=calc_running_hours(prev_date,prev_time,current_date,current_time);
        sprintf(running_hours_string,"%.4lf",running_hours);
        find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL1",running_hours_string);
        if(contain_or_not(node_name,"compute")==0){
            vcpu=get_cpu_num(compute_config);
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"master")==0){
            vcpu=get_cpu_num(master_config);
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        if(strcmp(node_name,"database")==0||strcmp(node_name,"natgw")==0){
            vcpu=2;
            cpu_hours=vcpu*running_hours;
            sprintf(cpu_hours_string,"%.4lf",cpu_hours);
            find_and_replace(usage_file,unique_cluster_id,node_name,"NULL","","","NULL2",cpu_hours_string);
            return 0;
        }
        return -1;
    }
    return -1;
}

int get_vault_info(char* workdir, char* crypto_keyfile, char* root_flag){
    char md5sum[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    char* crypto_exec=NOW_CRYPTO_EXEC;
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    char username[32]="";
    char password[32]="";
    char enable_flag[16]="";
    char header_string[64]="";
    char tail_string[64]="";
    char bucket_header[16]="";
    char bucket_name[32]="";
    int i=0;
    if(cluster_empty_or_not(workdir)==0){
        return -1;
    }
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    decrypt_single_file(crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    decrypt_single_file(crypto_exec,filename_temp,md5sum);

    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return -1;
    }
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(filename_temp,"r");
    if(file_p_2==NULL){
        fclose(file_p);
        return -1;
    }

    printf(WARN_YELLO_BOLD "\n+------------ HPC-NOW CLUSTER SENSITIVE INFORMATION: ------------+\n" RESET_DISPLAY);
    while(fgetline(file_p,single_line)==0&&i<8){
        if(strlen(single_line)!=0){
            if(contain_or_not(single_line,"Address")==0){
                get_seq_string(single_line,' ',3,bucket_header);
                get_seq_string(single_line,' ',4,bucket_name);
                printf(GENERAL_BOLD "| NetDisk Address: " RESET_DISPLAY "%s%s\n",bucket_header,bucket_name);
                
            }
            else{
                get_seq_string(single_line,':',1,header_string);
                get_seq_string(single_line,':',2,tail_string);
                if(contain_or_not(single_line,"Password")==0){
                    if(strcmp(root_flag,"root")==0){
                        printf(GENERAL_BOLD "| %s:" RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,header_string,tail_string);
                    }
                }
                else if(contain_or_not(single_line,"Key")==0){
                    printf(GENERAL_BOLD "| %s:" RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,header_string,tail_string);
                }
                else{
                    printf(GENERAL_BOLD "| %s:" RESET_DISPLAY "%s\n",header_string,tail_string);
                }
            }
        }
        i++;
    }
    fclose(file_p);
    printf(WARN_YELLO_BOLD "+---------------- CLUSTER USERS AND *PASSWORDS* -----------------+\n" RESET_DISPLAY);
    while(fgetline(file_p_2,single_line)==0){
        if(strlen(single_line)!=0){
            get_seq_string(single_line,' ',2,username);
            get_seq_string(single_line,' ',3,password);
            get_seq_string(single_line,' ',4,enable_flag);
            if(strcmp(enable_flag,"DISABLED")==0){
                printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY WARN_YELLO_BOLD "%s\n" RESET_DISPLAY,username,password,enable_flag);
            }
            else{
                printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY GENERAL_BOLD "%s\n" RESET_DISPLAY,username,password,enable_flag);
            }
        }
    }
    printf(WARN_YELLO_BOLD "+---------- DO NOT DISCLOSE THE INFORMATION TO OTHERS -----------+\n" RESET_DISPLAY);
    fclose(file_p_2);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int confirm_to_operate_cluster(char* current_cluster_name){
    char doubleconfirm[64]="";
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You are operating the cluster %s now, which may affect\n",current_cluster_name);
    printf("|          the resources|data|jobs. Please input 'y-e-s' to continue.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",doubleconfirm);
    getchar();
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only 'y-e-s' is accepted to continue. You chose to deny this operation.\n");
        printf("|          Nothing changed. Exit now.\n");
        return 1;
    }
    return 0;
}

int check_down_nodes(char* workdir){
    char statefile[FILENAME_LENGTH];
    char stackdir[DIR_LENGTH];
    create_and_get_stackdir(workdir,stackdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    return get_compute_node_num(statefile,"down");
}

int cluster_ssh(char* workdir, char* username){
    char stackdir[DIR_LENGTH]="";
    char statefile[FILENAME_LENGTH];
    char master_address[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char private_sshkey[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(statefile)!=0){
        return -1;
    }
    file_p=fopen(statefile,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
    sprintf(private_sshkey,"%s%snow-cluster-login",SSHKEY_DIR,PATH_SLASH);
    if(strlen(username)==0){
        sprintf(cmdline,"ssh -i %s root@%s",private_sshkey,master_address);
    }
    else{
        sprintf(cmdline,"ssh -i %s %s@%s",private_sshkey,username,master_address);
    }
    return system(cmdline);
}

int node_file_to_running(char* stackdir, char* node_name, char* cloud_flag){
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%shpc_stack_%s.tf",stackdir,PATH_SLASH,node_name);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Stopped","Running");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","false","true");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"stopped","running");
    }
    return 0;
}

int node_file_to_stop(char* stackdir, char* node_name, char* cloud_flag){
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%shpc_stack_%s.tf",stackdir,PATH_SLASH,node_name);
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        global_replace(filename_temp,"Running","Stopped");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        find_and_replace(filename_temp,"running_flag","","","","","true","false");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        global_replace(filename_temp,"running","stopped");
    }
    return 0;
}

int get_cluster_bucket_id(char* workdir, char* crypto_keyfile, char* bucket_id){
    char vaultdir[DIR_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[64]="";
    create_and_get_vaultdir(workdir,vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return -1;
    }
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    find_and_get(filename_temp,"NetDisk Address:","","",1,"NetDisk Address:","","",' ',4,bucket_id);
    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    return 0;
}

int tail_f_for_windows(char* filename){
    FILE* file_p=fopen(filename,"r");
    char ch='\0';
    time_t start_time;
    time_t current_time;
    time(&start_time);
    if(file_p==NULL){
        return -1;
    }
    fseek(file_p,-1,SEEK_END);
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

int get_ucid(char* workdir, char* ucid_string){
    char vaultdir[DIR_LENGTH]="";
    char filename_ucid[FILENAME_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_ucid,"%s%sUCID_LATEST.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_ucid,"r");
    if(file_p==NULL){
        return -1;
    }
    fgetline(file_p,ucid_string);
    fclose(file_p);
    return 0;
}

int decrypt_user_passwords(char* workdir, char* crypto_keyfile){
    char vaultdir[DIR_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    char filename_temp[FILENAME_LENGTH]="";
    char* crypto_exec=NOW_CRYPTO_EXEC;
    char md5sum[64]="";
    get_crypto_key(crypto_keyfile,md5sum); 
    sprintf(filename_temp,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return -1;
    }
    decrypt_single_file(crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return 1;
    }
    return 0;
}

void delete_decrypted_user_passwords(char* workdir){
    char vaultdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
}

void encrypt_and_delete_user_passwords(char* workdir, char* crypto_keyfile){
    char vaultdir[DIR_LENGTH]="";
    char md5sum[64]="";
    char filename_temp[FILENAME_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum);
}

int sync_user_passwords(char* workdir, char* sshkey_dir){
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    return remote_copy(workdir,sshkey_dir,filename_temp,"/root/.cluster_secrets/user_secrets.txt","root","put");
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
    char username[USERNAME_LENGTH_MAX]="";
    char enable_flag[16]="";
    FILE* file_p=NULL;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p=fopen(filename_temp,"r");
    while(fgetline(file_p,single_line)==0){
        get_seq_string(single_line,' ',2,username);
        get_seq_string(single_line,' ',4,enable_flag);
        if(strcmp(enable_flag,"ENABLED")==0){
            printf(HIGH_GREEN_BOLD "|          +- username: %s %s\n" RESET_DISPLAY,username,enable_flag);
        }
        else{
            printf(WARN_YELLO_BOLD "|          +- username: %s %s\n" RESET_DISPLAY,username,enable_flag);
        }
    }
    fclose(file_p);
    if(decrypt_flag==0){
        delete_decrypted_user_passwords(workdir);
    }
    return 0;
}

int username_check(char* user_registry, char* username_input){
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
    sprintf(username_ext," %s ",username_input);
    if(find_multi_keys(user_registry,username_ext,"","","","")!=0){
        return 7;
    }
    return 0;
}

int hpc_user_add(char* workdir, char* sshkey_dir, char* crypto_keyfile, char* username, char* password){
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char username_input[64]="";
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    int username_check_flag=0;
    char* password_temp=NULL;
    char password_prompt[128]="";
    char password_input[USER_PASSWORD_LENGTH_MAX]="";
    char password_confirm[USER_PASSWORD_LENGTH_MAX]="";
    char password_final[USER_PASSWORD_LENGTH_MAX]="";
    char remote_commands[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(user_registry_file,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(user_registry_file)!=0){
        return -1;
    }
    if(strlen(username)==0){
        printf(GENERAL_BOLD"[ INPUT: ]" RESET_DISPLAY " Please input a *UNIQUE* username (A-Z | a-z | 0-9 | - , Length %d-%d)\n",USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
        printf(GENERAL_BOLD"[ INPUT: ]" RESET_DISPLAY " Do *NOT* begin with '-' : ");
        fflush(stdin);
        scanf("%s",username_input);
        getchar();
    }
    else{
        strcpy(username_input,username);
    }
    username_check_flag=username_check(user_registry_file,username_input);
    if(username_check_flag!=0){
        if(username_check_flag==-3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The length is out of range (%d - %d).\n" RESET_DISPLAY,USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
            delete_decrypted_user_passwords(workdir);
            return -3;
        }
        else if(username_check_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] Do *NOT* begin with '-' .\n" RESET_DISPLAY);
            delete_decrypted_user_passwords(workdir);
            return -3;
        }
        else if(username_check_flag==5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Illegal character(s) found, only A-Z | a-z | - are valid.\n" RESET_DISPLAY);
            delete_decrypted_user_passwords(workdir);
            return -3;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Username duplicated. Current users:\n" RESET_DISPLAY);
            hpc_user_list(workdir,crypto_keyfile,1);
            delete_decrypted_user_passwords(workdir);
            return -3;
        }
    }
    printf(HIGH_GREEN_BOLD "|          Username: %s" RESET_DISPLAY "\n",username_input);
    sprintf(password_prompt,"[ INPUT: ] Password (MaxLength %d): ",USER_PASSWORD_LENGTH_MAX);
    if(strlen(password)==0){
        password_temp=GETPASS_FUNC(password_prompt);
        if(strlen(password_temp)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] The password is too long.\n" RESET_DISPLAY);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_input,password_temp);
        reset_string(password_temp);
        password_temp=GETPASS_FUNC("|          Re-type the Password   : ");                            
        if(strlen(password_temp)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password.\n" RESET_DISPLAY);
            printf(FATAL_RED_BOLD "|" RESET_DISPLAY GREY_LIGHT "          %s" RESET_DISPLAY WARN_YELLO_BOLD " !=" RESET_DISPLAY GREY_LIGHT " %s \n" RESET_DISPLAY,password_input,password_temp);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_confirm,password_temp);
        reset_string(password_temp);
        if(strcmp(password_input,password_confirm)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password.\n" RESET_DISPLAY);
            printf(FATAL_RED_BOLD "|" RESET_DISPLAY GREY_LIGHT "          %s" RESET_DISPLAY WARN_YELLO_BOLD " !=" RESET_DISPLAY GREY_LIGHT " %s \n" RESET_DISPLAY,password_input,password_confirm);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_final,password_input);
    }
    else{
        if(strlen(password)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] The password is too long. Max Length: %d\n" RESET_DISPLAY,USER_PASSWORD_LENGTH_MAX);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_final,password);
    }
    sprintf(remote_commands,"hpcmgr users add %s %s >> /var/log/hpcmgr.log 2>&1",username_input,password_final);
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0)==0){
        sprintf(remote_commands,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s | grep ENABLED >> /dev/null 2>&1",username_input);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0)==0){
            printf("[ -INFO- ] Updating the local user-info registry ...\n");
            file_p=fopen(user_registry_file,"a");
            fprintf(file_p,"username: %s %s ENABLED\n",username_input,password_final);
            fclose(file_p);
            printf("[ -DONE- ] The user %s has been added to your cluster successfully.\n",username_input);
            encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to add the user %s to your cluster. Exit now.\n",username_input);
            delete_decrypted_user_passwords(workdir);
            return 1;
        }
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status.\n");
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int delete_user_from_registry(char* user_registry_file, char* username){
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char single_line[LINE_LENGTH_SHORT]="";
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char username_temp[32]="";
    sprintf(filename_temp,"%s.tmp.2",user_registry_file);
    file_p_2=fopen(filename_temp,"w+");
    if(file_p_2==NULL){
        return -3;
    }
    file_p=fopen(user_registry_file,"r");
    if(file_p==NULL){
        fclose(file_p_2);
        return -1;
    }
    while(fgetline(file_p,single_line)==0){
        get_seq_string(single_line,' ',2,username_temp);
        if(strcmp(username_temp,username)==0){
            continue;
        }
        fprintf(file_p_2,"%s\n",single_line);
    }
    fclose(file_p);
    fclose(file_p_2);
    sprintf(cmdline,"%s %s %s %s",MOVE_FILE_CMD,filename_temp,user_registry_file,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    return 0;
}

int hpc_user_delete(char* workdir, char* crypto_keyfile, char* sshkey_dir, char* username){
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    char username_input[64]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(user_registry_file,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(strlen(username)==0){
        hpc_user_list(workdir,crypto_keyfile,1);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify the username: ");
        fflush(stdin);
        scanf("%s",username_input);
        getchar();
    }
    else{
        strcpy(username_input,username);
    }
    if(strcmp(username_input,"root")==0||strcmp(username_input,"user1")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user and user1 are protected and cannot be deleted.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    if(username_check(user_registry_file,username_input)!=7){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified user is not in the cluster.\n" RESET_DISPLAY);
        hpc_user_list(workdir,crypto_keyfile,1);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    sprintf(remote_commands,"echo y-e-s | hpcmgr users delete %s os",username_input);
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0)==0){
        sprintf(remote_commands,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s >> /dev/null 2>&1",username_input);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the user %s from your cluster. Exit now.\n" RESET_DISPLAY,username_input);
            delete_decrypted_user_passwords(workdir);
            return 1;
        }
        else{
            delete_user_from_registry(user_registry_file,username_input);
            encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully deleted user %s.\n",username_input);
            return 0;
        }
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int hpc_user_enable_disable(char* workdir, char* sshkey_dir, char* username, char* crypto_keyfile, char* option){
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char prev_keywords[16]="";
    char new_keywords[16]="";
    if(strcmp(option,"enable")==0){
        strcpy(prev_keywords,"DISABLED");
        strcpy(new_keywords,"ENABLED");
    }
    else if(strcmp(option,"disable")==0){
        strcpy(prev_keywords,"ENABLED");
        strcpy(new_keywords,"DISABLED");
    }
    else{
        return -127;
    }
    char username_input[64]="";
    char username_ext[128]="";
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char remote_commands[CMDLINE_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(user_registry_file,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(strlen(username)==0){
        hpc_user_list(workdir,crypto_keyfile,1);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify the username: ");
        fflush(stdin);
        scanf("%s",username_input);
        getchar();
    }
    else{
        strcpy(username_input,username);
    }
    if(strcmp(username_input,"root")==0||strcmp(username_input,"user1")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The root user and user1 are protected, cannot be enabled/disabled.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    if(username_check(user_registry_file,username_input)!=7){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid username. Valid cluster users:\n" RESET_DISPLAY);
        hpc_user_list(workdir,crypto_keyfile,1);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    sprintf(username_ext," %s ",username_input);
    if(find_multi_keys(user_registry_file,username_ext,new_keywords,"","","")>0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The user %s is already %s. Exit now.\n" RESET_DISPLAY,username_input,new_keywords);
        delete_decrypted_user_passwords(workdir);
        return -5;
    }
    if(strcmp(option,"enable")==0){
        sprintf(remote_commands,"hpcmgr users add %s >> /var/log/hpcmgr.log 2>&1",username_input);
    }
    else{
        sprintf(remote_commands,"hpcmgr users delete %s >> /var/log/hpcmgr.log 2>&1",username_input);
    }
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0)==0){
        find_and_replace(user_registry_file,username_ext,"","","","",prev_keywords,new_keywords);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully %s the user %s.\n",new_keywords,username_input);
        encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
        return 0;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int hpc_user_setpasswd(char* workdir, char* ssheky_dir, char* crypto_keyfile, char* username, char* password){
    if(decrypt_user_passwords(workdir,crypto_keyfile)!=0){
        return -1;
    }
    char vaultdir[DIR_LENGTH]="";
    char user_registry_file[FILENAME_LENGTH]="";
    char username_input[64]="";
    char username_ext[128]="";
    char* password_temp=NULL;
    char password_prompt[128]="";
    char password_prev[64]="";
    char password_input[USER_PASSWORD_LENGTH_MAX]="";
    char password_confirm[USER_PASSWORD_LENGTH_MAX]="";
    char password_final[USER_PASSWORD_LENGTH_MAX]="";
    char remote_commands[CMDLINE_LENGTH]="";

    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(user_registry_file,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(strlen(username)==0){
        hpc_user_list(workdir,crypto_keyfile,1);
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please specify the username: ");
        fflush(stdin);
        scanf("%s",username_input);
        getchar();
    }
    else{
        strcpy(username_input,username);
    }
    if(strcmp(username_input,"root")==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Modifying root user password is not allowed.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    if(username_check(user_registry_file,username_input)!=7){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid username. Valid cluster users:\n" RESET_DISPLAY);
        hpc_user_list(workdir,crypto_keyfile,1);
        delete_decrypted_user_passwords(workdir);
        return -3;
    }
    printf(HIGH_GREEN_BOLD "|          Username: %s" RESET_DISPLAY " \n",username_input);
    sprintf(password_prompt,"[ INPUT: ] Password (MaxLength %d): ",USER_PASSWORD_LENGTH_MAX);
    if(strlen(password)==0){
        password_temp=GETPASS_FUNC(password_prompt);
        if(strlen(password_temp)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] The password is too long.\n" RESET_DISPLAY);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_input,password_temp);
        reset_string(password_temp);
        password_temp=GETPASS_FUNC("|          Re-type the Password   : ");                            
        if(strlen(password_temp)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password.\n" RESET_DISPLAY);
            printf(FATAL_RED_BOLD "|" RESET_DISPLAY GREY_LIGHT "          %s" RESET_DISPLAY WARN_YELLO_BOLD " !=" RESET_DISPLAY GREY_LIGHT " %s \n" RESET_DISPLAY,password_input,password_temp);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_confirm,password_temp);
        reset_string(password_temp);
        if(strcmp(password_input,password_confirm)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to confirm the password.\n" RESET_DISPLAY);
            printf(FATAL_RED_BOLD "|" RESET_DISPLAY GREY_LIGHT "          %s" RESET_DISPLAY WARN_YELLO_BOLD " !=" RESET_DISPLAY GREY_LIGHT " %s \n" RESET_DISPLAY,password_input,password_confirm);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_final,password_input);
    }
    else{
            if(strlen(password)>USER_PASSWORD_LENGTH_MAX){
            printf(FATAL_RED_BOLD "[ FATAL: ] The password is too long. Max Length: %d\n" RESET_DISPLAY,USER_PASSWORD_LENGTH_MAX);
            delete_decrypted_user_passwords(workdir);
            return -5;
        }
        strcpy(password_final,password);
    }
    sprintf(remote_commands,"echo \"%s\" | passwd %s --stdin >> /dev/null 2>&1",password_final,username_input);
    if(remote_exec_general(workdir,ssheky_dir,"root",remote_commands,0)==0){
        sprintf(username_ext," %s ",username_input);
        find_and_get(user_registry_file,username_ext,"","",1,username_ext,"","",' ',3,password_prev);
        find_and_replace(user_registry_file,username_ext,"","","","",password_prev,password_final);
        sync_user_passwords(workdir,ssheky_dir);
        encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully updated the password for user %s.\n",username_input);
        return 0;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to connect to the cluster. Please check the cluster status.\n" RESET_DISPLAY);
        delete_decrypted_user_passwords(workdir);
        return 3;
    }
}

int usrmgr_prereq_check(char* workdir, char* option){
    char confirm[64]="";
    if(check_down_nodes(workdir)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] There are down nodes. The user management efforts cannot take effect immediately.\n" RESET_DISPLAY);
        if(strcmp(option,"list")==0){
            return 1;
        }
        printf(WARN_YELLO_BOLD "|          After any user management operations, you *MUST* run the commands below:\n");
        printf("|            1. hpcopr wakeup all\n");
        printf("|            2. hpcopr ssh user1\n");
        printf("|            3. sudo hpcmgr connect\n");
        printf("|            4. sudo hpcmgr all\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|          You can also exit now, run" RESET_DISPLAY HIGH_GREEN_BOLD " 'hpcopr wakeup all'" RESET_DISPLAY GENERAL_BOLD " and then manage the users.\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Would you like to continue? Only 'y-e-s' is accepted.\n");
        printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
        fflush(stdin);
        scanf("%s",confirm);
        getchar();
        if(strcmp(confirm,"y-e-s")!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You chose to deny the operation. Exit now.\n");
            return 3;
        }
        else{
            return 5;
        }
    }
    return 0;
}

void usrmgr_remote_exec(char* workdir, char* sshkey_folder, int prereq_check_flag){
    if(prereq_check_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote executing now ...\n");
        remote_exec(workdir,sshkey_folder,"connect",1);
        remote_exec(workdir,sshkey_folder,"all",2);
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    }
    else{
        printf(WARN_YELLO_BOLD "[ -WARN- ] You *MUST* run the commands above to update the cluster users.\n" RESET_DISPLAY);
    }
}

/*int create_protection(char* workdir, int minutes){
    char protection_file[FILENAME_LENGTH]="";
    char deprotect_bat[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo=localtime(&rawtime);
#ifdef _WIN32
    sprintf(protection_file,"%s\\%s",workdir,PROTECTION_FILE_NAME);
    sprintf(deprotect_bat,"%s\\deprotect.bat",workdir);
#else
    sprintf(protection_file,"%s/%s",workdir,protection_file);
#endif
    file_p=fopen(protection_file,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"ARRANGED AT: %d/%d/%d-%d:%d:%d\n",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    fprintf(file_p,"WAIT_PERIOD: %d minutes.\n",minutes);
    fclose(file_p);
#ifdef _WIN32
    file_p=fopen(deprotect_bat,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"del /f /q %s %s",protection_file,SYSTEM_CMD_REDIRECT);
    fclose(file_p);
    sprintf(cmdline,"schtasks /create /tn deprotect /tr %s /sc minute 1",deprotect_bat);
#else

#endif
    return 0;
}
int check_protection(char* workdir){
    return 0;
}
int delete_protection(char* workdir){
    return 0;
}*/