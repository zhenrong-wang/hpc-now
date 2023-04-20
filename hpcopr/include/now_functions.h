/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef NOW_FUNCTIONS_H
#define NOW_FUNCTIONS_H

void print_empty_cluster_info(void);
void print_operation_in_progress(void);
void print_help(void);
void print_header(void);
void print_tail(void);
void print_about(void);
void read_license(void);
void print_not_in_a_workdir(char* current_dir);

void reset_string(char* orig_string);
void datetime_to_num(char* date_string, char* time_string, struct tm* datetime_num);
double calc_running_hours(char* prev_date, char* prev_time, char* current_date, char* current_time);
int fgetline(FILE* file_p, char* line_string);
int contain_or_not(const char* line, const char* findkey);
int global_replace(char* filename, char* orig_string, char* new_string);

int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string);
int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string);
int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5);
int calc_str_num(char* line, char split_ch);
int get_seq_string(char* line, char split_ch, int string_seq, char* get_string);
int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string);

int get_crypto_key(char* crypto_key_filename, char* md5sum);

int cluster_name_check(char* real_cluster_name);
int add_to_cluster_registry(char* new_cluster_name);
int delete_from_cluster_registry(char* deleted_cluster_name);
int create_new_cluster(char* crypto_keyfile, char* cluster_name, char* cloud_ak, char* cloud_sk);
int list_all_cluster_names(void);
int glance_clusters(char* target_cluster_name, char* crypto_keyfile);
int switch_to_cluster(char* target_cluster_name);
int show_current_cluster(char* cluster_workdir,char* current_cluster_name, int silent_flag);
int exit_current_cluster(void);
int remove_cluster(char* target_cluster_name, char* crypto_keyfile);

void create_and_get_stackdir(char* workdir, char* stackdir);
void create_and_get_vaultdir(char* workdir, char* vaultdir);
int remote_copy(char* workdir, char* sshkey_dir, char* option);
int remote_exec(char* workdir, char* sshkey_folder, char* exec_type, int delay_minutes);
int terraform_execution(char* tf_exec, char* execution_name, char* workdir, char* crypto_keyfile, char* error_log);

int file_exist_or_not(char* filename);
int file_empty_or_not(char* filename);
int folder_exist_or_not(char* foldername);
int valid_loc_format_or_not(char* loc_string);
int reset_locations(void);
int get_locations(void);
int show_locations(void);
int configure_locations(void);
int generate_random_passwd(char* password);
int generate_random_db_passwd(char* password);
int generate_random_string(char* random_string);

int check_internet(void);
int check_current_user(void);
int file_validity_check(char* filename, int repair_flag, char* target_md5);
int check_and_install_prerequisitions(int repair_flag);
int get_ak_sk(char* secret_file, char* crypto_key_file, char* ak, char* sk, char* cloud_flag);
int get_cpu_num(const char* vm_model);
int check_pslock(char* workdir);
int get_compute_node_num(char* currentstate_file, char* option);
int decrypt_files(char* workdir, char* crypto_key_filename);
int delete_decrypted_files(char* workdir, char* crypto_key_filename);
int getstate(char* workdir, char* crypto_filename);
int generate_sshkey(char* sshkey_folder, char* pubkey);
int graph(char* workdir, char* crypto_keyfile);
int update_cluster_summary(char* workdir, char* crypto_keyfile);
void archive_log(char* stackdir);
int wait_for_complete(char* workdir, char* option);

int aws_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int qcloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);
int alicloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);

int cluster_empty_or_not(char* workdir);
int cluster_asleep_or_not(char* workdir);

int update_usage_summary(char* workdir, char* crypto_keyfile, char* node_name, char* option);

int cluster_destroy(char* workdir, char* crypto_keyfile,int forceflag);
int delete_compute_node(char* workdir, char* crypto_keyfile, char* param);
int add_compute_node(char* workdir, char* crypto_keyfile, char* add_number_string);
int shudown_compute_nodes(char* workdir, char* crypto_keyfile, char* param);
int turn_on_compute_nodes(char* workdir, char* crypto_keyfile, char* param);
int check_reconfigure_list(char* workdir);
int reconfigure_compute_node(char* workdir, char* crypto_keyfile, char* new_config, char* htflag);
int reconfigure_master_node(char* workdir, char* crypto_keyfile, char* new_config);
int cluster_sleep(char* workdir, char* crypto_keyfile);
int cluster_wakeup(char* workdir, char* crypto_keyfile, char* option);
int rotate_new_keypair(char* workdir, char* cloud_ak, char* cloud_sk, char* crypto_keyfile);
int get_default_conf(char* workdir, char* crypto_keyfile);
int edit_configuration_file(char* workdir);

int get_usage(char* usage_logfile);
int get_syslog(char* operation_logfile);
int get_vault_info(char* workdir, char* crypto_keyfile);
int system_cleanup(void);
int write_log(char* workdir, char* operation_logfile, char* operation, int runflag);

#endif