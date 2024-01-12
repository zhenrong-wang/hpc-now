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

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "cluster_general_funcs.h"
#include "general_funcs.h"
#include "general_print_info.h"
#include "cluster_init.h"
#include "components.h"

extern char url_code_root_var[LOCATION_LENGTH];
extern char url_shell_scripts_var[LOCATION_LENGTH];
extern char url_initutils_root_var[LOCATION_LENGTH];
extern char url_app_pkgs_root_var[LOCATION_LENGTH];
extern char url_app_inst_root_var[LOCATION_LENGTH];
extern char az_environment[16];
extern int code_loc_flag_var;

/*
 * 
 *
 */

void reset_initinfo(cluster_initinfo* init_info, char* cluster_id){
    strcpy(init_info->cluster_id,cluster_id);
    strcpy(init_info->region_id,"");
    strcpy(init_info->zone_id,"");
    init_info->node_num=0;
    init_info->hpc_user_num=0;
    init_info->hpc_nfs_volume=0;
    strcpy(init_info->master_init_param,"db skip");
    strcpy(init_info->master_passwd,"");
    strcpy(init_info->compute_passwd,"");
    strcpy(init_info->master_inst,"");
    init_info->master_bandwidth=0;
    strcpy(init_info->compute_inst,"");
    strcpy(init_info->os_image_raw,"");
    strcpy(init_info->ht_flag,"");
}

void empty_initinfo(cluster_initinfo* init_info){
    reset_initinfo(init_info,"");
    strcpy(init_info->master_init_param,"");
}

int get_static_conf_files(char* confdir, char* cloud_name, int code_loc_flag, char* url_code_root){
    char cmdline[CMDLINE_LENGTH]="";
    char url_code[DIR_LENGTH_EXT]="";
    char filename_temp[FILENAME_LENGTH]="";

    if(strcmp(cloud_name,"alicloud")!=0&&strcmp(cloud_name,"aws")!=0&&strcmp(cloud_name,"azure")!=0&&strcmp(cloud_name,"baidu")&&strcmp(cloud_name,"gcp")!=0&&strcmp(cloud_name,"hwcloud")!=0&&strcmp(cloud_name,"qcloud")!=0){
        return 1;
    }
    if(valid_loc_format_or_not(url_code_root)!=0){
        return 1;
    }
    if(code_loc_flag==1){
        snprintf(url_code,DIR_LENGTH_EXT-1,"%s%s%s%s",url_code_root,PATH_SLASH,cloud_name,PATH_SLASH);
    }
    else{
        snprintf(url_code,DIR_LENGTH_EXT-1,"%s%s/",url_code_root,cloud_name);
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        if(code_loc_flag==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sreconf.list %s %s",COPY_FILE_CMD,url_code,PATH_SLASH,filename_temp,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %sreconf.list -o %s -s",url_code,filename_temp);
        }
        if(system(cmdline)!=0){
            return 2;
        }
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sregions.list",confdir,PATH_SLASH);
    if(file_exist_or_not(filename_temp)!=0){
        if(code_loc_flag==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%sregions.list %s %s",COPY_FILE_CMD,url_code,PATH_SLASH,filename_temp,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %sregions.list -o %s -s",url_code,filename_temp);
        }
        if(system(cmdline)!=0){
            return 2;
        }
    }
    if(strcmp(cloud_name,"alicloud")==0||strcmp(cloud_name,"qcloud")==0){
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%snas_zones.list",confdir,PATH_SLASH);
        if(file_exist_or_not(filename_temp)!=0){
            if(code_loc_flag==1){
                snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%snas_zones_%s.txt %s %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,filename_temp,SYSTEM_CMD_REDIRECT);
            }
            else{
                snprintf(cmdline,CMDLINE_LENGTH-1,"curl %snas_zones_%s.txt -o %s -s",url_code,cloud_name,filename_temp);
            }
            if(system(cmdline)!=0){
                return 2;
            }
        }
    }
    return 0;
}

int get_tf_templates(char* confdir, char* stackdir, char* cloud_name, int code_loc_flag, char* url_code_root){
    char cmdline[CMDLINE_LENGTH]="";
    char url_code[DIR_LENGTH_EXT]="";
    char tf_conf[FILENAME_LENGTH]="";
    if(strcmp(cloud_name,"alicloud")!=0&&strcmp(cloud_name,"aws")!=0&&strcmp(cloud_name,"azure")!=0&&strcmp(cloud_name,"baidu")&&strcmp(cloud_name,"gcp")!=0&&strcmp(cloud_name,"hwcloud")!=0&&strcmp(cloud_name,"qcloud")!=0){
        return 1;
    }
    if(valid_loc_format_or_not(url_code_root)!=0){
        return 1;
    }
    if(code_loc_flag==1){
        snprintf(url_code,DIR_LENGTH_EXT-1,"%s%s%s%s",url_code_root,PATH_SLASH,cloud_name,PATH_SLASH);
    }
    else{
        snprintf(url_code,DIR_LENGTH_EXT-1,"%s%s/",url_code_root,cloud_name);
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(tf_conf,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(tf_conf)==1){
        printf(GENERAL_BOLD "[ -INFO- ] IMPORTANT: No configure file found. Use the default one." RESET_DISPLAY "\n");
        if(code_loc_flag==1){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf.v2 %s %s", COPY_FILE_CMD,url_code,PATH_SLASH,tf_conf,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %stf_prep.conf.v2 -s -o %s", url_code,tf_conf);
        }
        if(system(cmdline)!=0){
            return 2;
        }
    }
    if(code_loc_flag==1){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.base %s%shpc_stack.base %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.base -o %s%shpc_stack.base -s",url_code,cloud_name,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        return 2;
    }
    if(code_loc_flag==1){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.master %s%shpc_stack.master %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.master -o %s%shpc_stack.master -s",url_code,cloud_name,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        return 2;
    }
    if(code_loc_flag==1){
        if(strcmp(cloud_name,"aws")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.compute.v2 %s%shpc_stack.compute %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.compute %s%shpc_stack.compute %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        }
    }
    else{
        if(strcmp(cloud_name,"aws")==0){
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.compute.v2 -o %s%shpc_stack.compute -s",url_code,cloud_name,stackdir,PATH_SLASH);
        }
        else{
            snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.compute -o %s%shpc_stack.compute -s",url_code,cloud_name,stackdir,PATH_SLASH);
        }
    }
    if(system(cmdline)!=0){
        return 2;
    }
    if(code_loc_flag==1){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.database %s%shpc_stack.database %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.database -o %s%shpc_stack.database -s",url_code,cloud_name,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        return 2;
    }
    if(code_loc_flag==1){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_%s.natgw %s%shpc_stack.natgw %s",COPY_FILE_CMD,url_code,PATH_SLASH,cloud_name,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"curl %shpc_stack_%s.natgw -o %s%shpc_stack.natgw -s",url_code,cloud_name,stackdir,PATH_SLASH);
    }
    if(system(cmdline)!=0){
        return 2;
    }
    return 0;
}

//You must guarantee the stackdir,vaultdir,logdir,confdir is long enough with DIR_LENGTH!
int create_init_dirs(char* workdir, char* stackdir, char* vaultdir, char* logdir, char* confdir, unsigned int dirlen_max){
    if(dirlen_max<DIR_LENGTH_SHORT){
        return -3;
    }
    if(folder_exist_or_not(workdir)==1){
        return -1;
    }
    if(create_and_get_subdir(workdir,"stack",stackdir,dirlen_max)!=0||create_and_get_subdir(workdir,"vault",vaultdir,dirlen_max)!=0||create_and_get_subdir(workdir,"log",logdir,dirlen_max)!=0||create_and_get_subdir(workdir,"conf",confdir,dirlen_max)!=0){
        return -1;
    }
    return 0;
}

