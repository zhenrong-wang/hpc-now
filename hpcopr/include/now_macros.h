/*
* This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
* The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
* It is distributed under the license: GNU Public License - v2.0
* Bug report: info@hpc-now.com
*/

#ifndef NOW_MACROS_H
#define NOW_MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define CORE_VERSION_CODE "0.2.0.0017"
#define INSTALLER_VERSION_CODE "0.2.0.0017"

#define ALI_TF_PLUGIN_VERSION "1.203.0"
#define QCLOUD_TF_PLUGIN_VERSION "1.80.5"
#define AWS_TF_PLUGIN_VERSION "4.64.0"

#ifdef _WIN32
#include <malloc.h>
#define CRYPTO_KEY_FILE "C:\\programdata\\hpc-now\\now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-usage.log" //This is a global file!
#define OPERATION_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-operation.log"
#define NOW_LIC_DIR "C:\\hpc-now\\LICENSES"
#define SSHKEY_DIR "C:\\hpc-now\\.ssh"
#define OPERATION_ERROR_LOG "c:\\hpc-now\\hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "c:\\programdata\\hpc-now\\"

#define NOW_CRYPTO_EXEC "c:\\programdata\\hpc-now\\bin\\now-crypto.exe"
#define TERRAFORM_EXEC "c:\\programdata\\hpc-now\\bin\\terraform.exe"
#define HPCOPR_EXEC "C:\\hpc-now\\hpcopr.exe"

#define LOCATION_CONF_FILE "c:\\programdata\\hpc-now\\etc\\locations.conf"
#define ALL_CLUSTER_REGISTRY "c:\\programdata\\hpc-now\\etc\\all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "c:\\programdata\\hpc-now\\etc\\current_cluster.dat"

#define MD5_TF_EXEC "4985e962d9bf3d4276fcaff0295ce203"
#define MD5_NOW_CRYPTO "b0834f0c932a8a736badbf52b3d339e2"

#define MD5_ALI_TF "39e1e304e49741e51fe5e532344dc8fb"
#define MD5_QCLOUD_TF "360d8dca890f0efed8b5fba3269dd1c9"
#define MD5_AWS_TF "8da674e9f24e82aecb27bde4448676ba"
#define MD5_ALI_TF_ZIP "b879dc55dae4357c1ed5f01e4ebad938"
#define MD5_QCLOUD_TF_ZIP "4ec9c71674c9eb879b0079f9d32e1ac0"
#define MD5_AWS_TF_ZIP "b7287e8c02fd21755d4ff66c87afec86"

#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-windows.exe"
#define DEFAULT_URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_windows_amd64.exe"

#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#define CRYPTO_KEY_FILE "/usr/.hpc-now/.now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "/usr/.hpc-now/.now-cluster-usage.log" // This is a global file!
#define OPERATION_LOG_FILE "/usr/.hpc-now/.now-cluster-operation.log"
#define NOW_LIC_DIR "/home/hpc-now/LICENSES"
#define SSHKEY_DIR "/home/hpc-now/.now-ssh"
#define OPERATION_ERROR_LOG "/home/hpc-now/hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "/usr/.hpc-now/"

#define NOW_CRYPTO_EXEC "/usr/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/usr/.hpc-now/.bin/terraform.exe"
#define HPCOPR_EXEC "/home/hpc-now/.bin/hpcopr"

#define LOCATION_CONF_FILE "/usr/.hpc-now/.etc/locations.conf"
#define ALL_CLUSTER_REGISTRY "/usr/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/usr/.hpc-now/.etc/current_cluster.dat"

#define MD5_TF_EXEC "9777407ccfce2be14fe4bec072af4738"
#define MD5_NOW_CRYPTO "26ae6fb1a741dcb8356b650b0812710c"

#define MD5_ALI_TF "88f0da5ec9687a0c0935bb7f0e3306a4"
#define MD5_QCLOUD_TF "5a1b40aa8343d3618277031633e3194f"
#define MD5_AWS_TF "ca30654989fe33ad84ecd8e13ef4a563"
#define MD5_ALI_TF_ZIP "b50080f0c3b76ae5f376eb3734ae110b"
#define MD5_QCLOUD_TF_ZIP "8840d19efbdd4e56ec12cc71e1ae4945"
#define MD5_AWS_TF_ZIP "4044f73b071c681d88e793dd15617b5a"

#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-linux.exe"
#define DEFAULT_URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_linux_amd64"

#elif __APPLE__
#include <sys/time.h>
#define CRYPTO_KEY_FILE "/Applications/.hpc-now/.now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "/Applications/.hpc-now/.now-cluster-usage.log" // This is a global file!
#define OPERATION_LOG_FILE "/Applications/.hpc-now/.now-cluster-operation.log"
#define NOW_LIC_DIR "/Users/hpc-now/LICENSES"
#define SSHKEY_DIR "/Users/hpc-now/.now-ssh"
#define OPERATION_ERROR_LOG "/Users/hpc-now/hpc-now.err.log"
#define HPC_NOW_ROOT_DIR "/Applications/.hpc-now/"

#define NOW_CRYPTO_EXEC "/Applications/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/Applications/.hpc-now/.bin/terraform"
#define HPCOPR_EXEC "/Users/hpc-now/.bin/hpcopr"

#define LOCATION_CONF_FILE "/Applications/.hpc-now/.etc/locations.conf"
#define ALL_CLUSTER_REGISTRY "/Applications/.hpc-now/.etc/.all_clusters.dat"
#define CURRENT_CLUSTER_INDICATOR "/Applications/.hpc-now/.etc/current_cluster.dat"

#define MD5_TF_EXEC "821bf13764e8afbc0fb73a73e25aebad"
#define MD5_NOW_CRYPTO "202082eac600db6f6f429a1ceb047044"

#define MD5_ALI_TF "6091b17f8454fb6eb58c265938f0852f"
#define MD5_QCLOUD_TF "98e1a2b414fa3d38a9af20950deba844"
#define MD5_AWS_TF "aa46b5649535029d96fe6ada7fff1a20"
#define MD5_ALI_TF_ZIP "a8994ffac784d05f1d4bb45af65371ba"
#define MD5_QCLOUD_TF_ZIP "13c2632876e3cbd02687d777e30a4708"
#define MD5_AWS_TF_ZIP "1f01e3b61fa0d4a5e3514a775e27a826"

#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-darwin.exe"
#define DEFAULT_URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_darwin_amd64"

#endif

/* Usually you don't need to modify the macros in this section.*/
#define CMDLINE_LENGTH 2048
#define CLUSTER_ID_LENGTH_MAX 24
#define CLUSTER_ID_LENGTH_MIN 8
#define CLUSTER_ID_LENGTH_MAX_PLUS 25
#define DIR_LENGTH 256
#define FILENAME_LENGTH 512
#define LOCATION_LENGTH 512
#define LOCATION_LENGTH_EXTENDED 768
#define LINE_LENGTH 4096
#define LINE_LENGTH_SHORT 128
#define AKSK_LENGTH 128
#define CONF_STRING_LENTH 64

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
#define MAXIMUM_ADD_USER_NUMBER 8
/* This macro guarantees the maximum waiting time for terraform running */
#define MAXIMUM_WAIT_TIME 600

/* 
 * Usually you don't need to modify the macros in this sections
 * Unless you are going to build your own default public repository
 * There are some subdirectory paths in the program, please follow the directory structure
 * Otherwise the repository won't work properly
 */

#define DEFAULT_URL_TF_ROOT "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform-root/"
#define DEFAULT_URL_CODE_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/"
#define DEFAULT_URL_SHELL_SCRIPTS "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/scripts/"

#endif