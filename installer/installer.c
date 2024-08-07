/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <sys\stat.h>
#include "..\\hpcopr\\now_macros.h"
#include "..\\hpcopr\\general_funcs.h"
#include "..\\hpcopr\\opr_crypto.h"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "../hpcopr/now_macros.h"
#include "../hpcopr/general_funcs.h"
#include "../hpcopr/opr_crypto.h"
#endif

#include "installer.h"

static char preinst_check_failures[5][16]={
    "ssh",
    "scp",
    "curl",
    "tar",
    "unzip"
};

int check_linux_packman(char* linux_packman, int length){
#ifndef __linux__
    return 0; /* For Windows or macOS, this function is skipped. */
#else
    if(system("which yum >> /dev/null 2>&1")==0){
        strncpy(linux_packman,"yum",length-1);
        return 0;
    }
    else if(system("which dnf >> /dev/null 2>&1")==0){
        strncpy(linux_packman,"dnf",length-1);
        return 0;
    }
    else if(system("which apt >> /dev/null 2>&1")==0){
        strncpy(linux_packman,"apt",length-1);
        return 0;
    }
    else if(system("which zypper >> /dev/null 2>&1")==0){
        strncpy(linux_packman,"zypper",length-1);
        return 0;
    }
    else if(system("which pacman >> /dev/null 2>&1")==0){
        strncpy(linux_packman,"pacman",length-1);
        return 0;
    }
    else{
        strncpy(linux_packman,"",length-1);
        return 1;
    }
#endif
}

int check_internet_installer(void){
    int run_flag=check_connectivity("www.baidu.com","443",5);
    if(run_flag!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Internet connectivity check failed. Please check your DNS service\n");
        printf("[  ****  ] or network and then retry. Error Code: %d." RESET_DISPLAY "\n",run_flag);
        return 1;
    }
    return 0;
}

/**
 * @brief
 *   Check whether the prerequisites are satisfied. 
 *   Microsoft Windows and macOS doesn't provide a package manager.
 *   Therefore, any absence of prerequisites should be reported and 
 *   The users need to manually install them.
 *   For GNU/Linux, this function does nothing.
 * 
 * @param [in] void
 * 
 * @returns
 *   0 if all checked successfully
 *   1 ~ 5 as the index+1 of static char preinst_check_failures[5][16]
 *   6 if the compiler failed to detect the system-specific macros
 * 
 */
int preinstall_check(void){
#ifdef _WIN32
    if(system("where ssh >nul 2>&1")!=0){
        return 1;
    }
    if(system("where scp >nul 2>&1")!=0){
        return 2;
    }
    if(system("where curl >nul 2>&1")!=0){
        return 3;
    }
    if(system("where tar >nul 2>&1")!=0){
        return 4;
    }
    return 0;
#elif __APPLE__
    if(system("which ssh >>/dev/null 2>&1")!=0){
        return 1;
    }
    if(system("which scp >>/dev/null 2>&1")!=0){
        return 2;
    }
    if(system("which curl >>/dev/null 2>&1")!=0){
        return 3;
    }
    if(system("which tar >>/dev/null 2>&1")!=0){
        return 4;
    }
    if(system("which unzip >>/dev/null 2>&1")!=0){
        return 5;
    }
    return 0;
#elif __linux__
    return 0;
#else
    return 6;
#endif
}

/* 
 * return 0: Not installed
 * return 1: Already installed
 */
int installed_or_not(void){
#ifdef _WIN32
    USER_INFO_0* user_info0=NULL;
    DWORD result=NetUserGetInfo(NULL,L"hpc-now",0,(BYTE**)&user_info0);
    if(user_info0!=NULL){
        NetApiBufferFree(user_info0);
    }
    if(result==NERR_Success){
        return 1;
    }
    return 0;
#else
    struct passwd* pwd=getpwnam("hpc-now");
    if(pwd!=NULL){
        return 1;
    }
    return 0;
#endif
}

void print_header_installer(void){
    time_t current_time_long;
    struct tm* time_p=NULL;
    time(&current_time_long);
    time_p=localtime(&current_time_long);
    reset_windows_cmd_display();
    printf(HIGH_GREEN_BOLD "  __  __   ______   __  __" RESET_DISPLAY "\n");
    printf(HIGH_GREEN_BOLD " /  \\/ /" RESET_DISPLAY ":" HIGH_GREEN_BOLD " / __  /" RESET_DISPLAY ":" HIGH_GREEN_BOLD " /  /  /" RESET_DISPLAY ":  :. " GENERAL_BOLD "Welcome to HPC-NOW Installer! V. %s" RESET_DISPLAY "\n",INSTALLER_VERSION_CODE);
    printf(HIGH_GREEN_BOLD "/_/\\__/" RESET_DISPLAY ":'" HIGH_GREEN_BOLD "/_____/" RESET_DISPLAY ":'" HIGH_GREEN_BOLD "/_____/" RESET_DISPLAY ":' :. "  GENERAL_BOLD "Current: %d-%d-%d %d:%d:%d    LICENSE: MIT" RESET_DISPLAY "\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
    printf(RESET_DISPLAY "':: :::' '::::::' '::::::' :. " GENERAL_BOLD "[C] Shanghai HPC-NOW Technologies Co., Ltd" RESET_DISPLAY "\n\n");
    printf("| This is free software; see the source for copying conditions.  There is NO\n");
    printf("| warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
}

void print_tail_installer(void){
    printf("\n");
    printf(GENERAL_BOLD ":. visit:" RESET_DISPLAY " https://www.hpc-now.com    " GENERAL_BOLD ":. mailto:" RESET_DISPLAY " info@hpc-now.com\n");
}

/* Print out help info for this installer */
void print_help_installer(void){
#ifdef _WIN32
    printf(GENERAL_BOLD "| USAGE: Open a Command Prompt Window as Administrator.\n");
    printf("|        Type the command using either way below:" RESET_DISPLAY "\n");
    printf("|   + " GENERAL_BOLD "ABSOLUTE_PATH GENERAL_OPTION ADVANCED_OPTION(S)" RESET_DISPLAY "\n");
    printf("|     - Example 1: " HIGH_GREEN_BOLD "C:\\Users\\ABC\\installer.exe install --accept" RESET_DISPLAY "\n");
    printf("|   + " GENERAL_BOLD "RELATIVE_PATH GENERAL_OPTION ADVANCED_OPTION(S)" RESET_DISPLAY "\n");
    printf("|     - Example 2: " HIGH_GREEN_BOLD ".\\installer.exe install --cloc .\\now-crypto-aes.exe" RESET_DISPLAY "\n");
#else
    printf(GENERAL_BOLD "| USAGE: Open a Terminal and type the command using either way below:" RESET_DISPLAY "\n");
    printf("|   + " GENERAL_BOLD "sudo ABSOLUTE_PATH GENERAL_OPTION ADVANCED_OPTION(S)" RESET_DISPLAY "\n");
    printf("|     - Example 1: " HIGH_GREEN_BOLD "sudo /home/ABC/installer.exe install --accept" RESET_DISPLAY "\n");
    printf("|   + " GENERAL_BOLD "sudo RELATIVE_PATH GENERAL_OPTION ADVANCED_OPTION(S)" RESET_DISPLAY "\n");
    printf("|     - Example 2: " HIGH_GREEN_BOLD "sudo ./installer.exe install --cloc ./now-crypto-aes.exe" RESET_DISPLAY "\n");
#endif
    printf("|   " GENERAL_BOLD "General Options (REQUIRED):" RESET_DISPLAY "\n");
    printf("|     " HIGH_GREEN_BOLD "install" RESET_DISPLAY "       : Install or repair the HPC-NOW Services on your device.\n");
    printf("|     " HIGH_GREEN_BOLD "update" RESET_DISPLAY "        : Update the hpcopr to the latest or your own version.\n");
    printf("|     " HIGH_GREEN_BOLD "uninstall" RESET_DISPLAY "     : Remove the HPC-NOW services and all relevant data.\n");
    printf("|     " HIGH_GREEN_BOLD "setpass" RESET_DISPLAY "       : Update the operator's keystring.\n");
    printf("|     " HIGH_GREEN_BOLD "help" RESET_DISPLAY "          : Show this information.\n");
    printf("|     " HIGH_GREEN_BOLD "version" RESET_DISPLAY "       : Show the version code of this installer.\n");
    printf("|     " HIGH_GREEN_BOLD "verlist" RESET_DISPLAY "       : Show the available version list of hpcopr.\n");
    printf("|   " GENERAL_BOLD "Advanced Options (OPTIONAL):" RESET_DISPLAY "\n");
    printf("|     --pass PASS * Only valid for " HIGH_GREEN_BOLD "install" RESET_DISPLAY " or " HIGH_GREEN_BOLD "setpass" RESET_DISPLAY " option.\n");
    printf("|                   : Set/update the operator's keystring that includes 3\n");
    printf("|                     of 4 types: " WARN_YELLO_BOLD "A-Z  a-z  0-9  %s" RESET_DISPLAY "\n",SPECIAL_PASSWORD_CHARS);
    printf("|     --hloc LOC  * Only valid for " HIGH_GREEN_BOLD "install" RESET_DISPLAY " or " HIGH_GREEN_BOLD "update" RESET_DISPLAY " option.\n");
    printf("|                   : Provide your own location of hpcopr, both URL and local\n");
    printf("|                     filesystem path are accepted. You should guarantee that\n");
    printf("|                     the location points to a valid hpcopr executable.\n");
    printf("|     --cloc LOC  * Only valid for " HIGH_GREEN_BOLD "install" RESET_DISPLAY " or " HIGH_GREEN_BOLD "update" RESET_DISPLAY " option.\n");
    printf("|                   : Provide your own location of now-crypto, similar to\n");
    printf("|                     the --hloc parameter above.\n");
    printf("|     --hver VER  * Only valid when " GENERAL_BOLD "--hloc LOC" RESET_DISPLAY " is absent.\n");
    printf("|                   : Specify the version code of hpcopr, e.g. 0.2.0.0161\n");
    printf("|     --rdp         : Install the RDP client for GNU/Linux or macOS. Default:\n");
    printf("|                     GNU/Linux: Remmina | macOS: Microsoft RDP\n");
    printf("|     --accept      : accept the license terms to " HIGH_GREEN_BOLD "install" RESET_DISPLAY " or " HIGH_GREEN_BOLD "update" RESET_DISPLAY ".\n");
}

/*
 * This installer *MUST* be executed with root/Administrator previlege.
 */
int check_current_user_root(void){
#ifdef _WIN32
    BOOL elevation_status=FALSE;
    HANDLE token_handle=NULL;
    DWORD dw_size;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token_handle)){
        return -1;
    }
    GetTokenInformation(token_handle,TokenElevationType,(LPVOID)&elevation_status,sizeof(BOOL),&dw_size);
    CloseHandle(token_handle);
    if(elevation_status!=TokenElevationTypeFull){
        print_help_installer();
        printf(FATAL_RED_BOLD "\n[ FATAL: ] Please switch to Administrator or users with admin privilege:\n");
        printf("[  ****  ] 1. Run a Command Prompt Window as the Administrator Role\n");
        printf("[  ****  ] 2. Type the full path of this installer with an option, for example\n");
        printf("[  ****  ]    C:\\Users\\ABC\\installer-win.exe install" RESET_DISPLAY "\n");
        print_tail_installer();
        return -1; 
    }
#else
    if(geteuid()!=0){
        print_help_installer();
        printf(FATAL_RED_BOLD "\n[ FATAL: ] Please either run with 'sudo', or switch to the root/admin." RESET_DISPLAY "\n");
        print_tail_installer();
        return -1;    
    }
#endif
    return 0;
}