int cluster_init_conf(char* cluster_name, char* crypto_keyfile, int batch_flag_local, int code_loc_flag_local, char* url_code_root, int argc, char* argv[]){
    char cloud_flag[16]="";
    char workdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH_EXT]="";
    char stackdir[DIR_LENGTH_EXT]="";
    char vaultdir[DIR_LENGTH_EXT]="";
    char logdir[DIR_LENGTH_EXT]="";
    char tf_prep_conf[FILENAME_LENGTH]="";
    char confirm[8]="";
    char cloud_name[16]="";
    FILE* file_p=NULL;
    char default_region[32]="";
    char real_region[32]="";
    char default_zone[64]="";
    char real_zone[64]="";
    int default_node_num=1;
    char real_node_num_string[8]="";
    int real_node_num;
    int default_user_num=3;
    char real_user_num_string[8]="";
    int real_user_num;
    char default_master_inst[16]="";
    char real_master_inst[16]="";
    char default_compute_inst[16]="";
    char real_compute_inst[16]="";
    char default_os_image[64]="";
    char real_os_image[96]="";
    char real_ht_flag[8]="";
    char real_nfs_volume[8]="";
    int real_nfs_vol;
    char app_inst_script_url_specified[LOCATION_LENGTH]="";
    char app_inst_pkgs_url_specified[LOCATION_LENGTH]="";

    if(get_nworkdir(workdir,DIR_LENGTH,cluster_name)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get a valid working directory." RESET_DISPLAY "\n");
        return -1;
    }
    if(get_cloud_flag(workdir,crypto_keyfile,cloud_flag,16)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cloud flag." RESET_DISPLAY "\n");
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        strcpy(cloud_name,"alicloud");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        strcpy(cloud_name,"qcloud");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(cloud_name,"aws");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        strcpy(cloud_name,"hwcloud");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        strcpy(cloud_name,"baidu");
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        strcpy(cloud_name,"azure");
    }
    else if(strcmp(cloud_flag,"CLOUD_G")==0){
        strcpy(cloud_name,"gcp");
    }
    else{
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the cloud flag." RESET_DISPLAY "\n");
        return -1;
    }
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create working directories for this cluster." RESET_DISPLAY "\n");
        return -1;
    }
    if(get_static_conf_files(confdir,cloud_name,code_loc_flag_local,url_code_root)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to get the static files for this cluster." RESET_DISPLAY "\n");
        return -1;
    }
    snprintf(tf_prep_conf,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    if(file_exist_or_not(tf_prep_conf)==0){
        if(cmd_flag_check(argc,argv,"--force")!=0){
            if(batch_flag_local==0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] Config file found (downloaded or saved). Using it now." RESET_DISPLAY "\n");
                return 0; // If the conf file already exists, exit, unless force specified or batch mode.
            }
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Config file found. Input " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " to use it, others to abandon.\n");
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING)==0){
                return 0;
            }
            else{
                printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Previous config file abandoned. Creating a new one.\n");
            }
        }
        else{
            printf(WARN_YELLO_BOLD "[ -WARN- ] Config file found (downloaded or saved). Using it now." RESET_DISPLAY "\n");
            return 0;
        }   
    }
    
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        strcpy(default_region,"cn-hangzhou");
        strcpy(default_zone,"cn-hangzhou-j");
        strcpy(default_master_inst,"a8c16g");
        strcpy(default_compute_inst,"a4c8g");
        strcpy(default_os_image,"centoss9");
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        strcpy(default_region,"ap-guangzhou");
        strcpy(default_zone,"ap-guangzhou-6");
        strcpy(default_master_inst,"a8c16g");
        strcpy(default_compute_inst,"a4c8g");
        strcpy(default_os_image,"centoss9");
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        strcpy(default_region,"cn-northwest-1");
        strcpy(default_zone,"cn-northwest-1a");
        strcpy(default_master_inst,"a8c16g");
        strcpy(default_compute_inst,"a4c8g");
        strcpy(default_os_image,"centoss9");
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        strcpy(default_region,"cn-north-4");
        strcpy(default_zone,"cn-north-4a");
        strcpy(default_master_inst,"i8c16g");
        strcpy(default_compute_inst,"i4c8g");
        strcpy(default_os_image,"rocky9");
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        strcpy(default_region,"bj");
        strcpy(default_zone,"cn-bj-a");
        strcpy(default_master_inst,"a8c16g");
        strcpy(default_compute_inst,"a4c8g");
        strcpy(default_os_image,"centoss9");
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        strcpy(default_region,"Japan-East"); //Azure
        strcpy(default_zone,"*NULL*");
        strcpy(default_master_inst,"i8c16g");
        strcpy(default_compute_inst,"i4c8g");
        strcpy(default_os_image,"*Oracle_Linux_9.2*-IMMUTABLE");
    }
    else{
        strcpy(default_region,"us-central1");
        strcpy(default_zone,"us-central1-a");
        strcpy(default_master_inst,"a4c8g");
        strcpy(default_compute_inst,"a4c8g");
        strcpy(default_os_image,"centoss9");
    }
    strcpy(real_ht_flag,"ON");
    if(cmd_keyword_ncheck(argc,argv,"--rg",real_region,32)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "select" RESET_DISPLAY " a region (Default: " GENERAL_BOLD "%s" RESET_DISPLAY "): ",default_region);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                list_cloud_regions(cluster_name,1);
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Select one from the list above: ");
                fflush(stdin);
                scanf("%31s",real_region);
                getchar();
            }
            else{
                strcpy(real_region,default_region);
            }
        }
        else{
            strcpy(real_region,default_region);
        }
    }
    if(valid_region_or_not(cluster_name,real_region)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The region name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid." RESET_DISPLAY "\n",real_region);
        goto invalid_conf;
    }
    if(strcmp(cloud_flag,"CLOUD_F")!=0){
        if(get_default_nzone(cluster_name,real_region,default_zone,64)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The regions.list file may be incorrect." RESET_DISPLAY "\n");
            goto invalid_conf;
        }
        if(cmd_keyword_ncheck(argc,argv,"--az",real_zone,64)!=0){
            if(batch_flag_local!=0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "select" RESET_DISPLAY " a zone (Default: " GENERAL_BOLD "%s" RESET_DISPLAY "): ",default_zone);
                fflush(stdin);
                scanf("%7s",confirm);
                getchar();
                if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                    list_cloud_zones(cluster_name,real_region,1);
                    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Select one from the list above: ");
                    fflush(stdin);
                    scanf("%63s",real_zone);
                    getchar();
                }
                else{
                    strcpy(real_zone,default_zone);
                }
            }
            else{
                strcpy(real_zone,default_zone);
            }
        }
        if(valid_region_zone_or_not(cluster_name,real_region,real_zone)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The zone name " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid for region " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " ." RESET_DISPLAY "\n",real_zone,real_region);
            goto invalid_conf;
        }
    }
    else{
        strcpy(real_zone,"null no-need-to-specify");
    }
    if(cmd_keyword_ncheck(argc,argv,"--nn",real_node_num_string,8)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "specify" RESET_DISPLAY " node num (Default: " GENERAL_BOLD "%d" RESET_DISPLAY "): ",default_node_num);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input a number [Range: %d-%d]: ",MINIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
                fflush(stdin);
                scanf("%7s",real_node_num_string);
                getchar();
                real_node_num=string_to_positive_num(real_node_num_string);
            }
            else{
                real_node_num=default_node_num;
            }
        }
        else{
            real_node_num=default_node_num;
        }
    }
    else{
        real_node_num=string_to_positive_num(real_node_num_string);
    }
    if(real_node_num<MINIMUM_ADD_NODE_NUMBER||real_node_num>MAXIMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The node num " RESET_DISPLAY GENERAL_BOLD "%d" RESET_DISPLAY WARN_YELLO_BOLD " is out of range. Using the default " RESET_DISPLAY GENERAL_BOLD "%d" RESET_DISPLAY " .\n",real_node_num,default_node_num);
        real_node_num=default_node_num;
    }
    if(cmd_keyword_ncheck(argc,argv,"--un",real_user_num_string,8)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "specify" RESET_DISPLAY " user num (Default: " GENERAL_BOLD "%d" RESET_DISPLAY "): ",default_user_num);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input a number [Range: %d-%d]: ",MINIMUM_ADD_USER_NUNMBER,MAXIMUM_ADD_USER_NUMBER);
                fflush(stdin);
                scanf("%7s",real_user_num_string);
                getchar();
                real_user_num=string_to_positive_num(real_user_num_string);
            }
            else{
                real_user_num=default_user_num;
            }
        }
        else{
            real_user_num=default_user_num;
        }
    }
    else{
        real_user_num=string_to_positive_num(real_user_num_string);
    }
    if(real_user_num<MINIMUM_ADD_USER_NUNMBER||real_node_num>MAXIMUM_ADD_USER_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The user num " RESET_DISPLAY GENERAL_BOLD "%d" RESET_DISPLAY WARN_YELLO_BOLD " is out of range. Using the default" RESET_DISPLAY GENERAL_BOLD "%d" RESET_DISPLAY " .\n",real_user_num,default_user_num);
        real_user_num=default_user_num;
    }
    if(cmd_keyword_ncheck(argc,argv,"--mi",real_master_inst,16)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "select" RESET_DISPLAY " a master node type (Default: " GENERAL_BOLD "%s" RESET_DISPLAY "): ",default_master_inst);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                if(check_reconfigure_list(workdir,1)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The reconf.list file may be incorrect." RESET_DISPLAY "\n");
                    goto invalid_conf; 
                }
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Select one from the list above: ");
                fflush(stdin);
                scanf("%15s",real_master_inst);
                getchar();
            }
            else{
                strcpy(real_master_inst,default_master_inst);
            }
        }
        else{
            strcpy(real_master_inst,default_master_inst);
        }
    }
    if(valid_vm_config_or_not(workdir,real_master_inst)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The instance type " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid." RESET_DISPLAY "\n",real_master_inst);
        goto invalid_conf;
    }
    if(cmd_keyword_ncheck(argc,argv,"--ci",real_compute_inst,16)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "select" RESET_DISPLAY " a compute node type (Default: " GENERAL_BOLD "%s" RESET_DISPLAY "): ",default_compute_inst);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                if(check_reconfigure_list(workdir,1)!=0){
                    printf(FATAL_RED_BOLD "[ FATAL: ] The reconf.list file may be incorrect." RESET_DISPLAY "\n");
                    goto invalid_conf; 
                }
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Select one from the list above: ");
                fflush(stdin);
                scanf("%15s",real_compute_inst);
                getchar();
            }
            else{
                strcpy(real_compute_inst,default_compute_inst);
            }
        }
        else{
            strcpy(real_compute_inst,default_compute_inst);
        }
    }
    if(valid_vm_config_or_not(workdir,real_compute_inst)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] The instance type " RESET_DISPLAY WARN_YELLO_BOLD "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid." RESET_DISPLAY "\n",real_compute_inst);
        goto invalid_conf;
    }
    if(cmd_keyword_ncheck(argc,argv,"--os",real_os_image,96)!=0){
        if(batch_flag_local!=0){
            printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "specify" RESET_DISPLAY " an OS type or image_ID (Default: " GENERAL_BOLD "%s" RESET_DISPLAY "): ",default_os_image);
            fflush(stdin);
            scanf("%7s",confirm);
            getchar();
            if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " : ");
                fflush(stdin);
                scanf("%95s",real_os_image);
                getchar();
            }
            else{
                strcpy(real_os_image,default_os_image);
            }
        }
        else{
            strcpy(real_os_image,default_os_image);
        }
    }
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        if(cmd_keyword_ncheck(argc,argv,"--ht",real_ht_flag,8)!=0){
            if(batch_flag_local!=0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD "OFF" RESET_DISPLAY " to " WARN_YELLO_BOLD "turn off" RESET_DISPLAY " hyperthreading (Default: " GENERAL_BOLD "ON" RESET_DISPLAY "): ");
                fflush(stdin);
                scanf("%7s",confirm);
                getchar();
                if(strcmp(confirm,"OFF")==0){
                    strcpy(real_ht_flag,"OFF");
                }
                else{
                    strcpy(real_ht_flag,"ON");
                }
            }
            else{
                strcpy(real_ht_flag,"ON");
            }
        }
    }
    if(strcmp(real_ht_flag,"ON")!=0&&strcmp(real_ht_flag,"OFF")!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The ht flag " RESET_DISPLAY GENERAL_BOLD "%s" RESET_DISPLAY WARN_YELLO_BOLD " is invalid. Using to default " RESET_DISPLAY GENERAL_BOLD "ON" RESET_DISPLAY " .\n",real_ht_flag);
        strcpy(real_ht_flag,"ON");
    }
    if(strcmp(cloud_flag,"CLOUD_D")==0||strcmp(cloud_flag,"CLOUD_F")==0||strcmp(cloud_flag,"CLOUD_G")==0){
        if(cmd_keyword_ncheck(argc,argv,"--vol",real_nfs_volume,8)!=0){
            if(batch_flag_local!=0){
                printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input " WARN_YELLO_BOLD CONFIRM_STRING_QUICK RESET_DISPLAY " to " WARN_YELLO_BOLD "specify" RESET_DISPLAY " NFS initial volume in GB (Default: " GENERAL_BOLD "100" RESET_DISPLAY "): ");
                fflush(stdin);
                scanf("%7s",confirm);
                getchar();
                if(strcmp(confirm,CONFIRM_STRING_QUICK)==0){
                    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Input a number [Range: 100-32000]: ");
                    fflush(stdin);
                    scanf("%127s",real_nfs_volume);
                    getchar();
                    real_nfs_vol=string_to_positive_num(real_nfs_volume);
                }
                else{
                    real_nfs_vol=100;
                }
            }
            else{
                real_nfs_vol=100;
            }
        }
        else{
            real_nfs_vol=string_to_positive_num(real_nfs_volume);
        }
        if(real_nfs_vol<100||real_node_num>32000){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The volume " RESET_DISPLAY GENERAL_BOLD "%d" RESET_DISPLAY WARN_YELLO_BOLD " is out of range. Using the default " RESET_DISPLAY GENERAL_BOLD "100" RESET_DISPLAY " .\n",real_nfs_vol);
            real_nfs_vol=100;
        }
    }
    if(cmd_keyword_ncheck(argc,argv,"--inst",app_inst_script_url_specified,384)==0){
        if(valid_loc_format_or_not(app_inst_script_url_specified)==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Using a self-defined app_inst_script_url " RESET_DISPLAY GENERAL_BOLD "%s" RESET_DISPLAY "\n",app_inst_script_url_specified);
            strncpy(url_app_inst_root_var,app_inst_script_url_specified,LOCATION_LENGTH-1);
        }
        else{
            printf("[ -INFO- ] The app_inst_script_url " GENERAL_BOLD "%s" RESET_DISPLAY " is invalid. Using the default.",app_inst_script_url_specified);
        }
    }
    if(cmd_keyword_ncheck(argc,argv,"--repo",app_inst_pkgs_url_specified,384)==0){
        if(valid_loc_format_or_not(app_inst_pkgs_url_specified)==0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Using a self-defined app_inst_pkgs_url " RESET_DISPLAY GENERAL_BOLD "%s" RESET_DISPLAY "\n",app_inst_pkgs_url_specified);
            strncpy(url_app_pkgs_root_var,app_inst_pkgs_url_specified,LOCATION_LENGTH-1);
        }
        else{
            printf("[ -INFO- ] The app_inst_script_url " GENERAL_BOLD "%s" RESET_DISPLAY " is invalid. Using the default.",app_inst_pkgs_url_specified);
        }
    }
    file_p=fopen(tf_prep_conf,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a configuration file." RESET_DISPLAY "\n");
        return -1;
    }
    fprintf(file_p,"# This file is generated and maintained by the HPC-NOW services. We do not recommend\n");
    fprintf(file_p,"# to edit it manually. If need to do so, please follow the strict format.\n");
    fprintf(file_p,"ITEM_NAME           : CONFIGURATION  \n");
    fprintf(file_p,"cluster_id          : %s do-not-change\n",cluster_name);
    if(strcmp(cloud_flag,"CLOUD_F")!=0){
        fprintf(file_p,"region_id           : %s\n",real_region);
    }
    else{
        fprintf(file_p,"region_id           : az.%s\n",real_region);
    }
    fprintf(file_p,"zone_id             : %s\n",real_zone);
    fprintf(file_p,"node_num            : %d\n",real_node_num);
    fprintf(file_p,"hpc_user_num        : %d\n",real_user_num);
    fprintf(file_p,"master_init_param   : db skip do-not-change\n");
    fprintf(file_p,"master_passwd       : *AUTOGEN* you-can-modify\n");
    fprintf(file_p,"compute_passwd      : *AUTOGEN* you-can-modify\n");
    fprintf(file_p,"master_inst         : %s\n",real_master_inst);
    if(strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        fprintf(file_p,"master_bandwidth    : 50\n");
    }
    fprintf(file_p,"compute_inst        : %s\n",real_compute_inst);
    fprintf(file_p,"os_image            : %s\n",real_os_image);
    if(strcmp(cloud_flag,"CLOUD_C")==0){
        fprintf(file_p,"hyperthreading      : %s\n",real_ht_flag);
    }
    if(strcmp(cloud_flag,"CLOUD_D")==0||strcmp(cloud_flag,"CLOUD_F")==0||strcmp(cloud_flag,"CLOUD_G")==0){
        fprintf(file_p,"nfs_volume          : %d\n",real_nfs_vol);
    }
    fclose(file_p);
    return 0;
