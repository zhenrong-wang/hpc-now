/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_MACROS_H
#define NOW_MACROS_H

#define CORE_VERSION_CODE "0.2.0.0160"

#ifdef _WIN32
#define PATH_SLASH "\\"
#define CRYPTO_KEY_FILE "C:\\programdata\\hpc-now\\now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-usage.log" //This is a global file!
#define OPERATION_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "C:\\programdata\\hpc-now\\system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">nul 2>>C:\\programdata\\hpc-now\\system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">nul 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>nul"
#define NULL_STREAM "nul"
#define DESTROYED_DIR "c:\\programdata\\hpc-now\\.destroyed\\"
#define NOW_LIC_DIR "C:\\hpc-now\\hpc-now.licenses\\"
#define SSHKEY_DIR "C:\\programdata\\hpc-now\\.now-ssh\\"
#define HPC_NOW_ROOT_DIR "c:\\programdata\\hpc-now\\"
#define TF_LOCAL_PLUGINS "c:\\programdata\\hpc-now-tf\\"

#define NOW_CRYPTO_EXEC "c:\\hpc-now\\utils\\now-crypto.exe"
#define TERRAFORM_EXEC "c:\\hpc-now\\utils\\terraform.exe"
#define HPCOPR_EXEC "C:\\hpc-now\\hpcopr.exe"

#define DELETE_FILE_CMD "del /f /q /s"
#define DELETE_FOLDER_CMD "rd /s /q"
#define COPY_FILE_CMD "copy /y"
#define MOVE_FILE_CMD "move /y"
#define CAT_FILE_CMD "type 2>nul"
#define GREP_CMD "findstr"
#define SET_ENV_CMD "set"
#define START_BG_JOB "start /b"
#define MKDIR_CMD "mkdir"
#define EDITOR_CMD "notepad"
#define CLEAR_SCREEN_CMD "cls"

#define NOW_BINARY_DIR "c:\\hpc-now\\utils\\"
#define GENERAL_CONF_DIR "c:\\programdata\\hpc-now\\etc\\"
#define LOCATION_CONF_FILE "c:\\programdata\\hpc-now\\etc\\locations.conf"
#define VERS_MD5_CONF_FILE "c:\\programdata\\hpc-now\\etc\\md5values.conf"
#define ALL_CLUSTER_REGISTRY "c:\\programdata\\hpc-now\\etc\\all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "c:\\programdata\\hpc-now\\etc\\current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "win"
#define FILENAME_SUFFIX_FULL "windows"

#define GETTIMEOFDAY_FUNC mingw_gettimeofday
#define GETPASS_FUNC getpass_win

// The urls below are permenant and fast to visit. Use them directly.
#define URL_COSCLI "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-windows.exe"
#define URL_OSSUTIL "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-windows-amd64.zip"
#define URL_AWSCLI "https://awscli.amazonaws.com/AWSCLIV2.msi"
#define URL_OBSUTIL "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_windows_amd64.zip"
#define URL_BCECMD "https://doc.bce.baidu.com/bce-documentation/BOS/windows-bcecmd-0.4.1.zip"
#define URL_AZCOPY "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_windows_amd64_10.20.1.zip"
#define URL_GCLOUD "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-449.0.0-windows-x86_64-bundled-python.zip"

#define S3CLI_EXEC "\"c:\\program files\\amazon\\awscliv2\\aws.exe\""
#define COSCLI_EXEC "c:\\hpc-now\\utils\\coscli.exe"
#define OSSUTIL_EXEC "c:\\hpc-now\\utils\\ossutil64.exe"
#define OBSUTIL_EXEC "c:\\hpc-now\\utils\\obsutil.exe"
#define BCECMD_EXEC "c:\\hpc-now\\utils\\bcecmd.exe"
#define AZCOPY_EXEC "c:\\hpc-now\\utils\\azcopy.exe"
#define GCLOUD_CLI "c:\\hpc-now\\utils\\google-cloud-sdk\\bin\\gcloud"

#elif __linux__
#define PATH_SLASH "/"
#define CRYPTO_KEY_FILE "/usr/.hpc-now/.now_crypto_seed.lock"
#define USAGE_LOG_FILE "/usr/.hpc-now/.now-cluster-usage.log"
#define OPERATION_LOG_FILE "/usr/.hpc-now/.now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "/usr/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">>/dev/null 2>>/usr/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">>/dev/null 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>/dev/null"
#define NULL_STREAM "/dev/null"
#define DESTROYED_DIR "/usr/.hpc-now/.destroyed/"
#define NOW_LIC_DIR "/home/hpc-now/hpc-now.licenses/"
#define SSHKEY_DIR "/usr/.hpc-now/.now-ssh/"
#define HPC_NOW_ROOT_DIR "/usr/.hpc-now/"
#define TF_LOCAL_PLUGINS "/usr/share/terraform/"

#define NOW_CRYPTO_EXEC "/home/hpc-now/.bin/utils/now-crypto.exe"
#define TERRAFORM_EXEC "/home/hpc-now/.bin/utils/terraform"
#define HPCOPR_EXEC "/home/hpc-now/.bin/hpcopr"

