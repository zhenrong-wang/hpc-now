/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef NOW_MACROS_H
#define NOW_MACROS_H

#define CORE_VERSION_CODE "0.2.0.0128"

#ifdef _WIN32
#define PATH_SLASH "\\"
#define CRYPTO_KEY_FILE "C:\\programdata\\hpc-now\\now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-usage.log" //This is a global file!
#define OPERATION_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "C:\\programdata\\hpc-now\\system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">nul 2>>C:\\programdata\\hpc-now\\system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">nul 2>&1"
#define DESTROYED_DIR "c:\\programdata\\hpc-now\\.destroyed\\"
#define NOW_LIC_DIR "C:\\hpc-now\\LICENSES"
#define SSHKEY_DIR "C:\\hpc-now\\.now-ssh"
#define OPERATION_ERROR_LOG "c:\\hpc-now\\hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "c:\\programdata\\hpc-now\\"

#define NOW_CRYPTO_EXEC "c:\\programdata\\hpc-now\\bin\\now-crypto.exe"
#define TERRAFORM_EXEC "c:\\programdata\\hpc-now\\bin\\terraform.exe"
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

#define NOW_BINARY_DIR "c:\\programdata\\hpc-now\\bin\\"
#define GENERAL_CONF_DIR "c:\\programdata\\hpc-now\\etc\\"
#define LOCATION_CONF_FILE "c:\\programdata\\hpc-now\\etc\\locations.conf"
#define VERS_MD5_CONF_FILE "c:\\programdata\\hpc-now\\etc\\md5values.conf"
#define ALL_CLUSTER_REGISTRY "c:\\programdata\\hpc-now\\etc\\all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "c:\\programdata\\hpc-now\\etc\\current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "win"
#define FILENAME_SUFFIX_FULL "windows"

#define GETTIMEOFDAY_FUNC mingw_gettimeofday
#define GETPASS_FUNC getpass_win

#elif __linux__
#define PATH_SLASH "/"
#define CRYPTO_KEY_FILE "/usr/.hpc-now/.now_crypto_seed.lock"
#define USAGE_LOG_FILE "/usr/.hpc-now/.now-cluster-usage.log"
#define OPERATION_LOG_FILE "/usr/.hpc-now/.now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "/usr/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">>/dev/null 2>>/usr/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">>/dev/null 2>&1"
#define DESTROYED_DIR "/usr/.hpc-now/.destroyed/"
#define NOW_LIC_DIR "/home/hpc-now/LICENSES"
#define SSHKEY_DIR "/home/hpc-now/.now-ssh"
#define OPERATION_ERROR_LOG "/home/hpc-now/hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "/usr/.hpc-now/"
#define TF_LOCAL_PLUGINS "/usr/share/terraform/"

#define NOW_CRYPTO_EXEC "/usr/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/usr/.hpc-now/.bin/terraform"
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

#define NOW_BINARY_DIR "/usr/.hpc-now/.bin/"
#define GENERAL_CONF_DIR "/usr/.hpc-now/.etc/"
#define LOCATION_CONF_FILE "/usr/.hpc-now/.etc/locations.conf"
#define VERS_MD5_CONF_FILE "/usr/.hpc-now/.etc/md5values.conf"
#define ALL_CLUSTER_REGISTRY "/usr/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/usr/.hpc-now/.etc/current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "lin"
#define FILENAME_SUFFIX_FULL "linux"

#define GETTIMEOFDAY_FUNC gettimeofday
#define GETPASS_FUNC getpass

#elif __APPLE__
#define PATH_SLASH "/"
#define CRYPTO_KEY_FILE "/Applications/.hpc-now/.now_crypto_seed.lock"
#define USAGE_LOG_FILE "/Applications/.hpc-now/.now-cluster-usage.log"
#define OPERATION_LOG_FILE "/Applications/.hpc-now/.now-cluster-operation.log"
#define SYSTEM_CMD_ERROR_LOG "/Applications/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT ">>/dev/null 2>>/Applications/.hpc-now/system_command_error.log"
#define SYSTEM_CMD_REDIRECT_NULL ">>/dev/null 2>&1"
#define DESTROYED_DIR "/Applications/.hpc-now/.destroyed/"
#define NOW_LIC_DIR "/Users/hpc-now/LICENSES"
#define SSHKEY_DIR "/Users/hpc-now/.now-ssh"
#define OPERATION_ERROR_LOG "/Users/hpc-now/hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "/Applications/.hpc-now/"
#define TF_LOCAL_PLUGINS "/Library/Application Support/io.terraform/"

#define NOW_CRYPTO_EXEC "/Applications/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/Applications/.hpc-now/.bin/terraform"
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

#define NOW_BINARY_DIR "/Applications/.hpc-now/.bin/"
#define GENERAL_CONF_DIR "/Applications/.hpc-now/.etc/"
#define LOCATION_CONF_FILE "/Applications/.hpc-now/.etc/locations.conf"
#define VERS_MD5_CONF_FILE "/Applications/.hpc-now/.etc/md5values.conf"
#define ALL_CLUSTER_REGISTRY "/Applications/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/Applications/.hpc-now/.etc/current_cluster.dat"
#define FILENAME_SUFFIX_SHORT "dwn"
#define FILENAME_SUFFIX_FULL "darwin"

