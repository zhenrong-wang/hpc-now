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
#include <time.h>

#ifdef _WIN32
#include <malloc.h>
#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#elif __APPLE__
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "cluster_general_funcs.h"
#include "cluster_init.h"
#include "cluster_operations.h"
#include "general_funcs.h"
#include "general_print_info.h"
#include "components.h"
#include "prereq_check.h"
#include "time_process.h"
#include "usage_and_logs.h"

char url_code_root_var[LOCATION_LENGTH]="";
char url_tf_root_var[LOCATION_LENGTH]="";
char url_shell_scripts_var[LOCATION_LENGTH]="";
char url_now_crypto_var[LOCATION_LENGTH]="";
int tf_loc_flag_var=0;
int code_loc_flag_var=0;
int now_crypto_loc_flag_var=0;

char terraform_version_var[16]="";
char ali_tf_plugin_version_var[16]="";
char qcloud_tf_plugin_version_var[16]="";
char aws_tf_plugin_version_var[16]="";

char md5_tf_exec_var[64]="";
char md5_tf_zip_var[64]="";
char md5_now_crypto_var[64]="";
char md5_ali_tf_var[64]="";
char md5_ali_tf_zip_var[64]="";
char md5_qcloud_tf_var[64]="";
char md5_qcloud_tf_zip_var[64]="";
char md5_aws_tf_var[64]="";
char md5_aws_tf_zip_var[64]="";

/* 
 * Return values:
 * 0   - normal exit
 * -1  - current user is not hpc-now
 * -2  - Key folder is missing
 * -3  - internet check failed
 * -4  - prerequisition check failed
 * -5  - reset location failed
 * -6  - command incorrect
 * 
 *  ... Still working on defining return values ... Should be much more organized in the future. 
 * 
 * -127 - *Probably* key file not found.
 */

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char buffer1[64];
    char buffer2[64];
    char cloud_flag[16];
    int run_flag=0;
    int current_cluster_flag=0;
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char* syserror_log=SYSTEM_CMD_ERROR_LOG;
    char string_temp[128]="";
    char current_cluster_name[CLUSTER_ID_LENGTH_MAX]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    print_header();

