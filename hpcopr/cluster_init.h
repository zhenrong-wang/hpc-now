/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#ifndef CLUSTER_INIT_H
#define CLUSTER_INIT_H

int aws_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);

int qcloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);

int alicloud_cluster_init(char* cluster_id_input, char* workdir, char* crypto_keyfile);

#endif