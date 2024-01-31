/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef LOCATIONS_H
#define LOCATIONS_H

int valid_loc_format_or_not(char* loc_string);
int get_locations(void);
int reset_locations(void);
int show_locations(void);
int configure_locations(int batch_flag_local);
int reset_tf_running(void);
int get_tf_running(tf_exec_config* tf_config, char* tf_config_file);
int show_tf_running_config(void);
int update_tf_running(char* new_tf_runner, char* new_dbg_level, int new_max_time);

int valid_ver_or_not(char* version_code);
int valid_sha_or_not(char* sha_input);
int valid_ver_or_not_tofu(char* version_code);
int get_vers_sha_vars(void);
int reset_vers_sha_vars(void);
int show_vers_sha_vars(void);

#endif