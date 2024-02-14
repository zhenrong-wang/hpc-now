/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include "../hpcopr/now_macros.h"
#include "../hpcopr/general_funcs.h"

#else
#include "..\\hpcopr\\now_macros.h"
#include "..\\hpcopr\\general_funcs.h"
#endif

int main(int argc, char** argv){
    if(argc<5){
        printf("\nINVALID_FORMAT!\n\n");
        return 1;
    }
    //printf("%d---------\n",fuzzy_strcmp("ofast.exe","*.exe",LINE_LENGTH_SHORT));
    int run_flag=batch_file_operation(argv[2],argv[3],argv[4],argv[1]);
    printf("\nRESULT: %d\n\n",run_flag);
    if(run_flag==0){
        return 0;
    }
    return 3;
}
