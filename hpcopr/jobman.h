/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef JOBMAN_H
#define JOBMAN_H

int job_submit(char* workdir, char* option, char* user_name, char* sshkey_dir);
int job_cancel();
int job_list();

#endif