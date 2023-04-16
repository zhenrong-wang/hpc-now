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

#define VERSION_CODE "0.1.91.0016"

#ifdef _WIN32
#include <malloc.h>
#define CRYPTO_KEY_FILE "C:\\programdata\\hpc-now\\now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-usage.log" //This is a global file!
#define OPERATION_LOG_FILE "C:\\programdata\\hpc-now\\now-cluster-operation.log"
#define NOW_LIC_DIR "C:\\hpc-now\\LICENSES"
#define SSHKEY_DIR "C:\\hpc-now\\.ssh"
#define NOW_CRYPTO_EXEC "c:\\programdata\\hpc-now\\bin\\now-crypto.exe"
#define TERRAFORM_EXEC "c:\\programdata\\hpc-now\\bin\\terraform.exe"
#define LOCATION_CONF_FILE "c:\\programdata\\hpc-now\\etc\\locations.conf"
#define ALI_TF_PLUGIN_VERSION "1.198.0"
#define QCLOUD_TF_PLUGIN_VERSION "1.79.10"
#define AWS_TF_PLUGIN_VERSION "4.55.0"
#define MD5_TF_EXEC "4985e962d9bf3d4276fcaff0295ce203"
#define MD5_NOW_CRYPTO "b0834f0c932a8a736badbf52b3d339e2"
#define MD5_ALI_TF "eafd246cfd00d605a64e60fc528a49d2"
#define MD5_QCLOUD_TF "02c253659ffc3fbe980e07d78345ddee"
#define MD5_AWS_TF "991928c1863f2f8956a2589e9031cbe3"
#define MD5_ALI_TF_ZIP "d65783aa8504c517ae9c79aa8ca8ea9b"
#define MD5_QCLOUD_TF_ZIP "9dcad88abf9e06d5e6cd615e9c569be6"
#define MD5_AWS_TF_ZIP "c064dfa26dcd4f37940e1aef36b4e5b0"
#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-windows.exe"

#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#define CRYPTO_KEY_FILE "/usr/.hpc-now/.now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "/usr/.hpc-now/.now-cluster-usage.log" // This is a global file!
#define OPERATION_LOG_FILE "/usr/.hpc-now/.now-cluster-operation.log"
#define NOW_LIC_DIR "/home/hpc-now/LICENSES"
#define SSHKEY_DIR "/home/hpc-now/.now-ssh"
#define NOW_CRYPTO_EXEC "/usr/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/usr/.hpc-now/.bin/terraform.exe"
#define LOCATION_CONF_FILE "/usr/.hpc-now/.etc/locations.conf"
#define ALI_TF_PLUGIN_VERSION "1.199.0"
#define QCLOUD_TF_PLUGIN_VERSION "1.79.12"
#define AWS_TF_PLUGIN_VERSION "4.56.0"
#define MD5_TF_EXEC "9777407ccfce2be14fe4bec072af4738"
#define MD5_NOW_CRYPTO "26ae6fb1a741dcb8356b650b0812710c"
#define MD5_ALI_TF "52a7b48f682a79909fc122b4ec3afc3e"
#define MD5_QCLOUD_TF "65740525e092fa6abf89386855594217"
#define MD5_AWS_TF "c200d65e3301456524a40ae32ddf4eae"
#define MD5_ALI_TF_ZIP "14b6a80e77b5b8a7ef0a16a40df344cc"
#define MD5_QCLOUD_TF_ZIP "2a08a0092162ba4cf2173be962654b6c"
#define MD5_AWS_TF_ZIP "c6281e969b9740c69f6c5164e87900f4"
#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-windows.exe"

#elif __APPLE__
#include <sys/time.h>
#define CRYPTO_KEY_FILE "/Applications/.hpc-now/.now_crypto_seed.lock" // This is a global file!
#define USAGE_LOG_FILE "/Applications/.hpc-now/.now-cluster-usage.log" // This is a global file!
#define OPERATION_LOG_FILE "/Applications/.hpc-now/.now-cluster-operation.log"
#define NOW_LIC_DIR "/Users/hpc-now/LICENSES"
#define SSHKEY_DIR "/Users/hpc-now/.now-ssh"
#define NOW_CRYPTO_EXEC "/Applications/.hpc-now/.bin/now-crypto.exe"
#define TERRAFORM_EXEC "/Applications/.hpc-now/.bin/terraform"
#define LOCATION_CONF_FILE "/Applications/.hpc-now/.etc/locations.conf"
#define ALI_TF_PLUGIN_VERSION "1.199.0"
#define QCLOUD_TF_PLUGIN_VERSION "1.79.12"
#define AWS_TF_PLUGIN_VERSION "4.56.0"
#define MD5_TF_EXEC "821bf13764e8afbc0fb73a73e25aebad"
#define MD5_NOW_CRYPTO "202082eac600db6f6f429a1ceb047044"
#define MD5_ALI_TF "327c71c64bf913c0e6d90cd7b1a15d41"
#define MD5_QCLOUD_TF "cdb4d3b08328f1dbcbb2df294351f399"
#define MD5_AWS_TF "4086f1c70b04ebc43d5a58d56021fc81"
#define MD5_ALI_TF_ZIP "0e23e305aa0d6962a87f3013a1607ae9"
#define MD5_QCLOUD_TF_ZIP "5ea4e09ae46602959e40c09acd21b4e2"
#define MD5_AWS_TF_ZIP "463fb946564c91965d58d38e085ebc35"
#define DEFAULT_URL_NOW_CRYPTO "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/now-crypto-darwin.exe"

#endif

#define CMDLINE_LENGTH 2048
#define CLUSTER_ID_LENGTH_MAX 24
#define CLUSTER_ID_LENGTH_MIN 8
#define DIR_LENGTH 256
#define FILENAME_LENGTH 512
#define LOCATION_LENGTH 512
#define LINE_LENGTH 4096 //It has to be very long, because tfstate file may contain very long line
#define AKSK_LENGTH 128
#define CONF_STRING_LENTH 64
#define URL_LICENSE "https://gitee.com/zhenrong-wang/hpc-now/raw/master/LICENSE"
#define URL_LICENSE_FSF "https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt"
#define PASSWORD_LENGTH 19
#define PASSWORD_STRING_LENGTH 20
#define RANDSTR_LENGTH_PLUS 11
#define AWS_SLEEP_TIME_GLOBAL 180
#define AWS_SLEEP_TIME_CN 180
#define ALI_SLEEP_TIME 60
#define QCLOUD_SLEEP_TIME 20
#define GENERAL_SLEEP_TIME 30
#define MAXIMUM_ADD_NODE_NUMBER 16 // You can modify this number to adding more than 16 nodes once
#define MAXIMUM_WAIT_TIME 600
#define DEFAULT_URL_REPO_ROOT "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/"
#define DEFAULT_URL_ALICLOUD_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-alicloud/"
#define DEFAULT_URL_AWS_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-aws/"
#define DEFAULT_URL_QCLOUD_ROOT "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/tf-templates-qcloud/"
#define DEFAULT_URL_SHELL_SCRIPTS "https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/scripts/"

#endif