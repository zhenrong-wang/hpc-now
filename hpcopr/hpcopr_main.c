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
char url_initutils_root_var[LOCATION_LENGTH]="";
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

char commands[COMMAND_NUM][COMMAND_STRING_LENGTH_MAX]={
    "envcheck",
    "new-cluster",
    "ls-clusters",
    "switch",
    "glance",
    "refresh",
    "exit-current",
    "remove",
    "help",
    "usage",
    "history",
    "syserr",
    "ssh",
    "configloc",
    "showloc",
    "resetloc",
    "showmd5",
    "new-keypair",
    "get-conf",
    "edit-conf",
    "init",
    "rebuild",
    "vault",
    "graph",
    "viewlog",
    "delc",
    "addc",
    "shutdownc",
    "turnonc",
    "reconfc",
    "reconfm",
    "sleep",
    "wakeup",
    "destroy",
    "userman",
    "about",
    "version",
    "license",
    "repair"
};

/*
-127 USER_CHECK_ERROR
-125 KEY_FOLDER_ERROR
-123 INTERNET_CHECK_FAILED
1 NOT_A_VALID_COMMAND
3 USER_DENIED
5 LACK_PARAMS
7 MISSING_KEY_FILE
9 PARAM_FORMAT_ERROR

11 Prereq - Components Download and install failed
13 Prereq - Other failed
15 Prereq - Envcheck Failed
17 Prereq - Config Location Failed
19 Prereq - Vers and md5 Error
21 CLUSTER_NAME_CHECK_FAILED
23 INVALID_KEYPAIR
25 Not Operating Clusters
27 EMPTY_CLUSTER_OR_IN_PROGRESS
29 OPERATION_IN_PROGRESS
31 REFRESHING FAILED
33 EMPTY REGISTRY
35 Failed to exit current
37 NO_NEED_TO_SWITCH
39 NOT_IN_THE_REGISTRY
41 DESTROY_ERROR
43 CLUSTER_ASLEEP
45 GRAPH_FAILED
47 GRAPH_NOT_UPDATED
49 CLUSTER_EMPTY
51 CLUSTER_NOT_EMPTY
53 PROCESS_LOCKED
55 NO_CONF_FILE
57 ALREADY_INITED
59 UNKNOWN_CLOUD
61 WORKDIR_NOT_EXIST
63 AWS_REGION_VALID_FAILED
65 GET_FILE_FAILED
67 ZONE_ID_ERROR
69 AWS_INVALID_KEYPAIR
71 TF_INIT_FAILED
73 TF_APPLY_FAILED_ROLLED_BACK
75 TF_ROLLBACK_FAILED
77 USERMAN PREREQ_CHECK_FAILED

123 FATAL_ABNORMAL
125 FATAL_INTERNAL_ERROR
127 File I/O Error
*/

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char command_name_prompt[128]="";
    char buffer1[64];
    char buffer2[64];
    char cloud_flag[16];
    int run_flag=0;
    int usrmgr_check_flag=0;
    int current_cluster_flag=0;
    char workdir[DIR_LENGTH]="";
    char target_workdir[DIR_LENGTH]="";
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
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* switch to the user 'hpc-now' to operate cloud clusters.\n");
        printf("|          Please switch to the user 'hpc-now' by ctrl+alt+delete and then:\n");
        printf("|          1. Run CMD by typing cmd in the Windows Search box\n");
        printf("|          2. hpcopr ls-clusters   (You will see all the clusters)\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return -127;
    }
#else
    if(check_current_user()!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* switch to the user 'hpc-now' to operate cloud clusters.\n");
        printf("|          Please run the commands below:\n");
        printf("|          1. su hpc-now   (You will be asked to input password without echo)\n");
        printf("|          2. hpcopr ls-clusters   (You will see all the clusters)\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return -127;
    }
#endif

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return -125;
    }
#elif __APPLE__
    if(folder_exist_or_not("/Applications/.hpc-now/")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return -125;
    }
#elif __linux__
    if(folder_exist_or_not("/usr/.hpc-now/")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return -125;
    }
