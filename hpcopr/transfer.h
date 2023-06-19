/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef TRANSFER_H
#define TRANSFER_H

int get_cluster_name_import(char* cluster_name_output, char* tmp_top_output, char* tmp_import_root, char* md5sum);
int user_list_check(char* cluster_name, char* user_list_read, char* user_list_final, int* user1_flag);
int export_cluster(char* cluster_name, char* user_list, char* admin_flag, char* crypto_keyfile, char* trans_keyfile, char* export_target_file);
int import_cluster(char* zip_file, char* trans_keyfile, char* crypto_keyfile);
int update_cluster_status(char* cluster_name, char* currentstate, char* compute_template);

#endif