invalid_conf:
    delete_file_or_dir(tf_prep_conf);
    return 1;
}

int get_tf_prep_conf(char* cluster_id, char* conf_file, char* reconf_list, cluster_initinfo* init_info){
    if(file_exist_or_not(conf_file)!=0){
        return -3; // If the conf file doesn't exist, exit.
    }
    FILE* file_p=fopen(conf_file,"r");
    char conf_line_buffer[LINE_LENGTH_SHORT]="";
    char header[64]="";
    char tail[64]="";
    char node_inst_ext[128]="";
    int read_conf_lines=0;
    int sum_temp=0;
    int i;

    reset_initinfo(init_info,cluster_id);
    while(!feof(file_p)){
        if(fngetline(file_p,conf_line_buffer,LINE_LENGTH_SHORT)!=0){
            continue;
        }
        get_seq_nstring(conf_line_buffer,' ',1,header,64);
        get_seq_nstring(conf_line_buffer,' ',3,tail,64);
        if(strcmp(header,"cluster_id")==0||strcmp(header,"master_init_param")==0){
            read_conf_lines++;
            continue; // The cluster id is immutable. Skip it.
        }
        else if(strcmp(header,"region_id")==0){
            if(*(tail+0)=='a'&&*(tail+1)=='z'&&*(tail+2)=='.'){
                for(i=3;i<strlen(tail);i++){
                    if(*(tail+i)=='-'){
                        *(init_info->region_id+i-3)=' ';
                    }
                    else{
                        *(init_info->region_id+i-3)=*(tail+i);
                    }
                }
                if(contain_or_not(tail,"China")==0){
                    strcpy(az_environment,"china");
                }
                else{
                    strcpy(az_environment,"public");
                }
            }
            else{
                strncpy(init_info->region_id,tail,31);
            }
            read_conf_lines++;
        }
        else if(strcmp(header,"zone_id")==0){
            strncpy(init_info->zone_id,tail,63);
            read_conf_lines++;
        }
        else if(strcmp(header,"node_num")==0){
            sum_temp=string_to_positive_num(tail);
            if(sum_temp<MINIMUM_ADD_NODE_NUMBER||sum_temp>MAXIMUM_ADD_NODE_NUMBER){
                fclose(file_p);
                return 1;
            }
            init_info->node_num=sum_temp;
            read_conf_lines++;
        }
        else if(strcmp(header,"hpc_user_num")==0){
            sum_temp=string_to_positive_num(tail);
            if(sum_temp<MINIMUM_ADD_USER_NUNMBER||sum_temp>MAXIMUM_ADD_USER_NUMBER){
                fclose(file_p);
                return 1;
            }
            init_info->hpc_user_num=sum_temp;
            read_conf_lines++;
        }
        else if(strcmp(header,"master_passwd")==0){
            strncpy(init_info->master_passwd,tail,31);
            read_conf_lines++;
        }
        else if(strcmp(header,"compute_passwd")==0){
            strncpy(init_info->compute_passwd,tail,31);
            read_conf_lines++;
        }
        else if(strcmp(header,"master_inst")==0){
            snprintf(node_inst_ext,127," %s ",tail);
            if(find_multi_nkeys(reconf_list,LINE_LENGTH_SHORT,node_inst_ext,"","","","")<1){
                fclose(file_p);
                return 2;
            }
            strncpy(init_info->master_inst,tail,15);
            read_conf_lines++;
        }
        else if(strcmp(header,"master_bandwidth")==0){
            sum_temp=string_to_positive_num(tail);
            if(sum_temp>50||sum_temp<0){
                init_info->master_bandwidth=50;
            }
            else{
                init_info->master_bandwidth=sum_temp;
            }
            read_conf_lines++;
        }
        else if(strcmp(header,"compute_inst")==0){
            snprintf(node_inst_ext,127," %s ",tail);
            if(find_multi_nkeys(reconf_list,LINE_LENGTH_SHORT,node_inst_ext,"","","","")<1){
                fclose(file_p);
                return 2;
            }
            strncpy(init_info->compute_inst,tail,15);
            read_conf_lines++;
        }
        else if(strcmp(header,"os_image")==0){
            strncpy(init_info->os_image_raw,tail,95);
            read_conf_lines++;
        }
        else if(strcmp(header,"hyperthreading")==0){
            strncpy(init_info->ht_flag,tail,8);
            read_conf_lines++;
        }
        else if(strcmp(header,"nfs_volume")==0){
            sum_temp=string_to_positive_num(tail);
            if(sum_temp<100||sum_temp>32000){
                fclose(file_p);
                return 1;
            }
            init_info->hpc_nfs_volume=sum_temp;
            read_conf_lines++;
        }
        else{
            continue;
        }
    }
    fclose(file_p);
    if(read_conf_lines<CONF_LINE_NUM){
        return 3;
    }
    else{
        return 0;
    }
}