int license_confirmation(void){
    char cmdline[CMDLINE_LENGTH]="";
    char confirmation[16]="";
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"curl -s %s",URL_LICENSE);
#else
    snprintf(cmdline,CMDLINE_LENGTH-1,"curl -s %s | more",URL_LICENSE);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Please read the following important information before continuing.\n");
    printf("[  ****  ] Press 'Enter' to continue reading, or press 'q' to quit reading.\n");
#endif
    if(system(cmdline)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Currently the installer failed to download or print out the license.\n");
        printf("[  ****  ] Please double check your internet connectivity and retry. If this issue\n");
        printf("[  ****  ] still occurs, please report to us via info@hpc-now.com ." RESET_DISPLAY "\n");
        return 1;
    }
    printf("\n");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " If you accept the terms above, please input " WARN_YELLO_BOLD "accept " RESET_DISPLAY ".\n");
    printf("[  ****  ] If you do not accept, this installation will exit immediately.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " Please input ( case-sensitive ): ");
    scanf("%15s",confirmation);
    fflush_stdin();
    if(strcmp(confirmation,"accept")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " This installation process is terminated because you didn't accept\n");
        printf("[  ****  ] the terms in the license.\n");
        return -1;
    }
    return 0;
}

/* 
 * Install HPC-NOW Services
 * If everything goes well, return 0; otherwise return non-zero value
 * 1. Check and add the dedicated user 'hpc-now'
 * 2. Create necessary directories, including /Applications/.hpc-now 
 * 3. Create the crypto key file for further encryption and decryption
 * 4. Manage the folder permissions
 */
int install_services(int hpcopr_loc_flag, char* hpcopr_loc, char* hpcopr_ver, char* opr_password, int crypto_loc_flag, char* now_crypto_loc, int rdp_flag){
    char cmdline1[CMDLINE_LENGTH]="";
    char cmdline2[CMDLINE_LENGTH]="";
    char random_string[PASSWORD_STRING_LENGTH]="";
    char opr_passwd_temp[PASSWORD_STRING_LENGTH]="";
    FILE* file_p=NULL;
    int run_flag1,run_flag2;
#ifdef __linux__
    char linux_packman[16]="";
#elif __APPLE__
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0,flag6=0;
#elif _WIN32
    char hpc_now_password[15]=""; /* Windows will prompt to confirm if the password string is longer than 14*/
#endif
    int preinst_check_flag=preinstall_check();
    if(preinst_check_flag!=0){
        if(preinst_check_flag==6){
            printf(FATAL_RED_BOLD "[ FATAL: ] Please build this software on Windows, macOS, or GNU/Linux." RESET_DISPLAY "\n");
        }
        else{
            printf(FATAL_RED_BOLD "[ FATAL: ] The utility '%s' is absent. Install it manually and retry." RESET_DISPLAY "\n",preinst_check_failures[preinst_check_flag-1]);
        }
        return 1;
    }
    if(installed_or_not()!=0){
#ifdef _WIN32
        printf(FATAL_RED_BOLD "[ FATAL: ] User 'hpc-now' found. The HPC-NOW has already been installed.\n");
        printf("[  ****  ] If you'd like to reinstall, please uninstall first. In order to\n");
        printf("[  ****  ] uninstall current HPC-NOW services, please:\n");
        printf("[  ****  ] 1. Run a Command Prompt Window with Administrator role\n");
        printf("[  ****  ] 2. Type the path of this installer with an option, for example\n");
        printf("[  ****  ]    C:\\Users\\ABC\\installer_windows_amd64.exe uninstall" RESET_DISPLAY "\n");
#else
        printf(FATAL_RED_BOLD "[ FATAL: ] User 'hpc-now' found. The HPC-NOW has already been installed.\n");
        printf("[  ****  ] If you'd like to reinstall, please uninstall first. In order to\n");
        printf("[  ****  ] uninstall current HPC-NOW services, please run the command:\n");
        printf("[  ****  ] sudo THIS_INSTALLER_PATH uninstall" RESET_DISPLAY "\n");
#endif
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking and cleaning up current environment ...\n");
#ifdef _WIN32
    generate_random_npasswd(hpc_now_password,15,SPECIAL_PASSWORD_CHARS,strlen(SPECIAL_PASSWORD_CHARS));
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"attrib -h -s -r %s > nul 2>&1",CRYPTO_KEY_FILE);
    system(cmdline1);
    rm_pdir(HPC_NOW_USER_DIR);
    rm_pdir(HPC_NOW_ROOT_DIR);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Adding the specific user 'hpc-now' to your OS ...\n");   
    snprintf(cmdline1,CMDLINE_LENGTH-1,"net user hpc-now \"%s\" /add > nul 2>&1",hpc_now_password);
    if(system(cmdline1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create the user 'hpc-now' to your system." RESET_DISPLAY "\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating and configuring the running directory ...\n");
    mk_pdir(HPC_NOW_USER_DIR);
    mk_pdir(NOW_BINARY_DIR);
    mk_pdir(NOW_LIC_DIR);
    mk_pdir(HPC_NOW_ROOT_DIR);
    mk_pdir(SSHKEY_DIR);
    mk_pdir(DESTROYED_DIR);
    mk_pdir(NOW_WORKDIR_ROOT);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /grant hpc-now:(OI)(CI)F /t > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /deny hpc-now:(DE) /t > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
#elif __linux__
    if(check_linux_packman(linux_packman,16)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Linux package manager not found. Please install the 'unzip' manually." RESET_DISPLAY "\n");
        return -1;
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ] Detected Linux package manager: %s\n",linux_packman);
    }
    if(system("which unzip >> /dev/null 2>&1")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Unzip not found. Install the utility 'unzip' with %s ...\n",linux_packman);
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s install unzip -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s -S unzip --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        if(system(cmdline1)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to install unzip. Please install it first." RESET_DISPLAY "\n");
            return -1;
        }
    }
    /*
     * For Linux, only check curl and unzip. Usually, the other required utilities `tar`, `ssh` and `scp` are
     * already installed. But this is just an assumption based on the experience. Maybe we need to add the 
     * check of them later. For now, it should be OK.
     */
    if(system("which curl >> /dev/null 2>&1")!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Curl not found. Install the utility 'curl' with %s ...\n",linux_packman);
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s install curl -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s -S curl --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        if(system(cmdline1)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to install curl. Please install it first." RESET_DISPLAY "\n");
            return -1;
        }
    }
    rm_pdir(HPC_NOW_USER_DIR);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chattr -i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline1);
    rm_pdir(HPC_NOW_ROOT_DIR);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Adding the specific user 'hpc-now' to your OS ...\n");
    if(strcmp(linux_packman,"zypper")==0){
        strncpy(cmdline1,"groupadd hpc-now >> /dev/null 2>&1",CMDLINE_LENGTH-1);
        /* For SUSE, we may need to add a group hpc-now first. */
        system(cmdline1);
        /* And then add the user hpc-now to the group. */
        strncpy(cmdline1,"useradd hpc-now -g hpc-now -m -s /bin/bash >> /dev/null 2>&1",CMDLINE_LENGTH-1);
    }
    else{
        /* For other distros, just add hpc-now and the group hpc-now would automatically created. */
        strncpy(cmdline1,"useradd hpc-now -m -s /bin/bash >> /dev/null 2>&1",CMDLINE_LENGTH-1);
    }
    if(system(cmdline1)!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting." RESET_DISPLAY "\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating and configuring the running directory ...\n");
    mk_pdir(NOW_BINARY_DIR);
    mk_pdir(HPC_NOW_ROOT_DIR);
    chmod(HPC_NOW_ROOT_DIR,0700);
#elif __APPLE__
    rm_pdir(HPC_NOW_USER_DIR);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chflags noschg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline1);
    rm_pdir(HPC_NOW_ROOT_DIR);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Adding the specific user 'hpc-now' to your OS ...\n");
    flag1=system("dscl . -create /Users/hpc-now >> /dev/null 2>&1");
    flag2=system("dscl . -create /Users/hpc-now UserShell /bin/bash >> /dev/null 2>&1");
    flag3=system("dscl . -create /Users/hpc-now RealName hpc-now >> /dev/null 2>&1");
    flag4=system("dscl . -create /Users/hpc-now UniqueID 1988 >> /dev/null 2>&1");
    flag5=system("dscl . -create /Groups/hpc-now PrimaryGroupID 1988 >> /dev/null 2>&1");
    flag5=system("dscl . -create /Users/hpc-now PrimaryGroupID 1988 >> /dev/null 2>&1");
    flag6=system("dscl . -create /Users/hpc-now NFSHomeDirectory /Users/hpc-now >> /dev/null 2>&1");
    if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0||flag6!=0){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create the user 'hpc-now' and directories." RESET_DISPLAY "\n");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating and configuring the running directory ...\n");
    mk_pdir(HPC_NOW_USER_DIR);
    mk_pdir(NOW_BINARY_DIR);
    mk_pdir(HPC_NOW_ROOT_DIR);
    chmod(HPC_NOW_ROOT_DIR,0700);
