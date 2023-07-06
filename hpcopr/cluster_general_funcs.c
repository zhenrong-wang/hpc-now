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
#include "general_print_info.h"

int cluster_role_detect(char* workdir, char* cluster_role){
    char vaultdir[DIR_LENGTH]="";
    char cloud_secret_file[FILENAME_LENGTH]="";
    char cluster_summary[FILENAME_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(cloud_secret_file,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    sprintf(cluster_summary,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    sprintf(user_passwords,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    if(file_empty_or_not(cloud_secret_file)>1){
        strcpy(cluster_role,"opr");
        return 0;
    }
    if(file_empty_or_not(cluster_summary)>1){
        strcpy(cluster_role,"admin");
        return 0;
    }
    if(file_empty_or_not(user_passwords)>1){
        strcpy(cluster_role,"user");
        return 0;
    }
    strcpy(cluster_role,"invalid");
    return 1;
}

int add_to_cluster_registry(char* new_cluster_name, char* import_flag){
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    FILE* file_p=fopen(cluster_registry,"a+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open/write to the cluster registry. Exit now." RESET_DISPLAY);
        return -1;
    }
    if(strcmp(import_flag,"imported")==0){
        fprintf(file_p,"< cluster name: %s > <imported>\n",new_cluster_name);
    }
    else{
        fprintf(file_p,"< cluster name: %s >\n",new_cluster_name);
    }
    fclose(file_p);
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

int decrypt_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_info){
    char vaultdir[DIR_LENGTH]="";
    char md5sum[64]="";
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(bucket_info,"%s%sbucket_info.txt.tmp",vaultdir,PATH_SLASH);
    sprintf(cmdline,"%s decrypt %s%sbucket_info.txt.tmp %s%sbucket_info.txt %s %s",NOW_CRYPTO_EXEC,vaultdir,PATH_SLASH,vaultdir,PATH_SLASH,md5sum,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int remote_copy(char* workdir, char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option, char* recursive_flag, int silent_flag){
    if(strcmp(option,"put")!=0&&strcmp(option,"get")!=0){
        return 1;
    }
    char real_recursive_flag[4]="";
    if(strcmp(recursive_flag,"-r")!=0){
        strcpy(real_recursive_flag,"");
    }
    else{
        strcpy(real_recursive_flag,"-r");
    }
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    get_state_value(workdir,"master_public_ip:",remote_address);
    if(strcmp(username,"root")==0){
        sprintf(private_key,"%s%snow-cluster-login",SSHKEY_DIR,PATH_SLASH);
    }
    else{
        get_cluster_name(cluster_name,workdir);
        sprintf(private_key,"%s%s.%s%s%s.key",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH,username);
        if(file_exist_or_not(private_key)!=0){
            return -3;
        }
    }
    if(strcmp(option,"put")==0){
        if(silent_flag==0){
            sprintf(cmdline,"scp %s -o StrictHostKeyChecking=no -i %s %s %s@%s:%s %s",real_recursive_flag,private_key,local_path,username,remote_address,remote_path,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"scp %s -o StrictHostKeyChecking=no -i %s %s %s@%s:%s",real_recursive_flag,private_key,local_path,username,remote_address,remote_path);
        }
    }
    else{
        if(silent_flag==0){
            sprintf(cmdline,"scp %s -o StrictHostKeyChecking=no -i %s %s@%s:%s %s %s",real_recursive_flag,private_key,username,remote_address,remote_path,local_path,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"scp %s -o StrictHostKeyChecking=no -i %s %s@%s:%s %s",real_recursive_flag,private_key,username,remote_address,remote_path,local_path);
        }
    }
    if(system(cmdline)!=0){
        return 3;
    }
    else{
        return 0;
    }
}

int activate_sshkey(char* ssh_privkey){
    if(file_exist_or_not(ssh_privkey)!=0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    FILE* file_p=NULL;
    char group_and_user[64]="";
    char line_seq_buffer[256]="";
    char line_seq_buffer2[128]="";
    char line_buffer[512]="";
    sprintf(cmdline,"takeown /f %s %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"icacls %s /c /t /inheritance:d %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"icacls %s /c /t /remove:g Users %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"icacls %s /c /t /remove:g \"Authenticated Users\" %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"icacls %s /c /t /remove:g Administrators %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"icacls %s > c:\\programdata\\hpc-now\\perm.txt",ssh_privkey);
    system(cmdline);
    file_p=fopen("c:\\programdata\\hpc-now\\perm.txt","r");
    if(file_p==NULL){
        return -3;
    }
    while(!feof(file_p)){
        fgetline(file_p,line_buffer);
        get_seq_string(line_buffer,' ',2,line_seq_buffer);
        get_seq_string(line_seq_buffer,'\\',2,line_seq_buffer2);
        get_seq_string(line_seq_buffer2,':',1,group_and_user);
        if(strcmp(group_and_user,"hpc-now")!=0&&strlen(group_and_user)!=0){
            sprintf(cmdline,"icacls %s /c /t /remove %s %s",ssh_privkey,group_and_user,SYSTEM_CMD_REDIRECT);
            system(cmdline);
        }
    }
    fclose(file_p);
    sprintf(cmdline,"%s c:\\programdata\\hpc-now\\perm.txt %s",DELETE_FILE_CMD,SYSTEM_CMD_REDIRECT);
    system(cmdline);
#else
    sprintf(cmdline,"chown -R hpc-now:hpc-now %s %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    sprintf(cmdline,"chmod 600 %s %s",ssh_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
#endif
    return 0;
}

int get_user_sshkey(char* cluster_name, char* user_name, char* user_status, char* sshkey_dir){
    char sshkey_subdir[DIR_LENGTH]="";
    char ssh_privkey[FILENAME_LENGTH]="";
    char ssh_privkey_remote[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char workdir[DIR_LENGTH];
    sprintf(sshkey_subdir,"%s%s.%s",sshkey_dir,PATH_SLASH,cluster_name);
    if(folder_exist_or_not(sshkey_subdir)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,sshkey_subdir,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    sprintf(ssh_privkey,"%s%s%s.key",sshkey_subdir,PATH_SLASH,user_name);
    get_workdir(workdir,cluster_name);
    if(strcmp(user_name,"root")==0){
        sprintf(ssh_privkey_remote,"/root/.ssh/id_rsa");
    }
    else{
        if(strcmp(user_status,"DISABLED")==0){
            sprintf(ssh_privkey_remote,"/root/.sshkey_deleted/id_rsa.%s",user_name);
        }
        else{
            sprintf(ssh_privkey_remote,"/home/%s/.ssh/id_rsa",user_name);
        }
    }
    if(remote_copy(workdir,sshkey_dir,ssh_privkey,ssh_privkey_remote,"root","get","",0)!=0){
        return 1;
    }
    else{
        activate_sshkey(ssh_privkey);
        return 0;
    }
}

void delete_user_sshkey(char* cluster_name, char* user_name, char* sshkey_dir){
    char user_privkey[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(user_privkey,"%s%s.%s%s%s.key",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,cluster_name);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,user_privkey,SYSTEM_CMD_REDIRECT);
    system(cmdline);
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
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    get_state_value(workdir,"master_public_ip:",remote_address);
    sprintf(private_key,"%s%snow-cluster-login",sshkey_folder,PATH_SLASH);
    sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr %s\" | at now + %d minutes\" %s",private_key,remote_address,exec_type,delay_minutes,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int remote_exec_general(char* workdir, char* sshkey_folder, char* username, char* commands, int delay_minutes, int silent_flag){
    if(delay_minutes<0){
        return -1;
    }
    char cmdline[CMDLINE_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char cluster_role[16]="";
    get_state_value(workdir,"master_public_ip:",remote_address);
    get_cluster_name(cluster_name,workdir);
    cluster_role_detect(workdir,cluster_role);
    if(strcmp(username,"root")==0&&strcmp(cluster_role,"opr")==0){
        sprintf(private_key,"%s%snow-cluster-login",SSHKEY_DIR,PATH_SLASH);
    }
    else{
        sprintf(private_key,"%s%s.%s%s%s.key",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH,username);
    }
    if(file_exist_or_not(private_key)!=0){
        return -3;
    }
    if(delay_minutes==0){
        if(silent_flag==0){
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"%s\" %s",private_key,username,remote_address,commands,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"%s\"",private_key,username,remote_address,commands);
        }
    }
    else{
        if(silent_flag==0){
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\" %s",private_key,username,remote_address,commands,delay_minutes,SYSTEM_CMD_REDIRECT);
        }
        else{
            sprintf(cmdline,"ssh -n -o StrictHostKeyChecking=no -i %s %s@%s \"echo \"%s\" | at now + %d minutes\"",private_key,username,remote_address,commands,delay_minutes);
        }
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

int get_compute_node_num(char* statefile, char* option){
    char get_num[4]="";
    if(strcmp(option,"all")==0){
        get_key_value(statefile,"total_compute_nodes:",' ',get_num); 
    }
    else if(strcmp(option,"on")==0){
        get_key_value(statefile,"running_compute_nodes:",' ',get_num); 
    }
    else{
        get_key_value(statefile,"down_compute_nodes:",' ',get_num); 
    }
    return string_to_positive_num(get_num);
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
        return system(cmdline);
    }
    else{
        return -1;
    }
}

int decrypt_single_file_general(char* now_crypto_exec, char* source_file, char* target_file, char* md5sum){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_exist_or_not(source_file)==0){
        sprintf(cmdline,"%s decrypt %s %s %s %s",now_crypto_exec,source_file,target_file,md5sum,SYSTEM_CMD_REDIRECT);
        return system(cmdline);
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
//    sprintf(filename_temp,"%s%sbucket.conf",vaultdir,PATH_SLASH);
//    encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
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

/*
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
 */
int getstate(char* workdir, char* crypto_filename){
    char cloud_flag[16]="";
    get_cloud_flag(workdir,cloud_flag);
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char tfstate[FILENAME_LENGTH]="";
    char compute_template[FILENAME_LENGTH]="";
    char master_tf[FILENAME_LENGTH]="";
    char statefile[FILENAME_LENGTH]="";
    char hostfile[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char master_config[16]="";
    char compute_config[16]="";
    char ht_flag[16]="";
    char string_temp[64]="";
    char string_temp2[64]="";
    char pay_method[8]="";
    char md5sum[64]="";
    int node_num_gs;
    int node_num_on_gs=0;
    int i;
    FILE* file_p_tfstate=NULL;
    FILE* file_p_statefile=NULL;
    FILE* file_p_hostfile=NULL;
    create_and_get_stackdir(workdir,stackdir);
    create_and_get_vaultdir(workdir,vaultdir);
    get_crypto_key(crypto_filename,md5sum);
    sprintf(tfstate,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    if(file_exist_or_not(tfstate)!=0){
        sprintf(filename_temp,"%s%sterraform.tfstate.tmp",stackdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum)!=0){
            return -1;
        }
        if(file_exist_or_not(tfstate)!=0){
            return -1;
        }
    }
    sprintf(master_tf,"%s%shpc_stack_master.tf",stackdir,PATH_SLASH);
    if(file_exist_or_not(master_tf)!=0){
        sprintf(filename_temp,"%s%shpc_stack_master.tf.tmp",stackdir,PATH_SLASH);
        if(decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum)!=0){
            return -1;
        }
        if(file_exist_or_not(master_tf)!=0){
            return -1;
        }
    }
    sprintf(compute_template,"%s%scompute_template",stackdir,PATH_SLASH);
    if(file_exist_or_not(compute_template)!=0){
        return -1;
    }
    file_p_tfstate=fopen(tfstate,"r");
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    file_p_statefile=fopen(statefile,"w+");
    if(file_p_statefile==NULL){
        fclose(file_p_tfstate);
        return -1;
    }
    sprintf(hostfile,"%s%shostfile_latest",stackdir,PATH_SLASH);
    file_p_hostfile=fopen(hostfile,"w+");
    if(file_p_hostfile==NULL){
        fclose(file_p_tfstate);
        fclose(file_p_statefile);
        return -1;
    }
    find_and_get(master_tf,"instance_type","","",1,"instance_type","","",'.',3,master_config);
    find_and_get(compute_template,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    if(find_multi_keys(compute_template,"cpu_threads_per_core = 1","","","","")!=0){
        strcpy(ht_flag,"htoff");
    }
    else{
        strcpy(ht_flag,"hton");
    }
    if(find_multi_keys(compute_template,"instance_charge_type = PrePaid","","","","")==0||find_multi_keys(compute_template,"instance_charge_type = PREPAID","","","","")==0){
        strcpy(pay_method,"month");
    }
    else{
        strcpy(pay_method,"od");
    }
    fprintf(file_p_statefile,"---GENERATED AND MAINTAINED BY HPC-NOW SERVICES INTERNALLY---\n");
    fprintf(file_p_statefile,"master_config: %s\ncompute_config: %s\nht_flag: %s\n",master_config,compute_config,ht_flag);
    if(strcmp(cloud_flag,"CLOUD_A")==0||strcmp(cloud_flag,"CLOUD_B")==0){
        node_num_gs=find_multi_keys(tfstate,"\"instance_name\": \"compute","","","","");
        find_and_get(tfstate,"\"instance_name\": \"master","","",50,"public_ip","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_get(tfstate,"\"instance_name\": \"master","","",50,"private_ip","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            find_and_get(tfstate,"\"instance_name\": \"master","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"master_status: %s\n",string_temp);
            find_and_get(tfstate,"\"instance_name\": \"database","","",50,"instance_status","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        }
        else{
            find_and_get(tfstate,"\"instance_name\": \"master","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"master_status: %s\n",string_temp);
            find_and_get(tfstate,"\"instance_name\": \"database","","",90,"\"status\":","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        }
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"instance_name\": \"compute%d",i+1);
            find_and_get(tfstate,string_temp2,"","",50,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            if(strcmp(cloud_flag,"CLOUD_B")==0){
                find_and_get(tfstate,string_temp2,"","",30,"instance_status","","",'\"',4,string_temp);
                fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            }
            else{
                find_and_get(tfstate,string_temp2,"","",90,"\"status\":","","",'\"',4,string_temp);
                fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            }
            if(strcmp(string_temp,"RUNNING")==0||strcmp(string_temp,"running")==0||strcmp(string_temp,"Running")==0){
                node_num_on_gs++;
            }
        }
    }
    else{
        node_num_gs=find_multi_keys(tfstate,"\"name\": \"compute","","","","");
        find_and_get(tfstate,"\"name\": \"master","","",90,"\"public_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"master_public_ip: %s\n",string_temp);
        find_and_get(tfstate,"\"name\": \"master","","",90,"\"private_ip\"","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"master_private_ip: %s\n",string_temp);
        fprintf(file_p_hostfile,"%s\tmaster\n",string_temp);
        find_and_get(tfstate,"\"name\": \"m_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"master_status: %s\n",string_temp);
        find_and_get(tfstate,"\"name\": \"db_state","","",30,"\"state\":","","",'\"',4,string_temp);
        fprintf(file_p_statefile,"database_status: %s\n",string_temp);
        for(i=0;i<node_num_gs;i++){
            sprintf(string_temp2,"\"name\": \"compute%d",i+1);
            find_and_get(tfstate,string_temp2,"","",90,"private_ip","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"compute%d_private_ip: %s\n",i+1,string_temp);
            fprintf(file_p_hostfile,"%s\tcompute%d\n",string_temp,i+1);
            sprintf(string_temp2,"\"name\": \"comp%d",i+1);
            find_and_get(tfstate,string_temp2,"","",30,"\"state\":","","",'\"',4,string_temp);
            fprintf(file_p_statefile,"compute%d_status: %s\n",i+1,string_temp);
            if(strcmp(string_temp,"RUNNING")==0||strcmp(string_temp,"running")==0||strcmp(string_temp,"Running")==0){
                node_num_on_gs++;
            }
        }
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

int get_state_value(char* workdir, char* key, char* value){
    char stackdir[DIR_LENGTH]="";
    char statefile[FILENAME_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    return get_key_value(statefile,key,' ',value);
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
    char vaultdir[DIR_LENGTH]="";
    char md5sum[33]="";
    char master_address[32]="";
    char master_address_prev[32]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    decrypt_single_file(now_crypto_exec,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    find_and_get(filename_temp,"Master","Node","IP:",1,"Master","Node","IP:",' ',4,master_address_prev);
    get_state_value(workdir,"master_public_ip:",master_address);
    if(strcmp(master_address,master_address_prev)!=0){
        sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
        global_replace(filename_temp,master_address_prev,master_address);
        encrypt_and_delete(now_crypto_exec,filename_temp,md5sum);
    }
    else{
        sprintf(cmdline,"%s %s%sCLUSTER_SUMMARY.txt %s",DELETE_FILE_CMD,vaultdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
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
    file_p_2=fopen(logfile,"w+");
    fclose(file_p_2);
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

int wait_for_complete(char* tf_realtime_log, char* option, char* errorlog, char* errlog_archive, int silent_flag){
//    char cmdline[CMDLINE_LENGTH]="";
    int i=0;
    int total_minutes=0;
    char* annimation="\\|/-";
    char findkey[32]="";
    if(strcmp(option,"init")==0){
        strcpy(findkey,"successfully initialized!");
        //sprintf(cmdline,"%s %s | %s successfully | %s initialized! %s",CAT_FILE_CMD,tf_realtime_log,GREP_CMD,GREP_CMD,SYSTEM_CMD_REDIRECT_NULL);
        total_minutes=1;
    }
    else if(strcmp(option,"apply")==0){
        strcpy(findkey,"Apply complete!");
        //sprintf(cmdline,"%s %s | %s complete! %s",CAT_FILE_CMD,tf_realtime_log,GREP_CMD,SYSTEM_CMD_REDIRECT_NULL);
        total_minutes=3;
    }
    else if(strcmp(option,"destroy")==0){
        strcpy(findkey,"Destroy complete!");
        total_minutes=3;
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] TF_OPTION_NOT_SUPPORTED.\n" RESET_DISPLAY);
        return -127;
    }
    while(find_multi_keys(tf_realtime_log,findkey,"","","","")<1&&i<MAXIMUM_WAIT_TIME){
        if(silent_flag!=0){
            fflush(stdin);
            printf(GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " This may need %d min(s). %d sec(s) passed ... (%c)\r",total_minutes,i,*(annimation+i%4));
            fflush(stdout);
        }
        i++;
        sleep(1);
        if(file_empty_or_not(errorlog)>0){
            if(find_multi_keys(errorlog,"Warning:","","","","")>0){
                archive_log(errlog_archive,errorlog);
            }
            else{
                if(silent_flag!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] TF_EXEC_ERROR.\n" RESET_DISPLAY);
                }
                return 127;
            }
        }
    }
    if(i==MAXIMUM_WAIT_TIME){
        if(silent_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] TF_EXEC_TIMEOUT.\n" RESET_DISPLAY);
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
    char cluster_name[64]="";
    char master_address[32]="";
    char master_status[16]="";
    char master_config[16]="";
    char db_status[16]="";
    char cloud_flag[16]="";
    char cluster_role[16]="";
    char string_temp[32]="";
    char compute_address[32]="";
    char compute_status[16]="";
    char compute_config[16]="";
    char statefile[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char ht_status[16]="";
    int node_num=0;
    char node_num_string[4]="";
    int running_node_num=0;
    char running_node_num_string[4]="";
    int i;
    create_and_get_stackdir(workdir,stackdir);
    get_cluster_name(cluster_name,workdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    if(file_empty_or_not(statefile)<1||get_cloud_flag(workdir,cloud_flag)==-1){
        return 1;
    }
    cluster_role_detect(workdir,cluster_role);
    get_key_value(statefile,"master_public_ip:",' ',master_address);
    get_key_value(statefile,"master_status:",' ',master_status);
    get_key_value(statefile,"database_status:",' ',db_status);
    get_key_value(statefile,"master_config:",' ',master_config);
    get_key_value(statefile,"compute_config:",' ',compute_config);
    get_key_value(statefile,"ht_flag:",' ',ht_status);
    get_key_value(statefile,"total_compute_nodes:",' ',node_num_string);
    node_num=string_to_positive_num(node_num_string);
    get_key_value(statefile,"running_compute_nodes:",' ',running_node_num_string);
    running_node_num=string_to_positive_num(running_node_num_string);
    if(graph_level==0){
        printf(GENERAL_BOLD "|        +-Cluster name: " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY GENERAL_BOLD " -+- Cluster role: " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY GENERAL_BOLD "\n",cluster_name,cluster_role);
        printf("|          +-master(%s,%s,%s)\n",master_address,master_status,master_config);
        printf("|            +-db(%s)\n" RESET_DISPLAY,db_status);
    }
    for(i=0;i<node_num;i++){
        sprintf(string_temp,"compute%d_private_ip:",i+1);
        get_key_value(statefile,string_temp,' ',compute_address);
        sprintf(string_temp,"compute%d_status:",i+1);
        get_key_value(statefile,string_temp,' ',compute_status);
        if(graph_level==0){
            if(strlen(ht_status)!=0){
                printf(GENERAL_BOLD "|              +-compute%d(%s,%s,%s,%s)\n" RESET_DISPLAY,i+1,compute_address,compute_status,compute_config,ht_status);
            }
            else{
                printf(GENERAL_BOLD "|              +-compute%d(%s,%s,%s)\n" RESET_DISPLAY,i+1,compute_address,compute_status,compute_config);
            }
        }
    }
    if(graph_level==1){
        if(strlen(ht_status)!=0){
            printf("%s %s | %s | %s %s %s | %d/%d | %s | %s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status);
        }
        else{
            printf("%s %s | %s | %s %s %s | %d/%d | %s \n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config);
        }
    }
    else if(graph_level==2){
        if(strlen(ht_status)!=0){
            printf("%s,%s,%s,%s,%s,%s,%d,%d,%s,%s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config,ht_status);
        }
        else{
            printf("%s,%s,%s,%s,%s,%s,%d,%d,%s\n",cluster_name,cluster_role,cloud_flag,master_address,master_config,master_status,running_node_num,node_num,compute_config);
        }
    }
    printf(RESET_DISPLAY);
    return 0;
}

int cluster_empty_or_not(char* workdir){
    char statefile[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char dot_terraform[FILENAME_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(statefile,"%s%scurrentstate",stackdir,PATH_SLASH);
    sprintf(dot_terraform,"%s%s.terraform",stackdir,PATH_SLASH);
    if(folder_exist_or_not(dot_terraform)!=0){
        return 0;
    }
    if(file_empty_or_not(statefile)<1){
        return 0;
    }
    else{
        return 1;
    }
}

int cluster_asleep_or_not(char* workdir){
    char master_state[32]="";
    get_state_value(workdir,"master_status:",master_state);
    if(strcmp(master_state,"running")!=0&&strcmp(master_state,"Running")!=0&&strcmp(master_state,"RUNNING")!=0){
        return 0;
    }
    else{
        return 1;
    }
}

int cluster_full_running_or_not(char* workdir){
    char stackdir[DIR_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    char filename_temp[FILENAME_LENGTH]="";
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    return get_compute_node_num(filename_temp,"down");
}

int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, int silent_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char tf_realtime_log[FILENAME_LENGTH];
    char tf_realtime_log_archive[FILENAME_LENGTH];
    char tf_error_log[FILENAME_LENGTH];
    char tf_error_log_archive[FILENAME_LENGTH];

    create_and_get_stackdir(workdir,stackdir);
    sprintf(tf_realtime_log,"%s%slog%stf_prep.log",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_realtime_log_archive,"%s%slog%stf_prep.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_error_log,"%s%slog%stf_prep.err.log",workdir,PATH_SLASH,PATH_SLASH);
    sprintf(tf_error_log_archive,"%s%slog%stf_prep.err.log.archive",workdir,PATH_SLASH,PATH_SLASH);
    archive_log(tf_realtime_log_archive,tf_realtime_log);
    archive_log(tf_error_log_archive,tf_error_log);
    sprintf(cmdline,"cd %s%s && %s TF_LOG=DEBUG&&%s TF_LOG_PATH=%s%slog%sterraform.log && echo yes | %s %s %s > %s 2>%s &",stackdir,PATH_SLASH,SET_ENV_CMD,SET_ENV_CMD,workdir,PATH_SLASH,PATH_SLASH,START_BG_JOB,tf_exec,execution_name,tf_realtime_log,tf_error_log);
    system(cmdline);
    if(silent_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Do not terminate this process manually. Max Exec Time: %d s\n",MAXIMUM_WAIT_TIME);
        printf("|          Command: %s. View log: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr viewlog --std\n" RESET_DISPLAY,execution_name);
    }
    if(wait_for_complete(tf_realtime_log,execution_name,tf_error_log,tf_error_log_archive,1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to operate the cluster. Operation command: %s.\n" RESET_DISPLAY,execution_name);
        archive_log(tf_error_log_archive,tf_error_log);
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
    get_cloud_flag(workdir,cloud_vendor);
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

int get_vault_info(char* workdir, char* crypto_keyfile, char* username, char* bucket_flag, char* root_flag){
    if(cluster_empty_or_not(workdir)==0){
        return 1;
    }
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char real_username[USERNAME_LENGTH_MAX]="";
    if(strlen(username)>0){
        get_cluster_name(cluster_name,workdir);
        if(user_name_quick_check(cluster_name,username,SSHKEY_DIR)!=0){
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] The specified username '%s' is invalid or unauthorized.\n" RESET_DISPLAY,username);
            strcpy(real_username,"");
        }
        else{
            strcpy(real_username,username);
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
    char md5sum[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    char unique_cluster_id[32]="";
    char username_temp[32]="";
    char password[32]="";
    char enable_flag[16]="";
    char master_address[32]="";
    char bucket_address[32]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char region_id[32]="";

    get_crypto_key(crypto_keyfile,md5sum);
    create_and_get_vaultdir(workdir,vaultdir);
    create_and_get_stackdir(workdir,stackdir);
    if(get_ucid(workdir,unique_cluster_id)!=0){
        return -1;
    }
    if(get_bucket_info(workdir,crypto_keyfile,bucket_address,region_id,bucket_ak,bucket_sk)!=0){
        return -3;
    }
    get_state_value(workdir,"master_public_ip:",master_address);
    printf(WARN_YELLO_BOLD "\n+------------ HPC-NOW CLUSTER SENSITIVE INFORMATION: ------------+\n" RESET_DISPLAY);
    printf(GENERAL_BOLD "| Unique Cluster ID: " RESET_DISPLAY "%s\n",unique_cluster_id);
    printf(WARN_YELLO_BOLD "+-------------- CLUSTER PORTAL AND *CREDENTIALS* ----------------+\n" RESET_DISPLAY);
    if(strlen(master_address)<7){
        printf(GENERAL_BOLD "| Cluster IP Address: " RESET_DISPLAY WARN_YELLO_BOLD "NOT_RUNNING\n" RESET_DISPLAY);
    }
    else{
        printf(GENERAL_BOLD "| Cluster IP Address: " RESET_DISPLAY "%s\n",master_address);
    }
    printf(GENERAL_BOLD "| Bucket Address:" RESET_DISPLAY " %s\n",bucket_address);
    printf(GENERAL_BOLD "| Cloud Region: " RESET_DISPLAY "%s\n",region_id);
    if(bucketflag==1){
        printf(GENERAL_BOLD "| Bucket AccessKey: " RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,bucket_ak);
        printf(GENERAL_BOLD "| Bucket SecretKey: " RESET_DISPLAY GREY_LIGHT "%s\n" RESET_DISPLAY,bucket_sk);
    }
    printf(WARN_YELLO_BOLD "+---------------- CLUSTER USERS AND *PASSWORDS* -----------------+\n" RESET_DISPLAY);
    if(rootflag==1){
        sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
        decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
        sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
        if(file_exist_or_not(filename_temp)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] You are not an operator or administrator.\n" RESET_DISPLAY);
        }
        else{
            find_and_get(filename_temp,"Master Node Root Password:","","",1,"Master Node Root Password:","","",':',2,password);
            printf(FATAL_RED_BOLD "| Root Password: " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY FATAL_RED_BOLD " ! DO NOT DISCLOSE TO ANYONE !\n" RESET_DISPLAY,password);
        }
    }
    
    sprintf(filename_temp,"%s%suser_passwords.txt.tmp",vaultdir,PATH_SLASH);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return -7;
    }
    file_p=fopen(filename_temp,"r");
    while(fgetline(file_p,single_line)==0){
        if(strlen(single_line)!=0){
            get_seq_string(single_line,' ',2,username_temp);
            get_seq_string(single_line,' ',3,password);
            get_seq_string(single_line,' ',4,enable_flag);
            if(strlen(real_username)==0||strcmp(real_username,username_temp)==0){
                if(strcmp(enable_flag,"DISABLED")==0){
                    printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY WARN_YELLO_BOLD "%s\n" RESET_DISPLAY,username_temp,password,enable_flag);
                }
                else{
                    printf(GENERAL_BOLD "| Username: %s    Password: " RESET_DISPLAY GREY_LIGHT "%s " RESET_DISPLAY GENERAL_BOLD "%s\n" RESET_DISPLAY,username_temp,password,enable_flag);
                }
            }
        }
    }
    printf(WARN_YELLO_BOLD "+---------- DO NOT DISCLOSE THE INFORMATION TO OTHERS -----------+\n" RESET_DISPLAY);
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
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You are operating the cluster" HIGH_CYAN_BOLD " %s" RESET_DISPLAY " now, which may affect\n",current_cluster_name);
    printf("|          the " GENERAL_BOLD "resources|data|jobs" RESET_DISPLAY ". Please input " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " to continue.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    fflush(stdin);
    scanf("%s",doubleconfirm);
    getchar();
    if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continue. You chose to deny this operation.\n");
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

int cluster_ssh(char* workdir, char* username, char* role_flag){
    char master_address[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char private_sshkey[FILENAME_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    get_state_value(workdir,"master_public_ip:",master_address);
    get_cluster_name(cluster_name,workdir);
    if(strcmp(role_flag,"opr")==0){
        sprintf(private_sshkey,"%s%snow-cluster-login",SSHKEY_DIR,PATH_SLASH);
    }
    else{
        sprintf(private_sshkey,"%s%s.%s%s%s.key",SSHKEY_DIR,PATH_SLASH,cluster_name,PATH_SLASH,username);
    }
    if(file_exist_or_not(private_sshkey)!=0){
        return -1;
    }
    sprintf(cmdline,"ssh -i %s -o StrictHostKeyChecking=no %s@%s",private_sshkey,username,master_address);
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

int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[64]="";
    char line_buffer[128]="";
    char header[16]="";
    char tail[64]="";
    sprintf(filename_temp,"%s%svault%sbucket_info.txt.tmp",workdir,PATH_SLASH,PATH_SLASH);
    get_crypto_key(crypto_keyfile,md5sum);
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%svault%sbucket_info.txt",workdir,PATH_SLASH,PATH_SLASH);
    FILE* file_p=fopen(filename_temp,"r");
    int i=0;
    if(file_p==NULL){
        return -1;
    }
    while(!feof(file_p)){
        fgetline(file_p,line_buffer);
        get_seq_string(line_buffer,' ',1,header);
        get_seq_string(line_buffer,' ',2,tail);
        if(strcmp(header,"BUCKET:")==0){
            strcpy(bucket_address,tail);
            i++;
        }
        else if(strcmp(header,"REGION:")==0){
            strcpy(region_id,tail);
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
        else{
            continue;
        }
    }
    fclose(file_p);
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,filename_temp,SYSTEM_CMD_REDIRECT);
    system(cmdline);
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

int get_cluster_bucket_id(char* workdir, char* crypto_keyfile, char* bucket_id){
    char vaultdir[DIR_LENGTH]="";
    char bucket_ak[128]="";
    char bucket_sk[128]="";
    char bucket_region[32]="";
    char bucket_address[32]="";
    char filename_temp[FILENAME_LENGTH]="";
    char md5sum[64]="";
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sbucket_info.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)==0){
        if(get_bucket_info(workdir,crypto_keyfile,bucket_address,bucket_region,bucket_ak,bucket_sk)==0){
            if(get_seq_string(bucket_address,'/',2,bucket_id)==0){
                return 0;
            }
        }
    }
    get_crypto_key(crypto_keyfile,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        return -1;
    }
    decrypt_single_file(NOW_CRYPTO_EXEC,filename_temp,md5sum);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt",vaultdir,PATH_SLASH);
    find_and_get(filename_temp,"NetDisk Address:","","",1,"NetDisk Address:","","",' ',4,bucket_id);
    encrypt_and_delete(NOW_CRYPTO_EXEC,filename_temp,md5sum);
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
    printf(WARN_YELLO_BOLD "[ -INFO- ] MAXIMUM DURATION: 30s.\n" RESET_DISPLAY);
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
    return remote_copy(workdir,sshkey_dir,filename_temp,"/root/.cluster_secrets/user_secrets.txt","root","put","",0);
}

int sync_statefile(char* workdir, char* sshkey_dir){
    char stackdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
    sprintf(filename_temp,"%s%scurrentstate",stackdir,PATH_SLASH);
    return remote_copy(workdir,sshkey_dir,filename_temp,"/usr/hpc-now/currentstate","root","put","",0);
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

int user_name_quick_check(char* cluster_name, char* user_name, char* sshkey_dir){
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    get_workdir(workdir,cluster_name);
    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%sCLUSTER_SUMMARY.txt.tmp",vaultdir,PATH_SLASH);
    if(strcmp(user_name,"root")==0){
        if(file_exist_or_not(filename_temp)==0){
            return 0;
        }
        else{
            return 2;
        }
    }
    char user_sshkey[FILENAME_LENGTH]="";
    sprintf(user_sshkey,"%s%s.%s%s%s.key",sshkey_dir,PATH_SLASH,cluster_name,PATH_SLASH,user_name);
    if(file_exist_or_not(user_sshkey)!=0){
        return 1;
    }
    else{
        return 0;
    }
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
    sprintf(username_ext,"username: %s ",username_input);
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
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
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
    sprintf(remote_commands,"hpcmgr users add %s %s",username_input,password_final);
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0,0)==0){
        sprintf(remote_commands,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s | grep ENABLED",username_input);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0,0)==0){
            printf("[ -INFO- ] Updating the local user-info registry ...\n");
            file_p=fopen(user_registry_file,"a");
            fprintf(file_p,"username: %s %s ENABLED\n",username_input,password_final);
            fclose(file_p);
            get_cluster_name(cluster_name,workdir);
            get_user_sshkey(cluster_name,username_input,"ENABLED",sshkey_dir);
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
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
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
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0,0)==0){
        sprintf(remote_commands,"cat /root/.cluster_secrets/user_secrets.txt | grep -w %s",username_input);
        if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0,0)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to delete the user %s from your cluster. Exit now.\n" RESET_DISPLAY,username_input);
            delete_decrypted_user_passwords(workdir);
            return 1;
        }
        else{
            delete_user_from_registry(user_registry_file,username_input);
            encrypt_and_delete_user_passwords(workdir,crypto_keyfile);
            get_cluster_name(cluster_name,workdir);
            delete_user_sshkey(cluster_name,username_input,sshkey_dir);
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
        sprintf(remote_commands,"hpcmgr users add %s",username_input);
    }
    else{
        sprintf(remote_commands,"hpcmgr users delete %s",username_input);
    }
    if(remote_exec_general(workdir,sshkey_dir,"root",remote_commands,0,0)==0){
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
    sprintf(remote_commands,"echo \"%s\" | passwd %s --stdin",password_final,username_input);
    if(remote_exec_general(workdir,ssheky_dir,"root",remote_commands,0,0)==0){
        sprintf(username_ext,"username: %s ",username_input);
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

int usrmgr_prereq_check(char* workdir, char* ucmd){
    char confirm[64]="";
    if(cluster_asleep_or_not(workdir)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not running. Please wake up first\n" RESET_DISPLAY);
        return -1;
    }
    int i=check_down_nodes(workdir);
    if(i!=0){
        if(strcmp(ucmd,"delete")==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] There are down nodes. When deleting users, although not required, it is\n");
            printf(WARN_YELLO_BOLD "|          *strongly* recommended to wake up the whole cluster first.\n" RESET_DISPLAY);
            printf(GENERAL_BOLD "[ -INFO- ] You can exit now, run" RESET_DISPLAY HIGH_GREEN_BOLD " 'hpcopr wakeup --all'" RESET_DISPLAY GENERAL_BOLD " to wake up all nodes.\n" RESET_DISPLAY);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Would you like to continue *WITHOUT* waking up? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted.\n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " You chose to deny the operation. Exit now.\n");
                return 3;
            }
            else{
                return 5;
            }
        }
        printf(WARN_YELLO_BOLD "[ -WARN- ] There are %d down nodes.\n" RESET_DISPLAY,i);
    }
    return 0;
}

void usrmgr_remote_exec(char* workdir, char* sshkey_folder, int prereq_check_flag){
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote executing now ...\n");
    remote_exec(workdir,sshkey_folder,"connect",1);
    remote_exec(workdir,sshkey_folder,"all",2);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remote execution commands sent.\n");
    if(prereq_check_flag!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The cluster user has been updated with down nodes.\n" RESET_DISPLAY);
    }
}

void get_workdir(char* cluster_workdir, char* cluster_name){
    sprintf(cluster_workdir,"%s%sworkdir%s%s%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,cluster_name,PATH_SLASH);
}

int get_cluster_name(char* cluster_name, char* cluster_workdir){
    char* path_seprator_str=PATH_SLASH;
    char path_seprator=path_seprator_str[0];
    int i=0;
    char dir_buffer[128]="";
    while(i<16){
        i++;
        get_seq_string(cluster_workdir,path_seprator,i,dir_buffer);
        if(strlen(dir_buffer)==0){
            return 0;
        }
        else{
            strcpy(cluster_name,dir_buffer);
        }
    }
    return 1;
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

int update_tf_passwords(char* base_tf, char* master_tf, char* user_passwords){
    if(file_exist_or_not(base_tf)!=0||file_exist_or_not(master_tf)!=0||file_exist_or_not(user_passwords)!=0){
        return -1;
    }
    FILE* file_p=NULL;
    FILE* file_p_base=NULL;
    char user_line_buffer[256]="";
    char line_temp[256]="";
    char user_name_temp[64]="";
    char user_passwd_temp[64]="";
    char user_status_temp[16]="";
    file_trunc_by_kwds(base_tf,"","user1_passwd",1);
    delete_lines_by_kwd(master_tf,"username:",1);
    file_p=fopen(user_passwords,"r");
    file_p_base=fopen(base_tf,"a");
    while(!feof(file_p)){
        fgetline(file_p,user_line_buffer);
        if(strlen(user_line_buffer)==0){
            continue;
        }
        get_seq_string(user_line_buffer,' ',2,user_name_temp);
        get_seq_string(user_line_buffer,' ',3,user_passwd_temp);
        get_seq_string(user_line_buffer,' ',4,user_status_temp);
        fprintf(file_p_base,"variable \"%s_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",user_name_temp,user_passwd_temp);
        sprintf(line_temp,"echo -e \"username: %s ${var.%s_passwd} %s\" >> /root/user_secrets.txt",user_name_temp,user_name_temp,user_status_temp);
        insert_lines(master_tf,"master_private_ip",line_temp);
    }
    fclose(file_p);
    fclose(file_p_base);
    return 0;
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
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently you are not operating any cluster.\n" RESET_DISPLAY);
        }
        return 1;
    }
    else{
        file_p=fopen(CURRENT_CLUSTER_INDICATOR,"r");
        fscanf(file_p,"%s",current_cluster_name);
        if(silent_flag==1){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Current cluster:" HIGH_GREEN_BOLD " %s" RESET_DISPLAY ".\n",current_cluster_name);
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
        fclose(file_p);
        return -1;
    }
    fclose(file_p);
    return 0;
}

int cluster_name_check(char* cluster_name){
    char cluster_name_ext[64]="";
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
    sprintf(cluster_name_ext,"< cluster name: %s >",cluster_name);
    if(find_multi_keys(ALL_CLUSTER_REGISTRY,cluster_name_ext,"","","","")>0){
        return -127;
    }
    else{
        return 0;
    }
}

int check_and_cleanup(char* prev_workdir){
    char current_workdir[DIR_LENGTH]="";
    char current_cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(strlen(prev_workdir)!=0){
        if(show_current_cluster(current_workdir,current_cluster_name,0)==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Currently there is no switched cluster.\n" RESET_DISPLAY);
        }
        else{
            if(strcmp(current_workdir,prev_workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The switched cluster is" RESET_DISPLAY HIGH_GREEN_BOLD " %s" RESET_DISPLAY WARN_YELLO_BOLD ".\n" RESET_DISPLAY,current_cluster_name);
            }
        }
    }
#ifdef _WIN32
    FILE* file_p=NULL;
    char cmdline[CMDLINE_LENGTH]="";
    char appdata_dir[DIR_LENGTH]="";
    system("echo %APPDATA% > c:\\programdata\\appdata.txt.tmp");
    file_p=fopen("c:\\programdata\\appdata.txt.tmp","r");
    fscanf(file_p,"%s",appdata_dir);
    fclose(file_p);
    system("del /f /s /q c:\\programdata\\appdata.txt.tmp > nul 2>&1");
    system("icacls C:\\programdata\\hpc-now\\workdir /deny Administrators:F /T > nul 2>&1");
    system("icacls C:\\programdata\\hpc-now\\etc /deny Administrators:F /T > nul 2>&1");
    sprintf(cmdline,"del /f /s /q %s\\Microsoft\\Windows\\Recent\\* > nul 2>&1",appdata_dir);
    system(cmdline);
    sprintf(cmdline,"rd /q /s %s\\Microsoft\\Windows\\Recent\\ > nul 2>&1",appdata_dir);
    system(cmdline);
#else
    //Keep it here for further use. 
#endif
    print_tail();
    return 0;
}

int list_all_cluster_names(int header_flag){
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
        fclose(file_p);
        return 1;
    }
    if(header_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " List of all the clusters:\n");
    }
    while(fgetline(file_p,registry_line)!=1){
        if(strlen(registry_line)!=0){
            if(file_exist_or_not(CURRENT_CLUSTER_INDICATOR)!=0){
                printf(RESET_DISPLAY "|        : %s\n" RESET_DISPLAY,registry_line);
            }
            else{
                get_seq_string(registry_line,' ',4,temp_cluster_name);
                if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,temp_cluster_name)==0){
                    printf(HIGH_GREEN_BOLD "|  switch: %s\n" RESET_DISPLAY,registry_line);
                }
                else{
                    printf(RESET_DISPLAY "|        : %s\n" RESET_DISPLAY,registry_line);
                }
            }
        }
    }
    fclose(file_p);
    return 0;
}

int exit_current_cluster(void){
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(cmdline,"%s %s %s",DELETE_FILE_CMD,CURRENT_CLUSTER_INDICATOR,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
}

int delete_from_cluster_registry(char* deleted_cluster_name){
    char* cluster_registry=ALL_CLUSTER_REGISTRY;
    char deleted_cluster_name_with_prefix[LINE_LENGTH_SHORT]="";
    char filename_temp[FILENAME_LENGTH]="";
    char temp_line[LINE_LENGTH_SHORT]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    FILE* file_p_tmp=NULL;
    sprintf(deleted_cluster_name_with_prefix,"< cluster name: %s >",deleted_cluster_name);
    sprintf(filename_temp,"%s.tmp",cluster_registry);
    file_p=fopen(cluster_registry,"r");
    file_p_tmp=fopen(filename_temp,"w+");
    while(!feof(file_p)){
        fgetline(file_p,temp_line);
        if(contain_or_not(temp_line,deleted_cluster_name_with_prefix)!=0&&strlen(temp_line)>0){
            fprintf(file_p_tmp,"%s\n",temp_line);
        }
    }
    fclose(file_p);
    fclose(file_p_tmp);
    if(current_cluster_or_not(CURRENT_CLUSTER_INDICATOR,deleted_cluster_name)==0){
        exit_current_cluster();
    }
    sprintf(cmdline,"%s %s %s %s",MOVE_FILE_CMD,filename_temp,cluster_registry,SYSTEM_CMD_REDIRECT);
    return system(cmdline);
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