/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef INSTALLER_H
#define INSTALLER_H

#define INSTALLER_VERSION_CODE "0.2.0.0062"
#define DEFAULT_URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/hpcopr-dev/"

int check_internet_installer(void);
void print_header_installer(void);
void print_tail_installer(void);
void print_help_installer(void);
int check_current_user_root(void);
int license_confirmation(void);
int install_services(int hpcopr_loc_flag, char* hpcopr_loc, int crypto_loc_flag, char* now_crypto_loc);
int uninstall_services(void);
int update_services(int hpcopr_loc_flag, char* hpcopr_loc, int crypto_loc_flag, char* now_crypto_loc);
int valid_loc_format_or_not(char* loc_string);
int split_parameter(char* param, char* param_head, char* param_tail);

#endif