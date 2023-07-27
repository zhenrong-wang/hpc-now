/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
 * Bug report: info@hpc-now.com
 */

#ifndef LOCATIONS_H
#define LOCATIONS_H

int valid_loc_format_or_not(char* loc_string);
int get_locations(void);
int reset_locations(void);
int show_locations(void);
int configure_locations(void);

int valid_ver_or_not(char* version_code);
int valid_md5_or_not(char* md5_input);
int get_vers_md5_vars(void);
int reset_vers_md5_vars(void);
int show_vers_md5vars(void);

#endif