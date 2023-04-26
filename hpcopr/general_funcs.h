/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef GENERAL_FUNCS_H
#define GENERAL_FUNCS_H

void reset_string(char* orig_string);
int fgetline(FILE* file_p, char* line_string);
int contain_or_not(const char* line, const char* findkey);
int global_replace(char* filename, char* orig_string, char* new_string);
int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string);
int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string);
int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5);
int calc_str_num(char* line, char split_ch);
int get_seq_string(char* line, char split_ch, int string_seq, char* get_string);
int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string);
int file_exist_or_not(char* filename);
int file_empty_or_not(char* filename);
int folder_exist_or_not(char* foldername);
int generate_random_passwd(char* password);
int generate_random_db_passwd(char* password);
int generate_random_string(char* random_string);

#endif