void print_read_conf_failed(int read_conf_flag){
    if(read_conf_flag==-3){
        printf(FATAL_RED_BOLD "[ FATAL: ] Configuration file not found." RESET_DISPLAY "\n");
    }
    else if(read_conf_flag==1){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid format for NODE_NUM and/or HPC_USER_NUM." RESET_DISPLAY "\n");
    }
    else if(read_conf_flag==2){
        printf(FATAL_RED_BOLD "[ FATAL: ] Invalid node configuration string." RESET_DISPLAY "\n");
    }
    else if(read_conf_flag==3){
        printf(FATAL_RED_BOLD "[ FATAL: ] Insufficient configuration params." RESET_DISPLAY "\n");
    }
}

int print_conf_summary(int batch_flag_local, cluster_initinfo* init_info){
    printf(HIGH_GREEN_BOLD "[ STEP 2 ] Cluster Configuration:" RESET_DISPLAY "\n");
    //printf(HIGH_GREEN_BOLD "[  ****  ] Cluster Name : %s " RESET_DISPLAY GREEN_LIGHT " ~ non-configurable\n",init_info->cluster_id);
    printf(HIGH_GREEN_BOLD "[  ****  ] Cloud Region : %s " RESET_DISPLAY GREEN_LIGHT " ~ provided by cloud\n",init_info->region_id);
    printf(HIGH_GREEN_BOLD "[  ****  ] Cloud AZ     : %s " RESET_DISPLAY GREEN_LIGHT " ~ availability zone\n",init_info->zone_id);
    printf(HIGH_GREEN_BOLD "[  ****  ] Num of Nodes : %d " RESET_DISPLAY GREEN_LIGHT " ~ initial nodes created\n",init_info->node_num);
    printf(HIGH_GREEN_BOLD "[  ****  ] Num of Users : %d " RESET_DISPLAY GREEN_LIGHT " ~ initial users created\n",init_info->hpc_user_num);
    printf(HIGH_GREEN_BOLD "[  ****  ] Master Node  : %s " RESET_DISPLAY GREEN_LIGHT " ~ configuration code\n",init_info->master_inst);
    printf(HIGH_GREEN_BOLD "[  ****  ] Compute Node : %s " RESET_DISPLAY GREEN_LIGHT " ~ configuration code\n",init_info->compute_inst);
    printf(HIGH_GREEN_BOLD "[  ****  ] OS Image     : %s " RESET_DISPLAY GREEN_LIGHT " ~ image_id or name" RESET_DISPLAY "\n",init_info->os_image_raw);
    if(strcmp(init_info->ht_flag,"OFF")==0){
        printf(HIGH_GREEN_BOLD "[  ****  ] HT-status    : %s " RESET_DISPLAY GREEN_LIGHT " ~ hyperthreading option\n",init_info->ht_flag);
    }
    if(init_info->hpc_nfs_volume>0){
        printf(HIGH_GREEN_BOLD "[  ****  ] NFS Vol(GB)  : %d " RESET_DISPLAY GREEN_LIGHT " ~ shared volume in GB" RESET_DISPLAY "\n",init_info->hpc_nfs_volume);
    }
    return confirm_to_init_cluster(init_info->cluster_id,batch_flag_local);
}