#endif
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating a file for encryption/decryption ...\n");
    generate_random_npasswd(random_string,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS,strlen(SPECIAL_PASSWORD_CHARS));
    
    if(strlen(opr_password)==0){
        getpass_stdin("[ INPUT: ] Specify an operator keystring (length < 20): ",opr_passwd_temp,20);
    }
    else{
        strncpy(opr_passwd_temp,opr_password,PASSWORD_STRING_LENGTH-1);
    }

    if(password_complexity_check(opr_passwd_temp,SPECIAL_PASSWORD_CHARS)!=0){
        generate_random_npasswd(opr_passwd_temp,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS,strlen(SPECIAL_PASSWORD_CHARS));
        printf(WARN_YELLO_BOLD "\n[ -WARN- ] The keystring is invalid. Generated: " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",opr_passwd_temp);
    }
    else{
        printf(GENERAL_BOLD "\n[ -INFO- ] Specified keystring: " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",opr_passwd_temp);
    }

    file_p=fopen(CRYPTO_KEY_FILE,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a key file. Exit now." RESET_DISPLAY "\n");
        return -1;
    }
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"SALT_STRING: %s\nUSER_STRING: %s\n\nEND OF LOCKED NOW CRYPTO KEY FILE\n",random_string,opr_passwd_temp);
    fclose(file_p);
#ifdef _WIN32
    snprintf(cmdline1,CMDLINE_LENGTH-1,"attrib +h +s +r %s",CRYPTO_KEY_FILE);
#elif __linux__
    chown(CRYPTO_KEY_FILE,0,0);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chattr +i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
#elif __APPLE__
    chown(CRYPTO_KEY_FILE,0,0);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chflags schg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
#endif
    system(cmdline1);

    if(hpcopr_loc_flag==-1){
        if(strlen(hpcopr_ver)==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the latest 'hpcopr' from the default URL.\n");
            snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %shpcopr-%s-latest.exe -o %s",DEFAULT_URL_HPCOPR_LATEST,FILENAME_SUFFIX_SHORT,HPCOPR_EXEC);
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the %s version 'hpcopr' from the default repo.\n",hpcopr_ver);
            printf(WARN_YELLO_BOLD "[ -INFO- ] MAY FAIL IF %s IS NOT A VALID VERSION CODE!\n" RESET_DISPLAY,hpcopr_ver);
            snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %shpcopr-%s-%s.exe -o %s",DEFAULT_URL_HPCOPR_LATEST,FILENAME_SUFFIX_SHORT,hpcopr_ver,HPCOPR_EXEC);
        }
    }
    else if(hpcopr_loc_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the main program 'hpcopr' from the specified URL:\n");
        printf("[  ****  ] -> %s\n",hpcopr_loc);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %s -o %s",hpcopr_loc,HPCOPR_EXEC);
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will copy the main program 'hpcopr' from local place:\n");
        printf("[  ****  ] -> %s\n",hpcopr_loc);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"%s %s %s %s ",COPY_FILE_CMD,hpcopr_loc,HPCOPR_EXEC,SYSTEM_CMD_REDIRECT_NULL);
    }
    if(crypto_loc_flag==-1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the component 'now-crypto' from the default URL.\n");
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %snow-crypto-aes-%s.exe -o %s",DEFAULT_URL_NOW_CRYPTO,FILENAME_SUFFIX_SHORT,NOW_CRYPTO_EXEC);
    }
    else if(crypto_loc_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the component 'now-crypto' from the specified URL:\n");
        printf("[  ****  ] -> %s\n",now_crypto_loc);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %s -o %s",now_crypto_loc,NOW_CRYPTO_EXEC);
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will copy the component 'now-crypto' from local place:\n");
        printf("[  ****  ] -> %s\n",now_crypto_loc);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"%s %s %s %s",COPY_FILE_CMD,now_crypto_loc,NOW_CRYPTO_EXEC,SYSTEM_CMD_REDIRECT_NULL);
    }
    run_flag1=system(cmdline1);
    run_flag2=system(cmdline2);
#ifdef _WIN32
    if(run_flag1!=0||run_flag2!=0){
        if(run_flag1!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'hpcopr'." RESET_DISPLAY "\n");
        }
        if(run_flag2!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'now-crypto'." RESET_DISPLAY "\n");
        }
        printf(FATAL_RED_BOLD "[ FATAL: ] This installation process is terminated. If you specified the\n");
        printf("[  ****  ] location of hpcopr executable, please make sure the location \n");
        printf("[  ****  ] is correct. Roll back and exit." RESET_DISPLAY "\n");
        snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_USER_DIR);
        system(cmdline1);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_USER_DIR);
        system(cmdline1);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_ROOT_DIR);
        system(cmdline1);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_ROOT_DIR);
        system(cmdline1);
        rm_pdir(HPC_NOW_USER_DIR);
        rm_pdir(HPC_NOW_ROOT_DIR);
        system("net user hpc-now /delete > nul 2>&1");
        return -1;
    }
    snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /grant hpc-now:F /t > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F /t > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline1);
    
    if(system("set PATH | findstr C:\\hpc-now >nul 2>&1")!=0){
        snprintf(cmdline1,CMDLINE_LENGTH-1,"setx PATH \"%%PATH%%;C:\\hpc-now\" /m >nul 2>&1");
        system(cmdline1);
    }
    if(file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws.exe")!=0||file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws_completer.exe")!=0){
        system("start /b msiexec.exe /i https://awscli.amazonaws.com/AWSCLIV2.msi /qn");
    }
    int i=0;
    while(file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws.exe")!=0||file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws_completer.exe")!=0){
        printf(GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " Installing additional component, %d sec(s) of max %ds passed ... \r",i,AWSCLI_INST_WAIT_TIME);
        fflush(stdout);
        i++;
        sleep_func(1);
        if(i==AWSCLI_INST_WAIT_TIME){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to install component. HPC-NOW dataman services may not work properly.");
            break;
        }
    }
    printf(RESET_DISPLAY "\n");
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Congrats! The HPC-NOW services are ready to run!\n");
    printf("[  ****  ] Created the user 'hpc-now' with password: " GREY_LIGHT "%s" RESET_DISPLAY "\n",hpc_now_password);
    printf("[  ****  ] Please follow the steps below:\n");
    printf("[  ****  ] 1. " HIGH_GREEN_BOLD "net user hpc-now YOUR_COMPLEX_PASSWORD" RESET_DISPLAY " [optional]\n");
    printf("[  ****  ] 2. " HIGH_GREEN_BOLD "runas /savecred /user:mymachine\\hpc-now cmd" RESET_DISPLAY "\n");
    printf("[  ****  ] 3. " GENERAL_BOLD "In the new CMD window" RESET_DISPLAY ", run " HIGH_GREEN_BOLD "hpcopr envcheck" RESET_DISPLAY "\n");
    printf(GENERAL_BOLD"[ -DONE- ] Enjoy you Cloud HPC journey!" RESET_DISPLAY "\n");
    return 0;
