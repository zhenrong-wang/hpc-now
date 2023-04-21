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

char URL_REPO_ROOT[LOCATION_LENGTH]="";
char URL_CODE_ROOT[LOCATION_LENGTH]="";
char URL_SHELL_SCRIPTS[LOCATION_LENGTH]="";
char URL_NOW_CRYPTO[LOCATION_LENGTH]="";
int REPO_LOC_FLAG=0;
int CODE_LOC_FLAG=0;
int NOW_CRYPTO_LOC_FLAG=0;