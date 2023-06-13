/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef DATAMAN_H
#define DATAMAN_H

void unset_aws_bucket_envs(void);
int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk);
int bucket_path_check(char* path_string, char* real_path, char* hpc_user);
int rf_flag_parser(const char* rf_flag, const char* cloud_flag, char* real_rflag, char* real_fflag);

int bucket_cp(char* workdir, char* hpc_user, char* source_path, char* target_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag);
int bucket_rm(char* workdir, char* hpc_user, char* remote_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag);
int bucket_mv(char* workdir, char* hpc_user, char* prev_path, char* new_path, char* force_flag, char* crypto_keyfile, char* cloud_flag);
int bucket_ls(char* workdir, char* hpc_user, char* remote_path, char* rf_flag, char* crypto_keyfile, char* cloud_flag);

int direct_path_check(char* path_string, char* real_path, char* hpc_user);
int direct_cp_mv(char* workdir, char* hpc_user, char* sshkey_dir, char* source_path, char* target_path, char* rf_flag, char* cmd_type);
int direct_rm_ls_mkdir(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* rf_flag, char* cmd_type);
int direct_file_operations(char* workdir, char* hpc_user, char* sshkey_dir, char* remote_path, char* cmd_type);

int remote_bucket_cp(char* workdir, char* hpc_user, char* sshkey_dir, char* bucket_path, char* remote_path, char* rf_flag, char* cloud_flag, char* cmd_type);

#endif