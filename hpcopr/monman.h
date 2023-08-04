/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef MONMAN_H
#define MONMAN_H

int get_cluster_mon_data(char* cluster_name, char* sshkey_dir, char* mon_data_file);
int update_all_mon_data(char* cluster_registry, char* sshkey_dir);
int valid_time_format_or_not(char* datetime_input, int extend_flag, char* date_string, char* time_string);
int show_cluster_mon_data(char* cluster_name, char* sshkey_dir, char* node_name_list, char* start_datetime, char* end_datetime, char* interval, char* view_option, char* export_dest);

#endif