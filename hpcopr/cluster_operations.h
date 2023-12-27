/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef CLUSTER_OPERATIONS_H
#define CLUSTER_OPERATIONS_H

int switch_to_cluster(char* target_cluster_name);
int glance_clusters(char* target_cluster_name, char* crypto_keyfile);
int remove_cluster(char* target_cluster_name, char*crypto_keyfile, char* force_flag, tf_exec_config* tf_run);
int refresh_cluster(char* target_cluster_name, char* crypto_keyfile, char* force_flag, tf_exec_config* tf_run);
int create_new_cluster(char* crypto_keyfile, char* cluster_name, char* cloud_ak, char* cloud_sk, char* az_subscription, char* az_tenant, char* echo_flag, char* gcp_flag, int batch_flag_local);
int rotate_new_keypair(char* workdir, char* cloud_ak, char* cloud_sk, char* crypto_keyfile, char* echo_flag, int batch_flag_local);
int cluster_destroy(char* workdir, char* crypto_keyfile, char* force_flag, int batch_flag_local, tf_exec_config* tf_run);
int delete_compute_node(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run);
int add_compute_node(char* workdir, char* crypto_keyfile, char* add_number_string, tf_exec_config* tf_run);
int shutdown_compute_nodes(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run);
int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param, int batch_flag_local, tf_exec_config* tf_run);
int reconfigure_compute_node(char* workdir, char* crypto_keyfile, char* new_config, char* htflag, tf_exec_config* tf_run);
int reconfigure_master_node(char* workdir, char* crypto_keyfile, char* new_config, tf_exec_config* tf_run);
int nfs_volume_up(char* workdir, char* crypto_keyfile, char* new_volume, tf_exec_config* tf_run);
int cluster_sleep(char* workdir, char* crypto_keyfile, tf_exec_config* tf_run);
int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option, tf_exec_config* tf_run);
int get_default_conf(char* cluster_name, char* crypto_keyfile, char* edit_flag);
int edit_configuration_file(char* cluster_name, char* crypto_keyfile, int batch_flag_local);
int remove_conf(char* cluster_name);
int rebuild_nodes(char* workdir, char* crypto_keyfile, char* option, int batch_flag_local, tf_exec_config* tf_run);
int view_run_log(char* workdir, char* stream, char* run_option, char* view_option, char* export_dest);
int switch_cluster_payment(char* cluster_name, char* new_payment_method, char* crypto_keyfile, tf_exec_config* tf_run);

#endif