#define GETTIMEOFDAY_FUNC gettimeofday
#define GETPASS_FUNC getpass

#endif

#define CONFIRM_STRING "y-e-s"

/* Usually you don't need to modify the macros in this section.*/
#define CMDLINE_LENGTH 2048
#define CLUSTER_ID_LENGTH_MAX 24
#define CLUSTER_ID_LENGTH_MIN 8
#define USERNAME_LENGTH_MAX 16
#define USERNAME_LENGTH_MIN 4
#define USER_PASSWORD_LENGTH_MAX 21
#define CLUSTER_ID_LENGTH_MAX_PLUS 25
#define DIR_LENGTH 384
#define FILENAME_LENGTH 512
#define LOCATION_LENGTH 384
#define LOCATION_LENGTH_EXTENDED 512
#define LINE_LENGTH 4096
#define LINE_LENGTH_SHORT 256
#define AKSK_LENGTH 128
#define CONF_STRING_LENTH 64
#define COMMAND_NUM 39
#define COMMAND_STRING_LENGTH_MAX 16

/* Usually you don't need to modify the macros in this section.*/
#define URL_LICENSE "https://gitee.com/zhenrong-wang/hpc-now/raw/master/LICENSE"
#define URL_LICENSE_FSF "https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt"
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
#define MAXIMUM_ADD_NODE_NUMBER 16
#define MINUMUM_ADD_NODE_NUMBER 1
#define MAXIMUM_ADD_USER_NUMBER 8
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

/* The macros below are deprecated. 

TERRAFORM_VERSION "1.4.5"
ALI_TF_PLUGIN_VERSION "1.203.0"
QCLOUD_TF_PLUGIN_VERSION "1.80.5"
AWS_TF_PLUGIN_VERSION "4.64.0"

MD5_TF_EXEC_WIN "4985e962d9bf3d4276fcaff0295ce203"
MD5_TF_ZIP_WIN "7fc59fe0e503896680bf343bb0687f6f"
MD5_NOW_CRYPTO_WIN "b0834f0c932a8a736badbf52b3d339e2"
MD5_ALI_TF_WIN "39e1e304e49741e51fe5e532344dc8fb"
MD5_QCLOUD_TF_WIN "360d8dca890f0efed8b5fba3269dd1c9"
MD5_AWS_TF_WIN "8da674e9f24e82aecb27bde4448676ba"
MD5_ALI_TF_ZIP_WIN "b879dc55dae4357c1ed5f01e4ebad938"
MD5_QCLOUD_TF_ZIP_WIN "4ec9c71674c9eb879b0079f9d32e1ac0"
MD5_AWS_TF_ZIP_WIN "b7287e8c02fd21755d4ff66c87afec86"

MD5_TF_EXEC_LIN "24dbe458378955df4bc37215c2273b4f"
MD5_TF_ZIP_LIN "5fefef8a8c95cc9614cf7d3435f82bd7"
MD5_NOW_CRYPTO_LIN "26ae6fb1a741dcb8356b650b0812710c"
MD5_ALI_TF_LIN "88f0da5ec9687a0c0935bb7f0e3306a4"
MD5_QCLOUD_TF_LIN "5a1b40aa8343d3618277031633e3194f"
MD5_AWS_TF_LIN "ca30654989fe33ad84ecd8e13ef4a563"
MD5_ALI_TF_ZIP_LIN "b50080f0c3b76ae5f376eb3734ae110b"
MD5_QCLOUD_TF_ZIP_LIN "8840d19efbdd4e56ec12cc71e1ae4945"
MD5_AWS_TF_ZIP_LIN "4044f73b071c681d88e793dd15617b5a"

MD5_TF_EXEC_DWN "55fda9502c6e1a3b5e30c77b73e5890d"
MD5_TF_ZIP_DWN "25b89c2bbd0dec71c307810cfe1bd622"
MD5_NOW_CRYPTO_DWN "202082eac600db6f6f429a1ceb047044"
MD5_ALI_TF_DWN "6091b17f8454fb6eb58c265938f0852f"
MD5_QCLOUD_TF_DWN "98e1a2b414fa3d38a9af20950deba844"
MD5_AWS_TF_DWN "aa46b5649535029d96fe6ada7fff1a20"
MD5_ALI_TF_ZIP_DWN "a8994ffac784d05f1d4bb45af65371ba"
MD5_QCLOUD_TF_ZIP_DWN "13c2632876e3cbd02687d777e30a4708"
MD5_AWS_TF_ZIP_DWN "1f01e3b61fa0d4a5e3514a775e27a826"

Terraform version is relatively stable. Here we use 1.4.5*/
/*#define PROTECTION_FILE_NAME "PROTECTION.txt"*/