/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_MACROS_H
#define NOW_MACROS_H

#define CORE_VERSION_CODE "0.3.1.0137"

#define NULL_PTR_ARG -127

/* Define the printf ANSI colors. */
#define FATAL_RED_BOLD   "\033[1;31m"
#define WARN_YELLO_BOLD  "\033[1;33m"
#define HIGH_GREEN_BOLD  "\033[1;32m"
#define GREEN_NORMAL     "\033[0;32m"
#define HIGH_CYAN_BOLD   "\033[1;36m"
#define GREY_LIGHT       "\033[2;37m"
#define GENERAL_BOLD     "\033[1m"
#define RESET_DISPLAY    "\033[0m"

/* Define the tf configuration */
typedef struct{
    char tf_runner_type[16];
    char tf_runner[256];
    char dbg_level[128];
    int max_wait_time;
} tf_exec_config;

#define CONFIRM_STRING               "y-e-s"
#define CONFIRM_STRING_QUICK         "y"
#define GFUNC_FILE_SUFFIX            ".gfuncs"
#define SPECIAL_PASSWORD_CHARS       "~@&(){}[]=,.;:!#$%+-/|\\" 
#define SPECIAL_PASSWORD_CHARS_SHORT "~@&(){}[]="
#define TRANSFER_HEADER              "EXPORTED AND TO BE IMPORTED BY HPC-NOW SERVICES"
#define INTERNAL_FILE_HEADER         "---GENERATED AND MAINTAINED BY HPC-NOW SERVICES INTERNALLY---"

/* You can modify the MAXIMUM_ADD_NODE_NUMBER to allow adding more nodes in one command */
#define MAXIMUM_ADD_NODE_NUMBER  64
#define MINIMUM_ADD_NODE_NUMBER  1
#define MAXIMUM_ADD_USER_NUMBER  32
#define MINIMUM_ADD_USER_NUNMBER 2

/* This macro guarantees the maximum waiting time for terraform/tofu running */
#define MAXIMUM_WAIT_TIME       600
#define MAXIMUM_WAIT_TIME_EXT   1200

/* 
 * Usually you don't need to modify the macros in this section
 * Unless you are going to build your own default public repository
 * There are some subdirectory paths in the program, please follow the directory structure
 * Otherwise the repository won't work properly
 */
#define DEFAULT_URL_TF_ROOT           "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/tf-root/"
#define DEFAULT_URL_CODE_ROOT         "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/infra-as-code/"
#define DEFAULT_URL_SHELL_SCRIPTS     "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/scripts/"
#define DEFAULT_URL_APPS_INST_SCRIPTS "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/scripts/apps-install/"
#define DEFAULT_URL_NOW_CRYPTO        "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/now-crypto-aes/"
#define DEFAULT_INITUTILS_REPO_ROOT   "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/utils/"
#define DEFAULT_APPS_PKGS_REPO_ROOT   "https://hpc-now-1308065454.cos.accelerate.myqcloud.com/packages/"
#define DEFAULT_LOCATIONS_COUNT       7

/* Bucket CLI download and installation indicators */
#define OSSUTIL_1_FAILED             1  /* 0x00000001 */
#define COSCLI_2_FAILED              2  /* 0x00000010 */
#define AWSCLI_3_FAILED              4  /* 0x00000100 */
#define OBSUTIL_4_FAILED             8  /* 0x00001000 */
#define BCECMD_5_FAILED              16 /* 0x00010000 */
#define AZCOPY_6_FAILED              32 /* 0x00100000 */
#define GCLOUD_7_FAILED              64 /* 0x01000000 */
#define AWSCLI_INST_WAIT_TIME        360