#define DELETE_FILE_CMD "rm -rf"
#define DELETE_FOLDER_CMD "rm -rf"
#define COPY_FILE_CMD "/bin/cp"
#define MOVE_FILE_CMD "mv"
#define CAT_FILE_CMD "cat"
#define GREP_CMD "grep"
#define SET_ENV_CMD "export"
#define START_BG_JOB ""
#define MKDIR_CMD "mkdir -p"
#define EDITOR_CMD "vi"
#define CLEAR_SCREEN_CMD "clear"

#define NOW_BINARY_DIR "/home/hpc-now/.bin/utils/"
#define GENERAL_CONF_DIR "/usr/.hpc-now/.etc/"
#define LOCATION_CONF_FILE "/usr/.hpc-now/.etc/locations.conf"
#define VERS_MD5_CONF_FILE "/usr/.hpc-now/.etc/md5values.conf"
#define ALL_CLUSTER_REGISTRY "/usr/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/usr/.hpc-now/.etc/current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "lin"
#define FILENAME_SUFFIX_FULL "linux"

#define GETTIMEOFDAY_FUNC gettimeofday
#define GETPASS_FUNC getpass

// The urls below are permenant and fast to visit. Use them directly.
#define URL_COSCLI "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-linux"
#define URL_OSSUTIL "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-linux-amd64.zip"
#define URL_AWSCLI "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip"
#define URL_OBSUTIL "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_linux_amd64.tar.gz"
#define URL_BCECMD "https://doc.bce.baidu.com/bce-documentation/BOS/linux-bcecmd-0.4.1.zip"
#define URL_AZCOPY "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_linux_amd64_10.20.1.tar.gz"
#define URL_GCLOUD "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-449.0.0-linux-x86_64.tar.gz"

#define S3CLI_EXEC "/home/hpc-now/.bin/utils/aws"
#define COSCLI_EXEC "/home/hpc-now/.bin/utils/coscli.exe"
#define OSSUTIL_EXEC "/home/hpc-now/.bin/utils/ossutil64.exe"
#define OBSUTIL_EXEC "/home/hpc-now/.bin/utils/obsutil.exe"
#define BCECMD_EXEC "/home/hpc-now/.bin/utils/bcecmd.exe"
#define AZCOPY_EXEC "/home/hpc-now/.bin/utils/azcopy.exe"
#define GCLOUD_CLI "/home/hpc-now/.bin/utils/google-cloud-sdk/bin/gcloud"

#elif __APPLE__
#define PATH_SLASH "/"
#define CRYPTO_KEY_FILE "/Applications/.hpc-now/.now_crypto_seed.lock"
#define USAGE_LOG_FILE "/Applications/.hpc-now/.now-cluster-usage.log"
#define OPERATION_LOG_FILE "/Applications/.hpc-now/.now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "/Applications/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">>/dev/null 2>>/Applications/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">>/dev/null 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>/dev/null"
#define NULL_STREAM "/dev/null"
#define DESTROYED_DIR "/Applications/.hpc-now/.destroyed/"
#define NOW_LIC_DIR "/Users/hpc-now/hpc-now.licenses/"
#define SSHKEY_DIR "/Applications/.hpc-now/.now-ssh/"
#define HPC_NOW_ROOT_DIR "/Applications/.hpc-now/"
#define TF_LOCAL_PLUGINS "/Library/Application Support/io.terraform/"

#define NOW_CRYPTO_EXEC "/Users/hpc-now/.bin/utils/now-crypto.exe"
#define TERRAFORM_EXEC "/Users/hpc-now/.bin/utils/terraform"
#define HPCOPR_EXEC "/Users/hpc-now/.bin/hpcopr"

#define DELETE_FILE_CMD "rm -rf"
#define DELETE_FOLDER_CMD "rm -rf"
#define COPY_FILE_CMD "/bin/cp"
#define MOVE_FILE_CMD "mv"
#define CAT_FILE_CMD "cat"
#define GREP_CMD "grep"
#define SET_ENV_CMD "export"
#define START_BG_JOB ""
#define MKDIR_CMD "mkdir -p"
#define EDITOR_CMD "vi"
#define CLEAR_SCREEN_CMD "clear"

#define NOW_BINARY_DIR "/Users/hpc-now/.bin/utils/"
#define GENERAL_CONF_DIR "/Applications/.hpc-now/.etc/"
#define LOCATION_CONF_FILE "/Applications/.hpc-now/.etc/locations.conf"
#define VERS_MD5_CONF_FILE "/Applications/.hpc-now/.etc/md5values.conf"
#define ALL_CLUSTER_REGISTRY "/Applications/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/Applications/.hpc-now/.etc/current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "dwn"
#define FILENAME_SUFFIX_FULL "darwin"

#define GETTIMEOFDAY_FUNC gettimeofday
#define GETPASS_FUNC getpass

