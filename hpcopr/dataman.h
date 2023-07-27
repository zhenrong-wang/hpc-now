/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
 * Bug report: info@hpc-now.com
 */

#ifndef DATAMAN_H
#define DATAMAN_H

void unset_aws_bucket_envs(void);
void bucket_path_check(char* path_string, char* hpc_user, char* real_path);
void rf_flag_parser(const char* rflag, const char* fflag, char* real_rflag, char* real_fflag);

int bucket_cp(char* workdir, char* hpc_user, char* source_path, char* target_path, char* rflag, char* fflag, char* crypto_keyfile, char* cloud_flag, char* cmd_type);
int bucket_rm_ls(char* workdir, char* hpc_user, char* remote_path, char* rflag, char* fflag, char* crypto_keyfile, char* cloud_flag, char* cmd_type);

int direct_cp_mv(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* target_path, char* recursive_flag, char* force_flag, char* cmd_type);
int direct_rm_ls_mkdir(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* force_flag, char* recursive_flag, char* cmd_type);
int direct_file_operations(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* cmd_type);

int remote_bucket_cp(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* dest_path, char* rflag, char* fflag, char* cloud_flag, char* crypto_keyfile, char* cmd_type);

#endif