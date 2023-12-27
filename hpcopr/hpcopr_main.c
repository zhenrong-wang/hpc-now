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
#include "monman.h"
#include "now_md5.h"
#include "opr_crypto.h"
#include "appman.h"
#include "jobman.h"
#include "userman.h"

char url_code_root_var[LOCATION_LENGTH]="";
char url_tf_root_var[LOCATION_LENGTH]="";
char url_shell_scripts_var[LOCATION_LENGTH]="";
char url_now_crypto_var[LOCATION_LENGTH]="";
char url_initutils_root_var[LOCATION_LENGTH]="";
char url_app_pkgs_root_var[LOCATION_LENGTH]="";
char url_app_inst_root_var[LOCATION_LENGTH]="";

int tf_loc_flag_var=0;
int code_loc_flag_var=0;
int now_crypto_loc_flag_var=0;

char terraform_version_var[32]="";
char tofu_version_var[32]=""; //Added openTofu version

char ali_tf_plugin_version_var[32]="";
char qcloud_tf_plugin_version_var[32]="";
char aws_tf_plugin_version_var[32]="";
char hw_tf_plugin_version_var[32]="";
char bd_tf_plugin_version_var[32]="";
char azrm_tf_plugin_version_var[32]="";
char azad_tf_plugin_version_var[32]="";
char az_environment[16]="";
char gcp_tf_plugin_version_var[32]="";

char md5_tf_exec_var[64]="";
char md5_tf_zip_var[64]="";

char md5_tofu_exec_var[64]=""; //Added openTofu md5
char md5_tofu_zip_var[64]="";  //Added openTofu zip md5

char md5_now_crypto_var[64]="";
char md5_ali_tf_var[64]="";
char md5_ali_tf_zip_var[64]="";
char md5_qcloud_tf_var[64]="";
char md5_qcloud_tf_zip_var[64]="";
char md5_aws_tf_var[64]="";
char md5_aws_tf_zip_var[64]="";
char md5_hw_tf_var[64]="";
char md5_hw_tf_zip_var[64]="";
char md5_bd_tf_var[64]="";
char md5_bd_tf_zip_var[64]="";
char md5_azrm_tf_var[64]="";
char md5_azrm_tf_zip_var[64]="";
char md5_azad_tf_var[64]="";
char md5_azad_tf_zip_var[64]="";
char md5_gcp_tf_var[64]="";
char md5_gcp_tf_zip_var[64]="";

int batch_flag=1; // If batch_flag=0: Batch Mode. If batch_flag!=0: interactive mode. use the -b flag
char final_command[512]="";
tf_exec_config tf_this_run;

/*
 * GEN: GENERAL COMMANDS
 * opr: ONLY Operator can execute
 * ADMIN: ONLY Operator and Admin can execute
 * 
 * CNAME: cluster_name is a must
 * UNAME: user_name is must
 */
