/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef JOBMAN_H
#define JOBMAN_H

typedef struct
{
    char* app_name;
    int node_num;
    int tasks_per_node;
    char* job_name;
    int duration_hours;
    char* job_exec;
    char* job_data;
} jobinfo;

int get_job_info(int argc, char** argv, char* workdir, char* user_name, char* sshkey_dir, char* crypto_keyfile, jobinfo* job_info);
int job_submit(char* workdir, char* user_name, char* sshkey_dir, jobinfo* job_info);
int job_cancel(char* workdir, char* user_name, char* sshkey_dir, char* job_id);
int job_list(char* workdir, char* user_name, char* sshkey_dir);

#endif