/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef OPR_CRYPTO_H
#define OPR_CRYPTO_H

int encrypt_decrypt_clusters(char* cluster_list, char* option, int chmod_flag, int batch_flag_local);
int decrypt_single_cluster(char* target_cluster_name, char* now_crypto_exec, char* crypto_keyfile); //Decrypt sensitive files of a cluster

#endif