#endif
    if(folder_exist_or_not(GENERAL_CONF_DIR)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,GENERAL_CONF_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    if(argc==1){
        print_help("");
        return 0;
    }

    if(command_name_check(argv[1],command_name_prompt)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid Command. Do you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
        print_tail();
        return 1;
    }

    if(strcmp(argv[1],"help")==0){
        if(argc==2){
            print_help("");
        }
        else{
            print_help(argv[2]);
        }
        return 0;
    }

    if(strcmp(argv[1],"about")==0){
        print_about();
        return 0;
    }
    
    if(strcmp(argv[1],"version")==0){
        print_version();
        return 0;
    }

    if(check_internet()!=0){
        write_operation_log("NULL",operation_log,"NULL","INTERNET_CHECK_FAILED",-123);
        check_and_cleanup("");
        return -123;
    }

    if(strcmp(argv[1],"license")==0){
        read_license();
        print_tail();
        return 0;
    }

    if(strcmp(argv[1],"repair")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Entering repair mode. All the locations will be reset to default,\n");
        printf("|          and all the core components will be replaced by the default ones.\n");
        printf("|          Would you like to continue? Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to confirm.\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,"y-e-s")!=0){
            printf("\n[ -INFO- ] Only " WARN_YELLO_BOLD "y-e-s" RESET_DISPLAY " is accepted to continue. You chose to deny this operation.\n");
            printf("|          Nothing changed. Exit now.\n");
            write_operation_log("NULL",operation_log,argv[1],"USER_DENIED",3);
            print_tail();
            return 3;
        }
        run_flag=check_and_install_prerequisitions(1);
        if(run_flag==3){
            write_operation_log("NULL",operation_log,argv[1],"COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",11);
            check_and_cleanup("");
            return 11;
        }
        else if(run_flag!=0){
            write_operation_log("NULL",operation_log,argv[1],"REPAIR_FAILED",run_flag);
            check_and_cleanup("");
            return 13;
        }
        else{
            write_operation_log("NULL",operation_log,argv[1],"REPAIR_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
    }

    if(strcmp(argv[1],"envcheck")==0){
        run_flag=check_and_install_prerequisitions(0);
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argv[1],"ENVCHECK_FAILED",run_flag);
            check_and_cleanup("");
            return 15;
        }
        write_operation_log("NULL",operation_log,argv[1],"ENVCHECK_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"configloc")==0){
        run_flag=configure_locations();
        if(run_flag==1){
            write_operation_log("NULL",operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        else if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"resetloc")==0){
        run_flag=reset_locations();
        if(run_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The locations have been reset to the default.\n");
            show_locations();
            write_operation_log("NULL",operation_log,argv[1],"OPERATION_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal error, failed to reset the locations.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
    }

    if(strcmp(argv[1],"showloc")==0){
        run_flag=show_locations();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"showmd5")==0){
        run_flag=show_vers_md5vars();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    run_flag=check_and_install_prerequisitions(0);
    if(run_flag==3){
        write_operation_log("NULL",operation_log,"INTERNAL_CHECK","COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",-11);
        check_and_cleanup("");
        return 11;
    }
    else if(run_flag==-1){
        write_operation_log("NULL",operation_log,"INTERNAL_CHECK","FILE_I/O_ERROR",127);
        check_and_cleanup("");
        return 127;
    }
    else if(run_flag==-3){
        write_operation_log("NULL",operation_log,"INTERNAL_CHECK","RESET_LOCATION_FAILED",13);
        check_and_cleanup("");
        return 13;
    }
    else if(run_flag==1){
        write_operation_log("NULL",operation_log,argv[1],"USER_DENIED",3);
        check_and_cleanup("");
        return 3;
    }
    else if(run_flag==5){
        write_operation_log("NULL",operation_log,argv[1],"CONFIG_LOCATION_FAILED",17);
        check_and_cleanup("");
        return 17;
    }
    else if(run_flag==7){
        write_operation_log("NULL",operation_log,argv[1],"VERSION_MD5SUM_ERROR",19);
        check_and_cleanup("");
        return 19;
    }

    if(strcmp(argv[1],"new-cluster")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        if(argc==2){
            run_flag=create_new_cluster(crypto_keyfile,"","","","");
        }
        else if(argc==3){
            if(strcmp(argv[2],"echo")==0){
                run_flag=create_new_cluster(crypto_keyfile,"","","","echo");
            }
            else{
                run_flag=create_new_cluster(crypto_keyfile,argv[2],"","","");
            }
        }
        else if(argc==4){
            if(strcmp(argv[3],"echo")==0){
                run_flag=create_new_cluster(crypto_keyfile,argv[2],"","","echo");
            }
            else{
                run_flag=create_new_cluster(crypto_keyfile,argv[2],"","","");
            }
        }
        else if(argc==5){
            if(strcmp(argv[4],"echo")==0){
                run_flag=create_new_cluster(crypto_keyfile,argv[2],"","","echo");
            }
            else{
                run_flag=create_new_cluster(crypto_keyfile,argv[2],argv[3],argv[4],"");
            }
        }
        else{
            if(strcmp(argv[5],"echo")==0){
                run_flag=create_new_cluster(crypto_keyfile,argv[2],argv[3],argv[4],"echo");
            }
            else{
                run_flag=create_new_cluster(crypto_keyfile,argv[2],argv[3],argv[4],"");
            }
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argv[1],"CLUSTER_NAME_CHECK_FAILED",21);
            check_and_cleanup(workdir);
            return 21;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==5){
            write_operation_log("NULL",operation_log,argv[1],"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log("NULL",operation_log,argv[1],"OPERATION_SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"ls-clusters")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        run_flag=list_all_cluster_names();
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argv[1],"EMPTY_REGISTRY",33);
            check_and_cleanup(workdir);
            return 33;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"glance")==0){
        show_current_cluster(workdir,current_cluster_name,2);
        if(argc<3){
            run_flag=glance_clusters("",crypto_keyfile);
        }
        else{
            run_flag=glance_clusters(argv[2],crypto_keyfile);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please swith to a cluster first, or specify one to glance.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,argv[2]);
            write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"refresh")==0){
        if(show_current_cluster(workdir,current_cluster_name,2)==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please swith to a cluster first, or specify one to refresh:\n" RESET_DISPLAY);
            list_all_cluster_names();
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(argc<3){
            run_flag=cluster_empty_or_not(workdir);
        }
        else if(argc==3&&strcmp(argv[2],"force")!=0){
            run_flag=cluster_empty_or_not(workdir);
        }
        else if(argc>3&&strcmp(argv[3],"force")!=0){
            run_flag=cluster_empty_or_not(workdir);
        }
        else{
            run_flag=3;
        }
        if(run_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster cannot be refreshed (either in init progress or empty).\n");
            printf("|          Please run 'hpcopr glance all' to check. Exit now.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"EMPTY_CLUSTER_OR_IN_PROGRESS",27);
            check_and_cleanup(workdir);
            return 27;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(argc<3){
            run_flag=refresh_cluster("",crypto_keyfile,"");
        }
        else if(argc==3){
            if(run_flag==3){
                run_flag=refresh_cluster("",crypto_keyfile,"force");
            }
            else{
                run_flag=refresh_cluster("",crypto_keyfile,"");
            }
        }
        else{
            if(run_flag==3){
                run_flag=refresh_cluster(argv[2],crypto_keyfile,"force");
            }
            else{
                run_flag=refresh_cluster(argv[2],crypto_keyfile,"");
            }
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please swith to a cluster first, or specify one to refresh:\n" RESET_DISPLAY);
            list_all_cluster_names();
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==-13){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==13){
            write_operation_log(argv[2],operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==-3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is in operation progress and cannot be refreshed.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"OPERATION_IN_PROGRESS",29);
            check_and_cleanup(workdir);
            return 29;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster %s is in operation progress and cannot be refreshed.\n" RESET_DISPLAY,argv[2]);
            write_operation_log(argv[2],operation_log,argv[1],"OPERATION_IN_PROGRESS",29);
            check_and_cleanup(workdir);
            return 29;
        }
        else if(run_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Refreshing operation failed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"OPERATION_FAILED",31);
            check_and_cleanup(workdir);
            return 31;
        }
        else if(run_flag==5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Refreshing operation failed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(argv[2],operation_log,argv[1],"OPERATION_FAILED",31);
            check_and_cleanup(workdir);
            return 31;
        }
        else if(run_flag==7){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,argv[2]);
            list_all_cluster_names();
            write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        else if(run_flag==2){
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The cluster %s was successfully refreshed.\n",argv[2]);
            write_operation_log(argv[2],operation_log,argv[1],"SUCCEEDED",0);
        }
        else{
            printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The current cluster was successfully refreshed.\n");
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        }
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"exit-current")==0){
        current_cluster_flag=show_current_cluster(workdir,current_cluster_name,2);
        if(current_cluster_flag!=0){
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(exit_current_cluster()==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exit the current cluster %s.\n",current_cluster_name);
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ -INFO- ] Failed to exit the current cluster %s.\n" RESET_DISPLAY,current_cluster_name);
            write_operation_log(current_cluster_name,operation_log,argv[1],"OPERATION_FAILED",35);
            check_and_cleanup("");
            return 35;
        }
    }

    if(strcmp(argv[1],"usage")==0){
        if(argc==2){
            run_flag=view_system_logs(usage_log,"","");
        }
        else if(argc==3){
            if(strcmp(argv[2],"read")!=0&&strcmp(argv[2],"print")!=0){
                run_flag=view_system_logs(usage_log,"",argv[2]);
            }
            else{
                run_flag=view_system_logs(usage_log,argv[2],"");
            }
        }
        else{
            run_flag=view_system_logs(usage_log,argv[2],argv[3]);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }
    if(strcmp(argv[1],"history")==0){
        if(argc==2){
            run_flag=view_system_logs(operation_log,"","");
        }
        else if(argc==3){
            if(strcmp(argv[2],"read")!=0&&strcmp(argv[2],"print")!=0){
                run_flag=view_system_logs(operation_log,"",argv[2]);
            }
            else{
                run_flag=view_system_logs(operation_log,argv[2],"");
            }
        }
        else{
            run_flag=view_system_logs(operation_log,argv[2],argv[3]);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"syserr")==0){
        if(argc==2){
            run_flag=view_system_logs(syserror_log,"","");
        }
        else if(argc==3){
            if(strcmp(argv[2],"read")!=0&&strcmp(argv[2],"print")!=0){
                run_flag=view_system_logs(syserror_log,"",argv[2]);
            }
            else{
                run_flag=view_system_logs(syserror_log,argv[2],"");
            }
        }
        else{
            run_flag=view_system_logs(syserror_log,argv[2],argv[3]);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    current_cluster_flag=show_current_cluster(workdir,current_cluster_name,1);
    if(strcmp(argv[1],"switch")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify which cluster to switch to.\n" RESET_DISPLAY);
            run_flag=list_all_cluster_names();
            write_operation_log("NULL",operation_log,argv[1],"TOO_FEW_PARAMS",5);
            check_and_cleanup("");
            return 5;
        }
        run_flag=switch_to_cluster(argv[2]);
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argv[1],"NO_NEED_TO_SWITCH",37);
            check_and_cleanup("");
            return 37;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"remove")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify which cluster to be removed.\n" RESET_DISPLAY);
            run_flag=list_all_cluster_names();
            write_operation_log("NULL",operation_log,argv[1],"TOO_FEW_PARAMS",5);
            check_and_cleanup("");
            return 5;
        }
        else{
            if(argc>3&&strcmp(argv[3],"force")==0){
                run_flag=remove_cluster(argv[2],crypto_keyfile,"force");
            }
            else{
                run_flag=remove_cluster(argv[2],crypto_keyfile,"");
            }
            if(run_flag==1){
                write_operation_log(argv[2],operation_log,argv[1],"CLUSTER_NAME_CHECK_FAILED",21);
                check_and_cleanup(workdir);
                return 21; 
            }
            else if(run_flag==3){
                write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
                check_and_cleanup(workdir);
                return 39;
            }
            else if(run_flag==5){
                write_operation_log(argv[2],operation_log,argv[1],"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            else if(run_flag==7){
                write_operation_log(argv[2],operation_log,argv[1],"CLUSTER_DESTROY_FAILED",41);
                check_and_cleanup(workdir);
                return 41;
            }
            write_operation_log(argv[2],operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"ssh")==0){
        if(argc==2){
            printf(GENERAL_BOLD "[ -INFO- ] Usage: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr ssh USER_NAME (Optional)CLUSTER_NAME" RESET_DISPLAY GENERAL_BOLD "\n");
            printf("|          A blank CLUSTER_NAME refers to the current cluster.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"TOO_FEW_PARAMS",5);
            check_and_cleanup("");
            return 5;
        }
        else if(argc==3){
            if(current_cluster_flag==1){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify or switch to a cluster first.\n" RESET_DISPLAY);
                list_all_cluster_names();
                write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
                check_and_cleanup(workdir);
                return 25;
            }
            if(cluster_asleep_or_not(workdir)==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the current cluster first.\n" RESET_DISPLAY);
                write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_GREEN_BOLD "%s@%s" RESET_DISPLAY ", may fail if the username is invalid.\n",argv[2],current_cluster_name);
            run_flag=cluster_ssh(workdir,argv[2]);
            if(run_flag==-1){
                write_operation_log(current_cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
                check_and_cleanup("");
                return 127;
            }
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",run_flag);
        }
        else{
            if(cluster_name_check_and_fix(argv[3],string_temp)!=-127){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid" RESET_DISPLAY ".\n",argv[3]);
                list_all_cluster_names();
                write_operation_log("NULL",operation_log,argv[1],"CLUSTER_NAME_CHECK_FAILED",21);
                check_and_cleanup(workdir);
                return 21;
            }
            get_workdir(target_workdir,argv[3]);
            if(cluster_asleep_or_not(target_workdir)==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to switch to and wake up " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,argv[3]);
                write_operation_log(argv[3],operation_log,argv[1],"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_CYAN_BOLD "%s@%s" RESET_DISPLAY ", may fail if the username is invalid.\n",argv[2],argv[3]);
            run_flag=cluster_ssh(target_workdir,argv[2]);
            if(run_flag==-1){
                write_operation_log(current_cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
                check_and_cleanup("");
                return 127;
            }
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",run_flag);
        }
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"viewlog")==0){
        if(argc==2){
            run_flag=view_run_log(workdir,"","","");
        }
        else if(argc==3){
            run_flag=view_run_log(workdir,argv[2],"","");
        }
        else if(argc==4){
            run_flag=view_run_log(workdir,argv[2],argv[3],"");
        }
        else{
            run_flag=view_run_log(workdir,argv[2],argv[3],argv[4]);
        }
        if(run_flag==-1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(current_cluster_name);
            return 127;
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],argv[2],0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(current_cluster_flag==1){
        run_flag=list_all_cluster_names();
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argv[1],"EMPTY_REGISTRY",33);
            check_and_cleanup("");
            return 33;
        }
        write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
        check_and_cleanup("");
        return 25;
    }

    if(strcmp(argv[1],"graph")==0){
        if(check_pslock(workdir)!=0){
            if(cluster_empty_or_not(workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n\n" RESET_DISPLAY,current_cluster_name);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n" RESET_DISPLAY,current_cluster_name);
            }
            run_flag=graph(workdir,crypto_keyfile,0);
            if(run_flag==1){
                write_operation_log(current_cluster_name,operation_log,argv[1],"GRAPH_FAILED",45);
                check_and_cleanup(workdir);
                return 47;
            }
            write_operation_log(current_cluster_name,operation_log,argv[1],"GRAPH_NOT_UPDATED",47);
            check_and_cleanup(workdir);
            return 47;
        }
        decrypt_files(workdir,crypto_keyfile);
        printf("|\n");
        run_flag=graph(workdir,crypto_keyfile,0);
        if(run_flag==1){
            print_empty_cluster_info();
            write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }
    if(strcmp(argv[1],"vault")==0){
        if(argc==2){
            run_flag=get_vault_info(workdir,crypto_keyfile,"");
        }
        else{
            run_flag=get_vault_info(workdir,crypto_keyfile,argv[2]);
        }
        if(run_flag==1){
            print_empty_cluster_info();
            write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        else if(run_flag==-1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"new-keypair")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(argc==2){
            run_flag=rotate_new_keypair(workdir,"","",crypto_keyfile,"");
        }
        else if(argc==3){
            run_flag=rotate_new_keypair(workdir,"","",crypto_keyfile,argv[2]);
        }
        else if(argc==4){
            if(strcmp(argv[3],"echo")==0){
                run_flag=rotate_new_keypair(workdir,"","",crypto_keyfile,"echo");
            }
            else{
                run_flag=rotate_new_keypair(workdir,argv[2],argv[3],crypto_keyfile,"");
            }
        }
        else{
            if(strcmp(argv[4],"echo")==0){
                run_flag=rotate_new_keypair(workdir,argv[2],argv[3],crypto_keyfile,"echo");
            }
            else{
                run_flag=rotate_new_keypair(workdir,argv[2],argv[3],crypto_keyfile,"");
            }
        }
        if(run_flag==-1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==-3){
            write_operation_log(current_cluster_name,operation_log,argv[1],"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==3){
            write_operation_log(current_cluster_name,operation_log,argv[1],"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the key file. Have you switched to any cluster?\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(current_cluster_name,operation_log,"INTERNAL","KEY_CHECK_FAILED",7);
        check_and_cleanup(workdir);
        return 7;
    }
    if(check_pslock(workdir)==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(current_cluster_name,operation_log,"INTERNAL","PROCESS_LOCKED",53);
        check_and_cleanup(workdir);
        return 53;
    }
    if(strcmp(argv[1],"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return -1;
        }
        run_flag=get_default_conf(workdir,crypto_keyfile,1);
        if(run_flag==1||run_flag==127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The default configuration file has been downloaded to the local place.\n");
            printf("|          You can init directly, or edit it before init. Exit now.\n");
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    if(strcmp(argv[1],"edit-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=edit_configuration_file(workdir,crypto_keyfile);
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] No configuration file found. Please run the command 'hpcopr get-conf' first.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"NO_CONFIG_FILE",55);
            check_and_cleanup(workdir);
            return 55;
        }
        else{
            write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    if(strcmp(argv[1],"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster has already been initialized. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"ALREADY_INITED",57);
            check_and_cleanup(workdir);
            return 57;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            run_flag=aws_cluster_init(current_cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            run_flag=qcloud_cluster_init(current_cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_A")==0){
            run_flag=alicloud_cluster_init(current_cluster_name,workdir,crypto_keyfile);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Unknown Cloud Service Provider. Exit now.\n" RESET_DISPLAY);
            check_and_cleanup(workdir);
            return 59;
        }
        if(run_flag==-1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"WORKDIR_NOT_EXISTS",61);
            check_and_cleanup(workdir);
            return 61;
        }
        else if(run_flag==1){
            write_operation_log(current_cluster_name,operation_log,argv[1],"AWS_REGION_VALID_FAILED",63);
            check_and_cleanup(workdir);
            return 63;
        }
        else if(run_flag==2){
            write_operation_log(current_cluster_name,operation_log,argv[1],"DOWNLOAD/COPY_FILE_FAILED",65);
            check_and_cleanup(workdir);
            return 65;
        }
        else if(run_flag==3){
            write_operation_log(current_cluster_name,operation_log,argv[1],"ZONE_ID_ERROR",67);
            check_and_cleanup(workdir);
            return 67;
        }
        else if(run_flag==4){
            write_operation_log(current_cluster_name,operation_log,argv[1],"AWS_INVALID_KEYPAIR",69);
            check_and_cleanup(workdir);
            return 69;
        }
        else if(run_flag==5){
            write_operation_log(current_cluster_name,operation_log,argv[1],"TF_INIT_FAILED",71);
            check_and_cleanup(workdir);
            return 71;
        }
        else if(run_flag==7){
            write_operation_log(current_cluster_name,operation_log,argv[1],"TF_APPLY_FAILED_ROLLED_BACK",73);
            check_and_cleanup(workdir);
            return 73;
        }
        else if(run_flag==9){
            write_operation_log(current_cluster_name,operation_log,argv[1],"TF_ROLLBACK_FAILED",75);
            check_and_cleanup(workdir);
            return 75;
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
        check_and_cleanup(workdir);
        return 49;
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
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify 'mc', 'mcdb', or 'all' as the second parameter.\n");
                printf("|          Run 'hpcopr help' for more details. Exit now.\n" RESET_DISPLAY);
                write_operation_log(current_cluster_name,operation_log,argv[1],"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            if(run_flag==1){
                write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify 'mc', 'mcdb', or 'all' as the second parameter.\n");
            printf("|          Run 'hpcopr help' for more details. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
    }

    if(strcmp(argv[1],"sleep")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile);
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"wakeup")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(argc==2){
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"minimal");
            sprintf(string_temp,"%s default",argv[1]);
        }
        else{
            run_flag=cluster_wakeup(workdir,crypto_keyfile,argv[2]);
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
        }
        write_operation_log(current_cluster_name,operation_log,string_temp,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(argc==2){
        if(strcmp(argv[1],"reconfc")==0||strcmp(argv[1],"reconfm")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Internal error. Please submit an issue to the community. Exit now.\n" RESET_DISPLAY);
                write_operation_log(current_cluster_name,operation_log,argv[1],"INTERNAL_ERROR",-1);
                check_and_cleanup(workdir);
                return -17;
            }
            if(strcmp(argv[1],"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to turn on all the compute nodes first.\n" RESET_DISPLAY);
            }
            if(strcmp(argv[1],"reconfm")==0&&cluster_asleep_or_not(workdir)==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to wake up the cluster first.\n" RESET_DISPLAY);
            }
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(strcmp(argv[1],"ssh")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify to login with which user. Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(strcmp(argv[1],"userman")==0){
            print_usrmgr_info("");
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
    }

    if(argc==3&&strcmp(argv[1],"userman")==0&&strcmp(argv[2],"list")==0){
        run_flag=hpc_user_list(workdir,crypto_keyfile,0);
        if(cluster_asleep_or_not(workdir)==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The current cluster is not running.\n" RESET_DISPLAY);
        }
        write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"destroy")==0){
        if(argc>2&&strcmp(argv[2],"force")==0){
            run_flag=cluster_destroy(workdir,crypto_keyfile,"force");
        }
        else{
            run_flag=cluster_destroy(workdir,crypto_keyfile,"");
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(cluster_asleep_or_not(workdir)==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not running. Please wake up first.\n");
        if(strcmp(argv[1],"addc")==0){
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        else{
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup minimal | all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        write_operation_log(current_cluster_name,operation_log,argv[1],"CLUSTER_IS_ASLEEP",43);
        check_and_cleanup(workdir);
        return 43;
    }
    
    if(strcmp(argv[1],"delc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number or 'all' as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=delete_compute_node(workdir,crypto_keyfile,argv[2]);
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"addc")==0){
        if(check_down_nodes(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before adding new nodes.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            check_and_cleanup(workdir);
            return 1;
        }
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,argv[2]);
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"shutdownc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,argv[2]);
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"turnonc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(current_cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(workdir,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,argv[2]);
        write_operation_log(current_cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"reconfc")==0){
        if(strcmp(cloud_flag,"CLOUD_B")==0){
            if(check_down_nodes(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before reconfiguring them.\n");
                printf("|          Exit now.\n" RESET_DISPLAY);
                check_and_cleanup(workdir);
                return 1;
            }
        }
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(argc==3){
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],"");
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_operation_log(current_cluster_name,operation_log,string_temp,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],argv[3]);
            sprintf(string_temp,"%s %s %s",argv[1],argv[2],argv[3]);
            write_operation_log(current_cluster_name,operation_log,string_temp,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }
    if(strcmp(argv[1],"reconfm")==0){
        if(confirm_to_operate_cluster(current_cluster_name)!=0){
            write_operation_log(current_cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=reconfigure_master_node(workdir,crypto_keyfile,argv[2]);
        sprintf(string_temp,"%s %s",argv[1],argv[2]);
        write_operation_log(current_cluster_name,operation_log,string_temp,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"userman")==0){
        if(strcmp(argv[2],"add")!=0&&strcmp(argv[2],"delete")!=0&&strcmp(argv[2],"enable")!=0&&strcmp(argv[2],"disable")!=0&&strcmp(argv[2],"list")!=0&&strcmp(argv[2],"passwd")!=0){
            print_usrmgr_info("");
            write_operation_log(current_cluster_name,operation_log,argv[1],"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        usrmgr_check_flag=usrmgr_prereq_check(workdir,argv[2]);
        if(usrmgr_check_flag==3){
            check_and_cleanup(workdir);
            write_operation_log(current_cluster_name,operation_log,"INTERNAL","USERMAN_PREREQ_CHECK_FAILED",77);
            return 77;
        }
        if(strcmp(argv[2],"list")==0){
            printf("\n");
            run_flag=hpc_user_list(workdir,crypto_keyfile,0);
            write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(argv[2],"enable")==0||strcmp(argv[2],"disable")==0){
            if(argc==3){
                run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,"",crypto_keyfile,argv[2]);
            }
            else{
                run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,argv[3],crypto_keyfile,argv[2]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(argv[2],"add")==0){
            if(argc==3){
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,"","");
            }
            else if(argc==4){
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],"");
            }
            else{
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],argv[4]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(argv[2],"delete")==0){
            if(argc==3){
                run_flag=hpc_user_delete(workdir,crypto_keyfile,SSHKEY_DIR,"");
            }
            else{
                run_flag=hpc_user_delete(workdir,crypto_keyfile,SSHKEY_DIR,argv[3]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            if(argc==3){
                run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,"","");
            }
            else if(argc==4){
                run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],"");
            }
            else{
                run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],argv[4]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(current_cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }
    write_operation_log(NULL,operation_log,argv[2],"FATAL_ABNORMAL",run_flag);
    check_and_cleanup("");
    return 123;
}