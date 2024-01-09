/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef GENERAL_PRINT_INFO_H
#define GENERAL_PRINT_INFO_H

void print_empty_cluster_info(void);
void print_cluster_init_done(void);
void print_help(char* cmd_name);
void print_header(void);
void print_version(void);
void print_tail(void);
void print_about(void);
int read_license(char* option);
void print_usrmgr_info(void);
void print_datamgr_info(void);
void print_appmgr_info(void);
void print_jobmgr_info(void);
void list_all_commands(void);
void print_new_cluster_done(int gcp_flag);

#endif