/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_SHA256_H
#define NOW_SHA256_H

#define FILEIO_BUFFER_SIZE_SHA 67108864 /* 64MB maximum memory buffer*/

/** 
 * Portability risk: The unsigned int does not always has 32 bits. It is *NOT*
 * defined by standards. We assert that on all the target platforms of this 
 * project, that is, x86_64 with Windows/Linux/macOS, the unsigned int is 32 bit.
 * This is an assertion! If you encounter any portability problems, please
 * submit issues to this repository.
 * 
 */
typedef unsigned char uint_8bit;
typedef unsigned int uint_32bit;
typedef unsigned long long int uint_64bit;
typedef signed long long int int_64bit;

#define s_rot_right(a,n) ((a>>n)|(a<<(32-n)))
#define rot_right(a,n) (a>>n)

#define ch(x,y,z) ((x&y)^((~x)&z))
#define maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define sigma_big0(x) (s_rot_right(x,2)^s_rot_right(x,13)^s_rot_right(x,22))
#define sigma_big1(x) (s_rot_right(x,6)^s_rot_right(x,11)^s_rot_right(x,25))
#define sigma_small0(x) (s_rot_right(x,7)^s_rot_right(x,18)^rot_right(x,3))
#define sigma_small1(x) (s_rot_right(x,17)^s_rot_right(x,19)^rot_right(x,10))

void print_buffer(uint_8bit buffer_8bit[]);
void state_init_sha256(uint_32bit state[]);
void padding_length_sha256(uint_8bit* ptr, uint_64bit length_64bit);
void generate_words(uint_32bit w_array[], uint_8bit raw_512bit[]);
void now_sha256_core(uint_32bit state[], uint_8bit raw_512bit[]);
int state_to_sha256_string(uint_32bit state[], char sha256_string[], uint_8bit sha256_len);
int now_sha256_for_file(char* input_file, char sha256_string[], int sha256_len);

#endif