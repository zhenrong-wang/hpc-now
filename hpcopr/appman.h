/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef APPMAN_H
#define APPMAN_H

int app_list(char* workdir, char* option, char* user_name, char* app_name, char* sshkey_dir, char* std_redirect);
int app_operation(char* workdir, char* user_name, char* option, char* app_name, char* sshkey_dir);

#endif