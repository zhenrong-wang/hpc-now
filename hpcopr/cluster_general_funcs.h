/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef CLUSTER_GENERAL_FUNCS_H
#define CLUSTER_GENERAL_FUNCS_H

int get_crypto_key(char* crypto_key_filename, char* md5sum);
void create_and_get_stackdir(char* workdir, char* stackdir);
int remote_copy(char* workdir, char* sshkey_dir, char* option);
void create_and_get_vaultdir(char* workdir, char* vaultdir);
int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes);
int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag);
int get_cpu_num(const char* vm_model);
int get_compute_node_num(char* currentstate_file, char* option);
int decrypt_files(char* workdir, char* crypto_key_filename);
int delete_decrypted_files(char* workdir, char* crypto_key_filename);
int getstate(char* workdir, char* crypto_filename);
int generate_sshkey(char* sshkey_folder, char* pubkey);
int update_cluster_summary(char* workdir, char* crypto_keyfile);
void archive_log(char* logdir, char* logfile);
void update_compute_template(char* stackdir, char* cloud_flag);
int wait_for_complete(char* workdir, char* option, char* errorlog);
int graph(char* workdir, char* crypto_keyfile, int graph_level);
int cluster_empty_or_not(char* workdir);
int cluster_asleep_or_not(char* workdir);
int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, char* error_log);
int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option);
int get_vault_info(char* workdir, char* crypto_keyfile);
int check_pslock(char* workdir);
int confirm_to_operate_cluster(char* current_cluster_name);
int check_down_nodes(char* workdir);

#endif