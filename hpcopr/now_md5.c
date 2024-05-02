/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#ifndef __APPLE__
#include <malloc.h>
#endif
#ifdef __linux__
#define __USE_LARGEFILE64 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "now_md5.h"

uint8_t padding[64]={
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void state_init(uint32_t state[]){
    state[0]=0x67452301;
    state[1]=0xefcdab89;
    state[2]=0x98badcfe;
    state[3]=0x10325476;
}

void padding_length(uint8_t* ptr, uint64_t length_64bit){
    for(uint8_t i=0;i<8;i++){
        *(ptr+i)=(length_64bit>>(i*8))&0xFF;
    }
}

/* 
 * transform 64*8 buffer to 32*16 buffer
 * so that the core function can use the 32bit buffer directly
 */
void assemb_buffer32(uint8_t buffer_8bit[], uint32_t buffer_32bit[]){
    uint8_t* ptr=buffer_8bit;
    for(uint8_t i=0;i<16;i++){
        buffer_32bit[i]=(*(ptr))|((*(ptr+1))<<8)|((*(ptr+2))<<16)|(*(ptr+3)<<24);
        ptr+=4;
    }
}

/* state[4], buffer[16], 64 rounds of operation */
int now_md5_core_transform(uint32_t state[], uint32_t buffer[]){
    uint32_t a=state[0];
    uint32_t b=state[1];
    uint32_t c=state[2];
    uint32_t d=state[3];

    /* Round 1 */
    a=FF(a,b,c,d,buffer[0],7,0xd76aa478);
    d=FF(d,a,b,c,buffer[1],12,0xe8c7b756);
    c=FF(c,d,a,b,buffer[2],17,0x242070db);
    b=FF(b,c,d,a,buffer[3],22,0xc1bdceee);
    a=FF(a,b,c,d,buffer[4],7,0xf57c0faf);
    d=FF(d,a,b,c,buffer[5],12,0x4787c62a);
    c=FF(c,d,a,b,buffer[6],17,0xa8304613);
    b=FF(b,c,d,a,buffer[7],22,0xfd469501);
    a=FF(a,b,c,d,buffer[8],7,0x698098d8); 
    d=FF(d,a,b,c,buffer[9],12,0x8b44f7af);
    c=FF(c,d,a,b,buffer[10],17,0xffff5bb1);
    b=FF(b,c,d,a,buffer[11],22,0x895cd7be);
    a=FF(a,b,c,d,buffer[12],7,0x6b901122);
    d=FF(d,a,b,c,buffer[13],12,0xfd987193);
    c=FF(c,d,a,b,buffer[14],17,0xa679438e);
    b=FF(b,c,d,a,buffer[15],22,0x49b40821);

    /* Round 2 */
    a=GG(a,b,c,d,buffer[1],5,0xf61e2562);
    d=GG(d,a,b,c,buffer[6],9,0xc040b340);
    c=GG(c,d,a,b,buffer[11],14,0x265e5a51);
    b=GG(b,c,d,a,buffer[0],20,0xe9b6c7aa);
    a=GG(a,b,c,d,buffer[5],5,0xd62f105d);
    d=GG(d,a,b,c,buffer[10],9,0x2441453);
    c=GG(c,d,a,b,buffer[15],14,0xd8a1e681);
    b=GG(b,c,d,a,buffer[4],20,0xe7d3fbc8);
    a=GG(a,b,c,d,buffer[9],5,0x21e1cde6);
    d=GG(d,a,b,c,buffer[14],9,0xc33707d6);
    c=GG(c,d,a,b,buffer[3],14,0xf4d50d87);
    b=GG(b,c,d,a,buffer[8],20,0x455a14ed);
    a=GG(a,b,c,d,buffer[13],5,0xa9e3e905);
    d=GG(d,a,b,c,buffer[2],9,0xfcefa3f8);
    c=GG(c,d,a,b,buffer[7],14,0x676f02d9);
    b=GG(b,c,d,a,buffer[12],20,0x8d2a4c8a);

    /* Round 3 */
    a=HH(a,b,c,d,buffer[5],4,0xfffa3942);
    d=HH(d,a,b,c,buffer[8],11,0x8771f681);
    c=HH(c,d,a,b,buffer[11],16,0x6d9d6122);
    b=HH(b,c,d,a,buffer[14],23,0xfde5380c);
    a=HH(a,b,c,d,buffer[1],4,0xa4beea44);
    d=HH(d,a,b,c,buffer[4],11,0x4bdecfa9);
    c=HH(c,d,a,b,buffer[7],16,0xf6bb4b60);
    b=HH(b,c,d,a,buffer[10],23,0xbebfbc70);
    a=HH(a,b,c,d,buffer[13],4,0x289b7ec6);
    d=HH(d,a,b,c,buffer[0],11,0xeaa127fa);
    c=HH(c,d,a,b,buffer[3],16,0xd4ef3085);
    b=HH(b,c,d,a,buffer[6],23,0x4881d05);
    a=HH(a,b,c,d,buffer[9],4,0xd9d4d039);
    d=HH(d,a,b,c,buffer[12],11,0xe6db99e5);
    c=HH(c,d,a,b,buffer[15],16,0x1fa27cf8);
    b=HH(b,c,d,a,buffer[2],23,0xc4ac5665);

    /* Round 4 */
    a=II(a,b,c,d,buffer[0],6,0xf4292244);
    d=II(d,a,b,c,buffer[7],10,0x432aff97);
    c=II(c,d,a,b,buffer[14],15,0xab9423a7);
    b=II(b,c,d,a,buffer[5],21,0xfc93a039);
    a=II(a,b,c,d,buffer[12],6,0x655b59c3);
    d=II(d,a,b,c,buffer[3],10,0x8f0ccc92);
    c=II(c,d,a,b,buffer[10],15,0xffeff47d);
    b=II(b,c,d,a,buffer[1],21,0x85845dd1);
    a=II(a,b,c,d,buffer[8],6,0x6fa87e4f);
    d=II(d,a,b,c,buffer[15],10,0xfe2ce6e0);
    c=II(c,d,a,b,buffer[6],15,0xa3014314);
    b=II(b,c,d,a,buffer[13],21,0x4e0811a1);
    a=II(a,b,c,d,buffer[4],6,0xf7537e82);
    d=II(d,a,b,c,buffer[11],10,0xbd3af235);
    c=II(c,d,a,b,buffer[2],15,0x2ad7d2bb);
    b=II(b,c,d,a,buffer[9],21,0xeb86d391);
    
    state[0]+=a;
    state[1]+=b;
    state[2]+=c;
    state[3]+=d;

    return 0;
}

void state_to_md5array(uint32_t state[], uint8_t md5_array[]){
    uint8_t i;
    for(i=0;i<4;i++){
        md5_array[i*4]=state[i]&0xFF;
        md5_array[i*4+1]=(state[i]>>8)&0xFF;
        md5_array[i*4+2]=(state[i]>>16)&0xFF;
        md5_array[i*4+3]=(state[i]>>24)&0xFF;
    }
}

/* Convert the lowest 4-bit to a char */
char hex_4bit_to_char(uint8_t hex_4bit){
    if((hex_4bit&0x0F)>9){
        return hex_4bit-10+'a';
    }
    else{
        return hex_4bit+'0';
    }
}

int md5_array_to_string(uint8_t md5_array[], char md5sum_string[], int md5sum_len){
    if(md5sum_len<33){ /* There should be a '\0' and the end, so the length should be > 32 */
        return -1;
    }
    uint8_t i;
    for(i=0;i<16;i++){
        md5sum_string[i*2]=hex_4bit_to_char((md5_array[i]>>4)&0x0F);
        md5sum_string[i*2+1]=hex_4bit_to_char((md5_array[i])&0x0F);
    }
    return 0;
}

/*
 * return -3: The given length is incorrect
 * return -1: Failed to open the input file
 * return -5: Failed to read the input file
 * return 0 : Successfully get the md5
 */
int now_md5_for_file(char* input_file, char md5sum_string[], int md5sum_len){
    if(md5sum_len<33){
        return -3;
    }
    FILE* file_p=fopen(input_file,"rb");
    if(file_p==NULL){
        return -1;
    }
    uint32_t state[4];
    uint8_t buffer_8bit[64];
    uint32_t buffer_32bit[16];
    uint8_t md5_array[16];
    uint8_t i;
    uint8_t read_byte=0;
    int64_t total_length_byte=0;
    uint32_t buffer_blocks=0;
    uint32_t buffer_block_length=0;
    uint8_t* buffer_block_ptr=NULL;
    uint32_t buffer_read;
    int final_flag=0;
    int break_flag;
    state_init(state);
    fseek(file_p,0L,SEEK_END);
#ifdef _WIN32
    total_length_byte=_ftelli64(file_p);
#else
    total_length_byte=ftello(file_p);
#endif
    rewind(file_p);
    if(total_length_byte<0){
        fclose(file_p);
        return -1;
    }
    while(final_flag==0){
        if(total_length_byte>FILEIO_BUFFER_SIZE*(buffer_blocks+1)){
            buffer_block_length=FILEIO_BUFFER_SIZE;
        }
        else{
            buffer_block_length=total_length_byte-FILEIO_BUFFER_SIZE*buffer_blocks;
            final_flag=1;
        }
        buffer_blocks++;
        buffer_block_ptr=(uint8_t*)malloc(sizeof(uint8_t)*buffer_block_length);
        if(buffer_block_ptr==NULL){
            fclose(file_p);
            return -7;
        }
        /* Initialize the break_flag to 0 (not final buffer block) or 1 (final buffer block)*/
        break_flag=final_flag; 
        /* Read a block < = 65536 byte */
        fread(buffer_block_ptr,sizeof(uint8_t),buffer_block_length,file_p);
        buffer_read=0; /* Reset buffer_read to 0*64 bytes */
        while(buffer_read<buffer_block_length){
            if(final_flag==0){ /* If not the final buffer block, each reay_byte is 64 */
                read_byte=64;
            }
            else{
                if(buffer_block_length-buffer_read>64){
                    read_byte=64;
                }
                else{
                    read_byte=buffer_block_length-buffer_read;
                }
            }
            /* read_byte can only be [1-64] */
            memcpy(buffer_8bit,buffer_block_ptr+buffer_read,read_byte);
            buffer_read+=64;
            if(read_byte<56){
                buffer_8bit[read_byte]=0x80;
                for(i=read_byte+1;i<56;i++){
                    buffer_8bit[i]=0x00;
                }
                padding_length(buffer_8bit+56,(total_length_byte<<3)&0xFFFFFFFF);
                assemb_buffer32(buffer_8bit,buffer_32bit);
                now_md5_core_transform(state,buffer_32bit);
                break_flag=3; /* Exit without an extra round. */
                break;
            }
            else if(read_byte<64){
                buffer_8bit[read_byte]=0x80;
                for(i=read_byte+1;i<64;i++){
                    buffer_8bit[i]=0x00;
                }
                assemb_buffer32(buffer_8bit,buffer_32bit);
                now_md5_core_transform(state,buffer_32bit);
                break_flag=5; /* Exit with an extra round. */
                break;
            }
            else{
                assemb_buffer32(buffer_8bit,buffer_32bit);
                now_md5_core_transform(state,buffer_32bit);
            }
        }
        free(buffer_block_ptr);
        /* break_flag could be 0, 1, 3, 5 */
        if(break_flag!=0){
            break;
        }
    }
    fclose(file_p);
    if(break_flag!=3){
        memcpy(buffer_8bit,padding,64);
        padding_length(buffer_8bit+56,(total_length_byte<<3)&0xFFFFFFFF);
        if(break_flag==5){
            buffer_8bit[0]=0x00;
        }
        assemb_buffer32(buffer_8bit,buffer_32bit);
        now_md5_core_transform(state,buffer_32bit);
    }
    state_to_md5array(state,md5_array);
    md5_array_to_string(md5_array,md5sum_string,md5sum_len);
    return 0;
}

/*int main(int argc, char** argv){
    char md5_string[64]="";
    now_md5_for_file("now_macros.h",md5_string,33);
    printf("%s\n",md5_string);
    return 0;
}*/