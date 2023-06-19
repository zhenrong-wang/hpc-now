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
#include "dataman.h"
#include "transfer.h"

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
    "export",
    "import",
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
    "dataman",
    "about",
    "version",
    "license",
    "repair"
};

char dataman_commands[DATAMAN_COMMAND_NUM][COMMAND_STRING_LENGTH_MAX]={
    "put",
    "rput",
    "get",
    "rget",
    "copy",
    "list",
    "delete",
    "move",
    "cp",
    "mv",
    "ls",
    "rm",
    "mkdir",
    "cat",
    "more",
    "less",
    "tail"
};

/*
1 NOT_A_VALID_COMMAND
3 USER_DENIED
5 LACK_PARAMS
7 MISSING_CLOUD_FLAG_FILE
9 PARAM_FORMAT_ERROR

11 Prereq - Components Download and install failed
13 Prereq - Other failed
15 Prereq - Envcheck Failed
17 Prereq - Config Location Failed
19 Prereq - Vers and md5 Error
21 CLUSTER_NAME_CHECK_FAILED
23 INVALID_KEYPAIR
25 Not Operating Clusters
26 INVALID_USER_NAME
27 EMPTY_CLUSTER_OR_IN_PROGRESS
28 DATAMAN_FAILED
29 OPERATION_IN_PROGRESS
30 EXPORT_FAILED
31 REFRESHING FAILED
32 IMPORT_FAILED
33 EMPTY REGISTRY
34 REBUILD_FAILED
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
    char cloud_flag[16]="";
    int command_flag=0;
    int level_flag=0;
    int run_flag=0;
    int usrmgr_check_flag=0;
    char workdir[DIR_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX]="";
    char invalid_cluster_name[128]="";
    char new_cluster_name[128]="";
    char cloud_ak[128]="";
    char cloud_sk[128]="";
    char stream_name[128]="";
    char log_type[128]="";
    char user_name[128]="";
    char pass_word[32]="";
    char user_name_list[1024]="";
    char vault_bucket_flag[8]="";
    char vault_root_flag[8]="";
    char export_dest[FILENAME_LENGTH]="";
    char import_source[FILENAME_LENGTH]="";
    char trans_keyfile[FILENAME_LENGTH]="";

    char data_cmd[128]="";
    char source_path[FILENAME_LENGTH]="";
    char destination_path[FILENAME_LENGTH]="";
    char target_path[FILENAME_LENGTH]="";
    char recursive_flag[16]="";
    char force_flag_string[16]="";
    char node_num_string[128]="";
    char user_cmd[128]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char* syserror_log=SYSTEM_CMD_ERROR_LOG;
    char string_temp[256]="";
    char string_temp2[256]="";
    char doubleconfirm[64]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_role[8]="";
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
        printf(GENERAL_BOLD "|          2. run the " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY GENERAL_BOLD " commands, i.e. " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr glance --all" RESET_DISPLAY "\n");
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

    command_flag=command_parser(argc,argv,command_name_prompt,workdir,cluster_name,cluster_role,invalid_cluster_name);
    if(command_flag==-1){
        print_help("all");
        return 0;
    }
    else if(command_flag>199){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid Command. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
        print_tail();
        return command_flag;
    }
    
    if(strcmp(argv[1],"help")==0){
        if(cmd_keyword_check(argc,argv,"--cmd",string_temp)!=0){
            print_help("all");
        }
        else{
            if(command_name_check(string_temp,command_name_prompt)>199){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified command name is incorrect. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup("");
                return 9;
            }
            print_help(string_temp);
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
        write_operation_log("NULL",operation_log,argc,argv,"INTERNET_CHECK_FAILED",121);
        check_and_cleanup("");
        return 121;
    }

    if(strcmp(argv[1],"license")==0){
        if(cmd_flag_check(argc,argv,"--print")==0){
            read_license("print");
        }
        else{
            read_license("read");
        }
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
            write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
            print_tail();
            return 3;
        }
        run_flag=check_and_install_prerequisitions(1);
        if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",11);
            check_and_cleanup("");
            return 11;
        }
        else if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"REPAIR_FAILED",run_flag);
            check_and_cleanup("");
            return 13;
        }
        else{
            write_operation_log("NULL",operation_log,argc,argv,"REPAIR_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
    }

    if(strcmp(argv[1],"envcheck")==0){
        run_flag=check_and_install_prerequisitions(0);
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_FAILED",run_flag);
            check_and_cleanup("");
            return 15;
        }
        write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"configloc")==0){
        run_flag=configure_locations();
        if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        else if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"resetloc")==0){
        run_flag=reset_locations();
        if(run_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The locations have been reset to the default.\n");
            show_locations();
            write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal error, failed to reset the locations.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
    }

    if(strcmp(argv[1],"showloc")==0){
        run_flag=show_locations();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"showmd5")==0){
        run_flag=show_vers_md5vars();
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    run_flag=check_and_install_prerequisitions(0);
    if(run_flag==3){
        write_operation_log("NULL",operation_log,argc,argv,"COMPONENTS_DOWNLOAD_AND_INSTALL_FAILED",-11);
        check_and_cleanup("");
        return 11;
    }
    else if(run_flag==-1){
        write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
        check_and_cleanup("");
        return 127;
    }
    else if(run_flag==-3){
        write_operation_log("NULL",operation_log,argc,argv,"RESET_LOCATION_FAILED",13);
        check_and_cleanup("");
        return 13;
    }
    else if(run_flag==1){
        write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
        check_and_cleanup("");
        return 3;
    }
    else if(run_flag==5){
        write_operation_log("NULL",operation_log,argc,argv,"CONFIG_LOCATION_FAILED",17);
        check_and_cleanup("");
        return 17;
    }
    else if(run_flag==7){
        write_operation_log("NULL",operation_log,argc,argv,"VERSION_MD5SUM_ERROR",19);
        check_and_cleanup("");
        return 19;
    }

    if(strcmp(argv[1],"new-cluster")==0){
        cmd_keyword_check(argc,argv,"--cname",new_cluster_name);
        cmd_keyword_check(argc,argv,"--ak",cloud_ak);
        cmd_keyword_check(argc,argv,"--sk",cloud_sk);
        if(cmd_flag_check(argc,argv,"--echo")==0){
            run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,cloud_ak,cloud_sk,"echo");
        }
        else{
            run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,cloud_ak,cloud_sk,"");
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"CLUSTER_NAME_CHECK_FAILED",21);
            check_and_cleanup(workdir);
            return 21;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==5){
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"ls-clusters")==0){
        run_flag=list_all_cluster_names(0);
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"EMPTY_REGISTRY",33);
            check_and_cleanup(workdir);
            return 33;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(command_flag==-3){
        printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid.\n" RESET_DISPLAY,invalid_cluster_name);
        list_all_cluster_names(1);
        write_operation_log("NULL",operation_log,argc,argv,"CLUSTER_NAME_CHECK_FAILED",21);
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
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=glance_clusters("all",crypto_keyfile);
        }
        else{
            if(command_flag==-5){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Use the command " HIGH_GREEN_BOLD "hpcopr glance --all" RESET_DISPLAY " to glance all the clusters.\n");
                list_all_cluster_names(1);
                write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
                check_and_cleanup(workdir);
                return 25;
            }
            run_flag=glance_clusters(cluster_name,crypto_keyfile);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry.\n" RESET_DISPLAY,cluster_name);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"refresh")==0){
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to one.\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"force");
        }
        else{
            if(confirm_to_operate_cluster(cluster_name)!=0){
                write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"");
        }
        if(run_flag!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to refresh cluster %s. Exit now.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"OPERATION_FAILED",31);
            check_and_cleanup(workdir);
            return 31;
        }
        printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " Successfully refreshed cluster %s.\n",cluster_name);
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"exit-current")==0){
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] No switched cluster to be exited.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(exit_current_cluster()==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exited the current cluster " HIGH_CYAN_BOLD "%s" RESET_DISPLAY ".\n",cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ -INFO- ] Failed to exit the current cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD ".\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"OPERATION_FAILED",35);
            check_and_cleanup("");
            return 35;
        }
    }

    if(strcmp(argv[1],"usage")==0){
        cmd_keyword_check(argc,argv,"--d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(usage_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(usage_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }
    if(strcmp(argv[1],"history")==0){
        cmd_keyword_check(argc,argv,"--d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(operation_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(operation_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"syserr")==0){
        cmd_keyword_check(argc,argv,"--d",export_dest);
        if(cmd_flag_check(argc,argv,"--read")==0){
            run_flag=view_system_logs(syserror_log,"read",export_dest);
        }
        else{
            run_flag=view_system_logs(syserror_log,"print",export_dest);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"import")==0){
        cmd_keyword_check(argc,argv,"--s",import_source);
        cmd_keyword_check(argc,argv,"--key",trans_keyfile);
        run_flag=import_cluster(import_source,trans_keyfile,crypto_keyfile);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"IMPORT_FAILED",32);
            check_and_cleanup("");
            return 32;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
    }

    if(strcmp(argv[1],"switch")==0){
        if(cmd_flag_check(argc,argv,"--list")==0){
            run_flag=list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c " RESET_DISPLAY FATAL_RED_BOLD ".\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        run_flag=switch_to_cluster(cluster_name);
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"NO_NEED_TO_SWITCH",37);
            check_and_cleanup("");
            return 37;
        }
        else if(run_flag==1){
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(argv[1],"remove")==0){
        if(command_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to one.\n" RESET_DISPLAY);
            list_all_cluster_names(1);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup("");
            return 25;
        }
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"force");
        }
        else{
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"");
        }
        if(run_flag==1){
            write_operation_log(argv[2],operation_log,argc,argv,"CLUSTER_NAME_CHECK_FAILED",21);
            check_and_cleanup(workdir);
            return 21; 
        }
        else if(run_flag==3){
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        else if(run_flag==5){
            write_operation_log(argv[2],operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==7){
            write_operation_log(argv[2],operation_log,argc,argv,"CLUSTER_DESTROY_FAILED",41);
            check_and_cleanup(workdir);
            return 41;
        }
        write_operation_log(argv[2],operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"viewlog")==0){
        if(cmd_flag_check(argc,argv,"--err")==0){
            strcpy(stream_name,"err");
        }
        else{
            strcpy(stream_name,"std");
        }
        if(cmd_flag_check(argc,argv,"--hist")==0){
            strcpy(log_type,"archive");
        }
        else{
            strcpy(log_type,"realtime");
        }
        if(cmd_flag_check(argc,argv,"--print")==0){
            run_flag=view_run_log(workdir,stream_name,log_type,"print");
        }
        else{
            run_flag=view_run_log(workdir,stream_name,log_type,"");
        }
        if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to open the log. Have you specified or switched to a cluster?\n" RESET_DISPLAY );
            list_all_cluster_names(1);
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,argv[2],0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(command_flag==-5){
        printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to one.\n" RESET_DISPLAY);
        list_all_cluster_names(1);
        write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
        check_and_cleanup("");
        return 25;
    }

    if(strcmp(argv[1],"ssh")==0){
        if(cmd_keyword_check(argc,argv,"-u",user_name)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid user name to ssh to the cluster.\n");
            hpc_user_list(workdir,crypto_keyfile,0);
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",user_name);
            getchar();
        }
        if(user_name_quick_check(cluster_name,user_name,SSHKEY_DIR)!=0){
            printf(FATAL_RED_BOLD "\n[ FATAL: ] The user name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid or unauthorized. Current users:\n" RESET_DISPLAY,argv[2]);
            hpc_user_list(workdir,crypto_keyfile,0);
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_HPC_USER_NAME",26);
            check_and_cleanup("");
            return 26;
        }
        if(cluster_asleep_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_GREEN_BOLD "%s@%s" RESET_DISPLAY ".\n",user_name,cluster_name);
        if(strcmp(user_name,"root")==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] SSH as root is VERY RISKY and *NOT* recommended! Only for operator or admins." RESET_DISPLAY "\n");
        }
        run_flag=cluster_ssh(workdir,user_name);
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"graph")==0){
        cmd_keyword_check(argc,argv,"--level",string_temp);
        if(strcmp(string_temp,"csv")==0){
            level_flag=2;
        }
        else if(strcmp(string_temp,"txt")==0){
            level_flag=1;
        }
        else{
            level_flag=0;
        }
        if(check_pslock(workdir)!=0){
            if(cluster_empty_or_not(workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n\n" RESET_DISPLAY,cluster_name);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n" RESET_DISPLAY,cluster_name);
            }
            run_flag=graph(workdir,crypto_keyfile,level_flag);
            if(run_flag==1){
                write_operation_log(cluster_name,operation_log,argc,argv,"GRAPH_FAILED",45);
                check_and_cleanup(workdir);
                return 47;
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"GRAPH_NOT_UPDATED",47);
            check_and_cleanup(workdir);
            return 47;
        }
        decrypt_files(workdir,crypto_keyfile);
        printf("\n");
        run_flag=graph(workdir,crypto_keyfile,level_flag);
        if(run_flag==1){
            print_empty_cluster_info();
            delete_decrypted_files(workdir,crypto_keyfile);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"vault")==0){
        cmd_keyword_check(argc,argv,"-u",user_name);
        if(cmd_flag_check(argc,argv,"--bkey")==0){
            strcpy(vault_bucket_flag,"bucket");
        }
        if(cmd_flag_check(argc,argv,"--rkey")==0){
            strcpy(vault_root_flag,"root");
        }
        run_flag=get_vault_info(workdir,crypto_keyfile,user_name,vault_bucket_flag,vault_root_flag);
        if(run_flag==1){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        else if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"new-keypair")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--ak",cloud_ak);
        cmd_keyword_check(argc,argv,"--sk",cloud_sk);
        if(cmd_flag_check(argc,argv,"--echo")==0){
            run_flag=rotate_new_keypair(workdir,cloud_ak,cloud_sk,crypto_keyfile,"echo");
        }
        else{
            run_flag=rotate_new_keypair(workdir,cloud_ak,cloud_sk,crypto_keyfile,"");
        }
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==-3){
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_KEYPAIR",23);
            check_and_cleanup(workdir);
            return 23;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(get_cloud_flag(workdir,cloud_flag)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cloud flag. Have you switched to any cluster?\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FLAG_CHECK_FAILED",7);
        check_and_cleanup(workdir);
        return 7;
    }

    if(strcmp(argv[1],"export")==0){
        if(cluster_empty_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is empty, nothing to be exported.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        cmd_keyword_check(argc,argv,"--ul",user_name_list);
        cmd_keyword_check(argc,argv,"--key",trans_keyfile);
        cmd_keyword_check(argc,argv,"--d",export_dest);
//        printf("%s ------------------ \n",trans_keyfile);
        if(cmd_flag_check(argc,argv,"--admin")==0){
            run_flag=export_cluster(cluster_name,user_name_list,"admin",crypto_keyfile,trans_keyfile,export_dest);
        }
        else{
            run_flag=export_cluster(cluster_name,user_name_list,"",crypto_keyfile,trans_keyfile,export_dest);
        }
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"EXPORT_FAILED",30);
            check_and_cleanup(workdir);
            return 30;
        }
    }

    if(strcmp(argv[1],"dataman")==0){
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(cmd_keyword_check(argc,argv,"--dcmd",data_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid command to use the data manager.\n");
            printf(GENERAL_BOLD "|       +- Bucket:" RESET_DISPLAY " put, get, copy, list, delete, move\n");
            printf(GENERAL_BOLD "|       +- Remote:" RESET_DISPLAY " cp, mv, ls, rm, mkdir, cat, more, less, tail, rput, rget \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",data_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(data_cmd,dataman_commands[i])!=0){
            i++;
            if(i==DATAMAN_COMMAND_NUM){
                break;
            }
        }
        if(i==DATAMAN_COMMAND_NUM){
            printf("[ FATAL: ] The command %s is incorrect. Please read the help for details.\n",data_cmd);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(cmd_keyword_check(argc,argv,"-u",user_name)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a valid user name to use the data manager.\n");
            hpc_user_list(workdir,crypto_keyfile,0);
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",user_name);
            getchar();
        }
        if(user_name_quick_check(cluster_name,user_name,SSHKEY_DIR)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The user name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid or unauthorized. Current users:\n" RESET_DISPLAY,user_name);
            hpc_user_list(workdir,crypto_keyfile,0);
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_HPC_USER_NAME",26);
            check_and_cleanup("");
            return 26;
        }
        if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"list")!=0&&strcmp(data_cmd,"delete")!=0&&strcmp(data_cmd,"move")!=0){
            if(cluster_asleep_or_not(workdir)==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
                write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0){
            if(cmd_keyword_check(argc,argv,"--s",source_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a source path for this command.\n");
                printf("|          Use prefix @h/ , @d/ , @p/, @a/, @R/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                printf("|          For " HIGH_CYAN_BOLD "local" RESET_DISPLAY " paths and " HIGH_CYAN_BOLD "bucket" RESET_DISPLAY " paths, no prefix needed.\n");
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",source_path);
                getchar();
            }
            if(cmd_keyword_check(argc,argv,"--d",destination_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a destination path for this command.\n");
                printf("|          Use prefix @h/ , @d/ , @p/, @a/, @R/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                printf("|          For " HIGH_CYAN_BOLD "local" RESET_DISPLAY " paths and " HIGH_CYAN_BOLD "bucket" RESET_DISPLAY " paths, no prefix needed.\n");
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",destination_path);
                getchar();
            }
        }
        else{
            if(cmd_keyword_check(argc,argv,"--t",target_path)!=0){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a target path for this command.\n");
                printf("|          Use prefix @h/ , @d/ , @p/, @a/, @R/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                printf("|          For " HIGH_CYAN_BOLD "local" RESET_DISPLAY " paths and " HIGH_CYAN_BOLD "bucket" RESET_DISPLAY " paths, no prefix needed.\n");
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",target_path);
                getchar();
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"delete")==0||strcmp(data_cmd,"list")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0||strcmp(data_cmd,"rm")==0){
            if(strcmp(data_cmd,"list")!=0&&cmd_flag_check(argc,argv,"--force")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-f")!=0){
                /*printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Would you like to do force operation (if applicable) ? y|n.\n");
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",string_temp);
                getchar();
                if(strcmp(string_temp,"y")==0||strcmp(string_temp,"Y")==0){
                    strcpy(force_flag_string,"force");
                }
                else{
                    strcpy(force_flag_string,"");
                }*/
                printf(GENERAL_BOLD "[ -INFO- ] You may need --force or -f flag to do force operation.\n" RESET_DISPLAY );
            }
            else{
                strcpy(force_flag_string,"force");
            }
            if(strcmp(data_cmd,"mv")!=0&&cmd_flag_check(argc,argv,"--recursive")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-r")!=0){
/*              printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Would you like to do recursive operation (if applicable) ? y|n.\n");
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%s",string_temp);
                getchar();
                if(strcmp(string_temp,"y")==0||strcmp(string_temp,"Y")==0){
                    strcpy(recursive_flag,"recursive");
                }
                else{
                    strcpy(recursive_flag,"");
                }*/
                printf(GENERAL_BOLD "[ -INFO- ] You may need --recursive or -r flag when operating folders.\n" RESET_DISPLAY );
            }
            else{
                strcpy(recursive_flag,"recursive");
            }
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Data operation started ...\n\n");
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0){
            run_flag=bucket_cp(workdir,user_name,source_path,destination_path,recursive_flag,force_flag_string,crypto_keyfile,cloud_flag,data_cmd);
            if(strcmp(data_cmd,"move")==0&&run_flag==0){
                run_flag=bucket_rm_ls(workdir,user_name,source_path,"recursive","",crypto_keyfile,cloud_flag,"delete");
            }
        }
        else if(strcmp(data_cmd,"list")==0||strcmp(data_cmd,"delete")==0){
            run_flag=bucket_rm_ls(workdir,user_name,target_path,recursive_flag,force_flag_string,crypto_keyfile,cloud_flag,data_cmd);
        }
        else if(strcmp(data_cmd,"rm")==0||strcmp(data_cmd,"ls")==0||strcmp(data_cmd,"mkdir")==0){
            run_flag=direct_rm_ls_mkdir(workdir,user_name,SSHKEY_DIR,target_path,force_flag_string,recursive_flag,data_cmd);
        }
        else if(strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0){
            run_flag=direct_cp_mv(workdir,user_name,SSHKEY_DIR,source_path,destination_path,recursive_flag,force_flag_string,data_cmd);
        }
        else if(strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0){
            run_flag=remote_bucket_cp(workdir,user_name,SSHKEY_DIR,source_path,destination_path,recursive_flag,force_flag_string,cloud_flag,crypto_keyfile,data_cmd);
        }
        else{
            run_flag=direct_file_operations(workdir,user_name,SSHKEY_DIR,target_path,data_cmd);
        }
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            if(run_flag==3){
                printf(FATAL_RED_BOLD "[ FATAL: ] The source and dest path must include @ prefixes.\n" RESET_DISPLAY);
            }
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] Data operation failed or canceled. Check the console output above.\n");
            printf("|      <>  Command: %s | Cluster: %s | User: %s\n" RESET_DISPLAY,data_cmd,cluster_name,user_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"DATAMAN_OPERATION_FAILED",28);
            check_and_cleanup(workdir);
            return 28;
        }
    }

    if(check_pslock(workdir)==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now.\n" RESET_DISPLAY);
        write_operation_log(cluster_name,operation_log,argc,argv,"PROCESS_LOCKED",53);
        check_and_cleanup(workdir);
        return 53;
    }
    if(strcmp(argv[1],"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=get_default_conf(cluster_name,crypto_keyfile,1);
        if(run_flag==1||run_flag==127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The default configuration file has been downloaded to the local place.\n");
            printf("|          You can init directly, or edit it before init. Exit now.\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    if(strcmp(argv[1],"edit-conf")==0||strcmp(argv[1],"rm-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
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
            write_operation_log(cluster_name,operation_log,argc,argv,"NO_CONFIG_FILE",55);
            check_and_cleanup(workdir);
            return 55;
        }
        else{
            if(strcmp(argv[1],"rm-conf")==0){
                printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The previous configuration file has been deleted.\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(argv[1],"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster has already been initialized. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"ALREADY_INITED",57);
            check_and_cleanup(workdir);
            return 57;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=cluster_init_conf(cluster_name,argc,argv);
        if(run_flag==-5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid cloud vendor. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        else if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a configuration file. Exit now.\n" RESET_DISPLAY);
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Invalid format for the --nn and/or --un. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
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
            write_operation_log(cluster_name,operation_log,argc,argv,"WORKDIR_NOT_EXISTS",61);
            check_and_cleanup(workdir);
            return 61;
        }
        else if(run_flag==1){
            write_operation_log(cluster_name,operation_log,argc,argv,"AWS_REGION_VALID_FAILED",63);
            check_and_cleanup(workdir);
            return 63;
        }
        else if(run_flag==2){
            write_operation_log(cluster_name,operation_log,argc,argv,"DOWNLOAD/COPY_FILE_FAILED",65);
            check_and_cleanup(workdir);
            return 65;
        }
        else if(run_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"ZONE_ID_ERROR",67);
            check_and_cleanup(workdir);
            return 67;
        }
        else if(run_flag==4){
            write_operation_log(cluster_name,operation_log,argc,argv,"AWS_INVALID_KEYPAIR",69);
            check_and_cleanup(workdir);
            return 69;
        }
        else if(run_flag==5){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_INIT_FAILED",71);
            check_and_cleanup(workdir);
            return 71;
        }
        else if(run_flag==7){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_APPLY_FAILED_ROLLED_BACK",73);
            check_and_cleanup(workdir);
            return 73;
        }
        else if(run_flag==9){
            write_operation_log(cluster_name,operation_log,argc,argv,"TF_ROLLBACK_FAILED",75);
            check_and_cleanup(workdir);
            return 75;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(cluster_empty_or_not(workdir)==0){
        print_empty_cluster_info();
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
        check_and_cleanup(workdir);
        return 49;
    }

    if(strcmp(argv[1],"rebuild")==0){
        if(cmd_flag_check(argc,argv,"--mc")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"mc");
        }
        else if(cmd_flag_check(argc,argv,"--mcdb")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"mcdb");
        }
        else if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=rebuild_nodes(workdir,crypto_keyfile,"all");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify '--mc', '--mcdb', or '--all' as the second param.\n");
            printf("|          Run 'hpcopr help --cmd rebuild' for more details. Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"REBUILD_FAILED",34);
            check_and_cleanup(workdir);
            return 34;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",34);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(argv[1],"sleep")==0){
        if(cluster_asleep_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is " RESET_DISPLAY HIGH_CYAN_BOLD "not running" RESET_DISPLAY FATAL_RED_BOLD ". No need to hibernate.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup("");
            return 43;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"wakeup")==0){
        if(cluster_full_running_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "fully running" RESET_DISPLAY FATAL_RED_BOLD ". No need to wake up.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
            check_and_cleanup(workdir);
            return 38;
        }
        if(cluster_asleep_or_not(workdir)!=0){
            if(cmd_flag_check(argc,argv,"--all")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "minimal running" RESET_DISPLAY FATAL_RED_BOLD ". Please try \n" RESET_DISPLAY);
                printf(FATAL_RED_BOLD "|          " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD " to wake up the whole cluster.\n" RESET_DISPLAY);
                write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
                check_and_cleanup(workdir);
                return 38;
            }
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")!=0){
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"minimal");
            sprintf(string_temp,"%s default",argv[1]);
        }
        else{
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"all");
            sprintf(string_temp,"%s all",argv[1]);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(argv[1],"reconfc")==0||strcmp(argv[1],"reconfm")==0){
        if(cmd_keyword_check(argc,argv,"--conf",string_temp)!=0||cmd_flag_check(argc,argv,"--list")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the list. Have you inited this cluster?\n" RESET_DISPLAY);
                write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
            }
            if(strcmp(argv[1],"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to turn on all the compute nodes first.\n" RESET_DISPLAY);
            }
            if(strcmp(argv[1],"reconfm")==0&&cluster_asleep_or_not(workdir)==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to wake up the cluster first.\n" RESET_DISPLAY);
            }
        }
    }

    if(strcmp(argv[1],"userman")==0){
        if(cmd_keyword_check(argc,argv,"--ucmd",user_cmd)!=0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please input a user manager command below:\n");
            printf("|          list | add | delete | enable | disable | passwd \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%s",user_cmd);
            getchar();
        }
        if(strcmp(user_cmd,"list")!=0&&strcmp(user_cmd,"add")!=0&&strcmp(user_cmd,"delete")!=0&&strcmp(user_cmd,"enable")!=0&&strcmp(user_cmd,"disable")!=0&&strcmp(user_cmd,"passwd")!=0){
            print_usrmgr_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(user_cmd,"list")==0){
            run_flag=hpc_user_list(workdir,crypto_keyfile,0);
            if(cluster_asleep_or_not(workdir)==0){
                if(command_flag==2){
                    printf(WARN_YELLO_BOLD "[ -WARN- ] The specified cluster is not running.\n" RESET_DISPLAY);
                }
                else{
                    printf(WARN_YELLO_BOLD "[ -WARN- ] The switched cluster is not running.\n" RESET_DISPLAY);
                }
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;   
        }
        usrmgr_check_flag=usrmgr_prereq_check(workdir);
        if(usrmgr_check_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster is not running. Please wake up first\n" RESET_DISPLAY);
            check_and_cleanup(workdir);
            write_operation_log(cluster_name,operation_log,argc,argv,"USERMAN_PREREQ_CHECK_FAILED",77);
            return 77;
        }
        else if(usrmgr_check_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(user_cmd,"enable")==0||strcmp(user_cmd,"disable")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,user_name,crypto_keyfile,user_cmd);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(user_cmd,"add")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            cmd_keyword_check(argc,argv,"-p",pass_word);
            run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else if(strcmp(user_cmd,"delete")==0){
            cmd_keyword_check(argc,argv,"-u",user_name);
            run_flag=hpc_user_delete(workdir,crypto_keyfile,SSHKEY_DIR,user_name);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
        else{
            cmd_keyword_check(argc,argv,"-u",user_name);
            cmd_keyword_check(argc,argv,"-p",pass_word);
            run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
            if(run_flag==0){
                usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;
        }
    }

    if(strcmp(argv[1],"destroy")==0){
        if(cmd_flag_check(argc,argv,"--force")==0){
            run_flag=cluster_destroy(workdir,crypto_keyfile,"force");
        }
        else{
            run_flag=cluster_destroy(workdir,crypto_keyfile,"");
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
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
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        else{
            printf("|          Command: " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup --min | --all" RESET_DISPLAY FATAL_RED_BOLD ". Exit now.\n" RESET_DISPLAY);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_IS_ASLEEP",43);
        check_and_cleanup(workdir);
        return 43;
    }
    
    if(strcmp(argv[1],"delc")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=delete_compute_node(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=delete_compute_node(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now.\n" RESET_DISPLAY);
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
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
        if(cmd_keyword_check(argc,argv,"--nn",node_num_string)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("|          Exit now.\n" RESET_DISPLAY);
            write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,node_num_string);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"shutdownc")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now.\n" RESET_DISPLAY);
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"turnonc")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(workdir,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,"all");
        }
        else if(cmd_keyword_check(argc,argv,"--nn",node_num_string)==0){
            run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,node_num_string);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify either '--all' or '--nn NODE_NUM'. Exit now.\n" RESET_DISPLAY);
            run_flag=9;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
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
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--conf",string_temp);
        cmd_keyword_check(argc,argv,"--ht",string_temp2);
        run_flag=reconfigure_compute_node(workdir,crypto_keyfile,string_temp,string_temp2);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(argv[1],"reconfm")==0){
        if(confirm_to_operate_cluster(cluster_name)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        cmd_keyword_check(argc,argv,"--conf",string_temp);
        run_flag=reconfigure_master_node(workdir,crypto_keyfile,string_temp);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    write_operation_log(NULL,operation_log,argc,argv,"FATAL_ABNORMAL",run_flag);
    check_and_cleanup("");
    return 123;
}