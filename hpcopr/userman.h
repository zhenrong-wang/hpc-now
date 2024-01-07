/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef USERMAN_H
#define USERMAN_H

int usrmgr_prereq_check(char* workdir, char* ucmd, int batch_mode_flag);
void usrmgr_remote_exec(char* workdir, char* sshkey_folder, int prereq_check_flag);
int hpc_user_list(char* workdir, char* crypto_keyfile, int decrypt_flag, int format_flag);
int hpc_user_delete(char* workdir, char* crypto_keyfile, char* sshkey_dir, char* username);
int hpc_user_enable_disable(char* workdir, char* sshkey_dir, char* username, char* crypto_keyfile, char* option);
int hpc_user_setpasswd(char* workdir, char* ssheky_dir, char* crypto_keyfile, char* username, char* password);
int hpc_user_add(char* workdir, char* sshkey_dir, char* crypto_keyfile, char* username, char* password);

#endif