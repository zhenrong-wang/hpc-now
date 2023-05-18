/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef USAGE_AND_LOGS_H
#define USAGE_AND_LOGS_H

int get_usage(char* usage_logfile);
int get_history(char* operation_logfile);
int get_syserrlog(char* system_errlog);
int write_log(char* workdir, char* operation_logfile, char* operation, int runflag);

#endif