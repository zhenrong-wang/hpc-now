/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef GENERAL_FUNCS_H
#define GENERAL_FUNCS_H

int string_to_positive_num(char* string);
int get_key_value(char* filename, char* key, char ch, char* value);
void reset_string(char* orig_string);
int fgetline(FILE* file_p, char* line_string);
int fngetline(FILE* file_p, char* line_string, unsigned int max_length);
int contain_or_not(const char* line, const char* findkey);
int global_replace(char* filename, char* orig_string, char* new_string);
int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string);
int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string);
int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5);
int calc_str_num(char* line, char split_ch);
int get_seq_string(char* line, char split_ch, int string_seq, char* get_string);
//int get_seq_string_general(char* line, char split_ch, int start, int end, char* get_string);
int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string);
int file_exist_or_not(char* filename);
int file_empty_or_not(char* filename);
int folder_exist_or_not(char* foldername);
int generate_random_passwd(char* password);
int password_complexity_check(char* password, const char* special_chars);
int generate_random_db_passwd(char* password);
int generate_random_string(char* random_string);
char* getpass_win(char* prompt);
int insert_lines(char* filename, char* keyword, char* insert_string);
int local_path_parser(char* path_string, char* path_final);
int direct_path_check(char* path_string, char* real_path, char* hpc_user);
int file_creation_test(char* filename);
int file_cr_clean(char* filename);
int file_trunc_by_kwds(char* filename, char* start_key, char* end_key, int overwrite_flag);
int delete_lines_by_kwd(char* filename, char* key, int overwrite_flag);
int get_crypto_key(char* crypto_key_filename, char* md5sum);
int get_nmd5sum(char* filename, char md5sum_string[], int md5sum_length);
int password_hash(char* password, char md5_hash[], int md5_length);

int cmd_flg_or_not(char* argv);
int cmd_key_or_not(char* argv);
int cmd_flag_check(int argc, char** argv, char* flag_string);
int cmd_keyword_check(int argc, char** argv, char* key_word, char* kwd_string);
int cmd_keyword_ncheck(int argc, char** argv, char* key_word, char* kwd_string, unsigned int n);

int include_string_or_not(int cmd_c, char** cmds, char* string);
int base64decode(char* encoded_string, char* exported_path);
int windows_path_to_string(char* input_string, char* new_string);

#endif