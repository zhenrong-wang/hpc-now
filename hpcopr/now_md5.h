/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef NOW_MD5_H
#define NOW_MD5_H

#define FILEIO_BUFFER_SIZE 67108864 /* 64MB maximum memory buffer*/

typedef unsigned char uint_8bit;
typedef unsigned short uint_16bit;
typedef unsigned int uint_32bit;
typedef unsigned long long int uint_64bit;

#define rot_left(a,n) (((a)<<(n))|((a)>>(32-(n))))
#define F(b,c,d) (((b)&(c))|((~b)&(d)))
#define G(b,c,d) (((b)&(d))|((c)&(~d)))
#define H(b,c,d) ((b)^(c)^(d))
#define I(b,c,d) ((c)^((b)|(~d)))

#define FF(a,b,c,d,Mj,s,ti) (b+rot_left(a+F(b,c,d)+Mj+ti,s))
#define GG(a,b,c,d,Mj,s,ti) (b+rot_left(a+G(b,c,d)+Mj+ti,s))
#define HH(a,b,c,d,Mj,s,ti) (b+rot_left(a+H(b,c,d)+Mj+ti,s))
#define II(a,b,c,d,Mj,s,ti) (b+rot_left(a+I(b,c,d)+Mj+ti,s))

void state_init(uint_32bit state[]);
void padding_length(uint_8bit* ptr, uint_64bit length_64bit);
void assemb_buffer32(uint_8bit buffer_8bit[], uint_32bit buffer_32bit[]);
int now_md5_core_transform(uint_32bit state[], uint_32bit buffer[]);
void state_to_md5array(uint_32bit state[], uint_8bit md5_array[]);
char hex_4bit_to_char(uint_8bit hex_4bit);
int md5_array_to_string(uint_8bit md5_array[], char md5sum_string[], int md5sum_len);
int now_md5_for_file(char* input_file, char md5sum_string[], int md5sum_len);

#endif