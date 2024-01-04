/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef CLUSTER_GENERAL_FUNCS_H
#define CLUSTER_GENERAL_FUNCS_H

typedef struct{
    char bucket_address[128];
    char region_id[32];
    char bucket_ak[128];
    char bucket_sk[128];
} bucket_info;

int cluster_role_detect(char* workdir, char cluster_role[], char cluster_role_ext[], unsigned int maxlen);
int add_to_cluster_registry(char* new_cluster_name, char* import_flag);

int create_and_get_subdir(char* workdir, char* subdir_name, char subdir_path[], unsigned int dir_maxlen);
int create_and_get_stackdir(char* workdir, char* stackdir);

void get_latest_hosts(char* stackdir, char* hostfile_latest);
int decrypt_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_info);
int get_cloud_flag(char* workdir, char cloud_flag[], unsigned int maxlen);
int remote_copy(char* workdir, char* sshkey_dir, char* local_path, char* remote_path, char* username, char* option, char* recursive_flag, int silent_flag);

int chmod_ssh_privkey(char* ssh_privkey);
int get_user_sshkey(char* cluster_name, char* user_name, char* user_status, char* sshkey_dir, char* crypto_keyfile);
int delete_user_sshkey(char* cluster_name, char* user_name, char* sshkey_dir);

/* The functions here will decrypt/encrypt with chmod. */
int decrypt_user_privkey(char* ssh_privkey_encrypted, char* crypto_keyfile);
int encrypt_user_privkey(char* ssh_privkey, char* crypto_keyfile);
int decrypt_opr_privkey(char* sshkey_folder, char* crypto_keyfile);
int encrypt_opr_privkey(char* sshkey_folder, char* crypto_keyfile);

/* These functions will not chmod, only encrypt/decrypt */
int encrypt_decrypt_all_user_ssh_privkeys(char* cluster_name, char* option, char* crypto_keyfile);
int encrypt_decrypt_opr_privkey(char* sshkey_folder, char* option, char* crypto_keyfile);

//Deprecated!
int generate_sshkey(char* sshkey_folder, char* pubkey); //This is deprecated!

int generate_encrypt_opr_sshkey(char* sshkey_folder, char* crypto_keyfile);
int get_opr_pubkey(char* sshkey_folder, char* pubkey, unsigned int length);

int create_and_get_vaultdir(char* workdir, char* vaultdir);
int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes);
int remote_exec_general(char* workdir, char* sshkey_folder, char* username, char* commands, char* extra_options, int delay_minutes, int silent_flag, char* std_redirect, char* err_redirect);
int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag);
int display_cloud_info(char* workdir);

int get_azure_info(char* workdir, char* az_subscription_id, char* az_tenant_id);
int get_azure_ninfo(char* workdir, unsigned int linelen_max, char* az_subscription_id, char* az_tenant_id, unsigned int id_len_max); //Newer function

int get_cpu_num(const char* vm_model);
int get_compute_node_num(char* currentstate_file, char* option);
int decrypt_single_file(char* now_crypto_exec, char* filename, char* md5sum);
int decrypt_single_file_general(char* now_crypto_exec, char* source_file, char* target_file, char* md5sum);
int decrypt_files(char* workdir, char* crypto_key_filename);
int encrypt_and_delete(char* now_crypto_exec, char* filename, char* md5sum);
int encrypt_and_delete_general(char* now_crypto_exec, char* source_file, char* target_file, char* md5sum);
int delete_decrypted_files(char* workdir, char* crypto_key_filename);
int decrypt_cloud_secrets(char* now_crypto_exec, char* workdir, char* md5sum);
int encrypt_cloud_secrets(char* now_crypto_exec, char* workdir, char* md5sum);
int decryption_status(char* workdir);
int getstate(char* workdir, char* crypto_keyfile);

int get_state_value(char* workdir, char* key, char* value);
int get_state_nvalue(char* workdir, char* key, char* value, unsigned int valen_max); //Newer function

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
int check_pslock(char* workdir, int decrypt_flag);
int check_pslock_all(void);

int create_local_tf_config(tf_exec_config* tf_run,char* stackdir);
int check_local_tf_config(char* workdir, char tf_running_config_local[], unsigned int filename_maxlen);
int delete_local_tf_config(char* stackdir);

int valid_vm_config_or_not(char* workdir, char* vm_config);

