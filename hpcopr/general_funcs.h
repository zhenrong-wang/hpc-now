/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef GENERAL_FUNCS_H
#define GENERAL_FUNCS_H

void sleep_func(unsigned int time);
int string_to_positive_num(char* string);
void fflush_stdin(void);

/* DEPRECATED. Please do not use. */
void reset_string(char* orig_string);
/* SECURE VERSION. */
void reset_nstring(char string[], unsigned int string_length);

/* DEPRECATED. Please do not use. */
int get_key_value(char* filename, char* key, char ch, char* value);
/* SECURE VERSION. */
int get_key_nvalue(char* filename, unsigned int linelen_max, char* key, char ch, char value[], unsigned int valen_max);

/* DEPRECATED. Please do not use. */
int calc_str_num(char* line, char split_ch);
/* SECURE VERSION. */
int calc_str_nnum(char* line, char split_ch);

/* DEPRECATED. Please do not use. */
int get_seq_string(char* line, char split_ch, int string_seq, char* get_string);
/* SECURE VERSION. */
int get_seq_nstring(char line[], char split_ch, int string_seq, char get_str[], unsigned int getstr_len_max);

/* DEPRECATED. Please do not use. */
int fgetline(FILE* file_p, char* line_string);
/* SECURE VERSION. */
int fngetline(FILE* file_p, char line_string[], unsigned int max_length);

/* DEPRECATED. Please do not use. */
int contain_or_not(const char* line, const char* findkey);
/* SECURE VERSION. */
int contain_or_nnot(char line[], char findkey[]);

/* DEPRECATED. Please do not use. */
int global_replace(char* filename, char* orig_string, char* new_string);
/* SECURE VERSION. */
int global_nreplace(char* filename, unsigned int linelen_max, char* orig_string, char* new_string);

/* DEPRECATED. Please do not use. */
int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string);
/* SECURE VERSION. */
char* line_nreplace(char* orig_line, int contain_count, char* orig_string, char* new_string);

/* DEPRECATED. Please do not use. */
int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string);
/* SECURE VERSION. */
int find_and_nreplace(char* filename, unsigned int linelen_max, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string);

/* DEPRECATED. Please do not use. */
int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5);
/* SECURE VERSION. */
int find_multi_nkeys(char* filename, unsigned int linelen_max, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5);

/* DEPRECATED. Please do not use. */
int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string);
/* SECURE VERSION. */
int find_and_nget(char* filename, unsigned int linelen_max, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char get_str[], int get_strlen_max);

int file_exist_or_not(char* filename);
int file_empty_or_not(char* filename);
int folder_exist_or_not(char* foldername);
int folder_check_general(char* foldername, int rw_flag);
int folder_empty_or_not(char* foldername);
int delete_file_or_dir(char* file_or_dir);
int_64bit get_filesize_byte(FILE* file_p);

int rm_file_or_dir(char* file_or_dir);
int mk_pdir(char* pathname);
int rm_pdir(char* pathname);
int cp_file(char* current_filename, char* new_filename, int force_flag);
int fuzzy_strcmp(char* target_string, char* fuzzy_string);
char* get_first_fuzzy_subpath(char* pathname, char* fuzzy_name, unsigned int buffer_size);
int batch_file_operation(char* source_dir, char* fuzzy_filename, char* target_dir, char* option, int force_flag); 

int password_complexity_check(char* password, char* special_chars);
int generate_random_passwd(char* password); /* This function is deprecated, please use generate_random_npasswd */
int generate_random_npasswd(char password_array[], unsigned int password_array_len, char special_chars_array[], unsigned int special_chars_array_len);
int generate_random_db_passwd(char password[], unsigned int len_max);
int generate_random_string(char* random_string); /* This function is deprecated, please use generate_random_nstring*/
int generate_random_nstring(char random_string[], unsigned int len_max, int start_flag);

char* getpass_win(char* prompt); /* This function is deprecated. Please use getpass_stdin() instead. */
int getpass_stdin(char* prompt, char pass_string[], unsigned int pass_length); 

int insert_lines(char* filename, char* keyword, char* insert_string);
int insert_nlines(char* filename, unsigned int linelen_max, char* keyword, char* insert_string);

int local_path_parser(char* path_string, char* path_final);
int local_path_nparser(char* path_string, char path_final[], unsigned int path_final_len_max);

int direct_path_check(char* path_string, char* hpc_user, char* real_path);
int direct_path_ncheck(char* path_string, char* hpc_user, char* real_path, unsigned int real_path_len_max);

int file_creation_test(char* filename);
int file_cr_clean(char* filename);

int file_trunc_by_kwds(char* filename, char* start_key, char* end_key, int overwrite_flag);
int file_ntrunc_by_kwds(char* filename, unsigned int linelen_max, char* start_key, char* end_key, int overwrite_flag);

int delete_lines_by_kwd(char* filename, char* key, int overwrite_flag);
int delete_nlines_by_kwd(char* filename, unsigned int linelen_max, char* key, int overwrite_flag);

/* Deprecating: MD5 algorithms */
int get_crypto_key(char* crypto_key_filename, char* md5sum);
int get_nmd5sum(char* filename, char md5sum_string[], int md5sum_length);
int password_md5_hash(char* password, char md5_hash[], int md5_length);

/* SHA-256 */
int get_file_sha_hash(char* filename, char hash_string[], int hash_length);
int get_file_sha_hash_full(char* filename, char hash_string_full[], int hash_length);
int password_sha_hash(char* password, char hash[], int hash_length);

int cmd_flg_or_not(char* argv);
int cmd_key_or_not(char* argv);
int cmd_flag_check(int argc, char** argv, char* flag_string);
int cmd_keyword_check(int argc, char** argv, char* key_word, char* kwd_string);
int cmd_keyword_ncheck(int argc, char** argv, char* key_word, char* kwd_string, unsigned int n);

int include_string_or_not(int cmd_c, char** cmds, char* string);
int windows_path_to_nstring(char* input_string, char new_string[], unsigned int maxlen);

/* Base64 decode */
char* base64_clear_CRLF(char orig[], int length);
unsigned char get_base64_index(char base64_char);
int base64decode(char* encoded_string, char* export_path);
int base64encode(char* plain_string, char* export_path);

int reset_windows_cmd_display(void);
int get_win_appdata_dir(char appdata[], unsigned int dir_lenmax);

#endif