#elif __linux__
    if(run_flag1!=0||run_flag2!=0){
        if(run_flag1!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'hpcopr'." RESET_DISPLAY "\n");
        }
        if(run_flag2!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'now-crypto'." RESET_DISPLAY "\n");
        }
        printf(FATAL_RED_BOLD "[ FATAL: ] This installation process is terminated. If you specified the\n");
        printf("[  ****  ] location of hpcopr executable, please make sure the location \n");
        printf("[  ****  ] is correct. Rolling back and exit." RESET_DISPLAY "\n");
        snprintf(cmdline1,CMDLINE_LENGTH-1,"chattr -i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
        system(cmdline1);
        rm_pdir(HPC_NOW_ROOT_DIR);
        system("userdel -f -r hpc-now >> /dev/null 2>&1");
        system("groupdel hpc-now >> /dev/null 2>&1");
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Setting up environment variables for 'hpc-now' ...\n"); 
    if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ >> /dev/null 2>&1")!=0){
        strncpy(cmdline1,"echo \"export PATH=/home/hpc-now/.bin/:$PATH\" >> /home/hpc-now/.bashrc",CMDLINE_LENGTH-1);
        system(cmdline1);
    }
    chmod(HPCOPR_EXEC,S_IRWXO|S_IXGRP|S_IXOTH);
    chmod(NOW_CRYPTO_EXEC,S_IRWXO|S_IXGRP|S_IXOTH);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating other key running directories ...\n");
    mk_pdir(NOW_LIC_DIR);
    mk_pdir(SSHKEY_DIR);
    mk_pdir(NOW_WORKDIR_ROOT);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
    system(cmdline1);
    system("mkdir -p /usr/share/terraform >> /dev/null 2>&1 && chmod -R 755 /usr/share/terraform >> /dev/null 2>&1 && chown -R hpc-now:hpc-now /usr/share/terraform >> /dev/null 2>&1");
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chmod 711 %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chmod -R 711 %s%s.bin",HPC_NOW_USER_DIR,PATH_SLASH);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"ln -s %s /usr/local/bin/hpcopr >> /dev/null 2>&1",HPCOPR_EXEC);
    system(cmdline1);
    
    /* Allow only local:hpc-now group access the x server. */
    system("sed -i '/xhost + >> \\/dev\\/null 2>&1 # Added by HPC-NOW/d' /etc/profile >> /dev/null 2>&1");
    if(system("grep -w \"xhost + local:hpc-now >> /dev/null 2>&1 # Added by HPC-NOW\" /etc/profile >> /dev/null 2>&1")!=0){
        system("echo \"xhost + local:hpc-now >> /dev/null 2>&1 # Added by HPC-NOW\" >> /etc/profile");
    }
    if(system("which xclip >> /dev/null 2>&1")!=0){
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s install xclip -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s -S xclip --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        system(cmdline1);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking Remmina (the RDP client for GNU/Linux) now ...\n");
    if(system("which remmina >> /dev/null 2>&1")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remmina has been installed to your OS.\n");
        goto linux_install_done;
    }
    if(rdp_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Installing Remmina: the RDP client for GNU/Linux now ...\n");
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s install remmina -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline1,CMDLINE_LENGTH-1,"%s -S remmina --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        if(system(cmdline1)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to install Remmina, RDP won't work properly." RESET_DISPLAY "\n");
        }
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remmina is absent. Please update with --rdp to install it later.\n");
    }
linux_install_done:
    printf(GENERAL_BOLD "\n[ -INFO- ]" RESET_DISPLAY " Congrats! The HPC-NOW services are ready to run!\n");
    printf("[  ****  ] Created the user 'hpc-now' " WARN_YELLO_BOLD "WITHOUT" RESET_DISPLAY " an initial password.\n\n");
    printf("[  ****  ] Please follow the steps below:\n");
    printf(GENERAL_BOLD "[  ****  ] + SUDO-MODE (simple and fast for *sudoers*): \n" RESET_DISPLAY );
    printf("[  ****  ] " HIGH_GREEN_BOLD "  sudo -Hu hpc-now hpcopr envcheck" RESET_DISPLAY "\n");
    printf("[  ****  ]     * You need to input the password of the current sudoer.\n");
    printf(GENERAL_BOLD "[  ****  ] + USER-MODE (a little bit more steps): " RESET_DISPLAY "\n");
    printf("[  ****  ]   1. " HIGH_GREEN_BOLD "sudo passwd hpc-now" RESET_DISPLAY "  * Set a password for 'hpc-now'.\n");
    printf("[  ****  ]   2. " HIGH_GREEN_BOLD "su hpc-now" RESET_DISPLAY "           * Input the password just set.\n");
    printf("[  ****  ]   3. " HIGH_GREEN_BOLD "hpcopr envcheck -b" RESET_DISPLAY "   * Check and init the environment.\n");
    printf(GENERAL_BOLD"[ -DONE- ] Enjoy you Cloud HPC journey!" RESET_DISPLAY "\n");
    return 0;
#elif __APPLE__
    if(run_flag1!=0||run_flag2!=0){
        if(run_flag1!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'hpcopr'." RESET_DISPLAY "\n");
        }
        if(run_flag2!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'now-crypto'." RESET_DISPLAY "\n");
        }
        printf(FATAL_RED_BOLD "[ FATAL: ] This installation process is terminated. If you specified the\n");
        printf("[  ****  ] location of hpcopr executable, please make sure the location \n");
        printf("[  ****  ] is correct. Rolling back and exit." RESET_DISPLAY "\n");
        snprintf(cmdline1,CMDLINE_LENGTH-1,"chflags noschg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
        system(cmdline1);
        rm_pdir(HPC_NOW_ROOT_DIR);
        system("dscl . -delete /Users/hpc-now >> /dev/null 2>&1");
        system("dscl . -delete /Groups/hpc-now >> /dev/null 2>&1");
        rm_pdir(HPC_NOW_USER_DIR);
        return -1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Setting up environment variables for 'hpc-now' ...\n");
    strncpy(cmdline1,"echo \"export PATH=/Users/hpc-now/.bin/:$PATH\" >> /Users/hpc-now/.bashrc",CMDLINE_LENGTH-1);
    system(cmdline1);
    chmod(HPCOPR_EXEC,S_IRWXO|S_IXGRP|S_IXOTH);
    chmod(NOW_CRYPTO_EXEC,S_IRWXO|S_IXGRP|S_IXOTH);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Creating other key running directories ...\n");
    mk_pdir(NOW_LIC_DIR);
    mk_pdir(SSHKEY_DIR);
    mk_pdir(NOW_WORKDIR_ROOT);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
    system(cmdline1);
    system("mkdir -p '/Library/Application Support/io.terraform' >> /dev/null 2>&1 && chmod -R 755 '/Library/Application Support/io.terraform' >> /dev/null 2>&1 && chown -R hpc-now:hpc-now '/Library/Application Support/io.terraform' >> /dev/null 2>&1");
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chmod 711 %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chmod -R 711 %s%s.bin >> /dev/null 2>&1",HPC_NOW_USER_DIR,PATH_SLASH);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline1);
    snprintf(cmdline1,CMDLINE_LENGTH-1,"mkdir -p /usr/local/bin && ln -s %s /usr/local/bin/hpcopr >> /dev/null 2>&1",HPCOPR_EXEC);
    system(cmdline1);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking the Microsoft RDP Client now ...\n");
    if(folder_exist_or_not("/Applications/msrdp.app")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Microsoft RDP has been installed to your OS.\n");
        goto mac_install_done;
    }
    if(rdp_flag!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The Microsoft RDP Client is absent. Please update with --rdp to install.\n");
        goto mac_install_done;
    }
    if(file_exist_or_not("/Users/Shared/rdp_for_mac.zip")!=0){
        snprintf(cmdline1,CMDLINE_LENGTH-1,"mkdir -p /Users/Shared/ && curl %s -o /Users/Shared/rdp_for_mac.zip",URL_MSRDP_FOR_MAC);
        if(system(cmdline1)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download the package, RDP won't work properly." RESET_DISPLAY "\n");
            goto mac_install_done;
        }
    }
    if(system("unzip -q /Users/Shared/rdp_for_mac.zip -d /Applications/ && mv '/Applications/Microsoft Remote Desktop Beta.app' /Applications/msrdp.app")!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to unzip the package, RDP won't work properly." RESET_DISPLAY "\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The Microsoft RDP Client has been installed.\n");
    }
mac_install_done:
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Congrats! The HPC-NOW services are ready to run!\n");
    printf("[  ****  ] Created the user 'hpc-now' " WARN_YELLO_BOLD "WITHOUT" RESET_DISPLAY " an initial password.\n\n");
    printf("[  ****  ] Please follow the steps below:\n");
    printf(GENERAL_BOLD "[  ****  ] + SUDO-MODE (simple and fast for *sudoers*): \n" RESET_DISPLAY );
    printf("[  ****  ] " HIGH_GREEN_BOLD "  cd /Applications && sudo -Hu hpc-now hpcopr envcheck" RESET_DISPLAY "\n");
    printf("[  ****  ]     * You will need to input the password for the current sudoer.\n");
    printf(GENERAL_BOLD "[  ****  ] + USER-MODE (a little bit more steps): " RESET_DISPLAY "\n");
    printf("[  ****  ]   1. " HIGH_GREEN_BOLD "sudo dscl . -passwd /Users/hpc-now YOUR_COMPLEX_PASSWORD" RESET_DISPLAY "\n");
    printf("[  ****  ]   2. " HIGH_GREEN_BOLD "su hpc-now" RESET_DISPLAY "                * Input the password just set.\n");
    printf("[  ****  ]   3. " HIGH_GREEN_BOLD "hpcopr envcheck -b" RESET_DISPLAY "        * Check and init the environment.\n");
    printf(GENERAL_BOLD"[ -DONE- ] Enjoy you Cloud HPC journey!" RESET_DISPLAY "\n");
    return 0;    
#endif
}

int restore_perm(void){
    char cmdline[CMDLINE_LENGTH]="";
    int cmd_flag=0;
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /grant hpc-now:F /t > nul 2>&1",HPC_NOW_ROOT_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F /t > nul 2>&1",HPC_NOW_ROOT_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_ROOT_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
#else
    snprintf(cmdline,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s",SSHKEY_DIR);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
    snprintf(cmdline,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s",NOW_WORKDIR_ROOT);
    if(system(cmdline)!=0){
        cmd_flag++;
    }
#endif
    return cmd_flag;
}

/*
 * If the opr_password is not 0, the password is valid.
 * return -5: Failed to restore permission for windows
 * return -3: Not installed
 * return -1: FILE input error
 * return 1: password invalid
 * return 3: failed to decrypt
 * return 5: failed to encrypt
 * return 0: normal exit
 */
int set_opr_password(char* opr_password){
    char random_string[PASSWORD_STRING_LENGTH]="";
    char opr_passwd_temp[PASSWORD_STRING_LENGTH]="";
    FILE* file_p=NULL;
    int run_flag;
    char cmdline[CMDLINE_LENGTH]="";

    if(installed_or_not()==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] User 'hpc-now' not found. It seems HPC-NOW has not been installed.\n");
        printf("[  ****  ] Please install it first in order to update." RESET_DISPLAY "\n");
        return -3;
    }

    /* The keystring's complexity has been checked before this function */
    if(strlen(opr_password)==0){
        getpass_stdin("[ INPUT: ] Specify a keystring (length < 20): ",opr_passwd_temp,20);
        if(password_complexity_check(opr_passwd_temp,SPECIAL_PASSWORD_CHARS)!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] The keystring " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY FATAL_RED_BOLD " is invalid." RESET_DISPLAY "\n",opr_passwd_temp);
            return 1;
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ] Specified keystring: " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",opr_passwd_temp);
        }
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ] Specified keystring: " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY "\n",opr_password);
        strncpy(opr_passwd_temp,opr_password,19);
    }
    printf(GENERAL_BOLD "\n[ STEP 1 ] Decrypting current files with previous crypto keystring..." RESET_DISPLAY "\n");
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s* /grant Administrators:F /t > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /grant Administrators:F > nul 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
#endif
    run_flag=encrypt_decrypt_clusters("all",CRYPTO_KEY_FILE,"decrypt",0);
    if(run_flag!=0){
        if(run_flag>20||run_flag==-7){
            printf(WARN_YELLO_BOLD "\n[ -WARN- ] Rolling back with previous crypto keystring ..." RESET_DISPLAY "\n");
            encrypt_decrypt_clusters("all",CRYPTO_KEY_FILE,"encrypt",0);
        }
        printf(FATAL_RED_BOLD "\n[ FATAL: ] Operation failed and keystring unchanged." RESET_DISPLAY "\n");
        restore_perm();
        return 3;
    }
    generate_random_npasswd(random_string,PASSWORD_STRING_LENGTH,SPECIAL_PASSWORD_CHARS,strlen(SPECIAL_PASSWORD_CHARS));

