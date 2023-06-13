/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef PREREQ_CHECK_H
#define PREREQ_CHECK_H

int check_internet(void);
int file_validity_check(char* filename, int repair_flag, char* target_md5);
int check_current_user(void);
int install_bucket_clis(int silent_flag);
int check_and_install_prerequisitions(int repair_flag);
int command_name_check(char* command_name_input, char* command_prompt);
int command_parser(int argc, char** argv, char* command_name_prompt, char* workdir, char* cluster_name);

#endif