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

#include "now_macros.h"
#include "general_funcs.h"
#include "cluster_general_funcs.h"
#include "transfer.h"

int export_cluster(char* cluster_name, char* user_name, char* trans_keyfile){
    char workdir[DIR_LENGTH]="";
    char tmp_root[DIR_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    sprintf(tmp_root,"%s%s%s-%s",HPC_NOW_ROOT_DIR,PATH_SLASH,cluster_name,user_name);
    sprintf(cmdline,"%s %s %s",MKDIR_CMD,tmp_root,SYSTEM_CMD_REDIRECT);
    system(cmdline);
    get_workdir(workdir,cluster_name);
    return 0;
}
int import_cluster(char* cluster_name, char* trans_keyfile){
    return 0;
}
