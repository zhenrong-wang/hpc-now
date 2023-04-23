/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef LOCATIONS_H
#define LOCATIONS_H

int valid_loc_format_or_not(char* loc_string);
int get_locations(void);
int reset_locations(void);
int show_locations(void);
int configure_locations(void);

#endif