#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"attrib -h -s -r %s > nul 2>&1",CRYPTO_KEY_FILE);
#elif __linux__
    snprintf(cmdline,CMDLINE_LENGTH-1,"chattr -i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
#else
    snprintf(cmdline,CMDLINE_LENGTH-1,"chflags noschg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
#endif
    system(cmdline);

    file_p=fopen(CRYPTO_KEY_FILE,"w+");
    if(file_p==NULL){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create the now_crypto_seed.lock file." RESET_DISPLAY "\n");
        restore_perm();
        return -1;
    }
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"SALT_STRING: %s\nUSER_STRING: %s\n\nEND OF LOCKED NOW CRYPTO KEY FILE\n",random_string,opr_passwd_temp);
    fclose(file_p);
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"attrib +h +s +r %s > nul 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
#else
    chown(CRYPTO_KEY_FILE,0,0);
#endif

#ifdef __linux__
    snprintf(cmdline,CMDLINE_LENGTH-1,"chattr +i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
#elif __APPLE__
    snprintf(cmdline,CMDLINE_LENGTH-1,"chflags schg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
#endif
    printf(GENERAL_BOLD "\n[ STEP 2 ] Encrypting files with the new crypto keystring..." RESET_DISPLAY "\n");
    run_flag=encrypt_decrypt_clusters("all",CRYPTO_KEY_FILE,"encrypt",0);
    if(run_flag!=0&&run_flag!=-1&&run_flag!=-11){
        printf(FATAL_RED_BOLD "[ FATAL: ] Failed to encrypt files with new crypto keystring." RESET_DISPLAY "\n");
        restore_perm();
        return 5;
    }
    restore_perm();
    printf( GENERAL_BOLD "\n[ -DONE- ] The operator keystring has been updated." RESET_DISPLAY "\n");
    return 0;
}

/*
 * Forcely uninstall the HPC-NOW services
 */
int uninstall_services(void){
    char doubleconfirm[16]="";
    char cmdline[CMDLINE_LENGTH]="";
    if(installed_or_not()==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] User 'hpc-now' not found. There is nothing to uninstall." RESET_DISPLAY "\n");
        return 1;
    }
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",NOW_WORKDIR_ROOT);
    system(cmdline);
#endif
    if(folder_check_general(NOW_WORKDIR_ROOT,0)==0&&folder_empty_or_not(NOW_WORKDIR_ROOT)!=0){
        restore_perm();
        printf(FATAL_RED_BOLD "[ FATAL: ] The workdir is not empty. Please switch to user 'hpc-now' and check:" RESET_DISPLAY "\n");
        printf(FATAL_RED_BOLD "[  ****  ] -> %s" RESET_DISPLAY "\n",NOW_WORKDIR_ROOT);
        return 5;
    }
    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("[  ****  ] YOU ARE UNINSTALLING THE HPC-NOW SERVICES, PLEASE CONFIRM:\n\n");
    printf("[  ****  ] 1. You have *DESTROYED* all the clusters managed by this device.\n");
    printf("[  ****  ] 2. You have *CHECKED* your cloud account(s) and no resource left.\n");
    printf("[  ****  ] 3. You have *EXPORTED* the relevant logs to a permenant directory.\n\n");
    printf("[  ****  ] THIS OPERATION IS UNRECOVERABLE!" RESET_DISPLAY "\n");
    printf("[ -INFO- ] ARE YOU SURE? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm.\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    scanf("%15s",doubleconfirm);
    fflush_stdin();
    if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. You denied the operation." RESET_DISPLAY "\n");
        return 3;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " UNINSTALLING THE SERVICES AND REMOVING THE DATA NOW ...\n");
#ifdef _WIN32
    char randstr[8]="";
    char tasklist_temp[FILENAME_LENGTH]="";
    char unins_trash_dir[DIR_LENGTH]="";
    generate_random_nstring(randstr,8,1);
    snprintf(tasklist_temp,FILENAME_LENGTH-1,"C:\\ProgramData\\hpc-now-tasks.%s.temp",randstr);
    snprintf(cmdline,CMDLINE_LENGTH-1,"tasklist /FI \"USERNAME eq hpc-now\" > %s 2>nul",tasklist_temp);
    system(cmdline);
    FILE* file_p=fopen(tasklist_temp,"r");
    char line_buffer[LINE_LENGTH_SHORT]="";
    char pid[16]="";
    if(file_p==NULL){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to creat/get the tasklist of hpc-now." RESET_DISPLAY "\n");
    }
    else{
        fngetline(file_p,line_buffer,LINE_LENGTH_SHORT);
        fngetline(file_p,line_buffer,LINE_LENGTH_SHORT);
        fngetline(file_p,line_buffer,LINE_LENGTH_SHORT);
        while(!feof(file_p)){
            fngetline(file_p,line_buffer,LINE_LENGTH_SHORT);
            get_seq_nstring(line_buffer,' ',2,pid,16);
            snprintf(cmdline,CMDLINE_LENGTH-1,"taskkill /pid %s > nul 2>&1",pid);
            system(cmdline);
        }
        fclose(file_p);
        rm_file_or_dir(tasklist_temp);
    }

    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s* /grant Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline);

    snprintf(cmdline,CMDLINE_LENGTH-1,"takeown /f %s /r /d y > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s* /grant Administrators:F > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"icacls %s /grant Administrators:F > nul 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
    snprintf(cmdline,CMDLINE_LENGTH-1,"attrib -h -s -r %s > nul 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
    system("net user hpc-now /delete > nul 2>&1");
    rm_pdir(HPC_NOW_USER_DIR);
    rm_pdir(HPC_NOW_ROOT_DIR);
    snprintf(unins_trash_dir,DIR_LENGTH-1,"C:\\Users\\hpc-now.user.unins.trash\\%s",randstr);
    snprintf(cmdline,CMDLINE_LENGTH-1,"move /y C:\\Users\\hpc-now %s > nul 2>&1",unins_trash_dir);
    int move_flag=system(cmdline);
#elif __APPLE__
    system("ps -ax | grep hpc-now | cut -c 1-6 | xargs kill -9 >> /dev/null 2>&1");
    system("unlink /usr/local/bin/hpcopr >> /dev/null 2>&1");
    snprintf(cmdline,CMDLINE_LENGTH-1,"chflags noschg %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
    rm_pdir(HPC_NOW_ROOT_DIR);
    system("dscl . -delete /Users/hpc-now >> /dev/null 2>&1");
    system("dscl . -delete /Groups/hpc-now >> /dev/null 2>&1");
    rm_pdir(HPC_NOW_USER_DIR);
#else
    system("ps -aux | grep hpc-now | cut -c 9-16 | xargs kill -9 >> /dev/null 2>&1");
    system("unlink /usr/local/bin/hpcopr >> /dev/null 2>&1");
    snprintf(cmdline,CMDLINE_LENGTH-1,"chattr -i %s >> /dev/null 2>&1",CRYPTO_KEY_FILE);
    system(cmdline);
    rm_pdir(HPC_NOW_ROOT_DIR);
    system("userdel -f -r hpc-now >> /dev/null 2>&1");
    system("groupdel hpc-now >> /dev/null 2>&1");
#endif

    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The HPC-NOW cluster services have been deleted.\n");
#ifdef _WIN32
    printf("[  ****  ] There might still be remaining files for the user 'hpc-now' in:\n");
    if(move_flag==0){
        printf("[  ****  ] -> %s\n",unins_trash_dir);
    }
    printf("[  ****  ] -> %s\n",TF_LOCAL_PLUGINS);
#elif __linux__
    if(system("cat /etc/profile | grep -w \"# Added by HPC-NOW\" >> /dev/null 2>&1")==0){
        system("sed -i '/# Added by HPC-NOW/d' /etc/profile");
    }
    printf("[  ****  ] There are still remaining files for reinstall. You can run:\n");
    printf("[  ****  ] " HIGH_GREEN_BOLD "sudo rm -rf /usr/share/terraform" RESET_DISPLAY " to erase them.\n");
#else
    printf("[  ****  ] There are still remaining files for reinstall. You can run:\n");
    printf("[  ****  ] " HIGH_GREEN_BOLD "sudo rm -rf '/Library/Application Support/io.terraform'" RESET_DISPLAY " to erase them.\n");
#endif
    return 0;
}

int update_services(int hpcopr_loc_flag, char* hpcopr_loc, char* hpcopr_ver, int crypto_loc_flag, char* now_crypto_loc, int rdp_flag){
    char doubleconfirm[16]="";
    char cmdline_temp[CMDLINE_LENGTH]="";
    char cmdline1[CMDLINE_LENGTH]="";
    char cmdline2[CMDLINE_LENGTH]="";
    char cmdline_dec[CMDLINE_LENGTH]="";
    char cmdline_enc[CMDLINE_LENGTH]="";
#ifdef __linux__
    char linux_packman[16]="";
#endif
    int run_flag1,run_flag2,decrypt_flag=0;
    
#ifdef _WIN32
    char randstr[8]="";
    char filename_temp[FILENAME_LENGTH]="";
    int i;
    generate_random_nstring(randstr,8,1);
#endif

    if(installed_or_not()==0){
        printf(FATAL_RED_BOLD "[ FATAL: ] User 'hpc-now' not found. It seems HPC-NOW has not been installed.\n");
        printf("[  ****  ] Please install it first in order to update." RESET_DISPLAY "\n");
        return 1;
    }

    printf(WARN_YELLO_BOLD "[ -WARN- ] C A U T I O N !\n");
    printf("[  ****  ] YOU ARE UPDATING THE HPC-NOW SERVICES. THE hpcopr executable WILL\n");
    printf("[  ****  ] BE REPLACED. IF YOU UPDATE WITH THE --hloc AND/OR --cloc OPTIONS,\n");
    printf("[  ****  ] PLEASE MAKE SURE THEY ARE VALID." RESET_DISPLAY "\n");
    printf("[ -INFO- ] ARE YOU SURE? Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm:\n");
    printf(GENERAL_BOLD "[ INPUT: ]" RESET_DISPLAY " ");
    scanf("%15s",doubleconfirm);
    fflush_stdin();
    if(strcmp(doubleconfirm,CONFIRM_STRING)!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Only " WARN_YELLO_BOLD CONFIRM_STRING RESET_DISPLAY " is accepted to confirm. You denied the operation." RESET_DISPLAY "\n");
        return 1;
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " UPDATING THE SERVICES NOW ...\n");
#ifdef _WIN32
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"takeown /f %s > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /remove Administrators > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"dir %s | findstr .now-ssh > nul 2>&1",HPC_NOW_ROOT_DIR);
    if(system(cmdline_temp)!=0){ /* For compatibility */
        printf("[ -INFO- ] Moving previous keys to the new directory ...\n");
        system("takeown /f C:\\hpc-now\\.now-ssh /r /d y > nul 2>&1");
        system("move /y C:\\hpc-now\\.now-ssh C:\\ProgramData\\hpc-now\\ > nul 2>&1");
        system("icacls C:\\ProgramData\\hpc-now\\.now-ssh /grant hpc-now:F /t > nul 2>&1");
    }

    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"dir %s | findstr utils > nul 2>&1",HPC_NOW_USER_DIR);
    if(system(cmdline_temp)!=0){
        printf("[ -INFO- ] Moving previous utilities to the new directory ...\n");
        system("takeown /f C:\\ProgramData\\hpc-now\\bin /r /d y > nul 2>&1");
        system("move /y C:\\ProgramData\\hpc-now\\bin C:\\hpc-now\\utils > nul 2>&1");
        system("icacls C:\\hpc-now\\utils /grant hpc-now:F /t > nul 2>&1");
    }
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"takeown /f %s /d y > nul 2>&1",HPCOPR_EXEC);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /grant Administrators:F > nul 2>&1",HPCOPR_EXEC);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"takeown /f %s /d y > nul 2>&1",NOW_CRYPTO_EXEC);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /grant Administrators:F > nul 2>&1",NOW_CRYPTO_EXEC);
    system(cmdline_temp);
    
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"%s version | findstr \"/C:Version: 0.3.1.00\" > nul 2>&1",HPCOPR_EXEC);
    if(system(cmdline_temp)==0){
        decrypt_flag=1;
        snprintf(cmdline_enc,CMDLINE_LENGTH-1,"runas /savecred /user:mymachine\\hpc-now \"cmd.exe /c hpcopr encrypt --all -b > C:\\ProgramData\\hpc-now\\enc.%s.temp 2>&1\" > nul",randstr);
        snprintf(cmdline_dec,CMDLINE_LENGTH-1,"runas /savecred /user:mymachine\\hpc-now \"cmd.exe /c hpcopr decrypt --all -b > C:\\ProgramData\\hpc-now\\dec.%s.temp 2>&1\" > nul",randstr);
    }
