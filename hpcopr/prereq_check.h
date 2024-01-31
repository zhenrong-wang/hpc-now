/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef PREREQ_CHECK_H
#define PREREQ_CHECK_H

int check_internet(void);
int check_internet_google(void);
int get_google_connectivity(void);
int file_validity_check(char* filename, int repair_flag, char* target_sha);
int check_current_user(void);
int install_bucket_clis(int silent_flag);
int repair_provider(char* plugin_root_path, char* cloud_name, char* provider_version, char* sha_exec, char* sha_zip, int force_repair_flag, char* seq_code);
int check_and_install_prerequisitions(int repair_flag);
int command_name_check(char* command_name_input, char command_prompt[], unsigned int prompt_len_max, char role_flag[], char cu_flag[], unsigned int flaglen_max);
int command_parser(int argc, char** argv, char command_name_prompt[], unsigned int prompt_len_max, char workdir[], unsigned int dir_len_max, char cluster_name[], unsigned int cluster_name_len_max, char user_name[], unsigned int user_name_len_max, char cluster_role[], unsigned int role_len_max, int* decrypt_flag);

#endif