/* System-related paths and macros */
#ifdef _WIN32
#define PATH_SLASH                   "\\"
#define HPC_NOW_ROOT_DIR             "C:\\ProgramData\\hpc-now\\"
#define HPC_NOW_USER_DIR             "C:\\hpc-now\\"
#define TF_LOCAL_PLUGINS             "C:\\ProgramData\\hpc-now-tf\\"
#define NOW_BINARY_DIR               HPC_NOW_USER_DIR"utils\\"
#define CRYPTO_KEY_FILE              HPC_NOW_ROOT_DIR"now_crypto_seed.lock"

#define USAGE_LOG_FILE_OLD           HPC_NOW_ROOT_DIR"now-cluster-usage.log"
#define OPERATION_LOG_FILE_OLD       HPC_NOW_ROOT_DIR"now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG_OLD     HPC_NOW_ROOT_DIR"system_command_error.log"
#define NOW_LOG_DIR                  HPC_NOW_ROOT_DIR"now_logs\\"
#define NOW_MON_DIR                  HPC_NOW_ROOT_DIR"mon_data\\"
#define NOW_TMP_DIR                  HPC_NOW_ROOT_DIR".tmp\\"
#define USAGE_LOG_FILE               NOW_LOG_DIR"now-cluster-usage.log"
#define OPERATION_LOG_FILE           NOW_LOG_DIR"now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG         NOW_LOG_DIR"system_command_error.log"

#define SYSTEM_CMD_REDIRECT          ">nul 2>>"SYSTEM_CMD_ERROR_LOG
#define SYSTEM_CMD_REDIRECT_NULL     ">nul 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>nul"
#define NULL_STREAM                  "nul"

#define DESTROYED_DIR                HPC_NOW_ROOT_DIR".destroyed\\"
#define NOW_LIC_DIR                  HPC_NOW_USER_DIR"hpc-now.licenses\\"
#define NOW_MIT_LIC_FILE             NOW_LIC_DIR"MIT.LICENSE"
#define SSHKEY_DIR                   HPC_NOW_ROOT_DIR".now-ssh\\"

#define NOW_WORKDIR_ROOT             HPC_NOW_ROOT_DIR"workdir\\"
#define GENERAL_CONF_DIR             HPC_NOW_ROOT_DIR"etc\\"
#define LOCATION_CONF_FILE           GENERAL_CONF_DIR"locations.conf"
#define VERS_SHA_CONF_FILE           GENERAL_CONF_DIR"components.conf"
#define ALL_CLUSTER_REGISTRY         GENERAL_CONF_DIR"all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR    GENERAL_CONF_DIR"current_cluster.dat"
#define TF_RUNNING_CONFIG            GENERAL_CONF_DIR"tf_running.conf"

#define NOW_CRYPTO_EXEC              NOW_BINARY_DIR"now-crypto-aes.exe"
#define TERRAFORM_EXEC               NOW_BINARY_DIR"terraform.exe"
#define TOFU_EXEC                    NOW_BINARY_DIR"tofu.exe"
#define HPCOPR_EXEC                  HPC_NOW_USER_DIR"hpcopr.exe"

#define S3CLI_EXEC                   "\"c:\\program files\\amazon\\awscliv2\\aws.exe\""
#define COSCLI_EXEC                  NOW_BINARY_DIR"coscli.exe"
#define OSSUTIL_EXEC                 NOW_BINARY_DIR"ossutil64.exe"
#define OBSUTIL_EXEC                 NOW_BINARY_DIR"obsutil.exe"
#define BCECMD_EXEC                  NOW_BINARY_DIR"bcecmd.exe"
#define AZCOPY_EXEC                  NOW_BINARY_DIR"azcopy.exe"
#define GCLOUD_CLI                   NOW_BINARY_DIR"google-cloud-sdk\\bin\\gcloud"

#define DELETE_FILE_CMD          "del /f /q /s"
#define DELETE_FOLDER_CMD        "rd /s /q"
#define COPY_FILE_CMD            "copy /y"
#define MOVE_FILE_CMD            "move /y"
#define CAT_FILE_CMD             "type 2>nul"
#define GREP_CMD                 "findstr"
#define SET_ENV_CMD              "set"
#define START_BG_JOB             "start /b"
#define MKDIR_CMD                "mkdir"
#define EDITOR_CMD               "notepad"
#define CLEAR_SCREEN_CMD         "cls"
#define RDP_EDIT_CMD             "mstsc /edit"
#define CLIPBOARD_CMD            "clip"
#define PIPE_TO_CLIPBOARD_CMD    "| clip"

