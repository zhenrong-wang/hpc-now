/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef APPMAN_H
#define APPMAN_H

int app_list(char* workdir, char* option, char* user_name, char* app_name, char* sshkey_dir, char* std_redirect);
int app_operation(char* workdir, char* user_name, char* option, char* app_name, char* sshkey_dir);

#endif