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
    if(argc<2){
        return 1;
    }
    return rm_pdir(argv[1]);
}