#elif __linux__
    if(system("ls -la /home/hpc-now/.bin | grep utils >> /dev/null 2>&1")!=0){
        printf("[ -INFO- ] Moving previous utilities to the new directory ...\n");
        system("mv /usr/.hpc-now/.bin /home/hpc-now/.bin/utils >> /dev/null 2>&1");
        system("chown -R hpc-now:hpc-now /home/hpc-now/.bin/utils >> /dev/null 2>&1");
    }
    if(system("ls -la /usr/.hpc-now | grep .now-ssh >> /dev/null 2>&1")!=0){
        printf("[ -INFO- ] Moving previous keys to the new directory ...\n");
        system("mv /home/hpc-now/.now-ssh /usr/.hpc-now/ >> /dev/null 2>&1");
        system("chown -R hpc-now:hpc-now /usr/.hpc-now/.now-ssh >> /dev/null 2>&1");
    }
    if(system("hpcopr version | grep \"Version: 0.3.1.00\" >> /dev/null 2>&1")==0){
        decrypt_flag=1;
        strncpy(cmdline_enc,"sudo -Hu hpc-now hpcopr encrypt --all -b > /dev/null",CMDLINE_LENGTH-1);
        strncpy(cmdline_dec,"sudo -Hu hpc-now hpcopr decrypt --all -b > /dev/null",CMDLINE_LENGTH-1);
    }
#elif __APPLE__
    if(system("ls -la /Users/hpc-now/.bin | grep utils >> /dev/null 2>&1")!=0){
        printf("[ -INFO- ] Moving previous utilities to the new directory ...\n");
        system("mv /Applications/.hpc-now/.bin /Users/hpc-now/.bin/utils >> /dev/null 2>&1");
        system("chmod -R 711 /Users/hpc-now/.bin >> /dev/null 2>&1");
        system("chown -R hpc-now:hpc-now /Users/hpc-now/.bin/utils >> /dev/null 2>&1");
    }
    if(system("ls -la /Applications/.hpc-now | grep .now-ssh >> /dev/null 2>&1")!=0){
        printf("[ -INFO- ] Moving previous keys to the new directory ...\n");
        system("mv /Users/hpc-now/.now-ssh /Applications/.hpc-now/ >> /dev/null 2>&1");
        system("chown -R hpc-now:hpc-now /Applications/.hpc-now/.now-ssh >> /dev/null 2>&1");
    }
    if(system("hpcopr version | grep \"Version: 0.3.1.00\" >> /dev/null 2>&1")==0){
        decrypt_flag=1;
        strncpy(cmdline_enc,"sudo -Hu hpc-now hpcopr encrypt --all -b > /dev/null",CMDLINE_LENGTH-1);
        strncpy(cmdline_dec,"sudo -Hu hpc-now hpcopr decrypt --all -b > /dev/null",CMDLINE_LENGTH-1);
    }
