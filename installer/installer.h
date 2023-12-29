/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef INSTALLER_H
#define INSTALLER_H

#define INSTALLER_VERSION_CODE "0.3.1.0009"
#define DEFAULT_URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/hpcopr-0.3.x/"
#define URL_MSRDP_FOR_MAC "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/packages/rdp_for_mac.zip"

int check_internet_installer(void);
void print_header_installer(void);
void print_tail_installer(void);
void print_help_installer(void);
int check_current_user_root(void);
int license_confirmation(void);
int install_services(int hpcopr_loc_flag, char* hpcopr_loc, char* hpcopr_ver, char* opr_password, int crypto_loc_flag, char* now_crypto_loc, int rdp_flag);
int uninstall_services(void);
void restore_perm_windows(void);
int set_opr_password(char* opr_password);
int update_services(int hpcopr_loc_flag, char* hpcopr_loc, char* hpcopr_ver, int crypto_loc_flag, char* now_crypto_loc, int rdp_flag);
int valid_loc_format_or_not(char* loc_string);
int get_valid_verlist(void);
int version_valid(char* hpcopr_ver);

#endif