int save_bucket_info(char* cloud_flag, char* bucket_info_file, char* bucket_id, char* region_id, char* bucket_ak, char* bucket_sk, char* gcp_bucket_key_file){
    FILE* file_p=fopen(bucket_info_file,"w+");
    char gcp_key_file_line[LINE_LENGTH_MID]="";
    if(file_p==NULL){
        return -1;
    }
    if(strcmp(cloud_flag,"CLOUD_A")!=0&&strcmp(cloud_flag,"CLOUD_B")!=0&&strcmp(cloud_flag,"CLOUD_C")!=0&&strcmp(cloud_flag,"CLOUD_D")!=0&&strcmp(cloud_flag,"CLOUD_E")!=0&&strcmp(cloud_flag,"CLOUD_F")!=0&&strcmp(cloud_flag,"CLOUD_G")!=0){
        fclose(file_p);
        return -3;
    }
    if(strcmp(cloud_flag,"CLOUD_A")==0){
        fprintf(file_p,"BUCKET: oss://%s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else if(strcmp(cloud_flag,"CLOUD_B")==0){
        fprintf(file_p,"BUCKET: cos://%s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else if(strcmp(cloud_flag,"CLOUD_C")==0){
        fprintf(file_p,"BUCKET: s3://%s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else if(strcmp(cloud_flag,"CLOUD_D")==0){
        fprintf(file_p,"BUCKET: obs://%s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else if(strcmp(cloud_flag,"CLOUD_E")==0){
        fprintf(file_p,"BUCKET: bos://%s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else if(strcmp(cloud_flag,"CLOUD_F")==0){
        fprintf(file_p,"BUCKET: %s\nREGION: \"%s\"\nBUCKET_AK: %s\nBUCKET_SK: %s\n",bucket_id,region_id,bucket_ak,bucket_sk);
    }
    else{
        FILE* file_p_gcp=fopen(gcp_bucket_key_file,"r");
        if(file_p_gcp==NULL){
            fclose(file_p);
            return -5;
        }
        fprintf(file_p,"BUCKET: gs://%s\nREGION: \"%s\"\nBUCKET_LINK: %s\n\n",bucket_id,region_id,bucket_ak);
        while(fngetline(file_p_gcp,gcp_key_file_line,LINE_LENGTH_MID)!=1){
            fprintf(file_p,"%s\n",gcp_key_file_line);
        }
        fclose(file_p_gcp);
    }
    fclose(file_p);
    return 0;
}

void node_user_num_fix(int* node_num, int* hpc_user_num){
    if(*node_num>MAXIMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,*node_num, MAXIMUM_ADD_NODE_NUMBER,MAXIMUM_ADD_NODE_NUMBER);
        *node_num=MAXIMUM_ADD_NODE_NUMBER;
    }
    else if(*node_num<MINIMUM_ADD_NODE_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of compute nodes %d is less than %d, reset to %d.\n" RESET_DISPLAY,*node_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_NODE_NUMBER);
        *node_num=MINIMUM_ADD_NODE_NUMBER;
    }
    if(*hpc_user_num>MAXIMUM_ADD_USER_NUMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d exceeds the maximum value %d, reset to %d.\n" RESET_DISPLAY,*hpc_user_num,MAXIMUM_ADD_USER_NUMBER,MAXIMUM_ADD_USER_NUMBER);
        *hpc_user_num=MAXIMUM_ADD_USER_NUMBER;
    }
    else if(*hpc_user_num<MINIMUM_ADD_USER_NUNMBER){
        printf(WARN_YELLO_BOLD "[ -WARN- ] The number of HPC users %d is less than %d, reset to %d.\n" RESET_DISPLAY,*hpc_user_num,MINIMUM_ADD_USER_NUNMBER,MINIMUM_ADD_USER_NUNMBER);
        *hpc_user_num=MINIMUM_ADD_USER_NUNMBER;
    }
}

/*
 * condition_flag=1: empty stackdir
 * condition_flag=2: empty stackdir && confdir
 * condition_flag=3: empty stackdir, confdir and vaultdir
 */
void clear_if_failed(char* stackdir, char* confdir, char* vaultdir, int condition_flag){
    char cmdline[CMDLINE_LENGTH]="";
    if(condition_flag==1){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    else if(condition_flag==2){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf %s%stf_prep.conf.failed %s",MOVE_FILE_CMD,confdir,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
    else{
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s",DELETE_FILE_CMD,DESTROYED_DIR,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,DESTROYED_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s* %s %s",MOVE_FILE_CMD,vaultdir,PATH_SLASH,DESTROYED_DIR,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%stf_prep.conf %s%stf_prep.conf.destroyed %s",MOVE_FILE_CMD,confdir,PATH_SLASH,confdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
        system(cmdline);
    }
}

void generate_tf_files(char* stackdir){
    char cmdline[CMDLINE_LENGTH]="";
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.base %s%shpc_stack_base.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.database %s%shpc_stack_database.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.master %s%shpc_stack_master.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.natgw %s%shpc_stack_natgw.tf %s",MOVE_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s",DELETE_FILE_CMD,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
}

int save_cluster_vaults(char* vaultdir, char* mast_passwd, char* comp_password, char* db_root_password, char* db_acct_password, char* ucid_short, char* cloud_flag, char* az_sub_id, char* az_tenant_id){
    char cluster_vaults[FILENAME_LENGTH]="";
    snprintf(cluster_vaults,FILENAME_LENGTH-1,"%s%scluster_vaults.txt",vaultdir,PATH_SLASH);
    FILE* file_p=fopen(cluster_vaults,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"%s\n",INTERNAL_FILE_HEADER);
    fprintf(file_p,"mast_root_password: %s\n",mast_passwd);
    fprintf(file_p,"comp_root_password: %s\n",comp_password);
    fprintf(file_p,"data_root_password: %s\n",db_root_password);
    fprintf(file_p,"data_acct_password: %s\n",db_acct_password);
    fprintf(file_p,"short_unique_id:    %s\n",ucid_short);
    fprintf(file_p,"cloud_flag_code:    %s\n",cloud_flag);
    if(strlen(az_sub_id)>0&&strlen(az_tenant_id)>0){
        fprintf(file_p,"azure_subscription_id: %s\n",az_sub_id);
        fprintf(file_p,"azure_tenant_id:       %s\n",az_sub_id);
    }
    fclose(file_p);
    return 0;
}

int aws_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    char user_passwords[FILENAME_LENGTH]="";
    cluster_initinfo init_info;
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char region_flag[16]="";
    char os_image[256]="";
    char db_os_image[64]="";
    char nat_os_image[64]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    int number_of_vcpu=0;
    int cpu_core_num=0;
    int threads;
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i;
    
    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"aws",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }    
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_info);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_info.node_num,&init_info.hpc_user_num);
    number_of_vcpu=get_cpu_num(init_info.compute_inst);
    cpu_core_num=number_of_vcpu/2;
    if(strcmp(init_info.ht_flag,"OFF")==0){
        threads=1;
    }
    else{
        threads=2;
    }
    if(contain_or_not(init_info.zone_id,init_info.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    if(strcmp(init_info.region_id,"cn-northwest-1")==0){
        strcpy(region_flag,"cn_regions");
        if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
            snprintf(os_image,255,"ami = \"${var.%scn.0}\"",init_info.os_image_raw);
        }
        else{
            snprintf(os_image,255,"ami = \"%s\"",init_info.os_image_raw);
        }
        strcpy(db_os_image,"ami = \"${var.centos7cn.0}\"");
        strcpy(nat_os_image,"ami = \"${var.centos7cn.0}\"");
    }
    else if(strcmp(init_info.region_id,"cn-north-1")==0){
        strcpy(region_flag,"cn_regions");
        if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
            snprintf(os_image,255,"ami = \"${var.%scn.1}\"",init_info.os_image_raw);
        }
        else{
            snprintf(os_image,255,"ami = \"%s\"",init_info.os_image_raw);
        }
        strcpy(db_os_image,"ami = \"${var.centos7cn.1}\"");
        strcpy(nat_os_image,"ami = \"${var.centos7cn.1}\"");
    }
    else{
        strcpy(region_flag,"global_regions");
        if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0||strcmp(init_info.os_image_raw,"openEuler")==0){
            snprintf(os_image,255,"ami = data.aws_ami.%s_x86_glb.image_id",init_info.os_image_raw);
        }
        else{
            snprintf(os_image,255,"ami = \"%s\"",init_info.os_image_raw);
        }
        strcpy(db_os_image,"ami = data.aws_ami.centos7_x86_glb.image_id");
        strcpy(nat_os_image,"ami = data.aws_ami.centos7_x86_glb.image_id");
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_info.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_info.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_info.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_info)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(string_temp,127,"vpc-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_VPC_NAME",string_temp);
    snprintf(string_temp,127,"subnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_SUBNET_NAME",string_temp);
    snprintf(string_temp,127,"pubnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_ACCESS_KEY_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ACCESS_POLICY",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_USER_ID",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ID",randstr);
    snprintf(string_temp,127,"%s-public",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_PUBLIC",string_temp);
    snprintf(string_temp,127,"%s-natgw",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_NATGW",string_temp);
    snprintf(string_temp,127,"%s-intra",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_INTRA",string_temp);
    snprintf(string_temp,127,"%s-mysql",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_MYSQL",string_temp);
    snprintf(string_temp,127,"%s-nag",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NAS_ACCESS_GROUP",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_info.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    snprintf(string_temp,127,"%d",init_info.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_info.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_info.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_info.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_info.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    if(strcmp(region_flag,"global_regions")==0){
        delete_nlines_by_kwd(filename_temp,LINE_LENGTH_SMALL,"DELETE_FOR_CN_REGIONS",1);
    }
    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_info.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_info.master_inst);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_AMI_HERE",os_image);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_info.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_AMI_HERE",os_image);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",unique_cluster_id);
    if(threads==1){ //Hyperthreading off
        snprintf(string_temp,127,"cpu_core_count = %d",cpu_core_num);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_HT_HERE",string_temp);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_HT_HERE","cpu_threads_per_core = 1");
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_AMI_HERE",db_os_image);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"#INSERT_AMI_HERE",nat_os_image);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",unique_cluster_id);
    for(i=0;i<init_info.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
        snprintf(string_temp,127,"comp%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NUMBER",string_temp);
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id,32);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"aws_iam_access_key","","",15,"\"id\":","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"aws_iam_access_key","","",15,"\"secret\":","","",'\"',4,bucket_sk,256);
    if(strcmp(region_flag,"global_regions")==0){
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_GLOBAL);
        for(i=0;i<AWS_SLEEP_TIME_GLOBAL;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_GLOBAL-i);
            fflush(stdout);
            sleep(1);
        }
    }
    else{
        printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",AWS_SLEEP_TIME_CN);
        for(i=0;i<AWS_SLEEP_TIME_CN;i++){
            printf("[ -WAIT- ] Still need to wait %d seconds ... \r",AWS_SLEEP_TIME_CN-i);
            fflush(stdout);
            sleep(1);
        }
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_info.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_info.master_passwd,init_info.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_info.master_inst);
    compute_vcpu=get_cpu_num(init_info.compute_inst);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    database_vcpu=get_cpu_num(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    natgw_vcpu=get_cpu_num(string_temp);
    if(*(init_info.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_info.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_info.region_id);
    for(i=0;i<init_info.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_info.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_info.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_info.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_info.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int qcloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_info;
    char user_passwords[FILENAME_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char NAS_Zone[CONF_STRING_LENTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i;

    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"qcloud",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_info);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_info.node_num,&init_info.hpc_user_num);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%snas_zones.list",confdir,PATH_SLASH);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,init_info.zone_id,"","","","")>0){
        strcpy(NAS_Zone,init_info.zone_id);
    }
    else{
        find_and_nget(filename_temp,LINE_LENGTH_SHORT,init_info.region_id,"","",1,init_info.region_id,"","",' ',1,NAS_Zone,64);
    }
    if(contain_or_not(init_info.zone_id,init_info.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_info.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_info.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_info.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_info)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(string_temp,127,"vpc-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_VPC_NAME",string_temp);
    snprintf(string_temp,127,"subnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_SUBNET_NAME",string_temp);
    snprintf(string_temp,127,"pubnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CFSID",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_ACCESS_KEY_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ACCESS_POLICY",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_USER_ID",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ID",randstr);
    snprintf(string_temp,127,"%s-public",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_PUBLIC",string_temp);
    snprintf(string_temp,127,"%s-intra",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_INTRA",string_temp);
    snprintf(string_temp,127,"%s-mysql",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_MYSQL",string_temp);
    snprintf(string_temp,127,"%s-natgw",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_NATGW",string_temp);
    snprintf(string_temp,127,"%s-nag",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NAS_ACCESS_GROUP",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_info.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NAS_ZONE",NAS_Zone);
    snprintf(string_temp,127,"%d",init_info.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_info.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_info.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_info.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_info.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);

    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_info.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_info.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    snprintf(string_temp,127,"%d",init_info.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_info.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_info.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.tencentcloud_images.OS_IMAGE.images.0.image_id",string_temp);
    }
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_info.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_info.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_info.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.tencentcloud_images.OS_IMAGE.images.0.image_id",string_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    snprintf(string_temp,127,"%d",init_info.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);

    for(i=0;i<init_info.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RUNNING_FLAG","true");
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id,32);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"secret_id","","",1,"secret_id","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"secret_key","","",1,"secret_key","","",'\"',4,bucket_sk,256);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",QCLOUD_SLEEP_TIME);
    for(i=0;i<QCLOUD_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",QCLOUD_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_info.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_info.master_passwd,init_info.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_info.master_inst);
    compute_vcpu=get_cpu_num(init_info.compute_inst);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    database_vcpu=get_cpu_num(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    natgw_vcpu=get_cpu_num(string_temp);
    if(*(init_info.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_info.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_info.region_id);
    for(i=0;i<init_info.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_info.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_info.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_info.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_info.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int alicloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_info;
    char user_passwords[FILENAME_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char NAS_Zone[CONF_STRING_LENTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i;
    
    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"alicloud",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_info);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_info.node_num,&init_info.hpc_user_num);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%snas_zones.list",confdir,PATH_SLASH);
    if(find_multi_nkeys(filename_temp,LINE_LENGTH_SHORT,init_info.zone_id,"","","","")>0){
        strcpy(NAS_Zone,init_info.zone_id);
    }
    else{
        find_and_nget(filename_temp,LINE_LENGTH_SHORT,init_info.region_id,"","",1,init_info.region_id,"","",' ',1,NAS_Zone,64);
    }
    if(contain_or_not(init_info.zone_id,init_info.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_info.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_info.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_info.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_info.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_info.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_info)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(string_temp,127,"vpc-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_VPC_NAME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_ACCESS_KEY_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ACCESS_POLICY",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_USER_ID",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ID",randstr);
    snprintf(string_temp,127,"%s-public",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_PUBLIC",string_temp);
    snprintf(string_temp,127,"%s-intra",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_INTRA",string_temp);
    
    /* CAUTION: The resource group name is immutable! */
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_NAME",randstr);

    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_DISPLAY_NAME",unique_cluster_id);
    snprintf(string_temp,127,"%s-mysql",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_MYSQL",string_temp);
    snprintf(string_temp,127,"%s-nag",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NAS_ACCESS_GROUP",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_info.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NAS_ZONE",NAS_Zone);
    snprintf(string_temp,127,"%d",init_info.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_info.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_info.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_info.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_info.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_info.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_DISPLAY_NAME",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_info.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    snprintf(string_temp,127,"%d",init_info.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_info.os_image_raw);
    }
    else{
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"${data.alicloud_images.OS_IMAGE.images.0.id}",init_info.os_image_raw);
    }
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_DISPLAY_NAME",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_info.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    if(strcmp(init_info.os_image_raw,"centos7")==0||strcmp(init_info.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_info.os_image_raw);
    }
    else{
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"${data.alicloud_images.OS_IMAGE.images.0.id}",init_info.os_image_raw);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_DISPLAY_NAME",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_info.zone_id);
    snprintf(string_temp,127,"%d",init_info.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RG_DISPLAY_NAME",unique_cluster_id);

    for(i=0;i<init_info.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",ALI_SLEEP_TIME);
    for(i=0;i<ALI_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",ALI_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id,32);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_secrets.txt",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk,256);
    delete_file_or_dir(filename_temp);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_info.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_info.master_passwd,init_info.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_info.master_inst);
    compute_vcpu=get_cpu_num(init_info.compute_inst);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    database_vcpu=get_cpu_num(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_type","","",1,"instance_type","","",'.',3,string_temp,128);
    natgw_vcpu=get_cpu_num(string_temp);
    if(*(init_info.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_info.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_info.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_info.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_info.region_id);
    for(i=0;i<init_info.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_info.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_info.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_info.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_info.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_info.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_info.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int hw_vm_series(const char* region_id, char* intel_generation, char* tiny_series_name, int* amd_flag){
    if(strcmp(region_id,"cn-north-4")==0||strcmp(region_id,"cn-east-3")==0||strcmp(region_id,"cn-south-1")==0){
        *amd_flag=0;
    }
    else{
        *amd_flag=1; // No amd available
    }
    if(strcmp(region_id,"cn-north-4")==0||strcmp(region_id,"cn-north-9")==0||strcmp(region_id,"cn-east-3")==0||strcmp(region_id,"cn-south-1")==0||strcmp(region_id,"cn-southwest-2")==0){
        strcpy(intel_generation,"c7");
    }
    else if(strcmp(region_id,"ap-southeast-4")==0||strcmp(region_id,"tr-west-1")==0){
        strcpy(intel_generation,"c7n");
    }
    else if(strcmp(region_id,"la-north-2")==0||strcmp(region_id,"af-south-1")){
        strcpy(intel_generation,"c6s");
    }
    else{
        strcpy(intel_generation,"c6");
    }
    if(strcmp(region_id,"cn-north-9")==0||strcmp(region_id,"cn-east-3")==0||strcmp(region_id,"cn-southwest-2")==0||strcmp(region_id,"ap-southeast-1")==0){
        strcpy(tiny_series_name,"s6");
    }
    else if(strcmp(region_id,"ap-southeast-4")==0||strcmp(region_id,"tr-west-1")==0){
        strcpy(tiny_series_name,"s7n");
    }
    else{
        strcpy(tiny_series_name,"t6");
    }
    return 0;
}

int hwcloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_conf;

    char user_passwords[FILENAME_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    int amd_flavor_flag=1;
    char intel_generation[16]="";
    char tiny_series_name[16]="";
    char usage_logfile[FILENAME_LENGTH]="";
    int i;

    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"hwcloud",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_conf);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_conf.node_num,&init_conf.hpc_user_num);
    if(contain_or_not(init_conf.zone_id,init_conf.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_conf.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_conf.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_conf.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_conf)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    hw_vm_series(init_conf.region_id,intel_generation,tiny_series_name,&amd_flavor_flag);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(string_temp,127,"vpc-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_VPC_NAME",string_temp);
    snprintf(string_temp,127,"subnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_SUBNET_NAME",string_temp);
    snprintf(string_temp,127,"pubnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_ACCESS_KEY_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_USER_ID",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ID",randstr);
    snprintf(string_temp,127,"%s-public",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_PUBLIC",string_temp);
    snprintf(string_temp,127,"%s-intra",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_INTRA",string_temp);
    snprintf(string_temp,127,"%s-mysql",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_MYSQL",string_temp);
    snprintf(string_temp,127,"%s-natgw",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_NATGW",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    snprintf(string_temp,127,"%d",init_conf.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_conf.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_conf.hpc_nfs_volume);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_STORAGE_VOLUME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_conf.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_conf.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_conf.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"INTEL_GENERATION",intel_generation);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"TINY_SERIE_NAME",tiny_series_name);

    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_conf.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_conf.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    snprintf(string_temp,127,"%d",init_conf.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    if(strcmp(init_conf.os_image_raw,"rocky9")==0||strcmp(init_conf.os_image_raw,"euleros")==0||strcmp(init_conf.os_image_raw,"centos7")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.huaweicloud_images_images.OS_IMAGE.images[0].id",string_temp);
    }
    
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_conf.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    if(strcmp(init_conf.os_image_raw,"rocky9")==0||strcmp(init_conf.os_image_raw,"euleros")==0||strcmp(init_conf.os_image_raw,"centos7")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.huaweicloud_images_images.OS_IMAGE.images[0].id",string_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    snprintf(string_temp,127,"%d",init_conf.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);

    for(i=0;i<init_conf.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    }

    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"bucket\"","","",1,"\"bucket\"","","",'\"',4,bucket_id,32);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"access_key","","",20,"\"id\":","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"access_key","","",20,"\"secret\":","","",'\"',4,bucket_sk,256);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",GENERAL_SLEEP_TIME);
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_conf.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_conf.master_passwd,init_conf.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_conf.master_inst);
    compute_vcpu=get_cpu_num(init_conf.compute_inst);
    database_vcpu=1;
    natgw_vcpu=1;
    if(*(init_conf.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_conf.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_conf.region_id);
    for(i=0;i<init_conf.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_conf.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_conf.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_conf.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_conf.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int baiducloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_conf;
    char user_passwords[FILENAME_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[32]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char natgw_inst[16]="";
    char db_inst[16]="";
    char usage_logfile[FILENAME_LENGTH]="";
    int i;

    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"baidu",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_conf);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_conf.node_num,&init_conf.hpc_user_num);
    if(contain_or_not(init_conf.zone_id,init_conf.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_conf.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_conf.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_conf.cluster_id,randstr);
    if(strcmp(init_conf.region_id,"hk")==0){
        strcpy(db_inst,"i2c2g-hk");
        strcpy(natgw_inst,"i2c2g-hk");
    }
    else{
        strcpy(db_inst,"i2c2g");
        strcpy(natgw_inst,"i2c2g");
    }
    if(print_conf_summary(batch_flag_local,&init_conf)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(string_temp,127,"vpc-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_VPC_NAME",string_temp);
    snprintf(string_temp,127,"subnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_SUBNET_NAME",string_temp);
    snprintf(string_temp,127,"pubnet-%s",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_PUB_SUBNET_NAME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_ACCESS_KEY_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_USER_ID",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BUCKET_ID",randstr);
    snprintf(string_temp,127,"%s-public",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_PUBLIC",string_temp);
    snprintf(string_temp,127,"%s-intra",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_INTRA",string_temp);
    snprintf(string_temp,127,"%s-mysql",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_MYSQL",string_temp);
    snprintf(string_temp,127,"%s-natgw",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"SECURITY_GROUP_NATGW",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    if(strcmp(init_conf.region_id,"gz")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NAS_ZONE","zoneC");
    }
    else{
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NAS_ZONE","zoneA");
    }
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NFSID",randstr);
    snprintf(string_temp,127,"%d",init_conf.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_conf.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_conf.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_conf.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_conf.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    snprintf(string_temp,127,"%d",init_conf.master_bandwidth);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_BANDWIDTH",string_temp);

    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_conf.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_conf.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    if(strcmp(init_conf.os_image_raw,"centos7")==0||strcmp(init_conf.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.baiducloud_images.OS_IMAGE.images[0].id",string_temp);
    }
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_conf.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    if(strcmp(init_conf.os_image_raw,"centos7")==0||strcmp(init_conf.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.baiducloud_images.OS_IMAGE.images[0].id",string_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DB_INST",db_inst);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"NATGW_INST",natgw_inst);

    for(i=0;i<init_conf.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCETAG",unique_cluster_id);
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"bucket\":","","",1,"\"bucket\":","","",'\"',4,bucket_id,32); 

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%saccess-key.txt",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"AccessKeyId","","",1,"AccessKeyId","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"AccessKeySecret","","",1,"AccessKeySecret","","",'\"',4,bucket_sk,256);
    delete_file_or_dir(filename_temp);

    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",GENERAL_SLEEP_TIME);
    for(i=0;i<GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_conf.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_conf.master_passwd,init_conf.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_conf.master_inst);
    compute_vcpu=get_cpu_num(init_conf.compute_inst);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_database.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_spec","","",1,"instance_spec","","",'.',3,string_temp,128);
    database_vcpu=get_cpu_num(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_natgw.tf",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"instance_spec","","",1,"instance_spec","","",'.',3,string_temp,128);
    natgw_vcpu=get_cpu_num(string_temp);
    if(*(init_conf.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_conf.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_conf.region_id);
    for(i=0;i<init_conf.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_conf.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_conf.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_conf.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_conf.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    bceconfig_convert(vaultdir,"generate",init_conf.region_id,bucket_ak,bucket_sk);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%scredentials",vaultdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket_creds/credentials","root","put","",0);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sconfig",vaultdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket_creds/config","root","put","",0);
    bceconfig_convert(vaultdir,"delete","","","");
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int azure_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_conf;
    char user_passwords[FILENAME_LENGTH]="";
    char access_key[AKSK_LENGTH]="";
    char secret_key[AKSK_LENGTH]="";
    char subscription_id[AKSK_LENGTH]="";
    char tenant_id[AKSK_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char random_storage_account[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    char bucket_id[128]="";
    char bucket_ak[AKSK_LENGTH]="";
    char bucket_sk[AKSK_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i;

    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.secrets.key",vaultdir,PATH_SLASH);
    if(get_ak_sk(filename_temp,crypto_keyfile,access_key,secret_key,cloud_flag)!=0){
        /*printf("HHHHHxxx! \n");*/
        return -1;
    }
    if(get_azure_ninfo(workdir,LINE_LENGTH_SHORT,crypto_keyfile,subscription_id,tenant_id,256)!=0){
        /*printf("HHHHHsss! \n");*/
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        /*printf("HHHHHaaaa! \n");*/
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"azure",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_conf);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 3;
    }
    node_user_num_fix(&init_conf.node_num,&init_conf.hpc_user_num);
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_conf.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_conf.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_conf.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_conf)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        return 1; // user denied.
    }
    generate_random_string(random_storage_account);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_CLIENT_ID",access_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_SECRET_KEY",secret_key);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_TENANT_ID",tenant_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_SUBCRIPTION_ID",subscription_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"ENVIRONMENT_OPTION",az_environment);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STORAGE_ACCOUNT",random_storage_account);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    snprintf(string_temp,127,"%d",init_conf.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_conf.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_conf.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_conf.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_conf.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    snprintf(string_temp,127,"%d",init_conf.hpc_nfs_volume);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_STORAGE_VOLUME",string_temp);

    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_conf.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_conf.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",unique_cluster_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_conf.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",unique_cluster_id);

    for(i=0;i<init_conf.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);

    getstate(workdir,crypto_keyfile);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"type\": \"azurerm_storage_container\",","","",20,"\"id\": \"","","",'\"',4,bucket_id,128);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"type\": \"azuread_application\",","","",20,"\"application_id\":","","",'\"',4,bucket_ak,256);
    find_and_nget(filename_temp,LINE_LENGTH_SMALL,"\"type\": \"azuread_application_password\",","","",20,"\"value\":","","",'\"',4,bucket_sk,256);
    
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",2*GENERAL_SLEEP_TIME);
    for(i=0;i<2*GENERAL_SLEEP_TIME;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",2*GENERAL_SLEEP_TIME-i);
        fflush(stdout);
        sleep(1);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,bucket_id,init_conf.region_id,bucket_ak,bucket_sk,"");
    save_cluster_vaults(vaultdir,init_conf.master_passwd,init_conf.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_conf.master_inst);
    compute_vcpu=get_cpu_num(init_conf.compute_inst);
    database_vcpu=1;
    natgw_vcpu=1;
    if(*(init_conf.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_conf.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_conf.region_id);
    for(i=0;i<init_conf.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_conf.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_conf.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_conf.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_conf.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    return 0;
}

int gcp_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run){
    char cluster_id_from_workdir[CLUSTER_ID_LENGTH_MAX_PLUS]="";
    if(get_cluster_nname(cluster_id_from_workdir,CLUSTER_ID_LENGTH_MAX_PLUS,workdir)!=0){
        return -3;
    }
    char stackdir[DIR_LENGTH]="";
    char vaultdir[DIR_LENGTH]="";
    char logdir[DIR_LENGTH]="";
    char confdir[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char conf_file[FILENAME_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    cluster_initinfo init_conf;
    char user_passwords[FILENAME_LENGTH]="";
    char cloud_flag[16]="";
    int read_conf_flag=0;
    char unique_cluster_id[64]="";
    char string_temp[128]="";
    char keyfile_path[FILENAME_LENGTH]="";
    char keyfile_path_ext[FILENAME_LENGTH_EXT]="";
    char gcp_project_id[128]="";
    char gcp_bucket_key[FILENAME_LENGTH]="";
    char randstr[RANDSTR_LENGTH_PLUS]="";
    char* sshkey_folder=SSHKEY_DIR;
    char pubkey[1024]="";
    FILE* file_p=NULL;
    FILE* file_p_2=NULL;
    char database_root_passwd[PASSWORD_STRING_LENGTH]="";
    char database_acct_passwd[PASSWORD_STRING_LENGTH]="";
    char user_passwd_temp[PASSWORD_STRING_LENGTH]="";
    char line_temp[LINE_LENGTH]="";
    time_t current_time_long;
    struct tm* time_p=NULL;
    char current_date[12]="";
    char current_time[12]="";
    char master_cpu_vendor[8]="";
    char compute_cpu_vendor[8]="";
    int master_vcpu,database_vcpu,natgw_vcpu,compute_vcpu;
    char usage_logfile[FILENAME_LENGTH]="";
    int i;
    char bucket_selflink[128]="";
    char bucket_private_key[LINE_LENGTH]="";

    printf("[ START: ] Start initializing the cluster ...\n");
    if(create_init_dirs(workdir,stackdir,vaultdir,logdir,confdir,DIR_LENGTH)!=0){
        return -1;
    }
    if(gcp_credential_convert(workdir,"decrypt",0)!=0){
        return -1;
    }
    if(get_opr_pubkey(sshkey_folder,pubkey,1023)!=0){
        return -1;
    }
    printf("[ STEP 1 ] Creating initialization files now ...\n");
    if(get_tf_templates(confdir,stackdir,"gcp",code_loc_flag_var,url_code_root_var)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy necessary file(s)." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,1);
        gcp_credential_convert(workdir,"delete",0);
        return 2;
    }
    snprintf(conf_file,FILENAME_LENGTH-1,"%s%stf_prep.conf",confdir,PATH_SLASH);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sreconf.list",confdir,PATH_SLASH);
    read_conf_flag=get_tf_prep_conf(cluster_id_from_workdir,conf_file,filename_temp,&init_conf);
    if(read_conf_flag!=0){
        print_read_conf_failed(read_conf_flag);
        clear_if_failed(stackdir,confdir,vaultdir,2);
        gcp_credential_convert(workdir,"delete",0);
        return 3;
    }
    node_user_num_fix(&init_conf.node_num,&init_conf.hpc_user_num);
    if(contain_or_not(init_conf.zone_id,init_conf.region_id)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Availability Zone ID doesn't match with Region ID, please double check." RESET_DISPLAY "\n");
        clear_if_failed(stackdir,confdir,vaultdir,2);
        gcp_credential_convert(workdir,"delete",0);
        return 3;
    }
    generate_random_db_passwd(database_root_passwd,PASSWORD_STRING_LENGTH);
    generate_random_db_passwd(database_acct_passwd,PASSWORD_STRING_LENGTH);
    if(strcmp(init_conf.master_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.master_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.master_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    if(strcmp(init_conf.compute_passwd,"*AUTOGEN*")==0||password_complexity_check(init_conf.compute_passwd,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(init_conf.compute_passwd,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a Unique Cluster ID now...\n");
    generate_random_nstring(randstr,RANDSTR_LENGTH_PLUS,0);
    snprintf(unique_cluster_id,63,"%s-%s",init_conf.cluster_id,randstr);
    if(print_conf_summary(batch_flag_local,&init_conf)!=0){
        clear_if_failed(stackdir,confdir,vaultdir,2);
        gcp_credential_convert(workdir,"delete",0);
        return 1; // user denied.
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.base",stackdir,PATH_SLASH);
    snprintf(keyfile_path,FILENAME_LENGTH-1,"%s%s.key.json",vaultdir,PATH_SLASH);
    windows_path_to_nstring(keyfile_path,keyfile_path_ext,DIR_LENGTH_EXT);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_CREDENTIAL_PATH",keyfile_path_ext);
    find_and_nget(keyfile_path,LINE_LENGTH_SHORT,"\"project_id\":","","",1,"\"project_id\":","","",'\"',4,gcp_project_id,128);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_PROJECT",gcp_project_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_REGION_ID",init_conf.region_id);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_ZONE_ID",init_conf.zone_id);
    snprintf(string_temp,127,"%d",init_conf.node_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_NODE_NUM",string_temp);
    snprintf(string_temp,127,"%d",init_conf.hpc_user_num);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_USER_NUM",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTERINI",init_conf.master_init_param);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_MASTER_PASSWD",init_conf.master_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_COMPUTE_PASSWD",init_conf.compute_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ROOT_PASSWD",database_root_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_DB_ACCT_PASSWD",database_acct_passwd);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"BLANK_URL_SHELL_SCRIPTS",url_shell_scripts_var);
    snprintf(string_temp,127,"%d",init_conf.hpc_nfs_volume);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"DEFAULT_STORAGE_VOLUME",string_temp);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCE_LABEL",unique_cluster_id);

    file_p=fopen(filename_temp,"a");
    snprintf(user_passwords,FILENAME_LENGTH-1,"%s%suser_passwords.txt",vaultdir,PATH_SLASH);
    file_p_2=fopen(user_passwords,"w+");
    for(i=0;i<init_conf.hpc_user_num;i++){
        generate_random_npasswd(user_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS_SHORT,strlen(SPECIAL_PASSWORD_CHARS_SHORT));
        fprintf(file_p,"variable \"user%d_passwd\" {\n  type = string\n  default = \"%s\"\n}\n\n",i+1,user_passwd_temp);
        fprintf(file_p_2,"username: user%d %s STATUS:ENABLED\n",i+1,user_passwd_temp);
    }
    fclose(file_p);
    fclose(file_p_2);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.master",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"MASTER_INST",init_conf.master_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCE_LABEL",unique_cluster_id);
    if(strcmp(init_conf.os_image_raw,"centos7")==0||strcmp(init_conf.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.google_compute_image.OS_IMAGE.self_link",string_temp);
    }
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"PUBLIC_KEY",pubkey);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(line_temp,LINE_LENGTH-1,"echo -e \"username: user%d ${var.user%d_passwd} STATUS:ENABLED\" >> /root/user_secrets.txt",i+1,i+1);
        insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export APPS_INST_SCRIPTS_URL=%s\\nexport APPS_INST_PKGS_URL=%s\" > /usr/hpc-now/appstore_env.sh",url_app_inst_root_var,url_app_pkgs_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"master_private_ip",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.compute",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"CLOUD_FLAG",cloud_flag);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_INST",init_conf.compute_inst);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCE_LABEL",unique_cluster_id);
    if(strcmp(init_conf.os_image_raw,"centos7")==0||strcmp(init_conf.os_image_raw,"centoss9")==0){
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"OS_IMAGE",init_conf.os_image_raw);
    }
    else{
        snprintf(string_temp,127,"\"%s\"",init_conf.os_image_raw);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"data.google_compute_image.OS_IMAGE.self_link",string_temp);
    }
    snprintf(line_temp,LINE_LENGTH-1,"echo -e \"export INITUTILS_REPO_ROOT=%s\" >> /etc/profile",url_initutils_root_var);
    insert_nlines(filename_temp,LINE_LENGTH_SMALL,"mount",line_temp);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.database",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCE_LABEL",unique_cluster_id);

    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack.natgw",stackdir,PATH_SLASH);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RANDOM_STRING",randstr);
    global_nreplace(filename_temp,LINE_LENGTH_SMALL,"RESOURCE_LABEL",unique_cluster_id);

    for(i=0;i<init_conf.node_num;i++){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack.compute %s%shpc_stack_compute%d.tf %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,i+1,SYSTEM_CMD_REDIRECT);
        system(cmdline);
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shpc_stack_compute%d.tf",stackdir,PATH_SLASH,i+1);
        snprintf(string_temp,127,"compute%d",i+1);
        global_nreplace(filename_temp,LINE_LENGTH_SMALL,"COMPUTE_NODE_N",string_temp);
    }
    generate_tf_files(stackdir);
    if(tf_execution(tf_run,"init",workdir,crypto_keyfile,0)!=0){
        gcp_credential_convert(workdir,"delete",0);
        clear_if_failed(stackdir,confdir,vaultdir,3);
        return 5;
    }
    if(tf_execution(tf_run,"apply",workdir,crypto_keyfile,1)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Rolling back and exit now ...\n");
        if(tf_execution(tf_run,"destroy",workdir,crypto_keyfile,1)==0){
            delete_decrypted_files(workdir,crypto_keyfile);
            clear_if_failed(stackdir,confdir,vaultdir,3);
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Successfully rolled back and destroyed the residual resources.\n");
            printf("[  ****  ] Run " HIGH_GREEN_BOLD "hpcopr -b viewlog --log err --hist --print" RESET_DISPLAY " for details.\n");
            gcp_credential_convert(workdir,"delete",0);
            return 7;
        }
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Failed to roll back. Please try 'hpcopr destroy' later.\n");
        delete_decrypted_files(workdir,crypto_keyfile);
        gcp_credential_convert(workdir,"delete",0);
        return 9;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%shpc_stack_compute1.tf %s%scompute_template %s",COPY_FILE_CMD,stackdir,PATH_SLASH,stackdir,PATH_SLASH,SYSTEM_CMD_REDIRECT);   
    system(cmdline);
    getstate(workdir,crypto_keyfile);
    printf("[ STEP 3 ] Remote executing now, please wait %d seconds for this step ...\n",GENERAL_SLEEP_TIME*3);
    for(i=0;i<GENERAL_SLEEP_TIME*3;i++){
        printf("[ -WAIT- ] Still need to wait %d seconds ... \r",GENERAL_SLEEP_TIME*3-i);
        fflush(stdout);
        sleep(1);
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sterraform.tfstate",stackdir,PATH_SLASH);
    find_and_nget(filename_temp,LINE_LENGTH_SHORT,"\"name\": \"hpc_storage\",","","",40,"\"self_link\":","","",'\"',4,bucket_selflink,128);
    find_and_nget(filename_temp,LINE_LENGTH_EXT,"\"name\": \"hpc_storage_key\",","","",20,"\"private_key\":","","",'\"',4,bucket_private_key,5120);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Sending commands and sync files ...\n");
    snprintf(gcp_bucket_key,FILENAME_LENGTH-1,"%s%sbucket_key.txt",vaultdir,PATH_SLASH);
    base64decode(bucket_private_key,gcp_bucket_key);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket_key.json","root","put","",0);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%sbucket_info.txt",vaultdir,PATH_SLASH);
    save_bucket_info(cloud_flag,filename_temp,randstr,init_conf.region_id,bucket_selflink,"",gcp_bucket_key);
    delete_file_or_dir(gcp_bucket_key);
    save_cluster_vaults(vaultdir,init_conf.master_passwd,init_conf.compute_passwd,database_root_passwd,database_acct_passwd,randstr,cloud_flag,"","");
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/hpc_data/cluster_data/.bucket.info","root","put","",0); 
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"connect",7);
    remote_exec(workdir,crypto_keyfile,sshkey_folder,"all",8);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " After the initialization:\n|\n");
    graph(workdir,crypto_keyfile,0);
    printf("|\n");
    time(&current_time_long);
    time_p=gmtime(&current_time_long);
    snprintf(current_date,11,"%d-%d-%d",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday);
    snprintf(current_time,11,"%d:%d:%d",time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    master_vcpu=get_cpu_num(init_conf.master_inst);
    compute_vcpu=get_cpu_num(init_conf.compute_inst);
    database_vcpu=2;
    natgw_vcpu=2;
    if(*(init_conf.master_inst+0)=='a'){
        strcpy(master_cpu_vendor,"amd64");
    }
    else{
        strcpy(master_cpu_vendor,"intel64");
    }
    if(*(init_conf.compute_inst+0)=='a'){
        strcpy(compute_cpu_vendor,"amd64");
    }
    else{
        strcpy(compute_cpu_vendor,"intel64");
    }
    strcpy(usage_logfile,USAGE_LOG_FILE);
    file_p=fopen(usage_logfile,"a+");
    fprintf(file_p,"%s-%s,%s,master,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,master_vcpu,current_date,current_time,master_cpu_vendor,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,database,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,database_vcpu,current_date,current_time,init_conf.region_id);
    fprintf(file_p,"%s-%s,%s,natgw,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,intel64,%s\n",init_conf.cluster_id,randstr,cloud_flag,natgw_vcpu,current_date,current_time,init_conf.region_id);
    for(i=0;i<init_conf.node_num;i++){
        fprintf(file_p,"%s-%s,%s,compute%d,%d,%s,%s,RUNNING_DATE,RUNNING_TIME,NULL1,NULL2,%s,%s\n",init_conf.cluster_id,randstr,cloud_flag,i+1,compute_vcpu,current_date,current_time,compute_cpu_vendor,init_conf.region_id);
    }
    fclose(file_p);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%shostfile_latest",stackdir,PATH_SLASH);
    remote_copy(workdir,crypto_keyfile,sshkey_folder,filename_temp,"/root/hostfile","root","put","",0);
    sync_statefile(workdir,crypto_keyfile,sshkey_folder);
    snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s%s.%s %s", MKDIR_CMD,sshkey_folder,PATH_SLASH,init_conf.cluster_id,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_user_sshkey(init_conf.cluster_id,"root","ENABLED",sshkey_folder,crypto_keyfile);
    for(i=0;i<init_conf.hpc_user_num;i++){
        snprintf(string_temp,127,"user%d",i+1);
        get_user_sshkey(init_conf.cluster_id,string_temp,"ENABLED",sshkey_folder,crypto_keyfile);
    }
    print_cluster_init_done();
    create_local_tf_config(tf_run,stackdir);
    delete_decrypted_files(workdir,crypto_keyfile);
    gcp_credential_convert(workdir,"delete",0);
    return 0;
}