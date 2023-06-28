/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef GENERAL_FUNCS_H
#define GENERAL_FUNCS_H

int string_to_positive_num(char* string);
int get_key_value(char* filename, char* key, char ch, char* value);
void reset_string(char* orig_string);
int fgetline(FILE* file_p, char* line_string);
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
int generate_random_db_passwd(char* password);
int generate_random_string(char* random_string);
char* getpass_win(char* prompt);
int insert_lines(char* filename, char* keyword, char* insert_string);
int local_path_parser(char* path_string, char* path_final);
int file_creation_test(char* filename);
int file_cr_clean(char* filename);
int file_trunc_by_kwds(char* filename, char* start_key, char* end_key, int overwrite_flag);
int delete_lines_by_kwd(char* filename, char* key, int overwrite_flag);
int get_crypto_key(char* crypto_key_filename, char* md5sum);
int password_hash(char* password, char* md5_hash);

int cmd_flg_or_not(char* argv);
int cmd_key_or_not(char* argv);
int cmd_flag_check(int argc, char** argv, char* flag_string);
int cmd_keyword_check(int argc, char** argv, char* key_word, char* kwd_string);

int include_string_or_not(int cmd_c, char** cmds, char* string);

#endif