// The urls below are permenant and fast to visit. Use them directly.
#define URL_COSCLI "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-mac"
#define URL_OSSUTIL "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-mac-amd64.zip"
#define URL_AWSCLI "https://awscli.amazonaws.com/AWSCLIV2.pkg"
#define URL_OBSUTIL "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_darwin_amd64.tar.gz"
#define URL_BCECMD "https://doc.bce.baidu.com/bce-documentation/BOS/mac-bcecmd-0.4.1.zip"
#define URL_AZCOPY "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_darwin_amd64_10.20.1.zip"
#define URL_GCLOUD "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-449.0.0-darwin-x86_64.tar.gz"

#define S3CLI_EXEC "/Users/hpc-now/.bin/utils/aws"
#define COSCLI_EXEC "/Users/hpc-now/.bin/utils/coscli.exe"
#define OSSUTIL_EXEC "/Users/hpc-now/.bin/utils/ossutil64.exe"
#define OBSUTIL_EXEC "/Users/hpc-now/.bin/utils/obsutil.exe"
#define BCECMD_EXEC "/Users/hpc-now/.bin/utils/bcecmd.exe"
#define AZCOPY_EXEC "/Users/hpc-now/.bin/utils/azcopy.exe"
#define GCLOUD_CLI "/Users/hpc-now/.bin/utils/google-cloud-sdk/bin/gcloud"

#endif

#define CONFIRM_STRING "y-e-s"

/* Usually you don't need to modify the macros in this section.*/
#define CMDLINE_LENGTH 2048
#define CMDLINE_LENGTH_EXT 8192
#define CLUSTER_ID_LENGTH_MAX 24
#define CLUSTER_ID_LENGTH_MIN 8
#define USERNAME_LENGTH_MAX 16
#define USERNAME_LENGTH_MIN 4
#define USER_PASSWORD_LENGTH_MAX 21
#define CLUSTER_ID_LENGTH_MAX_PLUS 25
#define DIR_LENGTH_SHORT 256
#define DIR_LENGTH 384
#define DIR_LENGTH_EXT 448
#define FILENAME_LENGTH 512
#define FILENAME_LENGTH_EXT 576
#define LOCATION_LENGTH 384
#define LOCATION_LENGTH_EXTENDED 512
#define LINE_LENGTH_EXT 8192
#define LINE_LENGTH 5120
#define LINE_LENGTH_MID 2048
#define LINE_LENGTH_SHORT 256
#define AKSK_LENGTH 256
#define CONF_STRING_LENTH 64
#define COMMAND_NUM 48
#define DATAMAN_COMMAND_NUM 17
#define COMMAND_STRING_LENGTH_MAX 64
#define CONF_LINE_NUM 11
#define CMD_FLAG_NUM 26
#define CMD_KWDS_NUM 42

/* Usually you don't need to modify the macros in this section.*/
#define URL_LICENSE "https://gitee.com/zhenrong-wang/hpc-now/raw/master/COPYING"
#define PASSWORD_LENGTH 19
#define PASSWORD_STRING_LENGTH 20
#define RANDSTR_LENGTH_PLUS 11

/* Usually you don't need to modify the macros in this section.*/
#define AWS_SLEEP_TIME_GLOBAL 180
#define AWS_SLEEP_TIME_CN 180
#define ALI_SLEEP_TIME 60
#define QCLOUD_SLEEP_TIME 60
#define GENERAL_SLEEP_TIME 30

/* You can modify the MAXIMUM_ADD_NODE_NUMBER to allow adding more than 16 nodes in one command */
#define MAXIMUM_ADD_NODE_NUMBER 32
#define MINIMUM_ADD_NODE_NUMBER 1
#define MAXIMUM_ADD_USER_NUMBER 32
#define MINIMUM_ADD_USER_NUNMBER 2
/* This macro guarantees the maximum waiting time for terraform running */
#define MAXIMUM_WAIT_TIME 600

/* 
 * Usually you don't need to modify the macros in this section
 * Unless you are going to build your own default public repository
 * There are some subdirectory paths in the program, please follow the directory structure
 * Otherwise the repository won't work properly
 */

#define DEFAULT_URL_TF_ROOT "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-root/"
#define DEFAULT_URL_CODE_ROOT "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/infra-as-code/"
#define DEFAULT_URL_SHELL_SCRIPTS "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/scripts/"
#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-crypto/"
#define DEFAULT_INITUTILS_REPO_ROOT "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/"

#define TRANSFER_HEADER "EXPORTED AND TO BE IMPORTED BY HPC-NOW SERVICES"

#define LOCATION_CONF_TOTAL_LINES 6
#define LOCATION_LINES 5

#define FATAL_RED_BOLD "\033[1;31m"
#define WARN_YELLO_BOLD "\033[1;33m"
#define HIGH_GREEN_BOLD "\033[1;32m"
#define HIGH_CYAN_BOLD "\033[1;36m"
#define GREY_LIGHT "\033[2;37m"
#define GENERAL_BOLD "\033[1m"
#define RESET_DISPLAY "\033[0m"

#endif