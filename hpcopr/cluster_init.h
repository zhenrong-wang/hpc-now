/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef CLUSTER_INIT_H
#define CLUSTER_INIT_H

typedef struct {
    char cluster_id[32];
    char region_id[32];
    char zone_id[64];
    int node_num;
    int hpc_user_num;
    int hpc_nfs_volume;
    char master_init_param[32];
    char master_passwd[32];
    char compute_passwd[32];
    char master_inst[16];
    int master_bandwidth;
    char compute_inst[16];
    char os_image_raw[96];
    char ht_flag[8];
} cluster_initinfo;

void reset_initinfo(cluster_initinfo* init_info, char* cluster_id);
void empty_initinfo(cluster_initinfo* init_info);
int cluster_init_conf(char* cluster_name, char* crypto_keyfile, int batch_flag_local, int code_loc_flag_local, char* url_code_root, int argc, char* argv[]);
int create_init_dirs(char* workdir, char* stackdir, char* vaultdir, char* logdir, char* confdir, unsigned int dirlen_max);
int get_static_conf_files(char* confdir, char* cloud_name, int code_loc_flag, char* url_code_root);
int get_tf_templates(char* confdir, char* stackdir, char* cloud_name, int code_loc_flag, char* url_code_root);
int get_tf_prep_conf(char* cluster_id, char* conf_file, char* reconf_list, cluster_initinfo* init_info);
void print_read_conf_failed(int read_conf_flag);
int print_conf_summary(int batch_flag_local, cluster_initinfo* init_info, char* ucid_short);
int save_bucket_info(char* cloud_flag, char* bucket_info_file, char* bucket_id, char* region_id, char* bucket_ak, char* bucket_sk, char* gcp_bucket_key_file);
void node_user_num_fix(int* node_num, int* hpc_user_num);
int clear_if_failed(char* stackdir, char* confdir, char* vaultdir, int condition_flag);
int generate_tf_files(char* stackdir);
int save_cluster_vaults(char* vaultdir, char* mast_passwd, char* comp_password, char* db_root_password, char* db_acct_password, char* ucid_short, char* cloud_flag, char* az_sub_id, char* az_tenant_id);
int aws_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int qcloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int alicloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int hw_vm_series(const char* region_id, char* intel_generation, char* tiny_series_name, int* amd_flag);
int hwcloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int baiducloud_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int azure_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);
int gcp_cluster_init(char* workdir, char* crypto_keyfile, int batch_flag_local, tf_exec_config* tf_run);

#endif