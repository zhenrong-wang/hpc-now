/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef APPMAN_H
#define APPMAN_H

int appman_update_conf(char* workdir, const char* new_inst_loc, const char* new_repo_loc, char* sshkey_dir, char* std_redirect);
int appman_check_conf(char* workdir, char* user_name, char* sshkey_dir);
int app_list(char* workdir, char* option, char* user_name, char* app_name, char* sshkey_dir, char* std_redirect, char* inst_loc);
int app_operation(char* workdir, char* user_name, char* option, char* app_name, char* sshkey_dir, char* inst_loc, char* repo_loc);

#endif