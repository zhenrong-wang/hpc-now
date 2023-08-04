/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef TIME_PROCESS_H
#define TIME_PROCESS_H

void datetime_to_num(char* date_string, char* time_string, struct tm* datetime_num);
double calc_running_hours(char* prev_date, char* prev_time, char* current_date, char* current_time);

#endif