int confirm_to_operate_cluster(char* current_cluster_name, int batch_flag_local);
int confirm_to_init_cluster(char* current_cluster_name, int batch_flag_local);
int prompt_to_confirm(const char* prompt_string, const char* confirm_string, int batch_flag_local);
int prompt_to_confirm_args(const char* prompt_string, const char* confirm_string, int batch_flag_local, int argc, char** argv, char* cmd_flag);
int prompt_to_input(const char* prompt_string, char* reply_string, int batch_flag_local);
int prompt_to_input_required_args(const char* prompt_string, char* reply_string, int batch_flag_local,int argc, char** argv, char* cmd_keyword);
int prompt_to_input_optional_args(const char* prompt_confirm, const char* confirm_string, const char* prompt_string, char* reply_string, int batch_flag_local,int argc, char** argv, char* cmd_keyword);

int check_down_nodes(char* workdir);
int cluster_ssh(char* workdir, char* username, char* cluster_role, char* sshkey_dir);
int node_file_to_running(char* stackdir, char* node_name, char* cloud_flag);
void single_file_to_running(char* filename, char* cloud_flag);
int node_file_to_stop(char* stackdir, char* node_name, char* cloud_flag);

int get_bucket_info(char* workdir, char* crypto_keyfile, char* bucket_address, char* region_id, char* bucket_ak, char* bucket_sk);
int get_bucket_ninfo(char* workdir, char* crypto_keyfile, unsigned int linelen_max, bucket_info* bucketinfo); //Newer function

int tail_f_for_windows(char* filename);

int get_ucid(char* workdir, char* ucid_string);
int get_nucid(char* workdir, char* ucid_string, unsigned int ucid_strlen_max); //Newer function

int decrypt_user_passwords(char* workdir, char* crypto_keyfile);
int delete_decrypted_user_passwords(char* workdir);
int encrypt_and_delete_user_passwords(char* workdir, char* crypto_keyfile);
int sync_user_passwords(char* workdir, char* sshkey_dir);
int sync_statefile(char* workdir, char* sshkey_dir);

int user_password_complexity_check(char* password, const char* special_chars);
int input_user_passwd(char* password_string, int batch_flag_local);
int user_name_quick_check(char* cluster_name, char* user_name, char* sshkey_dir);
int username_check(char* user_registry, char* username_input);
int username_check_add(char* workdir, char* username_input);
int username_check_select(char* user_registry, char* username_input);
int delete_user_from_registry(char* user_registry_file, char* username);

void get_workdir(char* cluster_workdir, char* cluster_name);
int get_nworkdir(char* cluster_workdir, unsigned int dirlen_max, char* cluster_name); //Newer function

int get_cluster_name(char* cluster_name, char* cluster_workdir);
int get_cluster_nname(char* cluster_name, unsigned int cluster_name_len_max, char* cluster_workdir); //Newer function

int create_cluster_registry(void);
int list_all_cluster_names(int header_flag);
int exit_current_cluster(void);
int delete_from_cluster_registry(char* deleted_cluster_name);
int update_tf_passwords(char* base_tf, char* master_tf, char* user_passwords);

int check_reconfigure_list(char* workdir, int print_flag);

int modify_payment_single_line(char* filename_temp, char* modify_flag, char* line_buffer);
int modify_payment_lines(char* stackdir, char* cloud_flag, char* modify_flag);
int generate_bceconfig(char* vaultdir, char* region_id, char* bucket_ak, char* bucket_sk);
int decrypt_bcecredentials(char* workdir);
int gcp_credential_convert(char* workdir, const char* operation, int key_flag);

int show_current_cluster(char* cluster_workdir, char* current_cluster_name, int silent_flag);
int show_current_ncluster(char* cluster_workdir, unsigned int dirlen_max, char* current_cluster_name, unsigned int cluster_name_len_max, int silent_flag); //Newer function

int current_cluster_or_not(char* current_indicator, char* cluster_name);
int cluster_name_check(char* cluster_name);
int check_and_cleanup(char* prev_workdir);
int get_max_cluster_name_length(void);

int password_to_clipboard(char* cluster_workdir, char* username);
int generate_rdp_file(char* cluster_name, char* master_address, char* username);
int start_rdp_connection(char* cluster_workdir, char* username, int password_flag);
int cluster_rdp(char* cluster_workdir, char* username, char* cluster_role, int password_flag);

FILE* check_regions_list_file(char* cluster_name);
int list_cloud_regions(char* cluster_name, int format_flag);
int list_cloud_zones(char* cluster_name, char* region, int format_flag);
int valid_region_or_not(char* cluster_name, char* region);
int valid_region_zone_or_not(char* cluster_name, char* region, char* zone);
int get_default_zone(char* cluster_name, char* region, char* default_zone);
int get_default_nzone(char* cluster_name, char* region, char* default_zone, unsigned int zone_len_max);
int valid_zone_or_not(char* cluster_name, char* zone);

#endif