/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef CLUSTER_GENERAL_FUNCS_H
#define CLUSTER_GENERAL_FUNCS_H

int cluster_role_detect(char* workdir, char* cluster_role, char* cluster_role_ext);
int add_to_cluster_registry(char* new_cluster_name, char* import_flag);
int create_and_get_stackdir(char* workdir, char* stackdir);
void get_latest_hosts(char* stackdir, char* hostfile_latest);
int decrypt_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_info);
int get_cloud_flag(char* workdir, char* cloud_flag);
int remote_copy(char* workdir, char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option, char* recursive_flag, int silent_flag);
int get_user_sshkey(char* cluster_name, char* user_name, char* user_status, char* sshkey_dir);
int activate_sshkey(char* ssh_privkey);
void delete_user_sshkey(char* cluster_name, char* user_name, char* sshkey_dir);
int create_and_get_vaultdir(char* workdir, char* vaultdir);
int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes);
int remote_exec_general(char* workdir, char* sshkey_folder, char* username, char* commands, char* extra_options, int delay_minutes, int silent_flag, char* std_redirect, char* err_redirect);
int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag);
int display_cloud_info(char* workdir);
int get_azure_info(char* workdir, char* az_subscription_id, char* az_tenant_id);
int get_cpu_num(const char* vm_model);
int get_compute_node_num(char* currentstate_file, char* option);
int decrypt_single_file(char* now_crypto_exec, char* filename, char* md5sum);
int decrypt_single_file_general(char* now_crypto_exec, char* source_file, char* target_file, char* md5sum);
int decrypt_files(char* workdir, char* crypto_key_filename);
void encrypt_and_delete(char* now_crypto_exec, char* filename, char* md5sum);
int delete_decrypted_files(char* workdir, char* crypto_key_filename);
int getstate(char* workdir, char* crypto_keyfile);
int get_state_value(char* workdir, char* key, char* value);
int generate_sshkey(char* sshkey_folder, char* pubkey);
int update_cluster_summary(char* workdir, char* crypto_keyfile);
int archive_log(char* logarchive, char* logfile);
void update_compute_template(char* stackdir, char* cloud_flag);
int wait_for_complete(char* tf_realtime_log, char* option, int max_time, char* errorlog, char* errlog_archive, int silent_flag);
int graph(char* workdir, char* crypto_keyfile, int graph_level);
int cluster_empty_or_not(char* workdir);
int cluster_asleep_or_not(char* workdir);
int cluster_full_running_or_not(char* workdir);
int tf_exec_config_validation(tf_exec_config* tf_run);
int tf_execution(tf_exec_config* tf_run, char* execution_name, char* workdir, char* crypto_keyfile, int silent_flag);
int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option);
int get_vault_info(char* workdir, char* crypto_keyfile, char* username, char* bucket_flag, char* root_flag);
int check_pslock(char* workdir);

int create_local_tf_config(tf_exec_config* tf_run,char* stackdir);
int check_local_tf_config(char* workdir, char* tf_running_config_local);
int delete_local_tf_config(char* stackdir);

int valid_vm_config_or_not(char* workdir, char* vm_config);

int confirm_to_operate_cluster(char* current_cluster_name, int batch_flag_local);
int prompt_to_confirm(const char* prompt_string, const char* confirm_string, int batch_flag_local);
int prompt_to_confirm_args(const char* prompt_string, const char* confirm_string, int batch_flag_local, int argc, char** argv, char* cmd_flag);
int prompt_to_input(const char* prompt_string, char* reply_string, int batch_flag_local);
int prompt_to_input_required_args(const char* prompt_string, char* reply_string, int batch_flag_local,int argc, char** argv, char* cmd_keyword);
int prompt_to_input_optional_args(const char* prompt_confirm, const char* confirm_string, const char* prompt_string, char* reply_string, int batch_flag_local,int argc, char** argv, char* cmd_keyword);

int check_down_nodes(char* workdir);
int cluster_ssh(char* workdir, char* username, char* cluster_role);
int node_file_to_running(char* stackdir, char* node_name, char* cloud_flag);
void single_file_to_running(char* filename, char* cloud_flag);
int node_file_to_stop(char* stackdir, char* node_name, char* cloud_flag);
int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk);
int tail_f_for_windows(char* filename);
int get_ucid(char* workdir, char* ucid_string);

int decrypt_user_passwords(char* workdir, char* crypto_keyfile);
void delete_decrypted_user_passwords(char* workdir);
void encrypt_and_delete_user_passwords(char* workdir, char* crypto_keyfile);
int sync_user_passwords(char* workdir, char* sshkey_dir);
int sync_statefile(char* workdir, char* sshkey_dir);

int user_password_complexity_check(char* password, const char* special_chars);
int input_user_passwd(char* password_string, int batch_flag_local);
int user_name_quick_check(char* cluster_name, char* user_name, char* sshkey_dir);
int username_check(char* user_registry, char* username_input);
int username_check_add(char* user_registry, char* username_input);
int username_check_select(char* user_registry, char* username_input);
int delete_user_from_registry(char* user_registry_file, char* username);

void get_workdir(char* cluster_workdir, char* cluster_name);
int get_cluster_name(char* cluster_name, char* cluster_workdir);
int create_cluster_registry(void);
int list_all_cluster_names(int header_flag);
int exit_current_cluster(void);
int delete_from_cluster_registry(char* deleted_cluster_name);
int update_tf_passwords(char* base_tf, char* master_tf, char* user_passwords);

int modify_payment_single_line(char* filename_temp, char* modify_flag, char* line_buffer);
int modify_payment_lines(char* stackdir, char* cloud_flag, char* modify_flag);
int generate_bceconfig(char* vaultdir, char* region_id, char* bucket_ak, char* bucket_sk);
int decrypt_bcecredentials(char* workdir);
int gcp_credential_convert(char* workdir, const char* operation, int key_flag);

/*
 * If silent_flag==1, verbose. Will tell the user which cluster is active
 * If silent_flag==0, silent. Will print nothing
 * If silent_flag== other_number, Will only show the warning
 */
int show_current_cluster(char* cluster_workdir, char* current_cluster_name, int silent_flag);
int current_cluster_or_not(char* current_indicator, char* cluster_name);
int cluster_name_check(char* cluster_name);
int check_and_cleanup(char* prev_workdir);
int get_max_cluster_name_length(void);

int password_to_clipboard(char* cluster_workdir, char* username);
int generate_rdp_file(char* cluster_name, char* master_address, char* username);
int start_rdp_connection(char* cluster_workdir, char* username, int password_flag);
int cluster_rdp(char* cluster_workdir, char* username, char* cluster_role, int password_flag);

int list_regions(char* cluster_name);

#endif