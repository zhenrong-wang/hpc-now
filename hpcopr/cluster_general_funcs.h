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
void get_latest_hosts(char* stackdir, char* hostfile_latest);
int decrypt_get_bucket_conf(char* workdir, char* crypto_keyfile, char* bucket_conf);
int get_cloud_flag(char* workdir, char* cloud_flag);
int remote_copy(char* workdir, char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option);
void create_and_get_vaultdir(char* workdir, char* vaultdir);
int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes);
int remote_exec_general(char* workdir, char* sshkey_folder, char* remote_user, char* commands, int delay_minutes);
int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag);
int get_cpu_num(const char* vm_model);
int get_compute_node_num(char* currentstate_file, char* option);
int decrypt_single_file(char* now_crypto_exec, char* filename, char* md5sum);
int decrypt_files(char* workdir, char* crypto_key_filename);
void encrypt_and_delete(char* now_crypto_exec, char* filename, char* md5sum);
int delete_decrypted_files(char* workdir, char* crypto_key_filename);
int getstate(char* workdir, char* crypto_filename);
int generate_sshkey(char* sshkey_folder, char* pubkey);
int update_cluster_summary(char* workdir, char* crypto_keyfile);
int archive_log(char* logarchive, char* logfile);
void update_compute_template(char* stackdir, char* cloud_flag);
int wait_for_complete(char* workdir, char* option, char* errorlog, int silent_flag);
int graph(char* workdir, char* crypto_keyfile, int graph_level);
int cluster_empty_or_not(char* workdir);
int cluster_asleep_or_not(char* workdir);
int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, char* error_log, int silent_flag);
int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option);
int get_vault_info(char* workdir, char* crypto_keyfile, char* root_flag);
int check_pslock(char* workdir);
int confirm_to_operate_cluster(char* current_cluster_name);
int check_down_nodes(char* workdir);
int cluster_ssh(char* workdir, char* username);
int node_file_to_running(char* stackdir, char* node_name, char* cloud_flag);
void single_file_to_running(char* filename, char* cloud_flag);
int node_file_to_stop(char* stackdir, char* node_name, char* cloud_flag);
int get_cluster_bucket_id(char* workdir, char* crypto_keyfilke, char* bucket_id);
int tail_f_for_windows(char* filename);
int get_ucid(char* workdir, char* ucid_string);

int decrypt_user_passwords(char* workdir, char* crypto_keyfile);
void delete_decrypted_user_passwords(char* workdir);
void encrypt_and_delete_user_passwords(char* workdir, char* crypto_keyfile);
int sync_user_passwords(char* workdir, char* sshkey_dir);
int hpc_user_list(char* workdir, char* crypto_keyfile, int decrypt_flag);
int username_check(char* user_registry, char* username_input);
int hpc_user_add(char* workdir, char* sshkey_dir, char* crypto_keyfile, char* username, char* password);
int delete_user_from_registry(char* user_registry_file, char* username);
int hpc_user_delete(char* workdir, char* crypto_keyfile, char* sshkey_dir, char* username);
int hpc_user_enable_disable(char* workdir, char* sshkey_dir, char* username, char* crypto_keyfile, char* option);
int hpc_user_setpasswd(char* workdir, char* ssheky_dir, char* crypto_keyfile, char* username, char* password);
int usrmgr_prereq_check(char* workdir, char* option);
void usrmgr_remote_exec(char* workdir, char* sshkey_folder, int prereq_check_flag);

//int create_protection(char* workdir, int minutes);
//int check_protection(char* workdir);
//int delete_protection(char* workdir);
//void backup_tf_files(char* stackdir);
//void delete_backups(char* stackdir);

#endif