char commands[COMMAND_NUM][COMMAND_STRING_LENGTH_MAX]={
    "envcheck,gen,NULL",
    "new-cluster,gen,NULL",
    "cloud-info,opr,CNAME",
    "ls-clusters,gen,NULL",
    "switch,gen,NULL",
    "glance,gen,NULL",
    "refresh,gen,CNAME",
    "export,gen,CNAME",
    "import,gen,NULL",
    "exit-current,gen,NULL",
    "remove,gen,CNAME",
    "help,gen,NULL",
    "usage,gen,NULL",
    "history,gen,NULL",
    "syserr,gen,NULL",
    "ssh,gen,UNAME",
    "rdp,gen,UNAME",
    "set-tf,gen,NULL",
    "decrypt,gen,NULL",
    "encrypt,gen,NULL",
    "configloc,gen,NULL",
    "showloc,gen,NULL",
    "resetloc,gen,NULL",
    "showmd5,gen,NULL",
    "rotate-key,opr,CNAME",
    "get-conf,opr,CNAME",
    "edit-conf,opr,CNAME",
    "rm-conf,opr,CNAME",
    "init,opr,CNAME",
    "rebuild,opr,CNAME",
    "vault,gen,CNAME",
    "graph,gen,CNAME",
    "viewlog,opr,CNAME",
    "delc,opr,CNAME",
    "addc,opr,CNAME",
    "shutdownc,opr,CNAME",
    "turnonc,opr,CNAME",
    "reconfc,opr,CNAME",
    "reconfm,opr,CNAME",
    "nfsup,opr,CNAME",
    "sleep,opr,CNAME",
    "wakeup,opr,CNAME",
    "destroy,opr,CNAME",
    "payment,opr,CNAME",
    "userman,gen,CNAME",
    "dataman,gen,UNAME",
    "appman,gen,UNAME",
    "jobman,gen,UNAME",
    "monman,admin,CNAME",
    "about,gen,NULL",
    "version,gen,NULL",
    "license,gen,NULL",
    "repair,gen,NULL"
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

char appman_commands[8][COMMAND_STRING_LENGTH_MAX]={
    "store",
    "avail",
    "build",
    "install",
    "check",
    "remove",
    "update-conf",
    "check-conf"
};

char jobman_commands[3][COMMAND_STRING_LENGTH_MAX]={
    "list",
    "submit",
    "cancel"
};

/*
1 NOT_A_VALID_COMMAND
3 USER_DENIED
4 FAILED_TO_GET_CLOUD_INFO
5 LACK_PARAMS
6 CLOUD_FUNCTION_UNSUPPORTED
7 MISSING_CLOUD_FLAG_FILE
8 CLOUD_FLAG_NOT_APPLICABLE
9 PARAM_FORMAT_ERROR

11 Prereq - Components Download and install failed
13 Prereq - Other failed
15 Prereq - Envcheck Failed
17 Prereq - Config Location Failed
19 Prereq - Vers and md5 Error
21 CLUSTER_NAME_CHECK_FAILED
23 INVALID_KEYPAIR
24 DECRYPTION_ENCRYPTION_FAILED
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
36 CLUSTER_ROLE_DOESN'T_MATCH
37 NO_NEED_TO_SWITCH
38 NO_NEED_TO_WAKEUP
39 NOT_IN_THE_REGISTRY
40 MONMAN_FAILED
41 DESTROY_ERROR
42 PAYMENT_SWITCH_FAILED
43 CLUSTER_ASLEEP
44 APPMAN_FAILED
45 GRAPH_FAILED
46 JOBMAN_FAILED
47 GRAPH_NOT_UPDATED
48 RDP_CONNECTION_FAILED
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
    int confirm_flag=0;
    int usrmgr_check_flag=0;
    char workdir[DIR_LENGTH]="";
    char cluster_name[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    char new_cluster_name[128]="";
    char gcp_flag[8]="";
    char key_echo_flag[8]="";
    char cloud_ak[AKSK_LENGTH]="";
    char cloud_sk[AKSK_LENGTH]="";
    char stream_name[128]="";
    char log_type[128]="";
    char user_name[128]="";
    char pass_word[128]="";
    char user_name_list[1024]="";
    char vault_bucket_flag[8]="";
    char vault_root_flag[8]="";
    char export_dest[FILENAME_LENGTH]="";
    char import_source[FILENAME_LENGTH]="";

    char data_cmd[128]="";
    char source_path[FILENAME_LENGTH]="";
    char destination_path[FILENAME_LENGTH]="";
    char target_path[FILENAME_LENGTH]="";
    char recursive_flag[16]="";
    char force_flag_string[16]="";
    char node_num_string[128]="";
    char user_cmd[128]="";
    char app_cmd[128]="";
    char app_name[128]="";
    char inst_loc[384]="";
    char repo_loc[384]="";
    char job_cmd[128]="";
    char job_id[32]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char* syserror_log=SYSTEM_CMD_ERROR_LOG;
    char string_temp[256]="";
    char string_temp2[256]="";
    char string_temp3[256]="";
    char string_temp4[4]="";
    char cmdline[CMDLINE_LENGTH]="";
    char cluster_role[8]="";
    int cluster_state_flag=0;
    int decrypt_flag=0;
    jobinfo job_info;

    print_header();

#ifdef _WIN32
    if(check_current_user()!=0){
        printf(WARN_YELLO_BOLD "|    Only the system user 'hpc-now' can run hpcopr. Steps:" RESET_DISPLAY "\n");
        printf("|    +- " GENERAL_BOLD "COMMAND MODE" RESET_DISPLAY " (pretty simple and fast):\n");
        printf("|       1. run command " HIGH_GREEN_BOLD "runas /savecred /user:mymachine\\hpc-now cmd" RESET_DISPLAY "\n");
        printf("|          The password of 'hpc-now' is needed for the first time.\n");
        printf("|       2. run " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY " commands in the " GENERAL_BOLD "new" RESET_DISPLAY " CMD Window.\n");
        printf("|    +- " GENERAL_BOLD "DESKTOP MODE" RESET_DISPLAY " (switch the whole desktop environment):\n");
        printf("|       1. Press " HIGH_GREEN_BOLD "Ctrl + Alt + Delete" RESET_DISPLAY "\n");
        printf("|       2. Select the user " HIGH_GREEN_BOLD "hpc-now" RESET_DISPLAY " and log in.\n");
        printf(FATAL_RED_BOLD "\n[ FATAL: ] You *MUST* run hpcopr as the system user 'hpc-now'." RESET_DISPLAY "\n");
        print_tail();
        return 117;
    }
#else
    if(check_current_user()!=0){
        printf(WARN_YELLO_BOLD "|    Only the system user 'hpc-now' can run hpcopr. Steps:" RESET_DISPLAY "\n");
        printf("|    +- " GENERAL_BOLD "SUDO MODE" RESET_DISPLAY " (pretty simple and fast for *sudoers*):\n");
        printf("|       Run with the " HIGH_GREEN_BOLD "sudo -u" RESET_DISPLAY " prefix, e.g. " HIGH_GREEN_BOLD "sudo -u hpc-now hpcopr ..." RESET_DISPLAY "\n");
        printf("|    +- " GENERAL_BOLD "USER MODE" RESET_DISPLAY " (for both *non-sudoers* and *sudoers*):" RESET_DISPLAY "\n");
        printf("|       1. " HIGH_GREEN_BOLD "su hpc-now" RESET_DISPLAY " (The password of 'hpc-now' is needed)\n");
        printf("|       2. run the " HIGH_GREEN_BOLD "hpcopr" RESET_DISPLAY " commands, e.g. " HIGH_GREEN_BOLD "hpcopr glance --all" RESET_DISPLAY "\n");
        printf(FATAL_RED_BOLD "\n[ FATAL: ] You *MUST* run hpcopr as the system user 'hpc-now'." RESET_DISPLAY "\n");
        print_tail();
        return 117;
    }
#endif

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
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
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
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
        printf("[ FATAL: ] Exit now." RESET_DISPLAY "\n");
        print_tail();
        return 119;
    }
#endif
    if(folder_exist_or_not(GENERAL_CONF_DIR)!=0){
        snprintf(cmdline,2047,"%s %s %s",MKDIR_CMD,GENERAL_CONF_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }

    command_flag=command_parser(argc,argv,command_name_prompt,workdir,cluster_name,user_name,cluster_role,&decrypt_flag);
    if(command_flag==-1){
        print_help("all");
        return 0;
    }
    else if(command_flag>199){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid Command. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
        print_tail();
        return command_flag;
    }
    else if(command_flag==-3){
        check_and_cleanup("");
        return 5;
    }
    else if(command_flag==-5){
        check_and_cleanup(workdir);
        return 5;
    }
    else if(command_flag==-7){
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
        check_and_cleanup(workdir);
        return 49;
    }
    else if(command_flag==1){
        check_and_cleanup(workdir);
        return 5;
    }
    if(strcmp(final_command,"help")==0){
        if(cmd_flag_check(argc,argv,"--all")==0){
            print_help("all");
            return 0;
        }
        if(cmd_keyword_check(argc,argv,"--cmd",string_temp)!=0){
            if(batch_flag==0){
                print_help("all");
                return 0;
            }
            confirm_flag=prompt_to_confirm("Select a command? (will display the whole doc if no command selected)",CONFIRM_STRING_QUICK,batch_flag);
            if(confirm_flag==1||confirm_flag==-1){
                print_help("all");
                return 0;
            }
            list_all_commands();
            prompt_to_input("Select one from the list above.",string_temp,batch_flag);
        }
        if(command_name_check(string_temp,command_name_prompt,string_temp2,string_temp3)>199){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified command name is incorrect. Did you mean " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ?\n" RESET_DISPLAY,command_name_prompt);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup("");
            return 9;
        }
        print_help(string_temp);
        return 0;
    } 

    if(strcmp(final_command,"about")==0){
        print_about();
        return 0;
    }
    
    if(strcmp(final_command,"version")==0){
        print_version();
        return 0;
    }

    if(check_internet()!=0){
        write_operation_log("NULL",operation_log,argc,argv,"INTERNET_CHECK_FAILED",121);
        check_and_cleanup("");
        return 121;
    }

    if(strcmp(final_command,"license")==0){
        if(cmd_flag_check(argc,argv,"--print")==0){
            read_license("print");
        }
        else{
            read_license("read");
        }
        print_tail();
        return 0;
    }

    if(strcmp(final_command,"repair")==0){
        confirm_flag=prompt_to_confirm("Entering repair mode and resetting to the defaults. Continue?",CONFIRM_STRING,batch_flag);
        if(confirm_flag==1){
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

    if(strcmp(final_command,"envcheck")==0){
        run_flag=prompt_to_confirm_args("Check the connectivity to GCP?",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--gcp");
        if(run_flag==0||run_flag==2){
            run_flag=check_and_install_prerequisitions(2);
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Will skip checking GCP connectivity if previously checked." RESET_DISPLAY "\n");
            run_flag=check_and_install_prerequisitions(0); // Check GCP Connectivity.
        }
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_FAILED",run_flag);
            check_and_cleanup("");
            return 15;
        }
        write_operation_log("NULL",operation_log,argc,argv,"ENVCHECK_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(final_command,"set-tf")==0){
        run_flag=show_tf_running_config();
        printf("\n");
        if(run_flag!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to get the tf running config file." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        run_flag=prompt_to_confirm("Update the tf running configurations listed above?",CONFIRM_STRING,batch_flag);
        if(run_flag==1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        prompt_to_input_required_args("Select a new tf execution:  terraform  tofu",string_temp,batch_flag,argc,argv,"--tf-run");
        prompt_to_input_required_args("Select a new debug log level: trace  debug  info  warn  error  off",string_temp2,batch_flag,argc,argv,"--dbg-level");
        prompt_to_input_required_args("Specify a new max wait time (600 - 1200) secs",string_temp3,batch_flag,argc,argv,"--max-time");
        run_flag=update_tf_running(string_temp,string_temp2,string_to_positive_num(string_temp3));
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(final_command,"configloc")==0){
        run_flag=configure_locations(batch_flag);
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

    if(strcmp(final_command,"decrypt")==0||strcmp(final_command,"encrypt")==0){
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=encrypt_decrypt_clusters("all",final_command,batch_flag);
        }
        else{
            if(cmd_keyword_ncheck(argc,argv,"-c",string_temp,255)==0){
                run_flag=encrypt_decrypt_clusters(string_temp,final_command,batch_flag);
            }
            else{
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a cluster list by " RESET_DISPLAY WARN_YELLO_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or use " RESET_DISPLAY WARN_YELLO_BOLD "--all" RESET_DISPLAY FATAL_RED_BOLD "." RESET_DISPLAY "\n");
                    write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
                    check_and_cleanup(workdir);
                    return 25;
                    
                }
                list_all_cluster_names(1);
                printf("[ -INFO- ] Input a list in the format " HIGH_GREEN_BOLD "cluster1:cluster2" RESET_DISPLAY " : ");
                fflush(stdin);
                scanf("%255s",string_temp);
                getchar();
                run_flag=encrypt_decrypt_clusters(string_temp,final_command,batch_flag);
            }
        }
        if(run_flag!=0){
            write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",24);
            check_and_cleanup("");
            return 24;
        }
        write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(final_command,"resetloc")==0){
        run_flag=reset_locations();
        if(run_flag==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The locations have been reset to the default.\n");
            show_locations();
            write_operation_log("NULL",operation_log,argc,argv,"OPERATION_SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal error, failed to reset the locations." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup("");
            return 127;
        }
    }

    if(strcmp(final_command,"showloc")==0){
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

    if(strcmp(final_command,"showmd5")==0){
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

    //Automatically encrypt the decrypted files for a specific cluster.
    if(decrypt_flag==1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The cluster " RESET_DISPLAY HIGH_GREEN_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD " is decrypted." RESET_DISPLAY "\n",cluster_name);
        encrypt_decrypt_clusters(cluster_name,"encrypt",0); //This operation is automatic. Use 0 as the batch flag.
    }
    
    if(strcmp(final_command,"new-cluster")==0){
        cmd_keyword_check(argc,argv,"--cname",new_cluster_name);
        cmd_keyword_check(argc,argv,"--sk",cloud_sk);
        cmd_keyword_check(argc,argv,"--ak",cloud_ak);
        cmd_keyword_check(argc,argv,"--az-sid",string_temp);
        cmd_keyword_check(argc,argv,"--az-tid",string_temp2);

        run_flag=prompt_to_confirm_args("Echo the input/imported credentials to this window (RISKY)?",CONFIRM_STRING,batch_flag,argc,argv,"--echo");
        if(run_flag==2||run_flag==0){
            strcpy(key_echo_flag,"echo");
        }
        run_flag=prompt_to_confirm_args("Use Google Cloud Platform? (Default: other cloud platforms)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--gcp");
        if(run_flag==2||run_flag==0){
            strcpy(gcp_flag,"gcp");
        }
        run_flag=create_new_cluster(crypto_keyfile,new_cluster_name,cloud_ak,cloud_sk,string_temp,string_temp2,key_echo_flag,gcp_flag,batch_flag);
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
            write_operation_log("NULL",operation_log,argc,argv,"INVALID_GCP_KEY_FILE",3);
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

    if(strcmp(final_command,"ls-clusters")==0){
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

    if(strcmp(final_command,"glance")==0){
        if(cmd_flag_check(argc,argv,"--all")==0){
            run_flag=glance_clusters("all",crypto_keyfile);
        }
        else{
            if(cmd_keyword_ncheck(argc,argv,"-c",cluster_name,24)!=0&&show_current_cluster(workdir,cluster_name,0)!=0){
                list_all_cluster_names(1);
                run_flag=prompt_to_input_required_args("Select a cluster name from the list above.",cluster_name,batch_flag,argc,argv,"-c");
                if(run_flag!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY WARN_YELLO_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster." RESET_DISPLAY "\n");
                    write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
                    check_and_cleanup(workdir);
                    return 25;
                }
            }
            run_flag=glance_clusters(cluster_name,crypto_keyfile);
        }
        if(run_flag==-1){
            write_operation_log("NULL",operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY HIGH_CYAN_BOLD "-c" RESET_DISPLAY FATAL_RED_BOLD ", or switch to a cluster." RESET_DISPLAY "\n");
            write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
            check_and_cleanup(workdir);
            return 25;
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified cluster name %s is not in the registry." RESET_DISPLAY "\n",cluster_name);
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup(workdir);
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"refresh")==0){
        run_flag=prompt_to_confirm_args("Do force refresh?",CONFIRM_STRING,batch_flag,argc,argv,"--all");
        if(run_flag==2||run_flag==0){
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"force",&tf_this_run);
        }
        else{
            if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
                write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
                check_and_cleanup(workdir);
                return 3;
            }
            run_flag=refresh_cluster(cluster_name,crypto_keyfile,"",&tf_this_run);
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

    if(strcmp(final_command,"exit-current")==0){
        if(exit_current_cluster()==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Exited the switched cluster. You can switch to one later.\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        else{
            printf(FATAL_RED_BOLD "[ -INFO- ] Failed to exit the switched cluster. Please retry later." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"OPERATION_FAILED",35);
            check_and_cleanup("");
            return 35;
        }
    }

    if(strcmp(final_command,"usage")==0){
        prompt_to_input_optional_args("Export to local path?",CONFIRM_STRING_QUICK,"Specify a local path (directory or file).",export_dest,batch_flag,argc,argv,"-d");
        run_flag=prompt_to_confirm_args("Read the usage log? (Default: Print)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--read");
        if(run_flag==2||run_flag==0){
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
    if(strcmp(final_command,"history")==0){
        prompt_to_input_optional_args("Export to local path?",CONFIRM_STRING_QUICK,"Specify a local path (directory or file).",export_dest,batch_flag,argc,argv,"-d");
        run_flag=prompt_to_confirm_args("Read the command history log? (Default: Print)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--read");
        if(run_flag==2||run_flag==0){
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

    if(strcmp(final_command,"syserr")==0){
        prompt_to_input_optional_args("Export to a local path?",CONFIRM_STRING_QUICK,"Specify a local path (directory or file).",export_dest,batch_flag,argc,argv,"-d");
        run_flag=prompt_to_confirm_args("Read the system error log? (Default: Print)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--read");
        if(run_flag==2||run_flag==0){
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

    if(strcmp(final_command,"import")==0){
        cmd_keyword_ncheck(argc,argv,"-s",import_source,511);
        cmd_keyword_check(argc,argv,"-p",pass_word);
        run_flag=import_cluster(import_source,pass_word,crypto_keyfile,batch_flag);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"IMPORT_FAILED",32);
            check_and_cleanup("");
            return 32;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(final_command,"monman")==0){
        prompt_to_input_optional_args("Specify node list? (Default: all)",CONFIRM_STRING_QUICK,"Specify nodes connected by :, i.e. compute1:compute2:master",string_temp,batch_flag,argc,argv,"-n");
        prompt_to_input_optional_args("Specify start date & time? (Default: The first timestamp)",CONFIRM_STRING_QUICK,"Specify a strictly-formatted start timestamp. i.e. 2023-1-1@12:10",string_temp2,batch_flag,argc,argv,"-s");
        prompt_to_input_optional_args("Specify end date & time? (Default: The last timestamp)",CONFIRM_STRING_QUICK,"Specify a strictly-formatted start timestamp. i.e. 2023-1-1@12:10",string_temp3,batch_flag,argc,argv,"-e");
        prompt_to_input_optional_args("Specify a time interval? (Default: 5 minutes)",CONFIRM_STRING_QUICK,"Specify a positive number.",string_temp4,batch_flag,argc,argv,"--level");
        prompt_to_input_optional_args("Export to a local path?",CONFIRM_STRING_QUICK,"Specify a local path (directory or file).",destination_path,batch_flag,argc,argv,"-d");
        run_flag=prompt_to_confirm_args("Read the monitor data? (Default: Print)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--read");
        if(run_flag==2||run_flag==0){
            run_flag=show_cluster_mon_data(cluster_name,SSHKEY_DIR,string_temp,string_temp2,string_temp3,string_temp4,"read",destination_path);
        }
        else{
            run_flag=show_cluster_mon_data(cluster_name,SSHKEY_DIR,string_temp,string_temp2,string_temp3,string_temp4,"print",destination_path);
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"MONITOR_MANAGER_FAILED",40);
            check_and_cleanup(workdir);
            return 40;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"switch")==0){
        if(cmd_flag_check(argc,argv,"--list")==0){
            run_flag=list_all_cluster_names(2);
            write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup("");
            return 0;
        }
        if(cmd_keyword_ncheck(argc,argv,"-c",cluster_name,24)!=0){
            if(batch_flag==0){
                list_all_cluster_names(1);
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify a target cluster by " RESET_DISPLAY WARN_YELLO_BOLD "-c CLUSTER_NAME" RESET_DISPLAY FATAL_RED_BOLD " ." RESET_DISPLAY "\n");
                write_operation_log("NULL",operation_log,argc,argv,"NOT_OPERATING_CLUSTERS",25);
                check_and_cleanup(workdir);
                return 25;
            }
            else{
                list_all_cluster_names(1);
                prompt_to_input_required_args("Select a target cluster name from the list above.",cluster_name,batch_flag,argc,argv,"-c");
            }
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
            write_operation_log("NULL",operation_log,argc,argv,"NOT_IN_THE_CLUSTER_REGISTRY",39);
            check_and_cleanup("");
            return 39;
        }
        write_operation_log("NULL",operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup("");
        return 0;
    }

    if(strcmp(final_command,"remove")==0){
        if(cmd_flag_check(argc,argv,"--force")==0||batch_flag==0){
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"force",&tf_this_run);
        }
        else{
            run_flag=remove_cluster(cluster_name,crypto_keyfile,"",&tf_this_run);
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

    if(strcmp(final_command,"viewlog")==0){
        prompt_to_input_optional_args("Export to a local path?",CONFIRM_STRING_QUICK,"Specify a local path (directory or file).",string_temp,batch_flag,argc,argv,"-d");
        prompt_to_input_optional_args("Specify a log stream? (Default: std output stream)",CONFIRM_STRING_QUICK,"Select a log stream: std(default)   err   dbg",stream_name,batch_flag,argc,argv,"--log");
        run_flag=prompt_to_confirm_args("View historical run log? (Default: realtime run log)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--hist");
        if(run_flag==2||run_flag==0){
            strcpy(log_type,"archive");
        }
        else{
            strcpy(log_type,"realtime");
        }
        run_flag=prompt_to_confirm_args("Print out the log? (Default: stream out the log)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--print");
        if(run_flag==2||run_flag==0){
            run_flag=view_run_log(workdir,stream_name,log_type,"print",string_temp);
        }
        else{
            run_flag=view_run_log(workdir,stream_name,log_type,"",string_temp);
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
    
    cluster_state_flag=cluster_asleep_or_not(workdir);
    if(strcmp(final_command,"ssh")==0){
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Trying to ssh " HIGH_GREEN_BOLD "%s@%s" RESET_DISPLAY ".\n",user_name,cluster_name);
        if(strcmp(user_name,"root")==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] SSH as root is VERY RISKY and *NOT* recommended !" RESET_DISPLAY "\n");
        }
        run_flag=cluster_ssh(workdir,user_name,cluster_role,SSHKEY_DIR);
        if(run_flag==-1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the ssh key. You can still try to use password to login." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"rdp")==0){
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        run_flag=prompt_to_confirm_args("Copy the password to system clipboard? (Default: not copy password)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--copypass");
        if(run_flag==2||run_flag==0){
            run_flag=cluster_rdp(workdir,user_name,cluster_role,0);
        }
        else{
            run_flag=cluster_rdp(workdir,user_name,cluster_role,1);
        }
        if(run_flag==0||run_flag==9){
            if(run_flag==9){
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " RDP client exited (either normally or unexpectedly).\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"RDP_EXITED",run_flag);
            check_and_cleanup(workdir);
            return 0;
        }
        else if(run_flag==-3){
            printf(FATAL_RED_BOLD "[ FATAL: ] You do not have the privilege to connect as root." RESET_DISPLAY "\n");
        }
        else if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get and copy the password of %s ." RESET_DISPLAY "\n",user_name);
        }
        else if(run_flag==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cluster name." RESET_DISPLAY "\n");
        }
        else if(run_flag==5){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the latest IP address of the cluster." RESET_DISPLAY "\n");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to generate an RDP connection file." RESET_DISPLAY "\n");
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"RDP_FAILED",48);
        check_and_cleanup(workdir);
        return 48;
    }

    if(strcmp(final_command,"graph")==0){
        prompt_to_input_optional_args("Specify a graph level? (Default: detailed graph)",CONFIRM_STRING_QUICK,"Select a level: csv   txt   graph(default)",string_temp,batch_flag,argc,argv,"--level");
        if(strcmp(string_temp,"csv")==0){
            level_flag=2;
        }
        else if(strcmp(string_temp,"txt")==0){
            level_flag=1;
        }
        else{
            level_flag=0;
        }
        if(check_pslock(workdir,decryption_status(workdir))!=0){
            if(cluster_empty_or_not(workdir)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !\n" RESET_DISPLAY "\n",cluster_name);
            }
            else{
                printf(WARN_YELLO_BOLD "[ -WARN- ] %s | * OPERATION-IN-PROGRESS * Graph NOT updated !" RESET_DISPLAY "\n",cluster_name);
            }
            run_flag=graph(workdir,crypto_keyfile,level_flag);
            if(run_flag==1){
                write_operation_log(cluster_name,operation_log,argc,argv,"GRAPH_FAILED",45);
                check_and_cleanup(workdir);
                return 47;
            }
            else if(run_flag==-1){
                write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
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
        else if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        delete_decrypted_files(workdir,crypto_keyfile);
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"vault")==0){
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(batch_flag!=0&&cmd_keyword_check(argc,argv,"-u",user_name)!=0){
            hpc_user_list(workdir,crypto_keyfile,0);
        }
        prompt_to_input_optional_args("Display credentials of a specific user? (Default: all users except root)",CONFIRM_STRING_QUICK,"Select a user from the list above.",user_name,batch_flag,argc,argv,"-u");
        run_flag=prompt_to_confirm_args("Display bucket credentials? (Default: hide)",CONFIRM_STRING,batch_flag,argc,argv,"--bkey");
        if(run_flag==2||run_flag==0){
            strcpy(vault_bucket_flag,"bucket");
        }
        run_flag=prompt_to_confirm_args("Display root password? (Default: hide)",CONFIRM_STRING,batch_flag,argc,argv,"--rkey");
        if(run_flag==2||run_flag==0){
            strcpy(vault_root_flag,"root");
        }
        run_flag=get_vault_info(workdir,crypto_keyfile,user_name,vault_bucket_flag,vault_root_flag);
        if(run_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"FILE_I/O_ERROR",127);
            check_and_cleanup(workdir);
            return 127;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"cloud-info")==0){
        run_flag=display_cloud_info(workdir);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"FAILED_TO_GET_CLOUD_INFO",4);
            check_and_cleanup(workdir);
            return 4;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"rotate-key")==0){
        get_cloud_flag(workdir,cloud_flag);
        cmd_keyword_ncheck(argc,argv,"--ak",cloud_ak,255);
        cmd_keyword_ncheck(argc,argv,"--sk",cloud_sk,255);
        run_flag=prompt_to_confirm_args("Echo the credentials to this window (RISKY)?",CONFIRM_STRING,batch_flag,argc,argv,"--echo");
        if(run_flag==2||run_flag==0){
            strcpy(key_echo_flag,"echo");
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=rotate_new_keypair(workdir,cloud_ak,cloud_sk,crypto_keyfile,key_echo_flag,batch_flag);
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
        printf("|          Exit now." RESET_DISPLAY "\n");
        write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FLAG_CHECK_FAILED",7);
        check_and_cleanup(workdir);
        return 7;
    }

    if(strcmp(final_command,"export")==0){
        if(cluster_empty_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is empty, nothing to be exported." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        cmd_keyword_ncheck(argc,argv,"--ul",user_name_list,1023);
        cmd_keyword_check(argc,argv,"-p",pass_word);
        cmd_keyword_ncheck(argc,argv,"-d",export_dest,511);
        run_flag=prompt_to_confirm_args("Export the admin privilege? (Default: no)",CONFIRM_STRING,batch_flag,argc,argv,"--admin");
        if(run_flag==2||run_flag==0){
            strcpy(string_temp,"admin");
        }
        run_flag=export_cluster(cluster_name,user_name_list,string_temp,crypto_keyfile,pass_word,export_dest,batch_flag);
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

    if(strcmp(final_command,"dataman")==0){
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(cmd_keyword_check(argc,argv,"--dcmd",data_cmd)!=0){
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] No dataman subcmd specified. Use --dcmd SUBCMD ." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                check_and_cleanup(workdir);
                return 5;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a valid command to use the data manager.\n");
            printf(GENERAL_BOLD "|       +- Bucket:" RESET_DISPLAY " put, get, copy, list, delete, move\n");
            printf(GENERAL_BOLD "|       +- Remote:" RESET_DISPLAY " cp, mv, ls, rm, mkdir, cat, more, less, tail, rput, rget \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%127s",data_cmd);
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
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Valid commands:" RESET_DISPLAY "\n",data_cmd);
            printf(GENERAL_BOLD "|       +- Bucket:" RESET_DISPLAY " put, get, copy, list, delete, move\n");
            printf(GENERAL_BOLD "|       +- Remote:" RESET_DISPLAY " cp, mv, ls, rm, mkdir, cat, more, less, tail, rput, rget \n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"list")!=0&&strcmp(data_cmd,"delete")!=0&&strcmp(data_cmd,"move")!=0){
            if(cluster_state_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to wake up the cluster " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " first.\n" RESET_DISPLAY,cluster_name);
                write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
                check_and_cleanup(workdir);
                return 43;
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0){
            if(cmd_keyword_ncheck(argc,argv,"-s",source_path,511)!=0){
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] No source path specified. Use -s SOURCE_PATH ." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                    check_and_cleanup(workdir);
                    return 5;
                }
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a source path for this command.\n");
                if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"move")!=0){
                    printf("|          Use prefix @h/ , @d/, @p/, @a/, @R/, @t/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%511s",source_path);
                getchar();
            }
            if(cmd_keyword_ncheck(argc,argv,"-d",destination_path,511)!=0){
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] No destination path specified. Use -d DEST_PATH ." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                    check_and_cleanup(workdir);
                    return 5;
                }
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a destination path for this command.\n");
                if(strcmp(data_cmd,"put")!=0&&strcmp(data_cmd,"get")!=0&&strcmp(data_cmd,"copy")!=0&&strcmp(data_cmd,"move")!=0){
                    printf("|          Use prefix @h/ , @d/, @p/, @a/, @R/, @t/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%511s",destination_path);
                getchar();
            }
        }
        else{
            if(cmd_keyword_ncheck(argc,argv,"-t",target_path,511)!=0){
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] No target path specified. Use -t TARGET_PATH ." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                    check_and_cleanup(workdir);
                    return 5;
                }
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a target path for this command.\n");
                if(strcmp(data_cmd,"list")!=0&&strcmp(data_cmd,"delete")!=0){
                    printf("|          Use prefix @h/ , @d/ , @p/, @a/, @R/ to specify " HIGH_CYAN_BOLD "Cluster paths" RESET_DISPLAY ".\n");
                }
                printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
                fflush(stdin);
                scanf("%511s",target_path);
                getchar();
            }
        }
        if(strcmp(data_cmd,"put")==0||strcmp(data_cmd,"get")==0||strcmp(data_cmd,"copy")==0||strcmp(data_cmd,"move")==0||strcmp(data_cmd,"delete")==0||strcmp(data_cmd,"list")==0||strcmp(data_cmd,"cp")==0||strcmp(data_cmd,"mv")==0||strcmp(data_cmd,"rput")==0||strcmp(data_cmd,"rget")==0||strcmp(data_cmd,"rm")==0){
            if(strcmp(data_cmd,"list")!=0&&cmd_flag_check(argc,argv,"--force")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-f")!=0){
                if(batch_flag==0){
                    printf(GENERAL_BOLD "[ -INFO- ] You may need --force or -f flag to do force operation.\n" RESET_DISPLAY );
                    strcpy(force_flag_string,"");
                }
                else{
                    if(prompt_to_confirm("Enable force operation (RISKY!) ?",CONFIRM_STRING,batch_flag)==0){
                        strcpy(force_flag_string,"force");
                    }
                    else{
                        strcpy(force_flag_string,"");
                    }
                }
            }
            else{
                strcpy(force_flag_string,"force");
            }
            if(strcmp(data_cmd,"mv")!=0&&cmd_flag_check(argc,argv,"--recursive")!=0&&cmd_flag_check(argc,argv,"-rf")!=0&&cmd_flag_check(argc,argv,"-r")!=0){
                if(batch_flag==0){
                    printf(GENERAL_BOLD "[ -INFO- ] You may need --recursive or -r flag when operating folders.\n" RESET_DISPLAY );
                    strcpy(recursive_flag,"");
                }
                else{
                    if(prompt_to_confirm("Enable recursive operation (required for folders)?",CONFIRM_STRING,batch_flag)==0){
                        strcpy(recursive_flag,"recursive");
                    }
                    else{
                        strcpy(recursive_flag,"");
                    }
                }
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
                printf(FATAL_RED_BOLD "[ FATAL: ] The source and dest path must include @ prefixes." RESET_DISPLAY "\n");
            }
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] Data operation failed or canceled. Check the console output above.\n");
            printf("|      <>  Command: %s | Cluster: %s | User: %s\n" RESET_DISPLAY,data_cmd,cluster_name,user_name);
            write_operation_log(cluster_name,operation_log,argc,argv,"DATAMAN_OPERATION_FAILED",28);
            check_and_cleanup(workdir);
            return 28;
        }
    }

    if(check_pslock(workdir,decryption_status(workdir))==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Another process is operating this cluster, please wait and retry.\n");
        printf("|          Exit now." RESET_DISPLAY "\n");
        write_operation_log(cluster_name,operation_log,argc,argv,"PROCESS_LOCKED",53);
        check_and_cleanup(workdir);
        return 53;
    }
    if(strcmp(final_command,"get-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        run_flag=prompt_to_confirm_args("Edit the conf file after downloading? (Default: don't edit)",CONFIRM_STRING_QUICK,batch_flag,argc,argv,"--edit");
        if(run_flag==2||run_flag==0){
            strcpy(string_temp,"edit");
        }
        else{
            strcpy(string_temp,"");
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=get_default_conf(cluster_name,crypto_keyfile,string_temp);
        if(run_flag==1||run_flag==127){
            printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting." RESET_DISPLAY "\n");
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

    if(strcmp(final_command,"edit-conf")==0||strcmp(final_command,"rm-conf")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          this operation is not allowed. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_NOT_EMPTY",51);
            check_and_cleanup(workdir);
            return 51;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(strcmp(final_command,"edit-conf")==0){
            run_flag=edit_configuration_file(cluster_name,crypto_keyfile,batch_flag);
        }
        else{
            run_flag=remove_conf(cluster_name);
        }
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] No configuration file found. You can run " WARN_YELLO_BOLD "hpcopr get-conf" RESET_DISPLAY FATAL_RED_BOLD " first.\n");
            printf("|          Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"NO_CONFIG_FILE",55);
            check_and_cleanup(workdir);
            return 55;
        }
        else{
            if(strcmp(final_command,"rm-conf")==0){
                printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The previous configuration file has been deleted.\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(final_command,"init")==0){
        if(cluster_empty_or_not(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster has already been initialized. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"ALREADY_INITED",57);
            check_and_cleanup(workdir);
            return 57;
        }
        run_flag=cluster_init_conf(cluster_name,batch_flag,code_loc_flag_var,url_code_root_var,argc,argv);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"INIT_CONFIG_FAILED",58);
            check_and_cleanup(workdir);
            return 58;
        }
        if(strcmp(cloud_flag,"CLOUD_A")==0){
            run_flag=alicloud_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_B")==0){
            run_flag=qcloud_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_C")==0){
            run_flag=aws_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_D")==0){
            run_flag=hwcloud_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_E")==0){
            run_flag=baiducloud_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_F")==0){
            run_flag=azure_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else if(strcmp(cloud_flag,"CLOUD_G")==0){
            run_flag=gcp_cluster_init(workdir,crypto_keyfile,batch_flag,&tf_this_run);
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Unknown Cloud Service Provider. Exit now." RESET_DISPLAY "\n");
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

    if(strcmp(final_command,"payment")==0){
        if(cmd_flag_check(argc,argv,"--od")!=0&&cmd_flag_check(argc,argv,"--month")!=0){
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify the new payment method with " WARN_YELLO_BOLD "--od" FATAL_RED_BOLD " or " WARN_YELLO_BOLD "--month" RESET_DISPLAY " .\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            prompt_to_input("Select an option by number: [1] Switch to OD    [2] Switch to Monthly",string_temp,batch_flag);
            if(strcmp(string_temp,"1")!=0&&strcmp(string_temp,"2")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified option " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid.\n",string_temp);
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            else if(strcmp(string_temp,"1")==0){
                strcpy(string_temp2,"od");
            }
            else{
                strcpy(string_temp2,"month");
            }
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=switch_cluster_payment(cluster_name,string_temp2,crypto_keyfile,&tf_this_run);
        if(run_flag==0){
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SWITCH_PAYMENT_FAILED",42);
            check_and_cleanup(workdir);
            return 42;
        }
    }

    if(strcmp(final_command,"rebuild")==0){
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please wake up the cluster first." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        if(cmd_flag_check(argc,argv,"--mc")==0){
            strcpy(string_temp,"mc");
        }
        else if(cmd_flag_check(argc,argv,"--mcdb")==0){
            strcpy(string_temp,"mcdb");
        }
        else if(cmd_flag_check(argc,argv,"--all")==0){
            strcpy(string_temp,"all");
        }
        else{
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Please specify '--mc', '--mcdb', or '--all' as the second param.\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            prompt_to_input("Select an option by number: [1] mc  [2] mddb  [3] all",string_temp2,batch_flag);
            if(strcmp(string_temp2,"1")!=0&&strcmp(string_temp2,"2")!=0&&strcmp(string_temp2,"3")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified option " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid.\n",string_temp2);
                write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                check_and_cleanup(workdir);
                return 9;
            }
            else if(strcmp(string_temp2,"1")==0){
                strcpy(string_temp,"mc");
            }
            else if(strcmp(string_temp2,"2")==0){
                strcpy(string_temp,"mcdb");
            }
            else{
                strcpy(string_temp,"all");
            }
        }
        run_flag=rebuild_nodes(workdir,crypto_keyfile,string_temp,batch_flag,&tf_this_run);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"REBUILD_FAILED",34);
            check_and_cleanup(workdir);
            return 34;
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",34);
        check_and_cleanup(workdir);
        return 0;
    }

    if(strcmp(final_command,"nfsup")==0){
        if(strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] This command is only applicable to CLOUD_D, CLOUD_F, and CLOUD_G." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FLAG_NOT_APPLICABLE",8);
            check_and_cleanup(workdir);
            return 8;
        }
        if(cluster_empty_or_not(workdir)==0){
            print_empty_cluster_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_EMPTY",49);
            check_and_cleanup(workdir);
            return 49;
        }
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please wake up the cluster first." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup(workdir);
            return 43;
        }
        run_flag=prompt_to_input_required_args("Please specify a positive number as the new volume",string_temp,batch_flag,argc,argv,"--vol");
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] New volume not specified. Exit now." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(string_to_positive_num(string_temp)<1){
            printf(FATAL_RED_BOLD "[ FATAL: ] The new volume %s is invalid." RESET_DISPLAY "\n",string_temp);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        get_state_value(workdir,"shared_volume_gb:",string_temp2);
        if(string_to_positive_num(string_temp)<string_to_positive_num(string_temp2)){
            printf(FATAL_RED_BOLD "[ FATAL: ] The new volume %s is not larger than the previous %s." RESET_DISPLAY "\n",string_temp,string_temp2);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=nfs_volume_up(workdir,crypto_keyfile,string_temp,&tf_this_run);
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"NFS_VOLUME_UP_FAILED",10);
            check_and_cleanup(workdir);
            return 10;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(final_command,"sleep")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(cluster_state_flag==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is not running. No need to hibernate." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_ASLEEP",43);
            check_and_cleanup("");
            return 43;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup("");
            return 3;
        }
        run_flag=cluster_sleep(workdir,crypto_keyfile,&tf_this_run);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    if(strcmp(final_command,"wakeup")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(cluster_full_running_or_not(workdir)==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "fully running" RESET_DISPLAY FATAL_RED_BOLD ". No need to wake up." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
            check_and_cleanup(workdir);
            return 38;
        }
        if(cluster_state_flag!=0){
            if(cmd_flag_check(argc,argv,"--all")!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] The cluster is already " RESET_DISPLAY HIGH_CYAN_BOLD "minimal running" RESET_DISPLAY FATAL_RED_BOLD ". Please try " RESET_DISPLAY "\n");
                printf(FATAL_RED_BOLD "|          " RESET_DISPLAY HIGH_GREEN_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD " to wake up the whole cluster." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"RUNNING_STATE",38);
                check_and_cleanup(workdir);
                return 38;
            }
        }
        run_flag=prompt_to_confirm_args("Wakeup all the nodes? (Default: minimal)",CONFIRM_STRING,batch_flag,argc,argv,"--all");
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        if(run_flag==2||run_flag==0){
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"all",&tf_this_run);
        }
        else{
            run_flag=cluster_wakeup(workdir,crypto_keyfile,"minimal",&tf_this_run);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"reconfc")==0||strcmp(final_command,"reconfm")==0){
        if(cmd_flag_check(argc,argv,"--list")==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Available configuration list:\n|\n");
            if(check_reconfigure_list(workdir,1)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the list. Have you inited this cluster?" RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
                check_and_cleanup(workdir);
                return 125;
            }
            if(strcmp(final_command,"reconfc")==0&&check_down_nodes(workdir)!=0&&strcmp(cloud_flag,"CLOUD_B")==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to turn on all the compute nodes first." RESET_DISPLAY "\n");
            }
            if(strcmp(final_command,"reconfm")==0&&cluster_state_flag==0){
                printf("|\n" WARN_YELLO_BOLD "[ -WARN- ] You need to wake up the cluster first." RESET_DISPLAY "\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
    }

    if(strcmp(final_command,"userman")==0){
        if(cmd_keyword_check(argc,argv,"--ucmd",user_cmd)!=0){
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] No userman subcmd specified. Use --ucmd SUBCMD ." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                check_and_cleanup(workdir);
                return 5;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a user manager command below:\n");
            printf("|          list | add | delete | enable | disable | passwd \n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%127s",user_cmd);
            getchar();
        }
        if(strcmp(user_cmd,"list")!=0&&strcmp(user_cmd,"add")!=0&&strcmp(user_cmd,"delete")!=0&&strcmp(user_cmd,"enable")!=0&&strcmp(user_cmd,"disable")!=0&&strcmp(user_cmd,"passwd")!=0){
            print_usrmgr_info();
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(user_cmd,"list")!=0&&strcmp(cluster_role,"opr")!=0&&strcmp(cluster_role,"admin")!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need the opr or admin role to run " WARN_YELLO_BOLD "--ucmd %s" RESET_DISPLAY FATAL_RED_BOLD " . Current role: " RESET_DISPLAY WARN_YELLO_BOLD "%s " RESET_DISPLAY "\n",user_cmd,cluster_role);
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_CLUSTER_ROLE",36);
            check_and_cleanup(workdir);
            return 36;
        }
        if(strcmp(user_cmd,"list")==0){
            run_flag=hpc_user_list(workdir,crypto_keyfile,0);
            if(cluster_state_flag==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The specified/switched cluster is not running." RESET_DISPLAY "\n");
            }
            write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
            check_and_cleanup(workdir);
            return run_flag;   
        }
        usrmgr_check_flag=usrmgr_prereq_check(workdir,user_cmd,batch_flag);
        //printf("\n\n %s \n\n",user_cmd);
        if(usrmgr_check_flag==-1){
            write_operation_log(cluster_name,operation_log,argc,argv,"USERMAN_PREREQ_CHECK_FAILED",77);
            check_and_cleanup(workdir);
            return 77;
        }
        else if(usrmgr_check_flag==3){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        //printf("\n\n %s \n\n",user_cmd);
        hpc_user_list(workdir,crypto_keyfile,0);
        if(strcmp(user_cmd,"add")==0){
            snprintf(string_temp,255,"Input a *UNIQUE* username (A-Z | a-z | 0-9 | - , Length %d-%d",USERNAME_LENGTH_MIN,USERNAME_LENGTH_MAX);
        }
        else{
            strcpy(string_temp,"Please select a username.");
        }
        if(cmd_keyword_check(argc,argv,"-u",user_name)!=0&&prompt_to_input(string_temp,user_name,batch_flag)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Username not specified." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(user_cmd,"add")==0||strcmp(user_cmd,"passwd")==0){
            if(cmd_keyword_check(argc,argv,"-p",pass_word)!=0){
                if(input_user_passwd(pass_word,batch_flag)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] Password not specified." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                    check_and_cleanup(workdir);
                    return 9;
                }
            }
            else{
                if(user_password_complexity_check(pass_word,SPECIAL_PASSWORD_CHARS)!=0){
                    write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
                    check_and_cleanup(workdir);
                    return 9;
                }
            }
        }
        if(strcmp(user_cmd,"enable")==0||strcmp(user_cmd,"disable")==0){
            run_flag=hpc_user_enable_disable(workdir,SSHKEY_DIR,user_name,crypto_keyfile,user_cmd);
        }
        else if(strcmp(user_cmd,"add")==0){
            run_flag=hpc_user_add(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
        }
        else if(strcmp(user_cmd,"delete")==0){
            run_flag=hpc_user_delete(workdir,crypto_keyfile,SSHKEY_DIR,user_name);
        }
        else{
            run_flag=hpc_user_setpasswd(workdir,SSHKEY_DIR,crypto_keyfile,user_name,pass_word);
        }
        if(run_flag==0){
            usrmgr_remote_exec(workdir,SSHKEY_DIR,usrmgr_check_flag);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,user_cmd,run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"destroy")==0){
        run_flag=prompt_to_confirm_args("Do force destroy? (RISKY!)",CONFIRM_STRING,batch_flag,argc,argv,"--force");
        if(run_flag==2||run_flag==0){
            run_flag=cluster_destroy(workdir,crypto_keyfile,"force",batch_flag,&tf_this_run);
        }
        else{
            run_flag=cluster_destroy(workdir,crypto_keyfile,"",batch_flag,&tf_this_run);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(cluster_state_flag==0){
        if(strcmp(final_command,"addc")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Cluster not running. Use " RESET_DISPLAY WARN_YELLO_BOLD "hpcopr wakeup --all" RESET_DISPLAY FATAL_RED_BOLD " to wake it up." RESET_DISPLAY "\n");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] Cluster not running. Use " RESET_DISPLAY WARN_YELLO_BOLD "hpcopr wakeup --all | --min" RESET_DISPLAY FATAL_RED_BOLD " to wake it up." RESET_DISPLAY "\n");
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"CLUSTER_IS_ASLEEP",43);
        check_and_cleanup(workdir);
        return 43;
    }

    if(strcmp(final_command,"appman")==0){
        if(cmd_keyword_check(argc,argv,"--acmd",app_cmd)!=0){
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] No appman subcmd specified. Use --acmd SUBCMD ." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                check_and_cleanup(workdir);
                return 5;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a valid command:\n");
            printf(GENERAL_BOLD "|         " RESET_DISPLAY " store, avail, build, install, check, remove, update-conf, check-conf\n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%127s",app_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(app_cmd,appman_commands[i])!=0){
            i++;
            if(i==8){
                break;
            }
        }
        if(i==8){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Valid commands:" RESET_DISPLAY "\n",app_cmd);
            printf(GENERAL_BOLD "|         " RESET_DISPLAY " store, avail, build, install, check, remove, update-conf, check-conf\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        cmd_keyword_ncheck(argc,argv,"--repo",repo_loc,383);
        if(strcmp(app_cmd,"store")==0){
            prompt_to_input_optional_args("Specify installation shell scripts location?",CONFIRM_STRING,"Input either an URL or a local path.",inst_loc,batch_flag,argc,argv,"--inst");
            run_flag=app_list(workdir,"all",user_name,"",SSHKEY_DIR,"",inst_loc);
        }
        else if(strcmp(app_cmd,"avail")==0){
            run_flag=app_list(workdir,"installed",user_name,"",SSHKEY_DIR,"","");
        }
        else if(strcmp(app_cmd,"check-conf")==0){
            run_flag=appman_check_conf(workdir,user_name,SSHKEY_DIR);
        }
        else if(strcmp(app_cmd,"update-conf")==0){
            prompt_to_input_required_args("Input either an URL or a local path for installation scripts.",inst_loc,batch_flag,argc,argv,"--inst");
            prompt_to_input_required_args("Input either an URL or a local path for application sources and packages.",repo_loc,batch_flag,argc,argv,"--repo");
            run_flag=appman_update_conf(workdir,inst_loc,repo_loc,SSHKEY_DIR,"");
        }
        else{
            if(cmd_keyword_check(argc,argv,"--app",app_name)!=0){
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] No app name specified. Use --app APP_NAME ." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                    check_and_cleanup(workdir);
                    return 5;
                }
                printf(GENERAL_BOLD "[ -INFO- ] Please specify an app name, e.g. " HIGH_CYAN_BOLD "mpich4" RESET_DISPLAY " :\n");
                printf(GENERAL_BOLD "[ INPUT: ] ");
                fflush(stdin);
                scanf("%127s",app_name);
                getchar();
            }
            if(strcmp(app_cmd,"check")==0){
                run_flag=app_list(workdir,"check",user_name,app_name,SSHKEY_DIR,"","");
            }
            else{
                prompt_to_input_optional_args("Specify installation shell scripts location?",CONFIRM_STRING,"Input either an URL or a local path.",inst_loc,batch_flag,argc,argv,"--inst");
                prompt_to_input_optional_args("Specify application sources and packages location?",CONFIRM_STRING,"Input either an URL or a local path.",inst_loc,batch_flag,argc,argv,"--repo");
                run_flag=app_operation(workdir,user_name,app_cmd,app_name,SSHKEY_DIR,inst_loc,repo_loc);
            }
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"APPMAN_FAILED",44);
            check_and_cleanup(workdir);
            return 44;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }

    if(strcmp(final_command,"jobman")==0){
        if(cluster_state_flag==1){
            printf(WARN_YELLO_BOLD "[ -WARN- ] No compute node is running." RESET_DISPLAY "\n");
        }
        if(cmd_keyword_check(argc,argv,"--jcmd",job_cmd)!=0){
            if(batch_flag==0){
                printf(FATAL_RED_BOLD "[ FATAL: ] No jobman subcmd specified. Use --jcmd SUBCMD ." RESET_DISPLAY "\n");
                write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                check_and_cleanup(workdir);
                return 5;
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Input a valid command: submit, list, cancel\n");
            printf(GENERAL_BOLD "[ INPUT: ] " RESET_DISPLAY);
            fflush(stdin);
            scanf("%127s",job_cmd);
            getchar();
        }
        int i=0;
        while(strcmp(job_cmd,jobman_commands[i])!=0){
            i++;
            if(i==3){
                break;
            }
        }
        if(i==3){
            printf(FATAL_RED_BOLD "[ FATAL: ] The command " WARN_YELLO_BOLD "%s" FATAL_RED_BOLD " is incorrect. Please read the help for details." RESET_DISPLAY "\n",job_cmd);
            printf(GENERAL_BOLD "|          " RESET_DISPLAY " submit, list, cancel\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"INVALID_PARAMS",9);
            check_and_cleanup(workdir);
            return 9;
        }
        if(strcmp(job_cmd,"submit")==0){
            run_flag=get_job_info(argc,argv,workdir,user_name,SSHKEY_DIR,crypto_keyfile,&job_info,batch_flag);
            if(run_flag!=0){
                write_operation_log(cluster_name,operation_log,argc,argv,"JOBMAN_FAILED",46);
                check_and_cleanup(workdir);
                return 46;
            }
            run_flag=job_submit(workdir,user_name,SSHKEY_DIR,&job_info);
        }
        else if(strcmp(job_cmd,"list")==0){
            run_flag=job_list(workdir,user_name,SSHKEY_DIR);
        }
        else{
            if(cmd_keyword_ncheck(argc,argv,"--jid",job_id,31)!=0){
                job_list(workdir,user_name,SSHKEY_DIR);
                if(batch_flag==0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] No jobID specified. Use --jid JOB_ID ." RESET_DISPLAY "\n");
                    write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
                    check_and_cleanup(workdir);
                    return 5;
                }
                printf("[ INPUT: ] Please specify the job id to be canceled: ");
                fflush(stdin);
                scanf("%31s",job_id);
                getchar();
            }
            run_flag=job_cancel(workdir,user_name,SSHKEY_DIR,job_id,batch_flag);
        }
        if(run_flag!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"JOBMAN_FAILED",46);
            check_and_cleanup(workdir);
            return 46;
        }
        else{
            write_operation_log(cluster_name,operation_log,argc,argv,"SUCCEEDED",0);
            check_and_cleanup(workdir);
            return 0;
        }
    }
    
    if(strcmp(final_command,"delc")==0){
        if(prompt_to_input_required_args("Specify how many nodes to be deleted.",string_temp,batch_flag,argc,argv,"--nn")==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a numbner." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 1;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=delete_compute_node(workdir,crypto_keyfile,string_temp,batch_flag,&tf_this_run);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"addc")==0){
        if(check_down_nodes(workdir)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before adding new nodes." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 1;
        }
        run_flag=prompt_to_input_required_args("Specify how many nodes to be added.",node_num_string,batch_flag,argc,argv,"--nn");
        if(run_flag==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            write_operation_log(cluster_name,operation_log,argc,argv,"TOO_FEW_PARAM",5);
            check_and_cleanup(workdir);
            return 5;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=add_compute_node(workdir,crypto_keyfile,node_num_string,&tf_this_run);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"shutdownc")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(prompt_to_input_required_args("Specify how many nodes to be shutdown.",string_temp,batch_flag,argc,argv,"--nn")==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a number or 'all'." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 1;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=shutdown_compute_nodes(workdir,crypto_keyfile,string_temp,batch_flag,&tf_this_run);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    
    if(strcmp(final_command,"turnonc")==0){
        if(strcmp(cloud_flag,"CLOUD_F")==0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Currently Azure (HPC-NOW Code: CLOUD_F) doesn't support this operation." RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"CLOUD_FUNCTION_UNSUPPORTED",6);
            check_and_cleanup("");
            return 6;
        }
        if(prompt_to_input_required_args("Specify how many nodes to be turned on.",string_temp,batch_flag,argc,argv,"--nn")==1){
            printf(FATAL_RED_BOLD "[ FATAL: ] You need to specify a numbner or 'all'." RESET_DISPLAY "\n");
            check_and_cleanup(workdir);
            return 1;
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(workdir,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }
        run_flag=turn_on_compute_nodes(workdir,crypto_keyfile,string_temp,batch_flag,&tf_this_run);
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }

    if(strcmp(final_command,"reconfc")==0||strcmp(final_command,"reconfm")==0){
        if(check_reconfigure_list(workdir,1)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the list. Have you initiated this cluster?" RESET_DISPLAY "\n");
            write_operation_log(cluster_name,operation_log,argc,argv,"FATAL_INTERNAL_ERROR",125);
            check_and_cleanup(workdir);
            return 125;
        }
        if(strcmp(final_command,"reconfc")==0&&strcmp(cloud_flag,"CLOUD_B")==0){
            if(check_down_nodes(workdir)!=0){
                printf(FATAL_RED_BOLD "[ FATAL: ] You need to turn all compute node(s) on before reconfiguring them." RESET_DISPLAY "\n");
                check_and_cleanup(workdir);
                return 1;
            }
        }
        run_flag=prompt_to_input_required_args("Select a configuration from the list above.",string_temp,batch_flag,argc,argv,"--conf");
        if(valid_vm_config_or_not(workdir,string_temp)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The specified configuration " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is not in the list." RESET_DISPLAY "\n",string_temp);
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 1;
        }
        if(strcmp(final_command,"reconfc")==0&&strcmp(cloud_flag,"CLOUD_C")==0){
            prompt_to_input_required_args("Specify the HT option (Case-Sensitive!): ON OFF skip(Default)",string_temp2,batch_flag,argc,argv,"--ht");
        }
        if(confirm_to_operate_cluster(cluster_name,batch_flag)!=0){
            write_operation_log(cluster_name,operation_log,argc,argv,"USER_DENIED",3);
            check_and_cleanup(workdir);
            return 3;
        }   
        if(strcmp(final_command,"reconfc")==0){
            run_flag=reconfigure_compute_node(workdir,crypto_keyfile,string_temp,string_temp2,&tf_this_run);
        }
        else{
            run_flag=reconfigure_master_node(workdir,crypto_keyfile,string_temp,&tf_this_run);
        }
        write_operation_log(cluster_name,operation_log,argc,argv,"",run_flag);
        check_and_cleanup(workdir);
        return run_flag;
    }
    write_operation_log(NULL,operation_log,argc,argv,"FATAL_ABNORMAL",run_flag);
    check_and_cleanup("");
    return 123;
}