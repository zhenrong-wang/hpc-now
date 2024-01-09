/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef DATAMAN_H
#define DATAMAN_H

void unset_bucket_envs(char* cloud_flag);
void bucket_path_check(char* path_string, char* hpc_user, char* real_path);
void rf_flag_parser(const char* rflag, const char* fflag, char* real_rflag, char* real_fflag);

int bucket_cp(char* workdir, char* crypto_keyfile, char* hpc_user, char* source_path, char* target_path, char* rflag, char* fflag, char* cloud_flag, char* cmd_type);
int bucket_rm_ls(char* workdir, char* crypto_keyfile, char* hpc_user, char* remote_path, char* rflag, char* fflag, char* cloud_flag, char* cmd_type);

int direct_cp_mv(char* workdir, char* crypto_keyfile, char* hpc_user, char* sshkey_dir, char* source_path, char* target_path, char* recursive_flag, char* force_flag, char* cmd_type);
int direct_rm_ls_mkdir(char* workdir, char* crypto_keyfile, char* hpc_user, char* sshkey_dir, char* remote_path, char* force_flag, char* recursive_flag, char* cmd_type);
int direct_file_operations(char* workdir, char* crypto_keyfile, char* hpc_user, char* sshkey_dir, char* remote_path, char* cmd_type);

int remote_bucket_cp(char* workdir, char* crypto_keyfile, char* hpc_user, char* sshkey_dir, char* source_path, char* dest_path, char* rflag, char* fflag, char* cloud_flag, char* cmd_type);

#endif