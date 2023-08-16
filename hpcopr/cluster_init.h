/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef CLUSTER_INIT_H
#define CLUSTER_INIT_H

typedef struct {
    char cluster_id[128];
    char region_id[128];
    char zone_id[128];
    int node_num;
    int hpc_user_num;
    int hpc_nfs_volume;
    char master_init_param[32];
    char master_passwd[32];
    char compute_passwd[32];
    char master_inst[16];
    char master_bandwidth[8];
    char compute_inst[16];
    char os_image_raw[16];
    char ht_flag[8];
} initinfo; // For future use if needed.

int cluster_init_conf(char* cluster_name, int argc, char* argv[]);
int get_tf_prep_conf(char* conf_file, char* reconf_list, char* cluster_id, char* region_id, char* zone_id, int* node_num, int* hpc_user_num, char* master_init_param, char* master_passwd, char* compute_passwd, char* master_inst, char* master_bandwidth, char* compute_inst, char* os_image_raw, char* ht_flag);
int save_bucket_info(char* bucket_id, char* region_id, char* bucket_ak, char* bucket_sk, char* bucket_info_file, char* cloud_flag);
void clear_if_failed(char* stackdir, char* confdir, char* vaultdir, int condition_flag);
void generate_tf_files(char* stackdir);
int aws_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int qcloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int alicloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int hwcloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int baiducloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);

#endif