#ifdef _WIN32
    if(check_current_user()!=0){
        printf("[ FATAL: ] You *MUST* switch to the user 'hpc-now' to operate cloud clusters.\n");
        printf("|          Please switch to the user 'hpc-now' by ctrl+alt+delete and then:\n");
        printf("|          1. Run CMD by typing cmd in the Windows Search box\n");
        printf("|          2. hpcopr ls-clusters   (You will see all the clusters)\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -1;
    }
#else
    if(check_current_user()!=0){
        printf("[ FATAL: ] You *MUST* switch to the user 'hpc-now' to operate cloud clusters.\n");
        printf("|          Please run the commands below:\n");
        printf("|          1. su hpc-now   (You will be asked to input password without echo)\n");
        printf("|          2. hpcopr ls-clusters   (You will see all the clusters)\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -1;
    }
#endif

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf("[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -2;
    }
#elif __APPLE__
    if(folder_exist_or_not("/Applications/.hpc-now/")!=0){
        printf("[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -2;
    }
#elif __linux__
    if(folder_exist_or_not("/usr/.hpc-now/")!=0){
        printf("[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -2;
    }
#endif
    if(folder_exist_or_not(GENERAL_CONF_DIR)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,GENERAL_CONF_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    if(argc==1){
        print_help();
        return 0;
    }

    if(strcmp(argv[1],"help")==0){
        print_help();
        return 0;
    }

    if(strcmp(argv[1],"about")==0){
        print_about();
        return 0;
    }

    if(check_internet()!=0){
        print_tail();
        write_log("NULL",operation_log,"INTERNET_FAILED",-3);
        system_cleanup();
        return -3;
    }

    if(strcmp(argv[1],"license")==0){
        read_license();
        print_tail();
        return 0;
    }

    if(strcmp(argv[1],"repair")==0){
        printf("[ -INFO- ] Entering repair mode. All the locations will be reset to default,\n");
        printf("|          and all the core components will be replaced by the default ones.\n");
        printf("|          Would you like to continue? Only 'y-e-s' is accepted to confirm.\n");
        printf("[ INPUT: ] ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf("\n[ -INFO- ] Only 'y-e-s' is accepted to continue. You chose to deny this operation.\n");
            printf("|          Nothing changed. Exit now.\n");
            print_tail();
            return 0;
        }
        run_flag=check_and_install_prerequisitions(1);
        if(run_flag==3){
            write_log("NULL",operation_log,"PREREQ_FAILED",-4);
            print_tail();
            system_cleanup();
            return -4;
        }
        else if(run_flag!=0){
            print_tail();
            system_cleanup();
            return -4;
        }
        else{
            print_tail();
            system_cleanup();
            return 0;
        }
    }

    if(strcmp(argv[1],"envcheck")==0){
        run_flag=check_and_install_prerequisitions(0);
        print_tail();
        system_cleanup();
        if(run_flag!=0){
            return -4;
        }
        return 0;
    }

    if(strcmp(argv[1],"configloc")==0){
        run_flag=configure_locations();
        print_tail();
        system_cleanup();
        if(run_flag!=0){
            return -5;
        }
        return 0;
    }

    if(strcmp(argv[1],"resetloc")==0){
        run_flag=reset_locations();
        if(run_flag==0){
            printf("[ -INFO- ] The locations have been reset to the default.\n");
            show_locations();
            print_tail();
            system_cleanup();
            return 0;
        }
        else{
            printf("[ FATAL: ] Internal error, failed to reset the locations.\n");
            print_tail();
            system_cleanup();
            return -5;
        }
    }

    if(strcmp(argv[1],"showloc")==0){
        run_flag=show_locations();
        print_tail();
        system_cleanup();
        if(run_flag!=0){
            return -127;
        }
        return 0;
    }

    if(strcmp(argv[1],"showmd5")==0){
        run_flag=show_vers_md5vars();
        print_tail();
        system_cleanup();
        if(run_flag!=0){
            return -127;
        }
        return 0;
    }

    run_flag=check_and_install_prerequisitions(0);
    if(run_flag==3){
        write_log("NULL",operation_log,"PREREQ_FAILED",-4);
        print_tail();
        system_cleanup();
        return -4;
    }
    else if(run_flag!=0){
        print_tail();
        system_cleanup();
        return -4;
    }
    
    if(strcmp(argv[1],"new-cluster")!=0&&strcmp(argv[1],"ls-clusters")!=0&&strcmp(argv[1],"switch")!=0&&strcmp(argv[1],"glance")!=0&&strcmp(argv[1],"exit-current")!=0&&strcmp(argv[1],"refresh")!=0&&strcmp(argv[1],"remove")!=0&&strcmp(argv[1],"usage")!=0&&strcmp(argv[1],"syserr")!=0&&strcmp(argv[1],"history")!=0&&strcmp(argv[1],"new-keypair")!=0&&strcmp(argv[1],"init")!=0&&strcmp(argv[1],"get-conf")!=0&&strcmp(argv[1],"edit-conf")!=0&&strcmp(argv[1],"vault")!=0&&strcmp(argv[1],"graph")!=0&&strcmp(argv[1],"delc")!=0&&strcmp(argv[1],"addc")!=0&&strcmp(argv[1],"shutdownc")!=0&&strcmp(argv[1],"turnonc")!=0&&strcmp(argv[1],"reconfc")!=0&&strcmp(argv[1],"reconfm")!=0&&strcmp(argv[1],"sleep")!=0&&strcmp(argv[1],"wakeup")!=0&&strcmp(argv[1],"destroy")!=0&&strcmp(argv[1],"ssh")!=0&&strcmp(argv[1],"rebuild")!=0){
        print_help();
        return -6;
    }

    if(strcmp(argv[1],"new-cluster")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        if(argc==2){
            run_flag=create_new_cluster(crypto_keyfile,"","","");
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            if(run_flag==-1){
                return -127;
            }
            else if(run_flag==1){
                return -7;
            }
            return run_flag;
        }
        else if(argc==3||argc==4){
            run_flag=create_new_cluster(crypto_keyfile,argv[2],"","");
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=create_new_cluster(crypto_keyfile,argv[2],argv[3],argv[4]);
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
    }
    if(strcmp(argv[1],"ls-clusters")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        run_flag=list_all_cluster_names();
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"glance")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        if(argc<3){
            run_flag=glance_clusters("",crypto_keyfile);
        }
        else{
            run_flag=glance_clusters(argv[2],crypto_keyfile);
        }
        if(run_flag==1){
            printf("[ FATAL: ] Please swith to a cluster first, or specify one to glance.\n");
        }
        else if(run_flag==-1){
            printf("[ FATAL: ] The specified cluster name %s is not in the registry.\n",argv[2]);
        }
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"refresh")==0){
        if(show_current_cluster(workdir,current_cluster_name,2)==1){
            printf("[ FATAL: ] Please swith to a cluster first, or specify one to refresh:\n");
            list_all_cluster_names();
            print_tail();
            system_cleanup();
            return -9;
        }
        if(cluster_empty_or_not(workdir)==0){
            printf("[ FATAL: ] The cluster cannot be refreshed (either in operation progress or empty).\n");
            printf("|          Please run 'hpcopr glance all' to check. Exit now.\n");
            print_tail();
            write_log("NULL",operation_log,argv[1],-9);
            system_cleanup();
            return -9;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            return -1;
        }
        if(argc<3){
            run_flag=refresh_cluster("",crypto_keyfile);
        }
        else{
            run_flag=refresh_cluster(argv[2],crypto_keyfile);
        }
        if(run_flag==1){
            printf("[ FATAL: ] Please swith to a cluster first, or specify one to refresh:\n");
            list_all_cluster_names();
        }
        else if(run_flag==-3){
            printf("[ FATAL: ] The current cluster is in operation progress and cannot be refreshed.\n");
        }
        else if(run_flag==3){
            printf("[ FATAL: ] The cluster %s is in operation progress and cannot be refreshed.\n",argv[2]);
        }
        else if(run_flag==5){
            printf("[ FATAL: ] Refreshing operation failed. Exit now.\n");
        }
        else if(run_flag==7){
            printf("[ FATAL: ] The specified cluster name %s is not in the registry.\n",argv[2]);
            list_all_cluster_names();
        }
        else{
            printf("[ -DONE- ] The cluster was successfully refreshed.\n");
        }
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"exit-current")==0){
        current_cluster_flag=show_current_cluster(workdir,current_cluster_name,2);
        if(current_cluster_flag!=0){
            print_tail();
            write_log("NULL",operation_log,argv[1],current_cluster_flag);
            system_cleanup();
            return current_cluster_flag;
        }
        if(exit_current_cluster()==0){
            printf("[ -INFO- ] Exit the current cluster %s.\n",current_cluster_name);
        }
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"usage")==0){
        run_flag=get_usage(usage_log);
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"history")==0){
        run_flag=get_history(operation_log);
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"syserr")==0){
        run_flag=get_syserrlog(syserror_log);
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    current_cluster_flag=show_current_cluster(workdir,current_cluster_name,1);
    if(strcmp(argv[1],"switch")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify which cluster to switch to.\n");
            run_flag=list_all_cluster_names();
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
        run_flag=switch_to_cluster(argv[2]);
        print_tail();
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"remove")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify which cluster to be removed.\n");
            run_flag=list_all_cluster_names();
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=remove_cluster(argv[2],crypto_keyfile);
            print_tail();
            write_log("NULL",operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
    }
    if(current_cluster_flag==1){
        run_flag=list_all_cluster_names();
        print_tail();
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"graph")==0){
        if(check_pslock(workdir)!=0){
            printf("[ -WARN- ] %s | * OPERATION-IN-PROGRESS * The graph here is *NOT* updated !\n|\n",current_cluster_name);
            run_flag=graph(workdir,crypto_keyfile,0);
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
        decrypt_files(workdir,crypto_keyfile);
        printf("|\n");
        run_flag=graph(workdir,crypto_keyfile,0);
        if(run_flag!=0){
            print_empty_cluster_info();
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        print_tail();
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"vault")==0){
        run_flag=get_vault_info(workdir,crypto_keyfile);
        if(run_flag==-1){
            print_empty_cluster_info();
        }
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"new-keypair")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            return -1;
        }
        if(argc==2||argc==3){
            run_flag=rotate_new_keypair(workdir,"","",crypto_keyfile);
        }
        else{
            run_flag=rotate_new_keypair(workdir,argv[2],argv[3],crypto_keyfile);
        }
        if(run_flag!=0){
            write_log(current_cluster_name,operation_log,"new keypair",-1);
            system_cleanup();
            print_tail();
            return -1;
        }
        write_log(current_cluster_name,operation_log,"new keypair",0);
        print_tail();
        system_cleanup();
        return 0;
    }

    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%s.secrets.txt",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag)!=0){
        printf("[ FATAL: ] Failed to get the key file. Have you switched to any cluster?\n");
        printf("|          Exit now.\n");
        print_tail();
        write_log(current_cluster_name,operation_log,"KEY_CHECK_FAILED",5);
        system_cleanup();
        return 5;
    }
    if(check_pslock(workdir)==1){
        printf("[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now.\n");
        print_tail();
        write_log(current_cluster_name,operation_log,"PROCESS_LOCKED",7);
        system_cleanup();
        return 7;
    }
    if(strcmp(argv[1],"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf("[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,"CLUSTER_NOT_EMPTY",23);
            system_cleanup();
            return 23;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            return -1;
        }
        run_flag=get_default_conf(workdir,crypto_keyfile,1);
        if(run_flag==1||run_flag==127){
            printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,"INTERNAL_ERROR",31);
            system_cleanup();
            return 31;
        }
        else{
            printf("[ -INFO- ] The default configuration file has been downloaded to the local place.\n");
            printf("|          You can init directly, or edit it before init. Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],0);
            system_cleanup();
            return 0;
        }
    }
    if(strcmp(argv[1],"edit-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf("[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,"CLUSTER_NOT_EMPTY",23);
            system_cleanup();
            return 23;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            return -1;
        }
        run_flag=edit_configuration_file(workdir,crypto_keyfile);
        if(run_flag==1){
            printf("[ FATAL: ] No configuration file found. Please run the command 'hpcopr get-conf' first.\n");
            printf("|          Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],-1);
            system_cleanup();
            return 1;
        }
        else{
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],0);
            system_cleanup();
            return 0;
        }
    }
    if(strcmp(argv[1],"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf("[ FATAL: ] The cluster has already been initialized. Exit now.\n");
            print_tail();
            system_cleanup();
            return -1;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            run_flag=aws_cluster_init(current_cluster_name,workdir,crypto_keyfile);
            write_log(current_cluster_name,operation_log,argv[1],run_flag);
            print_tail();
            system_cleanup();
            return run_flag;
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            run_flag=qcloud_cluster_init(current_cluster_name,workdir,crypto_keyfile);
            write_log(current_cluster_name,operation_log,argv[1],run_flag);
            print_tail();
            system_cleanup();
            return run_flag;
        }
        else if(strcmp(cloud_flag,"CLOUD_A")==0){
            run_flag=alicloud_cluster_init(current_cluster_name,workdir,crypto_keyfile);
            write_log(current_cluster_name,operation_log,argv[1],run_flag);
            print_tail();
            system_cleanup();
            return run_flag;
        }
        else{
            printf("[ FATAL: ] Unknown Cloud Service Provider. Exit now.\n");
            print_tail();
            system_cleanup();
            return -127;
        }
    }
    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        print_tail();
        write_log(current_cluster_name,operation_log,"EMPTY_CLUSTER",11);
        system_cleanup();
        return 11;
    }

    if(strcmp(argv[1],"rebuild")==0){
        if(argc>2){
            if(strcmp(argv[2],"mc")==0){
                run_flag=rebuild_nodes(workdir,crypto_keyfile,"mc");
            }
            else if(strcmp(argv[2],"mcdb")==0){
                run_flag=rebuild_nodes(workdir,crypto_keyfile,"mcdb");
            }
            else if(strcmp(argv[2],"all")==0){
                run_flag=rebuild_nodes(workdir,crypto_keyfile,"all");
            }
            else{
                printf("[ FATAL: ] Please specify 'mc', 'mcdb', or 'all' as the second parameter.\n");
                printf("|          Run 'hpcopr help' for more details. Exit now.\n");
            }
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            printf("[ FATAL: ] Please specify 'mc', 'mcdb', or 'all' as the second parameter.\n");
            printf("|          Run 'hpcopr help' for more details. Exit now.\n");
            print_tail();
            system_cleanup();
            write_log(current_cluster_name,operation_log,argv[1],13);
            return 13;
        }
    }

    if(strcmp(argv[1],"sleep")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"wakeup")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        if(argc==2){
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"minimal");
            sprintf(string_temp,"%s default",argv[1]);
            print_tail();
            write_log(current_cluster_name,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=cluster_wakeup(workdir,crypto_keyfile,argv[2]);
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            print_tail();
            write_log(current_cluster_name,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
    }
    if(argc==3&&strcmp(argv[1],"destroy")==0&&strcmp(argv[2],"force")==0){
        run_flag=cluster_destroy(workdir,crypto_keyfile,0);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(argc==2){
        if(strcmp(argv[1],"reconfc")==0||strcmp(argv[1],"reconfm")==0){
            printf("[ -INFO- ] Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir)!=0){
                printf("[ FATAL: ] Internal error. Please submit an issue to the community. Exit now.\n");
                print_tail();
                write_log(current_cluster_name,operation_log,argv[1],-1);
                system_cleanup();
                return -17;
            }
            if(strcmp(argv[1],"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n[ -WARN- ] You need to turn on all the compute nodes before reconfiguring them.\n");
            }
            if(strcmp(argv[1],"reconfm")==0&&cluster_asleep_or_not(workdir)==0){
                printf("|\n[ -WARN- ] You needd to wake up the cluster before reconfiguring the master node.\n");
            }
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        if(strcmp(argv[1],"ssh")==0){
            printf("[ FATAL: ] You need to specify to login with which user. Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
    }

    if(cluster_asleep_or_not(workdir)==0){
        printf("[ FATAL: ] The current cluster is in the state of hibernation. Please wake up\n");
        printf("|          first. Command: hpcopr wakeup minimal/all . Exit now.\n");
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],13);
        system_cleanup();
        return 13;
    }

    if(argc>2&&strcmp(argv[1],"ssh")==0){
        printf("[ -INFO- ] Trying to ssh to the cluster as %s, may fail if the username doesn't exist.\n",argv[2]);
        run_flag=cluster_ssh(workdir,argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"destroy")==0&&cluster_empty_or_not(workdir)==1){
        run_flag=cluster_destroy(workdir,crypto_keyfile,1);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"delc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a number or 'all' as the second parameter.\n");
            printf("|          Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=delete_compute_node(workdir,crypto_keyfile,argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"addc")==0){
        if(check_down_nodes(workdir)!=0){
            printf("[ FATAL: ] You need to turn all compute node(s) on before adding new nodes.\n");
            printf("|          Exit now.\n");
            print_tail();
            system_cleanup();
            return 1;
        }
        if(argc==2){
            printf("[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("|          Exit now.\n");
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"shutdownc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n");
            print_tail();
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"turnonc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n");
            write_log(current_cluster_name,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"reconfc")==0){
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            if(check_down_nodes(workdir)!=0){
                printf("[ FATAL: ] You need to turn all compute node(s) on before reconfiguring them.\n");
                printf("|          Exit now.\n");
                print_tail();
                system_cleanup();
                return 1;
            }
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        if(argc==3){
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],"");
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            print_tail();
            write_log(current_cluster_name,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],argv[3]);
            sprintf(string_temp,"%s %s %s",argv[1],argv[2],argv[3]);
            print_tail();
            write_log(current_cluster_name,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
    }
    if(strcmp(argv[1],"reconfm")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            print_tail();
            system_cleanup();
            return -1;
        }
        run_flag=reconfigure_master_node(workdir,crypto_keyfile,argv[2]);
        sprintf(string_temp,"%s %s",argv[1],argv[2]);
        print_tail();
        write_log(current_cluster_name,operation_log,string_temp,run_flag);
        system_cleanup();
        return run_flag;
    }
    system_cleanup();
    return 0;
}