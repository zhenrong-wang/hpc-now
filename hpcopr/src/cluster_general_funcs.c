/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifdef _WIN32
#include "..\\include\\now_macros.h"
#include "..\\include\\now_global_vars.h"
#include "..\\include\\now_functions.h" 

#else
#include "../include/now_macros.h"
#include "../include/now_global_vars.h"
#include "../include/now_functions.h" 
#endif

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
//    fgets(md5sum,128,md5_tmp);
    fclose(md5_tmp);
#ifdef _WIN32
    system("del /f /q md5.txt.tmp > nul 2>&1");
#else
    system("rm -rf /tmp/md5.txt.tmp >> /dev/null 2>&1");
#endif
    return 0;
}

void create_and_get_stackdir(char* workdir, char* stackdir){
    char cmdline[CMDLINE_LENGTH]="";
/*    char string_temp[4]="";
    int i;
    int j=0;
    int cluster_num=0;
    for(i=strlen(workdir)-1;i>0;i--){
        if(*(workdir+i)=='-'){
            break;
        }
        else if(*(workdir+i)>'9'||*(workdir+i)<'0'){
            continue;
        }
        else{
            *(string_temp+j)=*(workdir+i);
            j++;
        }
    }
    for(j=0;j<strlen(string_temp);j++){
        cluster_num+=(*(string_temp+j)-'0')*pow(10,j);
    }*/
#ifdef _WIN32
    sprintf(stackdir,"%s\\stack",workdir);
    sprintf(cmdline,"mkdir %s > nul 2>&1",stackdir);
    system(cmdline);
    sprintf(cmdline,"attrib +h +s +r %s",stackdir);
    system(cmdline);
#else
    sprintf(stackdir,"%s/.stack",workdir);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",stackdir);
    system(cmdline);
/*
#elif __linux__
    sprintf(stackdir,"/usr/.hpc-now/.stack/.cluster-%d",cluster_num);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",stackdir);
    system(cmdline);
#elif __APPLE__
    sprintf(stackdir,"/Applications/.hpc-now/.stack/.cluster-%d",cluster_num);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",stackdir);
    system(cmdline);*/
#endif
}

