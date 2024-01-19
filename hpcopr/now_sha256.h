/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_SHA256_H
#define NOW_SHA256_H

typedef unsigned char uint_8bit;
typedef unsigned short uint_16bit;
typedef unsigned int uint_32bit;
typedef unsigned long long int uint_64bit;

#define rot_right(a,n) (((a)>>(n))|((a)<<(32-(n))))

void state_init_sha256(uint_32bit state[]);
void padding_length_sha256(uint_8bit* ptr, uint_64bit length_64bit);


#endif