#endif
    if(decrypt_flag==1){
        printf(WARN_YELLO_BOLD "[ -WARN- ] This update needs to re-encrypt the files." RESET_DISPLAY "\n");
        system(cmdline_enc);
#ifdef _WIN32
        i=0;
        snprintf(cmdline1,CMDLINE_LENGTH-1,"findstr \"/C:info@hpc-now.com\" C:\\ProgramData\\hpc-now\\enc.%s.temp > nul 2>&1",randstr);
        while(system(cmdline1)!=0){
            i++;
            printf("[ -WAIT- ] Encryption in progress, %d seconds passed ... \r",i);
            fflush(stdout);
            sleep_func(1);
        }
        printf("\n");
#endif
        system(cmdline_dec);
#ifdef _WIN32
        i=0;
        snprintf(cmdline1,CMDLINE_LENGTH-1,"findstr \"/C:info@hpc-now.com\" C:\\ProgramData\\hpc-now\\dec.%s.temp > nul 2>&1",randstr);
        while(system(cmdline1)!=0){
            i++;
            printf("[ -WAIT- ] Decryption in progress, %d seconds passed ... \r",i);
            fflush(stdout);
            sleep_func(1);
        }
        printf("\n");
#endif
    }
    if(hpcopr_loc_flag==-1){
        if(strlen(hpcopr_ver)==0){
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the latest 'hpcopr' from the default URL.\n");
            snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %shpcopr-%s-latest.exe -o %s",DEFAULT_URL_HPCOPR_LATEST,FILENAME_SUFFIX_SHORT,HPCOPR_EXEC);
        }
        else{
            printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the %s version 'hpcopr' from the default repo.\n",hpcopr_ver);
            printf(WARN_YELLO_BOLD "[ -INFO- ] MAY FAIL IF %s IS NOT A VALID VERSION CODE!\n" RESET_DISPLAY,hpcopr_ver);
            snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %shpcopr-%s-%s.exe -o %s",DEFAULT_URL_HPCOPR_LATEST,FILENAME_SUFFIX_SHORT,hpcopr_ver,HPCOPR_EXEC);
        }
    }
    else if(hpcopr_loc_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the main program 'hpcopr' from the specified URL:\n");
        printf("[  ****  ] -> %s\n",hpcopr_loc);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"curl -s %s -o %s",hpcopr_loc,HPCOPR_EXEC);
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will copy the main program 'hpcopr' from local place:\n");
        printf("[  ****  ] -> %s\n",hpcopr_loc);
        snprintf(cmdline1,CMDLINE_LENGTH-1,"%s %s %s %s ",COPY_FILE_CMD,hpcopr_loc,HPCOPR_EXEC,SYSTEM_CMD_REDIRECT_NULL);
    }
    if(crypto_loc_flag==-1){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the component 'now-crypto' from the default URL.\n");
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %snow-crypto-aes-%s.exe -o %s",DEFAULT_URL_NOW_CRYPTO,FILENAME_SUFFIX_SHORT,NOW_CRYPTO_EXEC);
    }
    else if(crypto_loc_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will download the component 'now-crypto' from the specified URL:\n");
        printf("[  ****  ] -> %s\n",now_crypto_loc);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"curl -s %s -o %s",now_crypto_loc,NOW_CRYPTO_EXEC);
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Will copy the component 'now-crypto' from local place:\n");
        printf("[  ****  ] -> %s\n",now_crypto_loc);
        snprintf(cmdline2,CMDLINE_LENGTH-1,"%s %s %s %s",COPY_FILE_CMD,now_crypto_loc,NOW_CRYPTO_EXEC,SYSTEM_CMD_REDIRECT_NULL);
    }
    run_flag1=system(cmdline1);
    run_flag2=system(cmdline2);
    if(run_flag1!=0||run_flag2!=0){
        if(run_flag1!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'hpcopr'." RESET_DISPLAY "\n");
        }
        if(run_flag2!=0){
            printf(FATAL_RED_BOLD "[ FATAL: ] Failed to download/copy the 'now-crypto'." RESET_DISPLAY "\n");
        }
        printf(FATAL_RED_BOLD "[  ****  ] Please check and make sure:\n");
        printf("[  ****  ] 1. The HPC-NOW Services have been installed previously.\n");
        printf("[  ****  ] 2. The specified location (if specified) is correct.\n");
        printf("[  ****  ] 3. Your device is connected to the internet.\n");
        printf("[  ****  ] 4. Currently there is no 'hpcopr' thread(s) running." RESET_DISPLAY "\n");
        restore_perm();
        return 1;
    }
    if(decrypt_flag==1){
        system(cmdline_enc);
#ifdef _WIN32
        i=0;
        snprintf(cmdline1,CMDLINE_LENGTH-1,"findstr \"/C:info@hpc-now.com\" C:\\ProgramData\\hpc-now\\enc.%s.temp > nul 2>&1",randstr);
        while(system(cmdline1)!=0){
            i++;
            printf("[ -WAIT- ] Encryption in progress, %d seconds passed ... \r",i);
            fflush(stdout);
            sleep_func(1);
        }
        printf("\n");
        sleep_func(1);
#endif
    }
#ifdef _WIN32
    snprintf(filename_temp,FILENAME_LENGTH-1,"C:\\ProgramData\\hpc-now\\enc.%s.temp",randstr);
    rm_file_or_dir(filename_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"C:\\ProgramData\\hpc-now\\dec.%s.temp",randstr);
    rm_file_or_dir(filename_temp);
    mk_pdir(NOW_LIC_DIR);
    if(file_exist_or_not(NOW_MIT_LIC_FILE)!=0){
        snprintf(cmdline_temp,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
        system(cmdline_temp);
    }
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /grant hpc-now:F /t > nul 2>&1",NOW_CRYPTO_EXEC);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s* /deny Administrators:F > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"icacls %s /deny Administrators:F > nul 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);

    if(system("set PATH | findstr C:\\hpc-now >nul 2>&1")!=0){
        snprintf(cmdline_temp,CMDLINE_LENGTH-1,"setx PATH \"%%PATH%%;C:\\hpc-now\" /m >nul 2>&1");
        system(cmdline_temp);
    }
    if(file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws.exe")!=0||file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws_completer.exe")!=0){
        system("start /b msiexec.exe /i https://awscli.amazonaws.com/AWSCLIV2.msi /qn");
    }
    while(file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws.exe")!=0||file_exist_or_not("C:\\Program Files\\Amazon\\AWSCLIV2\\aws_completer.exe")!=0){
        printf(GENERAL_BOLD "[ -WAIT- ]" RESET_DISPLAY " Installing additional component, %d sec(s) of max %ds passed ... \r",i,AWSCLI_INST_WAIT_TIME);
        fflush(stdout);
        i++;
        sleep_func(1);
        if(i==AWSCLI_INST_WAIT_TIME){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to install component. HPC-NOW dataman services may not work properly.");
            goto update_done;
        }
    }
    printf(RESET_DISPLAY "\n");
#elif __linux__
    mk_pdir(NOW_LIC_DIR);
    if(file_exist_or_not(NOW_MIT_LIC_FILE)!=0){
        snprintf(cmdline_temp,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
        system(cmdline_temp);
    }
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod 711 %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod 700 %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod -R 711 %s%s.bin >> /dev/null 2>&1",HPC_NOW_USER_DIR,PATH_SLASH);
    system(cmdline_temp);

    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"ln -s %s /usr/local/bin/hpcopr >> /dev/null 2>&1",HPCOPR_EXEC);
    system(cmdline_temp);
    system("mkdir -p /usr/share/terraform >> /dev/null 2>&1 && chmod -R 755 /usr/share/terraform >> /dev/null 2>&1 && chown -R hpc-now:hpc-now /usr/share/terraform >> /dev/null 2>&1");
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod +x %s && chmod +x %s && chown -R hpc-now:hpc-now %s && chown -R hpc-now:hpc-now %s",HPCOPR_EXEC,NOW_CRYPTO_EXEC,HPCOPR_EXEC,NOW_CRYPTO_EXEC);
    system(cmdline_temp);

    /* Allow only local:hpc-now group access the x server. */
    system("sed -i '/xhost + >> \\/dev\\/null 2>&1 # Added by HPC-NOW/d' /etc/profile >> /dev/null 2>&1");
    if(system("grep -w \"xhost + local:hpc-now >> /dev/null 2>&1 # Added by HPC-NOW\" /etc/profile >> /dev/null 2>&1")!=0){
        system("echo \"xhost + local:hpc-now >> /dev/null 2>&1 # Added by HPC-NOW\" >> /etc/profile");
    }

    if(check_linux_packman(linux_packman,16)!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to detect the package manager." RESET_DISPLAY "\n");
        goto update_done;
    }
    if(system("which xclip >> /dev/null 2>&1")!=0){
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline_temp,CMDLINE_LENGTH-1,"%s install xclip -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline_temp,CMDLINE_LENGTH-1,"%s -S xclip --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        system(cmdline_temp);
    }
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking Remmina (the RDP client for GNU/Linux) now ...\n");
    if(system("which remmina >> /dev/null 2>&1")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remmina has been installed to your OS.\n");
        goto update_done;
    }
    if(rdp_flag==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Installing Remmina - the RDP client for GNU/Linux now ...\n");
        if(strcmp(linux_packman,"pacman")!=0){
            snprintf(cmdline_temp,CMDLINE_LENGTH-1,"%s install remmina -y >> /dev/null 2>&1",linux_packman);
        }
        else{
            snprintf(cmdline_temp,CMDLINE_LENGTH-1,"%s -S remmina --noconfirm >> /dev/null 2>&1",linux_packman);
        }
        if(system(cmdline_temp)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to install Remmina, RDP won't work properly." RESET_DISPLAY "\n");
        }
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Remmina is absent. Please update with --rdp to install it later.\n");
    }