#define FILENAME_SUFFIX_SHORT    "win"
#define FILENAME_SUFFIX_FULL     "windows"
#define CRLF_PASSWORD_HASH       "\n"

/* The urls below are permenant and fast to visit. Use them directly. */
#define URL_COSCLI    "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-windows.exe"
#define URL_OSSUTIL   "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-windows-amd64.zip"
#define URL_AWSCLI    "https://awscli.amazonaws.com/AWSCLIV2.msi"
#define URL_OBSUTIL   "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_windows_amd64.zip"
#define URL_BCECMD    "https://doc.bce.baidu.com/bce-documentation/BOS/windows-bcecmd-0.4.1.zip"
//#define URL_AZCOPY    "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_windows_amd64_10.20.1.zip"
#define URL_AZCOPY    "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/azcopy_windows_amd64_10.20.1.zip"
#define URL_GCLOUD    "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-449.0.0-windows-x86_64-bundled-python.zip"
#define URL_TOSUTIL   "https://tos-tools.tos-cn-beijing.volces.com/windows/tosutil"

#elif __linux__
#define PATH_SLASH                   "/"
#define HPC_NOW_ROOT_DIR             "/usr/.hpc-now/"
#define HPC_NOW_USER_DIR             "/home/hpc-now/"
#define TF_LOCAL_PLUGINS             "/usr/share/terraform/"
#define NOW_BINARY_DIR               HPC_NOW_USER_DIR".bin/utils/"
#define CRYPTO_KEY_FILE              HPC_NOW_ROOT_DIR".now_crypto_seed.lock"

#define USAGE_LOG_FILE_OLD           HPC_NOW_ROOT_DIR".now-cluster-usage.log"
#define OPERATION_LOG_FILE_OLD       HPC_NOW_ROOT_DIR".now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG_OLD     HPC_NOW_ROOT_DIR"system_command_error.log"
#define NOW_LOG_DIR                  HPC_NOW_ROOT_DIR"now_logs/"
#define NOW_MON_DIR                  HPC_NOW_ROOT_DIR"mon_data/"
#define NOW_TMP_DIR                  HPC_NOW_ROOT_DIR".tmp/"
#define USAGE_LOG_FILE               NOW_LOG_DIR"now-cluster-usage.log"
#define OPERATION_LOG_FILE           NOW_LOG_DIR"now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG         NOW_LOG_DIR"system_command_error.log"

#define SYSTEM_CMD_REDIRECT          ">>/dev/null 2>>"SYSTEM_CMD_ERROR_LOG
#define SYSTEM_CMD_REDIRECT_NULL     ">>/dev/null 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>/dev/null"
#define NULL_STREAM                  "/dev/null"

#define DESTROYED_DIR                HPC_NOW_ROOT_DIR".destroyed/"
#define NOW_LIC_DIR                  HPC_NOW_USER_DIR"hpc-now.licenses/"
#define NOW_MIT_LIC_FILE             NOW_LIC_DIR"MIT.LICENSE"
#define SSHKEY_DIR                   HPC_NOW_ROOT_DIR".now-ssh/"

#define NOW_WORKDIR_ROOT             HPC_NOW_ROOT_DIR"workdir/"
#define GENERAL_CONF_DIR             HPC_NOW_ROOT_DIR".etc/"
#define LOCATION_CONF_FILE           GENERAL_CONF_DIR"locations.conf"
#define VERS_SHA_CONF_FILE           GENERAL_CONF_DIR"components.conf"
#define ALL_CLUSTER_REGISTRY         GENERAL_CONF_DIR".all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR    GENERAL_CONF_DIR"current_cluster.dat"
#define TF_RUNNING_CONFIG            GENERAL_CONF_DIR"tf_running.conf"

