/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifdef _WIN32
#include "..\\include\\now_macros.h"
#include "..\\src\\global_vars.c"
#include "..\\src\\cluster_general_funcs.c"
#include "..\\src\\cluster_init.c"
#include "..\\src\\cluster_operations.c"
#include "..\\src\\general_funcs.c"
#include "..\\src\\general_print_info.c"
#include "..\\src\\locations.c"
#include "..\\src\\prereq_check.c"
#include "..\\src\\time_convert.c"
#include "..\\src\\usage_and_logs.c"

#else
#include "../include/now_macros.h"
#include "../src/global_vars.c"
#include "../src/cluster_general_funcs.c"
#include "../src/cluster_init.c"
#include "../src/cluster_operations.c"
#include "../src/general_funcs.c"
#include "../src/general_print_info.c"
#include "../src/locations.c"
#include "../src/prereq_check.c"
#include "../src/time_convert.c"
#include "../src/usage_and_logs.c"
#endif

int main(int argc, char* argv[]){
    char* crypto_keyfile=CRYPTO_KEY_FILE;
    char buffer1[64];
    char cloud_flag[16];
    int run_flag=0;
    char workdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char* usage_log=USAGE_LOG_FILE;
    char* operation_log=OPERATION_LOG_FILE;
    char string_temp[128]="";
    print_header();

    if(check_current_user()!=0){
        printf("[ FATAL: ] You *MUST* switch to the user 'hpc-now' to operate cloud clusters.\n");
#ifdef _WIN32
        printf("|          Please switch to the user 'hpc-now' by ctrl+alt+delete and then:\n");
        printf("|          1. Run CMD by typing cmd in the Windows Search box\n");
        printf("|          2. cd c:\\hpc-now\n");
        printf("|          3. run the 'hpcopr' command to operate your cloud clusters.\n");
        printf("[ FATAL: ] Exit now.\n");
#else
        printf("|          Please run the commands below:\n");
        printf("|          1. su hpc-now   (You will be asked to input password without echo)\n");
        printf("|          2. cd ~ && ls   (You will see all the current working directories)\n");
        printf("[ FATAL: ] Exit now.\n");
#endif
        print_tail();
        return -3;
    }

#ifdef _WIN32
    if(folder_exist_or_not("c:\\hpc-now")!=0){
        printf("[ FATAL: ] The key directory C:\\hpc-now\\ is missing. The services cannot start.\n");
        printf("|          Please switch to Administrator and re-install the services to fix.\n");
#elif __APPLE__
    if(folder_exist_or_not("/Applications/.hpc-now/")!=0){
        printf("[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
#elif __linux__
    if(folder_exist_or_not("/usr/.hpc-now/")!=0){
        printf("[ FATAL: ] The service is corrupted due to missing critical folder. Please exit\n");
        printf("|          and run the installer with 'sudo' to reinstall it. Sample command:\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH uninstall\n");
        printf("|          sudo YOUR_INSTALLER_FULL_PATH install\n");
#endif
        printf("|          If this issue still occurs, please contact us via info@hpc-now.com .\n");
        printf("[ FATAL: ] Exit now.\n");
        print_tail();
        return -3;
    }

    if(check_internet()!=0){
        write_log("NULL",operation_log,"INTERNET_FAILED",-3);
        system_cleanup();
        return -3;
    }

    if(argc==1){
        print_help();
        return 0;
    }

    if(strcmp(argv[1],"license")==0){
        read_license();
        print_tail();
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

#ifdef _WIN32
    system("mkdir -p c:\\programdata\\hpc-now\\etc\\ > nul 2>&1");
#elif __APPLE__
    system("mkdir -p /Applications/.hpc-now/.etc/ >> /dev/null 2>&1");
#elif __linux__
    system("mkdir -p /usr/.hpc-now/.etc/ >> /dev/null 2>&1");
#endif
    if(create_cluster_registry()!=0){
        printf("[ FATAL: ] Failed to open/write to the cluster registry. Exit now.");
        return -1;
    }

    if(strcmp(argv[1],"configloc")==0){
        run_flag=configure_locations();
        print_tail();
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"resetloc")==0){
        run_flag=reset_locations();
        printf("[ -INFO- ] The locations have been reset to the default ones.\n");
        show_locations();
        print_tail();
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"showloc")==0){
        run_flag=show_locations();
        print_tail();
        system_cleanup();
        return run_flag;
    }

    if(show_current_cluster(workdir)==1){
        run_flag=list_all_cluster_names();
        print_tail();
        return run_flag;
    }

    if(strcmp(argv[1],"ls-clusters")==0){
        return list_all_cluster_names();
    }

    if(strcmp(argv[1],"switch")==0&&argc==3){
        return switch_to_cluster(argv[2]);
    }

    if(strcmp(argv[1],"new-cluster")==0){
        return create_new_cluster(crypto_keyfile,"","","");
    }
    /*
    if(strcmp(argv[1],"new")!=0&&strcmp(argv[1],"init")!=0&&strcmp(argv[1],"graph")!=0&&strcmp(argv[1],"usage")!=0&&strcmp(argv[1],"delc")!=0&&strcmp(argv[1],"addc")!=0&&strcmp(argv[1],"shutdownc")!=0&&strcmp(argv[1],"turnonc")!=0&&strcmp(argv[1],"reconfc")!=0&&strcmp(argv[1],"reconfm")!=0&&strcmp(argv[1],"sleep")!=0&&strcmp(argv[1],"wakeup")!=0&&strcmp(argv[1],"destroy")!=0&&strcmp(argv[1],"vault")!=0&&strcmp(argv[1],"syslog")!=0&&strcmp(argv[1],"conf")!=0){
        print_help();
        return 1;
    }

    run_flag=check_and_install_prerequisitions();
    if(run_flag==3){
        write_log("NULL",operation_log,"PREREQ_FAILED",-3);
        print_tail();
        system_cleanup();
        return -3;
    }
    else if(run_flag!=0){
        print_tail();
        return -3;
    }

    if(strcmp(argv[1],"new")==0){
       if(argc==2){
            printf("[ -INFO- ] Please specify either 'workdir' or 'keypair' as the second parameter.\n");
            printf("|              workdir: creating a new working directory for a new cluster.\n");
            printf("|              keypair: Rotating a new keypair for an existing cluster.\n");
            printf("[ -INFO- ] Exit now.\n");
            print_tail();
            system_cleanup();
            return -1;
        }
        if(strcmp(argv[2],"workdir")==0){
            run_flag=create_new_workdir(crypto_keyfile);
            write_log("NULL",operation_log,"new workdir",run_flag);
            system_cleanup();
            return run_flag;
        }
        else if(strcmp(argv[2],"keypair")==0){
            if(envcheck(pwd)!=0){
                write_log(pwd,operation_log,"new keypair",3);
                system_cleanup();
                return -1;
            }
            run_flag=rotate_new_keypair(pwd,crypto_keyfile);
            if(run_flag!=0){
                write_log(pwd,operation_log,"new keypair",-1);
                system_cleanup();
                print_tail();
                return -1;
            }
            write_log(pwd,operation_log,"new keypair",0);
            print_tail();
            system_cleanup();
            return 0;
        }
        else{
            printf("[ -INFO- ] Please specify either 'workdir' or 'keypair' as the second parameter.\n");
            printf("|              workdir: creating a new working directory for a new cluster.\n");
            printf("|              keypair: Rotating a new keypair for an existing cluster.\n");
            printf("[ -INFO- ] Exit now.\n");
            print_tail();
            return -1;
        }
    }

    if(strcmp(argv[1],"usage")==0){
        run_flag=get_usage(usage_log);
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"syslog")==0){
        run_flag=get_syslog(operation_log);
        write_log("NULL",operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(envcheck(pwd)!=0){
        write_log(pwd,operation_log,"NONE",3);
        system_cleanup();
        return 3;
    }
    create_and_get_vaultdir(pwd,vaultdir);
    sprintf(filename_temp,"%s/.secrets.txt",vaultdir);
    if(get_ak_sk(filename_temp,crypto_keyfile,buffer1,buffer1,cloud_flag)!=0){
        printf("[ FATAL: ] Failed to get the key file. HPC-NOW services can not be started.\n");
        printf("|          Please contact info@hpc-now.com for technical supports.\n");
        printf("|          Exit now.\n");
        print_tail();
        write_log(pwd,operation_log,"KEY_CHECK_FAILED",5);
        system_cleanup();
        return 5;
    }

    if(check_pslock(pwd)==1){
        printf("[ FATAL: ] Another process is operating this cluster, please wait the termination\n");
        printf("|          of that process. Currently no extra operation is permitted. Exit now.\n");
        print_tail();
        write_log(pwd,operation_log,"PROCESS_LOCKED",7);
        system_cleanup();
        return 7;
    }

    if(strcmp(argv[1],"conf")==0){
        if(get_default_conf(pwd,crypto_keyfile)==-1){
            printf("[ FATAL: ] The current cluster is not empty. In order to protect current cluster,\n");
            printf("|          downloading default configuration file is not permitted. If you do want\n");
            printf("|          to reconfigure the cluster from the default configuration, please run\n");
            printf("|          the 'destroy' command first and retry. Exit now.\n");
            print_tail();
            write_log(pwd,operation_log,"CLUSTER_NOT_EMPTY",23);
            system_cleanup();
            return 23;
        }
        else if(get_default_conf(pwd,crypto_keyfile)==1){
            printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n");
            print_tail();
            write_log(pwd,operation_log,"INTERNAL_ERROR",31);
            system_cleanup();
            return 31;
        }
        else{
            printf("[ -INFO- ] The default configuration file has been downloaded to the 'conf' folder.\n");
            printf("|          You can edit it, and then run the 'init' command to build a customized\n");
            printf("|          HPC cluster. Exit now.\n");
            print_tail();
            write_log(pwd,operation_log,argv[1],0);
            system_cleanup();
            return 0;
        }
    }
    if(strcmp(argv[1],"init")==0){
        if(argc==2){
            if(strcmp(cloud_flag,"CLOUD_C")==0){
                run_flag=aws_cluster_init("",pwd,crypto_keyfile);
                write_log(pwd,operation_log,argv[1],run_flag);
                system_cleanup();
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                run_flag=qcloud_cluster_init("",pwd,crypto_keyfile);
                write_log(pwd,operation_log,argv[1],run_flag);
                system_cleanup();
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                run_flag=alicloud_cluster_init("",pwd,crypto_keyfile);
                write_log(pwd,operation_log,argv[1],run_flag);
                system_cleanup();
                return run_flag;
            }
        }
        else{
            if(strcmp(cloud_flag,"CLOUD_C")==0){
                run_flag=aws_cluster_init(argv[2],pwd,crypto_keyfile);
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                system_cleanup();
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_B")==0){
                run_flag=qcloud_cluster_init(argv[2],pwd,crypto_keyfile);
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                system_cleanup();
                return run_flag;
            }
            else if(strcmp(cloud_flag,"CLOUD_A")==0){
                run_flag=alicloud_cluster_init(argv[2],pwd,crypto_keyfile);
                sprintf(string_temp,"%s %s",argv[1],argv[2]);
                write_log(pwd,operation_log,string_temp,run_flag);
                system_cleanup();
                return run_flag;
            }
        }
        system_cleanup();
        return 0;
    }

    if(cluster_empty_or_not(pwd)==0){
        print_empty_cluster_info();
        print_tail();
        delete_decrypted_files(pwd,crypto_keyfile);
        write_log(pwd,operation_log,"EMPTY_CLUSTER",11);
        system_cleanup();
        return 11;
    }

    if(strcmp(argv[1],"graph")==0){
        decrypt_files(pwd,crypto_keyfile);
        run_flag=graph(pwd,crypto_keyfile);
        if(run_flag!=0){
            print_empty_cluster_info();
        }
        print_tail();
        delete_decrypted_files(pwd,crypto_keyfile);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"vault")==0){
        run_flag=get_vault_info(pwd,crypto_keyfile);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"sleep")==0){
        run_flag=cluster_sleep(pwd,crypto_keyfile);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"wakeup")==0){
        if(argc==2){
            run_flag=cluster_wakeup(pwd,crypto_keyfile,"minimal");
            sprintf(string_temp,"%s default",argv[1]);
            write_log(pwd,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=cluster_wakeup(pwd,crypto_keyfile,argv[2]);
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
    }

    if(argc==3&&strcmp(argv[1],"destroy")==0&&strcmp(argv[2],"force")==0){
        run_flag=cluster_destroy(pwd,crypto_keyfile,0);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(cluster_asleep_or_not(pwd)==0){
        printf("[ FATAL: ] The current cluster is in the state of hibernation. No modification is\n");
        printf("|          permitted. Please run 'wakeup' command first to modify the cluster. You\n");
        printf("|          can run 'wakeup minimal' option to turn the management nodes on, or\n");
        printf("|          run 'wakeup all' option to turn the whole cluster on. Exit now.\n");
        print_tail();
        write_log(pwd,operation_log,argv[1],13);
        system_cleanup();
        return 13;
    }

    if(strcmp(argv[1],"destroy")==0&&cluster_empty_or_not(pwd)==1){
        run_flag=cluster_destroy(pwd,crypto_keyfile,1);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }
    if(strcmp(argv[1],"delc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a number or 'all' as the second parameter.\n");
            printf("|          Exit now.\n");
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        run_flag=delete_compute_node(pwd,crypto_keyfile,argv[2]);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"addc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a number (range: 1-%d) as the second parameter.\n",MAXIMUM_ADD_NODE_NUMBER);
            printf("|          Exit now.\n");
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        run_flag=add_compute_node(pwd,crypto_keyfile,argv[2]);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"shutdownc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n");
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        run_flag=shudown_compute_nodes(pwd,crypto_keyfile,argv[2]);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"turnonc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify either 'all' or a number as the second parameter.\n");
            printf("|          Exit now.\n");
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        run_flag=turn_on_compute_nodes(pwd,crypto_keyfile,argv[2]);
        write_log(pwd,operation_log,argv[1],run_flag);
        system_cleanup();
        return run_flag;
    }

    if(strcmp(argv[1],"reconfc")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a configuration as the second parameter.\n");
            if(check_reconfigure_list(pwd)!=0){
                printf("[ FATAL: ] Internal error. Please contact HPC-NOW via info@hpc-now.com\n");
                printf("|          for technical supports. Exit now.\n");
                print_tail();
                system_cleanup();
                write_log(pwd,operation_log,argv[1],-1);
                system_cleanup();
                return -1;
            }
            print_tail();
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        else if(argc==3){
            run_flag=reconfigure_compute_node(pwd,crypto_keyfile,argv[2],"");
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
        else{
            run_flag=reconfigure_compute_node(pwd,crypto_keyfile,argv[2],argv[3]);
            sprintf(string_temp,"%s %s %s",argv[1],argv[2],argv[3]);
            write_log(pwd,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
    }

    if(strcmp(argv[1],"reconfm")==0){
        if(argc==2){
            printf("[ FATAL: ] You need to specify a configuration as the second parameter.\n");
            if(check_reconfigure_list(pwd)!=0){
                printf("[ FATAL: ] Internal error. Please contact HPC-NOW via info@hpc-now.com\n");
                printf("|          for technical supports. Exit now.\n");
                print_tail();
                system_cleanup();
                write_log(pwd,operation_log,argv[1],-1);
                system_cleanup();
                return -1;
            }
            print_tail();
            write_log(pwd,operation_log,argv[1],17);
            system_cleanup();
            return 17;
        }
        else{
            run_flag=reconfigure_master_node(pwd,crypto_keyfile,argv[2]);
            sprintf(string_temp,"%s %s",argv[1],argv[2]);
            write_log(pwd,operation_log,string_temp,run_flag);
            system_cleanup();
            return run_flag;
        }
    }
    system_cleanup();
    return 0;*/
}