#elif __APPLE__
    mk_pdir(NOW_LIC_DIR);
    if(file_exist_or_not(NOW_MIT_LIC_FILE)!=0){
        snprintf(cmdline_temp,CMDLINE_LENGTH-1,"curl -s %s -o %s",URL_LICENSE,NOW_MIT_LIC_FILE);
        system(cmdline_temp);
    }

    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chown -R hpc-now:hpc-now %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod 711 %s >> /dev/null 2>&1",HPC_NOW_USER_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod 700 %s >> /dev/null 2>&1",HPC_NOW_ROOT_DIR);
    system(cmdline_temp);
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod -R 711 %s%s.bin >> /dev/null 2>&1",HPC_NOW_USER_DIR,PATH_SLASH);
    system(cmdline_temp);

    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"mkdir -p /usr/local/bin && ln -s %s /usr/local/bin/hpcopr >> /dev/null 2>&1",HPCOPR_EXEC);
    system(cmdline_temp);
    system("mkdir -p '/Library/Application Support/io.terraform' >> /dev/null 2>&1 && chmod -R 755 '/Library/Application Support/io.terraform' >> /dev/null 2>&1 && chown -R hpc-now:hpc-now '/Library/Application Support/io.terraform' >> /dev/null 2>&1");
    snprintf(cmdline_temp,CMDLINE_LENGTH-1,"chmod +x %s && chmod +x %s && chown -R hpc-now:hpc-now %s && chown -R hpc-now:hpc-now %s",HPCOPR_EXEC,NOW_CRYPTO_EXEC,HPCOPR_EXEC,NOW_CRYPTO_EXEC);
    system(cmdline_temp);
    printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Checking the Microsoft RDP Client now ...\n");
    if(folder_exist_or_not("/Applications/msrdp.app")==0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " Microsoft RDP has been installed to your OS.\n");
        goto update_done;
    }
    if(rdp_flag!=0){
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The Microsoft RDP Client is absent. Please update with --rdp to install.\n");
        goto update_done;
    }
    if(file_exist_or_not("/Users/Shared/rdp_for_mac.zip")!=0){
        snprintf(cmdline1,CMDLINE_LENGTH-1,"mkdir -p /Users/Shared/ && curl %s -o /Users/Shared/rdp_for_mac.zip",URL_MSRDP_FOR_MAC);
        if(system(cmdline1)!=0){
            printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to download the package, RDP won't work properly." RESET_DISPLAY "\n");
            goto update_done;
        }
    }
    if(system("unzip -q /Users/Shared/rdp_for_mac.zip -d /Applications/ && mv '/Applications/Microsoft Remote Desktop Beta.app' /Applications/msrdp.app")!=0){
        printf(WARN_YELLO_BOLD "[ -WARN- ] Failed to unzip the package, RDP won't work properly." RESET_DISPLAY "\n");
    }
    else{
        printf(GENERAL_BOLD "[ -INFO- ]" RESET_DISPLAY " The Microsoft RDP Client has been installed.\n");
    }
#endif
update_done:
    printf(GENERAL_BOLD "[ -DONE- ]" RESET_DISPLAY " The HPC-NOW cluster services have been updated.\n");
    return 0;
}

int valid_loc_format_or_not(char* loc_string){
    int length;
    length=strlen(loc_string);
    if(length==0){
        return -1;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)==':'&&*(loc_string+5)=='/'&&*(loc_string+6)=='/'){
        return 0;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)=='s'&&*(loc_string+5)==':'&&*(loc_string+6)=='/'&&*(loc_string+7)=='/'){
        return 0;
    }
    return 1;
}

int get_valid_verlist(void){
    char cmdline[CMDLINE_LENGTH]="";
    snprintf(cmdline,CMDLINE_LENGTH-1,"curl -s %sverlist-0.3.x.txt",DEFAULT_URL_HPCOPR_LATEST);
    if(system(cmdline)!=0){
        return 1;
    }
    return 0;
}

int version_valid(char* hpcopr_ver){
    char cmdline[CMDLINE_LENGTH]="";
    char ver_ext[32]="";
    char randstr[8]="";
    char verlist_temp[FILENAME_LENGTH]="";
    generate_random_nstring(randstr,8,1);
#ifdef _WIN32
    snprintf(verlist_temp,FILENAME_LENGTH-1,"C:\\ProgramData\\verlist.%s.temp",randstr);
#else
    snprintf(verlist_temp,FILENAME_LENGTH-1,"/tmp/verlist.%s.temp",randstr);
#endif
    snprintf(cmdline,CMDLINE_LENGTH-1,"curl -s %sverlist-0.3.x.txt -o %s",DEFAULT_URL_HPCOPR_LATEST,verlist_temp);
    if(system(cmdline)!=0){
        return -1;
    }
    snprintf(ver_ext,31,"< %s >",hpcopr_ver);
    if(find_multi_nkeys(verlist_temp,LINE_LENGTH_SHORT,ver_ext,"","","","")>0){
        rm_file_or_dir(verlist_temp);
        return 0;
    }
    else{
        rm_file_or_dir(verlist_temp);
        return 1;
    }
}

/*
 * return 1: current user is not root
 * return 2: License not accepted
 * return 3: internet connection failed
 * return 4: option incorrect
 * return 5: no option
 * return 7: version list failed to get
 * return 9: uninstallation failed
 * return 11: Failed to set password
 * return 13: Failed to update
 * return 15: Failed to install
 * return 0: Normal exit
*/
int main(int argc, char* argv[]){
    int run_flag=0;
    int hpcopr_loc_flag=-1;
    int crypto_loc_flag=-1;
    char hpcopr_loc[LOCATION_LENGTH]="";
    char now_crypto_loc[LOCATION_LENGTH]="";
    char opr_password_arg[32]="";
    char opr_password[PASSWORD_STRING_LENGTH]="";
    char hpcopr_ver[256]="";
    int rdp_flag=cmd_flag_check(argc,argv,"--rdp");
    print_header_installer();
    if(check_current_user_root()!=0){
        return 1;
    }
    if(check_internet_installer()!=0){
        print_tail_installer();
        return 3;
    }  
    if(argc==1){
        print_help_installer();
        printf(FATAL_RED_BOLD "\n[ FATAL: ] Please specify option(s) from the list above." RESET_DISPLAY "\n");
        print_tail_installer();
        return 5;
    }
    if(strcmp(argv[1],"help")==0){
        print_help_installer();
        print_tail_installer();
        return 0;
    }

    if(strcmp(argv[1],"version")==0){
        printf(HIGH_GREEN_BOLD "Version: %s\n" RESET_DISPLAY,INSTALLER_VERSION_CODE);
        print_tail_installer();
        return 0;
    }

    if(strcmp(argv[1],"verlist")==0){
        run_flag=get_valid_verlist();
        print_tail_installer();
        if(run_flag!=0){
            return 7;
        }
        return 0;
    }

    if(strcmp(argv[1],"uninstall")==0){
        run_flag=uninstall_services();
        print_tail_installer();
        if(run_flag!=0){
            return 9;
        }
        return 0;
    }

    if(strcmp(argv[1],"update")!=0&&strcmp(argv[1],"install")!=0&&strcmp(argv[1],"setpass")!=0){
        print_help_installer();
        printf(FATAL_RED_BOLD "\n[ FATAL: ] The specified general option '%s' is invalid." RESET_DISPLAY "\n", argv[1]);
        print_tail_installer();
        return 4;
    }
    if(strcmp(argv[1],"install")==0||strcmp(argv[1],"update")==0){
        if(cmd_flag_check(argc,argv,"--accept")!=0&&license_confirmation()!=0){
            print_tail_installer();
            return 2;
        }
    }
    if(strcmp(argv[1],"setpass")==0||strcmp(argv[1],"install")==0){
        cmd_keyword_ncheck(argc,argv,"--pass",opr_password_arg,32);
        if(strlen(opr_password_arg)==0){
            strncpy(opr_password,"",PASSWORD_STRING_LENGTH-1);
        }
        else if(strlen(opr_password_arg)>19){
            printf(WARN_YELLO_BOLD "[ -WARN- ] The keystring " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY WARN_YELLO_BOLD " is too long." RESET_DISPLAY "\n",opr_password_arg);
            strncpy(opr_password,"",PASSWORD_STRING_LENGTH-1);
        }
        else{
            strncpy(opr_password,opr_password_arg,19);
            if(password_complexity_check(opr_password,SPECIAL_PASSWORD_CHARS)!=0){
                printf(WARN_YELLO_BOLD "[ -WARN- ] The keystring " RESET_DISPLAY GREY_LIGHT "%s" RESET_DISPLAY WARN_YELLO_BOLD " is not complex enough." RESET_DISPLAY "\n",opr_password_arg);
                strncpy(opr_password,"",PASSWORD_STRING_LENGTH-1);
            }
        }
    }
    if(strcmp(argv[1],"setpass")==0){
        run_flag=set_opr_password(opr_password);
        print_tail_installer();
        if(run_flag!=0){
            return 11;
        }
        return 0;
    }
    cmd_keyword_ncheck(argc,argv,"--hloc",hpcopr_loc,384);
    cmd_keyword_ncheck(argc,argv,"--cloc",now_crypto_loc,384);
    cmd_keyword_ncheck(argc,argv,"--hver",hpcopr_ver,256);
    hpcopr_loc_flag=valid_loc_format_or_not(hpcopr_loc);
    crypto_loc_flag=valid_loc_format_or_not(now_crypto_loc);
    if(hpcopr_loc_flag!=-1){
        strncpy(hpcopr_ver,"",255);
    }
    else{
        if(strlen(hpcopr_ver)!=0){
            run_flag=version_valid(hpcopr_ver);
            if(run_flag==-1){
                printf(FATAL_RED_BOLD "[ FATAL: ] Failed to create a tmp file. Please check the disk space." RESET_DISPLAY "\n");
                print_tail_installer();
                return -1;
            }
            else if(run_flag==1){
                printf(FATAL_RED_BOLD "[ FATAL: ] The specified version code %s is invalid." RESET_DISPLAY "\n",hpcopr_ver);
                get_valid_verlist();
                print_tail_installer();
                return 13;
            }
        }
    }
    if(strcmp(argv[1],"update")==0){
        run_flag=update_services(hpcopr_loc_flag,hpcopr_loc,hpcopr_ver,crypto_loc_flag,now_crypto_loc,rdp_flag);
        print_tail_installer();
        if(run_flag!=0){
            return 13;
        }
        return 0;
    }
    else{
        run_flag=install_services(hpcopr_loc_flag,hpcopr_loc,hpcopr_ver,opr_password,crypto_loc_flag,now_crypto_loc,rdp_flag);
        print_tail_installer();
        if(run_flag!=0){
            return 15;
        }
        return 0;
    }
}