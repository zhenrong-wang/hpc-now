/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef TIME_PROCESS_H
#define TIME_PROCESS_H

void datetime_to_num(char* date_string, char* time_string, struct tm* datetime_num);
double calc_running_hours(char* prev_date, char* prev_time, char* current_date, char* current_time);

#endif