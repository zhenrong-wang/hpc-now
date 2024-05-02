/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_SHA256_H
#define NOW_SHA256_H

#include <stdint.h>

#define FILEIO_BUFFER_SIZE_SHA 67108864 /* 64MB maximum memory buffer*/
#define s_rot_right(a,n) ((a>>n)|(a<<(32-n)))
#define rot_right(a,n) (a>>n)

#define ch(x,y,z) ((x&y)^((~x)&z))
#define maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define sigma_big0(x) (s_rot_right(x,2)^s_rot_right(x,13)^s_rot_right(x,22))
#define sigma_big1(x) (s_rot_right(x,6)^s_rot_right(x,11)^s_rot_right(x,25))
#define sigma_small0(x) (s_rot_right(x,7)^s_rot_right(x,18)^rot_right(x,3))
#define sigma_small1(x) (s_rot_right(x,17)^s_rot_right(x,19)^rot_right(x,10))

void print_buffer(uint8_t buffer_8bit[]);
void state_init_sha256(uint32_t state[]);
void padding_length_sha256(uint8_t* ptr, uint64_t length_64bit);
void generate_words(uint32_t w_array[], uint8_t raw_512bit[]);
void now_sha256_core(uint32_t state[], uint8_t raw_512bit[]);
int state_to_sha256_string(uint32_t state[], char sha256_string[], uint8_t sha256_len);
int now_sha256_for_file(char* input_file, char sha256_string[], int sha256_len);

#endif