#define NOW_CRYPTO_EXEC              NOW_BINARY_DIR"now-crypto-aes.exe"
#define TERRAFORM_EXEC               NOW_BINARY_DIR"terraform"
#define TOFU_EXEC                    NOW_BINARY_DIR"tofu"
#define HPCOPR_EXEC                  HPC_NOW_USER_DIR".bin/hpcopr"

#define S3CLI_EXEC                   NOW_BINARY_DIR"aws"
#define COSCLI_EXEC                  NOW_BINARY_DIR"coscli.exe"
#define OSSUTIL_EXEC                 NOW_BINARY_DIR"ossutil64.exe"
#define OBSUTIL_EXEC                 NOW_BINARY_DIR"obsutil.exe"
#define BCECMD_EXEC                  NOW_BINARY_DIR"bcecmd.exe"
#define AZCOPY_EXEC                  NOW_BINARY_DIR"azcopy.exe"
#define GCLOUD_CLI                   NOW_BINARY_DIR"google-cloud-sdk/bin/gcloud"

#define DELETE_FILE_CMD          "rm -rf"
#define DELETE_FOLDER_CMD        "rm -rf"
#define COPY_FILE_CMD            "/bin/cp"
#define MOVE_FILE_CMD            "mv"
#define CAT_FILE_CMD             "cat"
#define GREP_CMD                 "grep"
#define SET_ENV_CMD              "export"
#define START_BG_JOB             ""
#define MKDIR_CMD                "mkdir -p"
#define EDITOR_CMD               "vi"
#define CLEAR_SCREEN_CMD         "clear"
#define RDP_EDIT_CMD             "remmina --edit"
#define CLIPBOARD_CMD            "xclip"
#define PIPE_TO_CLIPBOARD_CMD    "| xclip -i -selection clipboard"

#define FILENAME_SUFFIX_SHORT    "lin"
#define FILENAME_SUFFIX_FULL     "linux"
#define CRLF_PASSWORD_HASH       "\r\n"

/* The urls below are permenant and fast to visit. Use them directly. */
#define URL_COSCLI    "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-linux"
#define URL_OSSUTIL   "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-linux-amd64.zip"
#define URL_AWSCLI    "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip"
#define URL_OBSUTIL   "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_linux_amd64.tar.gz"
#define URL_BCECMD    "https://doc.bce.baidu.com/bce-documentation/BOS/linux-bcecmd-0.4.1.zip"
//#define URL_AZCOPY    "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_linux_amd64_10.20.1.tar.gz"
#define URL_AZCOPY    "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/azcopy_linux_amd64_10.20.1.tar.gz"
#define URL_GCLOUD    "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-449.0.0-linux-x86_64.tar.gz"
#define URL_TOSUTIL   "https://tos-tools.tos-cn-beijing.volces.com/linux/tosutil"

#elif __APPLE__
#define PATH_SLASH                   "/"
#define HPC_NOW_ROOT_DIR             "/Applications/.hpc-now/"
#define HPC_NOW_USER_DIR             "/Users/hpc-now/"
#define TF_LOCAL_PLUGINS             "/Library/Application Support/io.terraform/"
#define NOW_BINARY_DIR               HPC_NOW_USER_DIR".bin/utils/"
#define CRYPTO_KEY_FILE              HPC_NOW_ROOT_DIR".now_crypto_seed.lock"

#define USAGE_LOG_FILE_OLD           HPC_NOW_ROOT_DIR".now-cluster-usage.log"
#define OPERATION_LOG_FILE_OLD       HPC_NOW_ROOT_DIR".now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG_OLD     HPC_NOW_ROOT_DIR"system_command_error.log"
#define NOW_LOG_DIR                  HPC_NOW_ROOT_DIR"now_logs/"
#define NOW_MON_DIR                  HPC_NOW_ROOT_DIR"mon_data/"
#define NOW_TMP_DIR                  HPC_NOW_ROOT_DIR".tmp/"
#define USAGE_LOG_FILE               NOW_LOG_DIR"now-cluster-usage.log"
#define OPERATION_LOG_FILE           NOW_LOG_DIR"now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG         NOW_LOG_DIR"system_command_error.log"