int remote_copy(char* workdir, char* sshkey_dir, char* option){
    if(strcmp(option,"hostfile")!=0){
        return -1;
    }
    char stackdir[DIR_LENGTH]="";
    char private_key[FILENAME_LENGTH]="";
    char remote_address[32]="";
    char hostfile[FILENAME_LENGTH]="";
    char currentstate[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    FILE* file_p=NULL;
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(currentstate,"%s\\currentstate",stackdir);
#else
    sprintf(currentstate,"%s/currentstate",stackdir);
#endif
    file_p=fopen(currentstate,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
#ifdef _WIN32
    sprintf(private_key,"%s\\now-cluster-login",sshkey_dir);
    sprintf(hostfile,"%s\\hostfile_latest",stackdir);
    sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/hostfile > nul 2>&1",private_key,hostfile,remote_address);
#else
    sprintf(private_key,"%s/now-cluster-login",sshkey_dir);
    sprintf(hostfile,"%s/hostfile_latest",stackdir);
    sprintf(cmdline,"scp -o StrictHostKeyChecking=no -i %s %s root@%s:/root/hostfile >> /dev/null 2>&1",private_key,hostfile,remote_address);
#endif
    system(cmdline);
    return 0;
}

void create_and_get_vaultdir(char* workdir, char* vaultdir){
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    sprintf(vaultdir,"%s\\vault",workdir);
    sprintf(cmdline,"mkdir %s > nul 2>&1",vaultdir);
    system(cmdline);
    sprintf(cmdline,"attrib +h +s +r %s",vaultdir);
    system(cmdline);
#else
    sprintf(vaultdir,"%s/vault",workdir);
    sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",vaultdir);
    system(cmdline);
#endif
}

int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes){
    if(strcmp(exec_type,"connect")!=0&&strcmp(exec_type,"all")!=0&&strcmp(exec_type,"clear")!=0){
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

#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    file_p=fopen(filename_temp,"r");
    if(file_p==NULL){
        return 1;
    }
    fgetline(file_p,remote_address);
    fclose(file_p);
    
#ifdef _WIN32
    sprintf(private_key,"%s\\now-cluster-login",sshkey_folder);
#else
    sprintf(private_key,"%s/now-cluster-login",sshkey_folder);
#endif

    if(strcmp(exec_type,"clear")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr clear\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else if(strcmp(exec_type,"connect")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr connect\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else if(strcmp(exec_type,"all")==0){
        sprintf(cmdline,"ssh -o StrictHostKeyChecking=no -i %s root@%s \"echo \"hpcmgr all\" | at now + %d minutes\"",private_key,remote_address,delay_minutes);
        return system(cmdline);
    }
    else{
        return -1;
    }
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
        printf("[ FATAL: ] Failed to get the crypto key. Exit now.\n");
        return -1;
    }
    sprintf(cmdline,"%s decrypt %s %s.dat %s", now_crypto_exec, secret_file, secret_file, md5);
    system(cmdline);
    sprintf(decrypted_file_name,"%s.dat",secret_file);
    decrypted_file=fopen(decrypted_file_name,"r");
    if(decrypted_file==NULL){
        return -1;
    }
    fscanf(decrypted_file,"%s\n%s\n%s",ak,sk,cloud_flag);
    fclose(decrypted_file);
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s > nul 2>&1", decrypted_file_name);
#else
    sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1", decrypted_file_name);
#endif
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
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf",stackdir);
#endif
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

int decrypt_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_base.tf.tmp %s\\hpc_stack_base.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/hpc_stack_base.tf.tmp %s/hpc_stack_base.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\terraform.tfstate.tmp %s\\terraform.tfstate %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/terraform.tfstate.tmp %s/terraform.tfstate %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate.backup.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate.backup.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
        get_crypto_key(crypto_key_filename,md5sum);
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\terraform.tfstate.backup.tmp %s\\terraform.tfstate.backup %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/terraform.tfstate.backup.tmp %s/terraform.tfstate.backup %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_master.tf.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_master.tf.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_master.tf.tmp %s\\hpc_stack_master.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/hpc_stack_master.tf.tmp %s/hpc_stack_master.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_database.tf.tmp %s\\hpc_stack_database.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/hpc_stack_database.tf.tmp %s/hpc_stack_database.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf.tmp",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf.tmp",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s decrypt %s\\hpc_stack_natgw.tf.tmp %s\\hpc_stack_natgw.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#else
        sprintf(cmdline,"%s decrypt %s/hpc_stack_natgw.tf.tmp %s/hpc_stack_natgw.tf %s",now_crypto_exec,stackdir,stackdir,md5sum);
#endif
        system(cmdline);
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    compute_node_num=get_compute_node_num(filename_temp,"all");
    for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf.tmp",stackdir,i);
#else
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf.tmp",stackdir,i);
#endif
        if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
            sprintf(cmdline,"%s decrypt %s\\hpc_stack_compute%d.tf.tmp %s\\hpc_stack_compute%d.tf %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
#else
            sprintf(cmdline,"%s decrypt %s/hpc_stack_compute%d.tf.tmp %s/hpc_stack_compute%d.tf %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
#endif
            system(cmdline);
        }
    }
    return 0;
}

int delete_decrypted_files(char* workdir, char* crypto_key_filename){
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char md5sum[33]="";
    char* now_crypto_exec=NOW_CRYPTO_EXEC;
    char stackdir[DIR_LENGTH]="";
    int compute_node_num=0;
    int i;
    create_and_get_stackdir(workdir,stackdir);
    get_crypto_key(crypto_key_filename,md5sum);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_base.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_base.tf",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_base.tf %s\\hpc_stack_base.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/hpc_stack_base.tf %s/hpc_stack_base.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\terraform.tfstate %s\\terraform.tfstate.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/terraform.tfstate %s/terraform.tfstate.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\terraform.tfstate.backup",stackdir);
#else
    sprintf(filename_temp,"%s/terraform.tfstate.backup",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\terraform.tfstate.backup %s\\terraform.tfstate.backup.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/terraform.tfstate.backup %s/terraform.tfstate.backup.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_master.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_master.tf",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_master.tf %s\\hpc_stack_master.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/hpc_stack_master.tf %s/hpc_stack_master.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_database.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_database.tf",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_database.tf %s\\hpc_stack_database.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/hpc_stack_database.tf %s/hpc_stack_database.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }

#ifdef _WIN32
    sprintf(filename_temp,"%s\\hpc_stack_natgw.tf",stackdir);
#else
    sprintf(filename_temp,"%s/hpc_stack_natgw.tf",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
#ifdef _WIN32
        sprintf(cmdline,"%s encrypt %s\\hpc_stack_natgw.tf %s\\hpc_stack_natgw.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s",filename_temp);
        system(cmdline);
#else
        sprintf(cmdline,"%s encrypt %s/hpc_stack_natgw.tf %s/hpc_stack_natgw.tf.tmp %s",now_crypto_exec,stackdir,stackdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s",filename_temp);
        system(cmdline);
#endif
    }
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    if(file_exist_or_not(filename_temp)==0){
        compute_node_num=get_compute_node_num(filename_temp,"all");
    }
    for(i=1;i<compute_node_num+1;i++){
#ifdef _WIN32
        sprintf(filename_temp,"%s\\hpc_stack_compute%d.tf",stackdir,i);
        if(file_exist_or_not(filename_temp)==0){
            sprintf(cmdline,"%s encrypt %s\\hpc_stack_compute%d.tf %s\\hpc_stack_compute%d.tf.tmp %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
            system(cmdline);
            sprintf(cmdline,"del /f /q %s",filename_temp);
            system(cmdline);
        }
#else
        sprintf(filename_temp,"%s/hpc_stack_compute%d.tf",stackdir,i);
        if(file_exist_or_not(filename_temp)==0){
            sprintf(cmdline,"%s encrypt %s/hpc_stack_compute%d.tf %s/hpc_stack_compute%d.tf.tmp %s",now_crypto_exec,stackdir,i,stackdir,i,md5sum);
            system(cmdline);
            sprintf(cmdline,"rm -rf %s",filename_temp);
            system(cmdline);
        }
#endif
    }
    return 0;
}

int getstate(char* workdir, char* crypto_filename){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cloud_flag[8]="";
    char buffer1[64]="";
    char buffer2[64]="";
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
#ifdef _WIN32
    sprintf(filename_tfstate,"%s\\terraform.tfstate",stackdir);
#else
    sprintf(filename_tfstate,"%s/terraform.tfstate",stackdir);
#endif
    file_p_tfstate=fopen(filename_tfstate,"r");
    if(file_p_tfstate==NULL){
        return -1;
    }
#ifdef _WIN32
    sprintf(filename_currentstate,"%s\\currentstate",stackdir);
        if(file_exist_or_not(filename_currentstate)==0){
            sprintf(cmdline,"del /f /q %s > nul 2>&1",filename_currentstate);
            system(cmdline);
        }
        sprintf(cmdline,"type nul > %s",filename_currentstate);
        system(cmdline);
        sprintf(filename_temp,"%s\\.secrets.txt",vaultdir);
        sprintf(filename_hostfile,"%s\\hostfile_latest",stackdir);
#else
        sprintf(filename_currentstate,"%s/currentstate",stackdir);
        if(file_exist_or_not(filename_currentstate)==0){
            sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename_currentstate);
            system(cmdline);
        }
        sprintf(cmdline,"echo \"\" > %s",filename_currentstate);
        system(cmdline);
        sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
        sprintf(filename_hostfile,"%s/hostfile_latest",stackdir);
#endif
    get_ak_sk(filename_temp,crypto_filename,buffer1,buffer2,cloud_flag);
    file_p_currentstate=fopen(filename_currentstate,"w+");
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
#ifdef _WIN32
    if(folder_exist_or_not(sshkey_folder)!=0){
        sprintf(cmdline,"mkdir %s > nul 2>&1",sshkey_folder);
        if(system(cmdline)!=0){
            return -1;
        }
    }
    sprintf(cmdline,"attrib +h +s +r %s",sshkey_folder);
    system(cmdline);
    sprintf(filename_temp,"%s\\now-cluster-login",sshkey_folder);
    sprintf(filename_temp2,"%s\\now-cluster-login.pub",sshkey_folder);
#else
    if(folder_exist_or_not(sshkey_folder)!=0){
        sprintf(cmdline,"mkdir -p %s >> /dev/null 2>&1",sshkey_folder);
        if(system(cmdline)!=0){
            return -1;
        }
    }
    sprintf(filename_temp,"%s/now-cluster-login",sshkey_folder);
    sprintf(filename_temp2,"%s/now-cluster-login.pub",sshkey_folder);
#endif
    if(file_exist_or_not(filename_temp)==0&&file_exist_or_not(filename_temp2)==0){
        file_p=fopen(filename_temp2,"r");
        fgetline(file_p,pubkey);
        fclose(file_p);
        return 0;
    }
    else{
#ifdef _WIN32
            sprintf(cmdline,"del /f /q %s\\now-cluster-login* > nul 2>&1",sshkey_folder);
            system(cmdline);
            sprintf(cmdline,"ssh-keygen -t rsa -N \"\" -f %s\\now-cluster-login -q",sshkey_folder);
            system(cmdline);
            sprintf(filename_temp,"%s\\now-cluster-login.pub",sshkey_folder);
#else
            sprintf(cmdline,"rm -rf %s/now-cluster-login* >> /dev/null 2>&1",sshkey_folder);
            system(cmdline);
            sprintf(cmdline,"ssh-keygen -t rsa -N \"\" -f %s/now-cluster-login -q",sshkey_folder);
            system(cmdline);
            sprintf(filename_temp,"%s/now-cluster-login.pub",sshkey_folder);
#endif
        file_p=fopen(filename_temp,"r");
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

#ifdef _WIN32
    sprintf(cmdline,"%s decrypt %s\\_CLUSTER_SUMMARY.txt %s\\_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
#else
    sprintf(cmdline,"%s decrypt %s/_CLUSTER_SUMMARY.txt %s/_CLUSTER_SUMMARY.txt.tmp %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
#endif
    system(cmdline);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#else
    sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
#endif
    find_and_get(filename_temp,"Master","Node","IP:",1,"Master","Node","IP:",' ',4,master_address_prev);

#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    file_p=fopen(filename_temp,"r");
    fgetline(file_p,master_address);
    fclose(file_p);
#ifdef _WIN32
    if(strcmp(master_address,master_address_prev)!=0){
        sprintf(filename_temp,"%s\\_CLUSTER_SUMMARY.txt.tmp",vaultdir);
        global_replace(filename_temp,master_address_prev,master_address);
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt > nul 2>&1",vaultdir);
        system(cmdline);
        sprintf(cmdline,"%s encrypt %s\\_CLUSTER_SUMMARY.txt.tmp %s\\_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
        system(cmdline);
    }
    else{
        sprintf(cmdline,"del /f /q %s\\_CLUSTER_SUMMARY.txt.tmp > nul 2>&1",vaultdir);
        system(cmdline);
    }
#else
    if(strcmp(master_address,master_address_prev)!=0){
        sprintf(filename_temp,"%s/_CLUSTER_SUMMARY.txt.tmp",vaultdir);
        global_replace(filename_temp,master_address_prev,master_address);
        sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt >> /dev/null 2>&1",vaultdir);
        system(cmdline);
        sprintf(cmdline,"%s encrypt %s/_CLUSTER_SUMMARY.txt.tmp %s/_CLUSTER_SUMMARY.txt %s",now_crypto_exec,vaultdir,vaultdir,md5sum);
        system(cmdline);
        sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt.tmp >> /dev/null 2>&1",vaultdir);
        system(cmdline);
    }
    else{
        sprintf(cmdline,"rm -rf %s/_CLUSTER_SUMMARY.txt.tmp >> /dev/null 2>&1",vaultdir);
        system(cmdline);
    }
#endif
    return 0;
}

void archive_log(char* logdir, char* logfile){
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    if(strlen(logfile)==0){
        sprintf(cmdline,"type %s\\tf_prep.log >> %s\\tf_prep_archive.log 2>nul",logdir,logdir);
    }
    else{
        sprintf(cmdline,"type %s >> %s\\tf_prep_archive.log 2>nul",logfile,logdir);
    }
#else
    if(strlen(logfile)==0){
        sprintf(cmdline,"cat %s/tf_prep.log >> %s/tf_prep_archive.log 2>/dev/null",logdir,logdir);
    }
    else{
        sprintf(cmdline,"cat %s >> %s/tf_prep_archive.log 2>/dev/null",logfile,logdir);
    }
#endif
    system(cmdline);
}

void update_compute_template(char* stackdir, char* cloud_flag){
    char cmdline[CMDLINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
#ifdef _WIN32
    sprintf(filename_temp,"%s\\compute_template",stackdir);
    sprintf(cmdline,"copy /y %s\\hpc_stack_compute1.tf %s > nul 2>&1",stackdir,filename_temp);
#else
    sprintf(filename_temp,"%s/compute_template",stackdir);
    sprintf(cmdline,"/bin/cp %s/hpc_stack_compute1.tf %s/compute_template >> /dev/null 2>&1",stackdir,filename_temp);
#endif
    system(cmdline);
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

int wait_for_complete(char* workdir, char* option, char* errorlog){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    int i=0;
    int total_minutes=0;
    char* annimation="\\|/-";

    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(logdir,"%s\\log\\",workdir);
#else
    sprintf(logdir,"%s/log/",workdir);
#endif

    if(strcmp(option,"init")==0){
#ifdef _WIN32
        sprintf(cmdline,"type %s\\tf_prep.log | findstr successfully | findstr initialized! > nul 2>&1",logdir);
#else
        sprintf(cmdline,"cat %s/tf_prep.log | grep \"successfully initialized!\" >> /dev/null 2>&1",logdir);
#endif
        total_minutes=1;
    }
    else{
#ifdef _WIN32
        sprintf(cmdline,"type %s\\tf_prep.log | findstr complete! > nul 2>&1",logdir);
#else
        sprintf(cmdline,"cat %s/tf_prep.log | grep \"complete!\" >> /dev/null 2>&1",logdir);
#endif
        total_minutes=3;
    }

    while(system(cmdline)!=0&&i<MAXIMUM_WAIT_TIME){
        fflush(stdin);
        printf("[ -WAIT- ] This may need %d min(s). %d sec(s) passed ... (%c)\r",total_minutes,i,*(annimation+i%4));
        fflush(stdout);
        i++;
        sleep(1);
        if(file_empty_or_not(errorlog)>0){
            printf("\n");
            return 127;
        }
    }
    if(i==MAXIMUM_WAIT_TIME){
        printf("\n");
        return 1;
    }
    else{
        printf("\n");
        return 0;
    }
}

int graph(char* workdir, char* crypto_keyfile, int graph_level){
    if(getstate(workdir,crypto_keyfile)!=0){
        return -1;
    }
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
    char cloud_flag_file[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char ht_status[16]="";
    int node_num=0;
    int running_node_num=0;
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(currentstate,"%s\\currentstate",stackdir);
    sprintf(compute_template,"%s\\compute_template",stackdir);
    sprintf(cloud_flag_file,"%s\\conf\\cloud_flag.flg",workdir);
#else
    sprintf(currentstate,"%s/currentstate",stackdir);
    sprintf(compute_template,"%s/compute_template",stackdir);
    sprintf(cloud_flag_file,"%s/conf/.cloud_flag.flg",workdir);
#endif
    FILE* file_p=fopen(currentstate,"r");
    if(file_p==NULL){
        return 1;
    }
    FILE* file_p_2=fopen(compute_template,"r");
    if(file_p_2==NULL){
        fclose(file_p);
        return 1;
    }
    fclose(file_p_2);
    FILE* file_p_3=fopen(cloud_flag_file,"r");
    if(file_p_3==NULL){
        fclose(file_p);
        fclose(file_p_2);
        return 1;
    }
    fgetline(file_p_3,cloud_flag);
    fclose(file_p_3);
    fgetline(file_p,master_address);
    fgetline(file_p,line_buffer);
    fgetline(file_p,master_status);
    fgetline(file_p,db_status);
    find_and_get(compute_template,"instance_type","","",1,"instance_type","","",'.',3,compute_config);
    if(find_multi_keys(compute_template,"cpu_threads_per_core = 1","","","","")!=0){
        strcpy(ht_status,"*HT-OFF*");
    }
    sprintf(master_tf,"%s/hpc_stack_master.tf",stackdir);
    find_and_get(master_tf,"instance_type","","",1,"instance_type","","",'.',3,master_config);
    if(graph_level==0){
        printf("|          +-master(%s,%s,%s)\n",master_address,master_status,master_config);
        printf("|            +-db(%s)\n",db_status);
    }
    while(fgetline(file_p,compute_address)==0){
        fgetline(file_p,compute_status);
        node_num++;
        if(strcmp(compute_status,"running")==0||strcmp(compute_status,"Running")==0||strcmp(compute_status,"RUNNING")==0){
            running_node_num++;
        }
        if(graph_level==0){
            if(strlen(ht_status)!=0){
                printf("|              +-compute%d(%s,%s,%s,%s)\n",node_num,compute_address,compute_status,compute_config,ht_status);
            }
            else{
                printf("|              +-compute%d(%s,%s,%s)\n",node_num,compute_address,compute_status,compute_config);
            }
        }
    }
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
    fclose(file_p);
    return 0;
}

int cluster_empty_or_not(char* workdir){
    char filename_temp[FILENAME_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
    if(file_exist_or_not(filename_temp)!=0||file_empty_or_not(filename_temp)==0){
        return 0;
    }
    return 1;
}

int cluster_asleep_or_not(char* workdir){
    char stackdir[DIR_LENGTH]="";
    char master_state[32]="";
    char buffer[32]="";
    int i;
    create_and_get_stackdir(workdir,stackdir);
    FILE* file_p=NULL;
    char filename_temp[FILENAME_LENGTH]="";
#ifdef _WIN32
    sprintf(filename_temp,"%s\\currentstate",stackdir);
#else
    sprintf(filename_temp,"%s/currentstate",stackdir);
#endif
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

int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, char* error_log){
    char cmdline[CMDLINE_LENGTH]="";
    char stackdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    create_and_get_stackdir(workdir,stackdir);
#ifdef _WIN32
    sprintf(logdir,"%s\\log\\",workdir);
#else
    sprintf(logdir,"%s/log/",workdir);
#endif
    archive_log(logdir,"");
#ifdef _WIN32
    sprintf(cmdline,"cd %s\\ && echo yes | start /b %s %s > %s\\tf_prep.log 2>%s",stackdir,tf_exec,execution_name,logdir,error_log);
#else
    sprintf(cmdline,"cd %s && echo yes | %s %s > %s/tf_prep.log 2>%s &",stackdir,tf_exec,execution_name,logdir,error_log);
#endif
    system(cmdline);
    printf("[ -INFO- ] Do not terminate this process manually. Max Exec Time: %d s\n",MAXIMUM_WAIT_TIME);
    printf("|          Operation Command: %s. Error log: %s\n",execution_name,OPERATION_ERROR_LOG);
    wait_for_complete(workdir,execution_name,error_log);
    if(file_empty_or_not(error_log)!=0){
        printf("[ FATAL: ] Failed to operate the cluster. Operation command: %s. Exit now.\n",execution_name);
        archive_log(logdir,error_log);
        delete_decrypted_files(workdir,crypto_keyfile);
        return -1;
    }
    return 0;
}