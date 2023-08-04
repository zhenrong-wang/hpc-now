/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef TRANSFER_H
#define TRANSFER_H

int get_cluster_name_import(char* cluster_name_output, char* tmp_top_output, char* tmp_import_root, char* md5sum);
int user_list_check(char* cluster_name, char* user_list_read, char* user_list_final, int* user1_flag);
int export_cluster(char* cluster_name, char* user_list, char* admin_flag, char* crypto_keyfile, char* password, char* export_target_file);
int import_cluster(char* zip_file, char* password, char* crypto_keyfile);
int update_cluster_status(char* cluster_name, char* currentstate);

#endif