#define SYSTEM_CMD_REDIRECT          ">>/dev/null 2>>"SYSTEM_CMD_ERROR_LOG
#define SYSTEM_CMD_REDIRECT_NULL     ">>/dev/null 2>&1"
#define SYSTEM_CMD_ERR_REDIRECT_NULL "2>/dev/null"
#define NULL_STREAM                  "/dev/null"

#define DESTROYED_DIR                HPC_NOW_ROOT_DIR".destroyed/"
#define NOW_LIC_DIR                  HPC_NOW_USER_DIR"hpc-now.licenses/"
#define NOW_MIT_LIC_FILE             NOW_LIC_DIR"MIT.LICENSE"
#define SSHKEY_DIR                   HPC_NOW_ROOT_DIR".now-ssh/"

#define NOW_WORKDIR_ROOT             HPC_NOW_ROOT_DIR"workdir/"
#define GENERAL_CONF_DIR             HPC_NOW_ROOT_DIR".etc/"
#define LOCATION_CONF_FILE           GENERAL_CONF_DIR"locations.conf"
#define VERS_SHA_CONF_FILE           GENERAL_CONF_DIR"components.conf"
#define ALL_CLUSTER_REGISTRY         GENERAL_CONF_DIR".all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR    GENERAL_CONF_DIR"current_cluster.dat"
#define TF_RUNNING_CONFIG            GENERAL_CONF_DIR"tf_running.conf"

#define NOW_CRYPTO_EXEC              NOW_BINARY_DIR"now-crypto-aes.exe"
#define TERRAFORM_EXEC               NOW_BINARY_DIR"terraform"
#define TOFU_EXEC                    NOW_BINARY_DIR"tofu"
#define HPCOPR_EXEC                  HPC_NOW_USER_DIR".bin/hpcopr"

#define S3CLI_EXEC                   NOW_BINARY_DIR"aws"
#define COSCLI_EXEC                  NOW_BINARY_DIR"coscli.exe"
#define OSSUTIL_EXEC                 NOW_BINARY_DIR"ossutil64.exe"
#define OBSUTIL_EXEC                 NOW_BINARY_DIR"obsutil.exe"
#define BCECMD_EXEC                  NOW_BINARY_DIR"bcecmd.exe"
#define AZCOPY_EXEC                  NOW_BINARY_DIR"azcopy.exe"
#define GCLOUD_CLI                   NOW_BINARY_DIR"google-cloud-sdk/bin/gcloud"

#define DELETE_FILE_CMD          "rm -rf"
#define DELETE_FOLDER_CMD        "rm -rf"
#define COPY_FILE_CMD            "/bin/cp"
#define MOVE_FILE_CMD            "mv"
#define CAT_FILE_CMD             "cat"
#define GREP_CMD                 "grep"
#define SET_ENV_CMD              "export"
#define START_BG_JOB             ""
#define MKDIR_CMD                "mkdir -p"
#define EDITOR_CMD               "vi"
#define CLEAR_SCREEN_CMD         "clear"
#define RDP_EDIT_CMD             "open /Applications/msrdp.app"
#define CLIPBOARD_CMD            "pbcopy"
#define PIPE_TO_CLIPBOARD_CMD    "| pbcopy"

#define FILENAME_SUFFIX_SHORT    "dwn"
#define FILENAME_SUFFIX_FULL     "darwin"
#define CRLF_PASSWORD_HASH       "\r\n"

