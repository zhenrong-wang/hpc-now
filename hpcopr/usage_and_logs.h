/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef USAGE_AND_LOGS_H
#define USAGE_AND_LOGS_H

int view_system_logs(char* logfile, char* view_option);
int write_operation_log(char* cluster_name, char* operation_logfile, char* operation, char* description, int runflag);

#endif