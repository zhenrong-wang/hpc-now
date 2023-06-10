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
    "rm-conf",
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
38 NO_NEED_TO_WAKEUP
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


117 USER_CHECK_ERROR
119 KEY_FOLDER_ERROR
121 INTERNET_CHECK_FAILED
123 FATAL_ABNORMAL
125 FATAL_INTERNAL_ERROR
127 File I/O Error

SPECIAL RETURN VALUES: when the command_input is wrong.

200~255: command_check_prompt_index

*/

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char command_name_prompt[128]="";
    char buffer1[64]="";
    char buffer2[64]="";
    char cloud_flag[16]="";
    int command_flag=0;
    int run_flag=0;
    int usrmgr_check_flag=0;
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char* syserror_log=SYSTEM_CMD_ERROR_LOG;
    char string_temp[128]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    print_header();

#ifdef _WIN32
    if(check_current_user()!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* run hpcopr as the OS user 'hpc-now'.\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|          Please follow the steps below:\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|          1. run the command " RESET_DISPLAY HIGH_GREEN_BOLD "runas /savecred /user:mymachine\\hpc-now cmd\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|          2. run the " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY GENERAL_BOLD " commands in the *new* CMD window\n" RESET_DISPLAY);
        printf(FATAL_RED_BOLD "[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return 117;
    }
#else
    if(check_current_user()!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] You *MUST* run hpcopr as the OS user 'hpc-now'.\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|          Please follow the instructions below:\n" RESET_DISPLAY);
        printf(GENERAL_BOLD "|     <> SUDO-MODE (simple and fast for *sudoers*): \n" RESET_DISPLAY );
        printf(GENERAL_BOLD "|          run the hpcopr as " RESET_DISPLAY HIGH_GREEN_BOLD "sudo -u hpc-now hpcopr ..." RESET_DISPLAY "\n");
        printf(GENERAL_BOLD "|     <> USER-MODE (for both *non-sudoers* and *sudoers*): \n" RESET_DISPLAY );
        printf(GENERAL_BOLD "|          1. " RESET_DISPLAY HIGH_GREEN_BOLD "su hpc-now" RESET_DISPLAY " (The password of 'hpc-now' is needed)\n");
        printf(GENERAL_BOLD "|          2. run the " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY GENERAL_BOLD " commands, i.e. " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr glance all" RESET_DISPLAY "\n");
        printf(FATAL_RED_BOLD "[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return 117;
    }
#endif

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n" RESET_DISPLAY);
        print_tail();
        return 119;
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
        return 119;
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
        return 119;
    }
#endif
    if(folder_exist_or_not(GENERAL_CONF_DIR)!=0){
        sprintf(cmdline,"%s %s %s",MKDIR_CMD,GENERAL_CONF_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    command_flag=command_parser(argc,argv,command_name_prompt,workdir,cluster_name);
    if(command_flag==-1){
        print_help("");
        return 0;
    }
    else if(command_flag>199){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid Command. Do you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
        print_tail();
        return command_flag;
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
        write_operation_log("NULL",operation_log,"NULL","INTERNET_CHECK_FAILED",121);
        check_and_cleanup("");
        return 121;
    }

    if(strcmp(argv[1],"license")==0){
        read_license();
        print_tail();
        return 0;
    }

    if(strcmp(argv[1],"repair")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Entering repair mode. All the locations will be reset to default,\n");
        printf("|          and all the core components will be replaced by the default ones.\n");
        printf("|          Would you like to continue? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm.\n");
        printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
        fflush(stdin);
        scanf("%s",doubleconfirm);
        if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
            printf("\n[ -INFO- ] Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to continue. You chose to deny this operation.\n");
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
        run_flag=list_all_cluster_names(0);
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

    if(command_flag==-3){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid.\n" RESET_DISPLAY,cluster_name);
        list_all_cluster_names(1);
        write_operation_log("NULL",operation_log,argv[1],"CLUSTER_NAME_CHECK_FAILED",21);
        check_and_cleanup("");
        return 21;
    }
    else if(command_flag==-5){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Currently no cluster specified or switched" RESET_DISPLAY ".\n");
    }
    else if(command_flag==2){
        printf(GENERAL_BOLD "[ -INFO- ] " RESET_DISPLAY "Using the " HIGH_CYAN_BOLD "specified" RESET_DISPLAY " cluster name " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",cluster_name);
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ] " RESET_DISPLAY "Using the " HIGH_CYAN_BOLD "switched" RESET_DISPLAY " cluster name " RESET_DISPLAY HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",cluster_name);
    }

    if(strcmp(argv[1],"glance")==0){
        if(argc>2&&strcmp(argv[2],"all")==0){
            run_flag=glance_clusters("all",crypto_keyfile);
        }
        else{
            if(command_flag==-5){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Use the command " HIGH_GREEN_BOLD "hpcopr glance all" RESET_DISPLAY " to glance all the clusters.\n");
                list_all_cluster_names(1);
                write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
                check_and_cleanup("");
                return 25;
            }
            run_flag=glance_clusters(cluster_name,crypto_keyfile);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,cluster_name);
            write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"refresh")==0){
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(argc<3){
            run_flag=cluster_empty_or_not(workdir);
        }
        else{
            if(strcmp(argv[2],"force")==0){
                run_flag=3;
            }
            else{
                if(argc>3&&strcmp(argv[3],"force")==0){
                    run_flag=3;
                }
                else{
                    run_flag=cluster_empty_or_not(workdir);
                }
            }
        }
        if(run_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster cannot be refreshed (either in init progress or empty).\n" RESET_DISPLAY);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please run " HIGH_GREEN_BOLD "hpcopr glance all" RESET_DISPLAY " to check. Exit now.\n");
            write_operation_log("NULL",operation_log,argv[1],"EMPTY_CLUSTER_OR_IN_PROGRESS",27);
            check_and_cleanup(workdir);
            return 27;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        if(argc<3){
            run_flag=refresh_cluster("",crypto_keyfile,"");
        }
        else{
            if(run_flag==5){
                run_flag=refresh_cluster("",crypto_keyfile,"force");
            }
            else if(run_flag==3){
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
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==-13){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
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
            write_operation_log(cluster_name,operation_log,argv[1],"OPERATION_IN_PROGRESS",29);
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
            write_operation_log(cluster_name,operation_log,argv[1],"OPERATION_FAILED",31);
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
            list_all_cluster_names(1);
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
            write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        }
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"exit-current")==0){
        if(command_flag==-5||show_current_cluster(workdir,cluster_name,0)==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] No switched cluster to be exited.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(exit_current_cluster()==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exited the current cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",cluster_name);
            write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ -INFO- ] Failed to exit the current cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD ".\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argv[1],"OPERATION_FAILED",35);
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

    if(strcmp(argv[1],"switch")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify which cluster to switch to:\n" RESET_DISPLAY);
            run_flag=list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argv[1],"TOO_FEW_PARAMS",5);
            check_and_cleanup("");
            return 5;
        }
        if(command_flag==2){
            run_flag=switch_to_cluster(cluster_name);
        }
        else{
            run_flag=switch_to_cluster(argv[2]);
        }

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
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argv[1],"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"remove")==0){
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(argc>3&&strcmp(argv[2],"force")==0){
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"force");
        }
        else{
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"");
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

    if(strcmp(argv[1],"viewlog")==0){
        if(argc==2||(argc==3&&command_flag==2)){
            run_flag=view_run_log(workdir,"","","");
        }
        else if(argc==3||(argc==4&&command_flag==2)){
            run_flag=view_run_log(workdir,argv[2],"","");
        }
        else if(argc==4||(argc==5&&command_flag==2)){
            run_flag=view_run_log(workdir,argv[2],argv[3],"");
        }
        else{
            run_flag=view_run_log(workdir,argv[2],argv[3],argv[4]);
        }
        if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the log. Have you specified or switched to a cluster?\n" RESET_DISPLAY );
            list_all_cluster_names(1);
            write_operation_log(cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argv[1],argv[2],0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(command_flag==-5){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c=" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
        list_all_cluster_names(1);
        write_operation_log("NULL",operation_log,argv[1],"NOT_OPERATING_CLUSTERS",25);
        check_and_cleanup("");
        return 25;
    }

    if(strcmp(argv[1],"ssh")==0){
        if(argc==2){
            printf(GENERAL_BOLD "[ -INFO- ] Usage: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr ssh USER_NAME (Optional)-c=CLUSTER_NAME" RESET_DISPLAY GENERAL_BOLD "\n");
            printf("|          A blank CLUSTER_NAME refers to the current cluster.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"TOO_FEW_PARAMS",5);
            check_and_cleanup("");
            return 5;
        }
        else if(argc==3&&command_flag==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify to login as which user. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        else{
            if(cluster_asleep_or_not(workdir)==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
                write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_GREEN_BOLD "%s@%s" RESET_DISPLAY ", may fail if the username is invalid.\n",argv[2],cluster_name);
            run_flag=cluster_ssh(workdir,argv[2]);
            if(run_flag==-1){
                write_operation_log(cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
                check_and_cleanup("");
                return 127;
            }
            write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",run_flag);
        }
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"graph")==0){
        if(check_pslock(workdir)!=0){
            if(cluster_empty_or_not(workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n\n" RESET_DISPLAY,cluster_name);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n" RESET_DISPLAY,cluster_name);
            }
            run_flag=graph(workdir,crypto_keyfile,0);
            if(run_flag==1){
                write_operation_log(cluster_name,operation_log,argv[1],"GRAPH_FAILED",45);
                check_and_cleanup(workdir);
                return 47;
            }
            write_operation_log(cluster_name,operation_log,argv[1],"GRAPH_NOT_UPDATED",47);
            check_and_cleanup(workdir);
            return 47;
        }
        decrypt_files(workdir,crypto_keyfile);
        printf("\n");
        run_flag=graph(workdir,crypto_keyfile,0);
        if(run_flag==1){
            print_empty_cluster_info();
            delete_decrypted_files(workdir,crypto_keyfile);
            write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        printf("\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
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
            write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        else if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"new-keypair")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
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
            write_operation_log(cluster_name,operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==-3){
            write_operation_log(cluster_name,operation_log,argv[1],"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argv[1],"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    create_and_get_vaultdir(workdir,vaultdir);
    sprintf(filename_temp,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer2,cloud_flag)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the key file. Have you switched to any cluster?\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(cluster_name,operation_log,"INTERNAL","KEY_CHECK_FAILED",7);
        check_and_cleanup(workdir);
        return 7;
    }
    if(check_pslock(workdir)==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(cluster_name,operation_log,"INTERNAL","PROCESS_LOCKED",53);
        check_and_cleanup(workdir);
        return 53;
    }
    if(strcmp(argv[1],"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=get_default_conf(cluster_name,crypto_keyfile,1);
        if(run_flag==1||run_flag==127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The default configuration file has been downloaded to the local place.\n");
            printf("|          You can init directly, or edit it before init. Exit now.\n");
            write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    if(strcmp(argv[1],"edit-conf")==0||strcmp(argv[1],"rm-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(argv[1],"edit-conf")==0){
            run_flag=edit_configuration_file(cluster_name,crypto_keyfile);
        }
        else{
            run_flag=remove_conf(cluster_name);
        }
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] No configuration file found. You can run " WARN_YELLO_BOLD "hpcopr get-conf" RESET_DISPLAY FATAL_RED_BOLD " first.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"NO_CONFIG_FILE",55);
            check_and_cleanup(workdir);
            return 55;
        }
        else{
            if(strcmp(argv[1],"rm-conf")==0){
                printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The previous configuration file has been deleted.\n");
            }
            write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster has already been initialized. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"ALREADY_INITED",57);
            check_and_cleanup(workdir);
            return 57;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=cluster_init_conf(cluster_name,argc,argv);
        if(run_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid cloud vendor. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a configuration file. Exit now.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argv[1],"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid format for the --nn and/or --un. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        else if(run_flag==-3){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Configuration file found. Omitted all the specified params.\n" RESET_DISPLAY);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Configuration file not found. Using the specified or default params.\n" RESET_DISPLAY);
        }
        if(strcmp(cloud_flag,"CLOUD_C")==0){
            run_flag=aws_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            run_flag=qcloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else if(strcmp(cloud_flag,"CLOUD_A")==0){
            run_flag=alicloud_cluster_init(cluster_name,workdir,crypto_keyfile);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Unknown Cloud Service Provider. Exit now.\n" RESET_DISPLAY);
            check_and_cleanup(workdir);
            return 59;
        }
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argv[1],"WORKDIR_NOT_EXISTS",61);
            check_and_cleanup(workdir);
            return 61;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argv[1],"AWS_REGION_VALID_FAILED",63);
            check_and_cleanup(workdir);
            return 63;
        }
        else if(run_flag==2){
            write_operation_log(cluster_name,operation_log,argv[1],"DOWNLOAD/COPY_FILE_FAILED",65);
            check_and_cleanup(workdir);
            return 65;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argv[1],"ZONE_ID_ERROR",67);
            check_and_cleanup(workdir);
            return 67;
        }
        else if(run_flag==4){
            write_operation_log(cluster_name,operation_log,argv[1],"AWS_INVALID_KEYPAIR",69);
            check_and_cleanup(workdir);
            return 69;
        }
        else if(run_flag==5){
            write_operation_log(cluster_name,operation_log,argv[1],"TF_INIT_FAILED",71);
            check_and_cleanup(workdir);
            return 71;
        }
        else if(run_flag==7){
            write_operation_log(cluster_name,operation_log,argv[1],"TF_APPLY_FAILED_ROLLED_BACK",73);
            check_and_cleanup(workdir);
            return 73;
        }
        else if(run_flag==9){
            write_operation_log(cluster_name,operation_log,argv[1],"TF_ROLLBACK_FAILED",75);
            check_and_cleanup(workdir);
            return 75;
        }
        write_operation_log(cluster_name,operation_log,argv[1],"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_EMPTY",49);
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
                write_operation_log(cluster_name,operation_log,argv[1],"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            if(run_flag==1){
                write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify 'mc', 'mcdb', or 'all' as the second parameter.\n");
            printf("|          Run 'hpcopr help' for more details. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
    }

    if(strcmp(argv[1],"sleep")==0){
        if(cluster_asleep_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is " RESET_DISPLAY HIGH_CYAN_BOLD "not running" RESET_DISPLAY FATAL_RED_BOLD ". No need to hibernate.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_ASLEEP",43);
            check_and_cleanup("");
            return 43;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"wakeup")==0){
        if(cluster_full_running_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "fully running" RESET_DISPLAY FATAL_RED_BOLD ". No need to wake up.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"RUNNING_STATE",38);
            check_and_cleanup(workdir);
            return 38;
        }
        if(cluster_asleep_or_not(workdir)!=0){
            if(argc==2){
                printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "minimal running" RESET_DISPLAY FATAL_RED_BOLD ". Please try\n" RESET_DISPLAY);
                printf(FATAL_RED_BOLD "|          " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup all" RESET_DISPLAY FATAL_RED_BOLD " to wake up the whole cluster.\n" RESET_DISPLAY);
                write_operation_log(cluster_name,operation_log,argv[1],"RUNNING_STATE",38);
                check_and_cleanup(workdir);
                return 38;
            }
            else if(strcmp(argv[2],"all")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "minimal running" RESET_DISPLAY FATAL_RED_BOLD ". Please try \n" RESET_DISPLAY);
                printf(FATAL_RED_BOLD "|          " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup all" RESET_DISPLAY FATAL_RED_BOLD " to wake up the whole cluster.\n" RESET_DISPLAY);
                write_operation_log(cluster_name,operation_log,argv[1],"RUNNING_STATE",38);
                check_and_cleanup(workdir);
                return 38;
            }
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
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
        write_operation_log(cluster_name,operation_log,string_temp,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(argc==2||(argc==3&&command_flag==2)){
        if(strcmp(argv[1],"reconfc")==0||strcmp(argv[1],"reconfm")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the list. Have you inited this cluster?\n" RESET_DISPLAY);
                write_operation_log(cluster_name,operation_log,argv[1],"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
            }
            if(strcmp(argv[1],"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to turn on all the compute nodes first.\n" RESET_DISPLAY);
            }
            if(strcmp(argv[1],"reconfm")==0&&cluster_asleep_or_not(workdir)==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to wake up the cluster first.\n" RESET_DISPLAY);
            }
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(strcmp(argv[1],"userman")==0){
            print_usrmgr_info("");
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
    }

    if(argc>2&&strcmp(argv[1],"userman")==0&&strcmp(argv[2],"list")==0){
        run_flag=hpc_user_list(workdir,crypto_keyfile,0);
        if(cluster_asleep_or_not(workdir)==0){
            if(command_flag==2){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster is not running.\n" RESET_DISPLAY);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] The switched cluster is not running.\n" RESET_DISPLAY);
            }
        }
        write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
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
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(cluster_asleep_or_not(workdir)==0){
        if(command_flag==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not running. Please wake up first\n" RESET_DISPLAY);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] The switched cluster is not running. Please wake up first\n" RESET_DISPLAY);
        }
        if(strcmp(argv[1],"addc")==0){
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        else{
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup minimal | all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        write_operation_log(cluster_name,operation_log,argv[1],"CLUSTER_IS_ASLEEP",43);
        check_and_cleanup(workdir);
        return 43;
    }
    
    if(strcmp(argv[1],"delc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number or 'all' as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=delete_compute_node(workdir,crypto_keyfile,argv[2]);
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
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
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,argv[2]);
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"shutdownc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,argv[2]);
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"turnonc")==0){
        if(argc==2){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argv[1],"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(workdir,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,argv[2]);
        write_operation_log(cluster_name,operation_log,argv[1],"",run_flag);
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
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(argc==3){
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],"");
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_operation_log(cluster_name,operation_log,string_temp,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,argv[2],argv[3]);
            sprintf(string_temp,"%s %s %s",argv[1],argv[2],argv[3]);
            write_operation_log(cluster_name,operation_log,string_temp,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }
    if(strcmp(argv[1],"reconfm")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argv[1],"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=reconfigure_master_node(workdir,crypto_keyfile,argv[2]);
        sprintf(string_temp,"%s %s",argv[1],argv[2]);
        write_operation_log(cluster_name,operation_log,string_temp,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"userman")==0){
        if(strcmp(argv[2],"add")!=0&&strcmp(argv[2],"delete")!=0&&strcmp(argv[2],"enable")!=0&&strcmp(argv[2],"disable")!=0&&strcmp(argv[2],"list")!=0&&strcmp(argv[2],"passwd")!=0){
            print_usrmgr_info("");
            write_operation_log(cluster_name,operation_log,argv[1],"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        usrmgr_check_flag=usrmgr_prereq_check(workdir,argv[2]);
        if(usrmgr_check_flag==3){
            check_and_cleanup(workdir);
            write_operation_log(cluster_name,operation_log,"INTERNAL","USERMAN_PREREQ_CHECK_FAILED",77);
            return 77;
        }
        if(strcmp(argv[2],"list")==0){
            run_flag=hpc_user_list(workdir,crypto_keyfile,0);
            write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(argv[2],"enable")==0||strcmp(argv[2],"disable")==0){
            if(argc==3||(argc==4&&command_flag==2)){
                run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,"",crypto_keyfile,argv[2]);
            }
            else{
                run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,argv[3],crypto_keyfile,argv[2]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(argv[2],"add")==0){
            if(argc==3||(argc==4&&command_flag==2)){
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,"","");
            }
            else if(argc==4||(argc==5&&command_flag==2)){
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],"");
            }
            else{
                run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,argv[3],argv[4]);
            }
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
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
            write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
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
            write_operation_log(cluster_name,operation_log,argv[2],"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }
    write_operation_log(NULL,operation_log,argv[2],"FATAL_ABNORMAL",run_flag);
    check_and_cleanup("");
    return 123;
}