/* The urls below are permenant and fast to visit. Use them directly. */
#define URL_COSCLI    "https://cosbrowser-1253960454.cos.ap-shanghai.myqcloud.com/software/coscli/coscli-mac"
#define URL_OSSUTIL   "https://gosspublic.alicdn.com/ossutil/1.7.16/ossutil-v1.7.16-mac-amd64.zip"
#define URL_AWSCLI    "https://awscli.amazonaws.com/AWSCLIV2.pkg"
#define URL_OBSUTIL   "https://obs-community.obs.cn-north-1.myhuaweicloud.com/obsutil/current/obsutil_darwin_amd64.tar.gz"
#define URL_BCECMD    "https://doc.bce.baidu.com/bce-documentation/BOS/mac-bcecmd-0.4.1.zip"
// #define URL_AZCOPY    "https://azcopyvnext.azureedge.net/releases/release-10.20.1-20230809/azcopy_darwin_amd64_10.20.1.zip"
#define URL_AZCOPY    "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/azcopy_darwin_amd64_10.20.1.zip"
#define URL_GCLOUD    "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-449.0.0-darwin-x86_64.tar.gz"
#define URL_TOSUTIL   "https://tos-tools.tos-cn-beijing.volces.com/darwin/amd64/tosutil"

#endif

/* Internal macros - usually you don't need to modify the macros in this section.*/
#define CMDLINE_LENGTH             2048
#define CMDLINE_LENGTH_EXT         8192
#define CLUSTER_ID_LENGTH_MAX      24
#define CLUSTER_ID_LENGTH_MIN      8
#define USERNAME_LENGTH_MAX        16
#define USERNAME_LENGTH_MIN        4
#define USER_PASSWORD_LENGTH_MIN   6
#define USER_PASSWORD_LENGTH_MAX   21
#define CLUSTER_ID_LENGTH_MAX_PLUS 25
/* 
 * Caution: Generally, the DIR_LENGTH for Windows should be
 * set to 260.
 * But for this program, the DIR_LENGTH is very unlikely to 
 * exceed even 128. Therefore, the DIR length here is OK.
 */
#define DIR_LENGTH_SHORT           256
#define DIR_LENGTH                 384
#define DIR_LENGTH_EXT             448
#define FILENAME_LENGTH            512
#define FILENAME_BASE_LENGTH       128
#define FILENAME_EXT_LENGTH        128
#define FILENAME_BASE_FULL_LENGTH  260
#define FILENAME_LENGTH_EXT        576
#define FILE_IO_BLOCK              4194304 /* 4 MiB buffer */
#define LOCATION_LENGTH            384
#define LOCATION_LENGTH_EXTENDED   512

#define LINE_LENGTH_EXT    8192
#define LINE_LENGTH        5120
#define LINE_LENGTH_MID    2048
#define LINE_LENGTH_SMALL  768
#define LINE_LENGTH_SHORT  256
#define LINE_LENGTH_TINY   128

#define AKSK_LENGTH               256
#define CONF_STRING_LENTH         64
#define COMMAND_NUM               55
#define DATAMAN_COMMAND_NUM       17
#define COMMAND_STRING_LENGTH_MAX 64
#define SUBCMD_STRING_LENGTH_MAX  32
#define CONF_LINE_NUM             11
#define CMD_FLAG_NUM              30
#define CMD_KWDS_NUM              49
#define VERS_SHA_LINES            11

/* Internal macros - usually you don't need to modify the macros in this section.*/
#define URL_LICENSE             "https://gitee.com/zhenrong-wang/hpc-now/raw/master/COPYING"
#define PASSWORD_LENGTH         19
#define PASSWORD_STRING_LENGTH  20
#define RANDSTR_LENGTH_PLUS     11

/* Internal macros - usually you don't need to modify the macros in this section.*/
#define AWS_SLEEP_TIME_GLOBAL  180
#define AWS_SLEEP_TIME_CN      180
#define ALI_SLEEP_TIME         60
#define QCLOUD_SLEEP_TIME      60
#define GENERAL_SLEEP_TIME     30

#endif