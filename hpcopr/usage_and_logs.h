/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef USAGE_AND_LOGS_H
#define USAGE_AND_LOGS_H

int view_system_logs(char* logfile, char* view_option, char* export_path);
int write_operation_log(char* cluster_name, char* operation_logfile, int argc, char** argv, char* description, int runflag);

#endif