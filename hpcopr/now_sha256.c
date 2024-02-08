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
#include <features.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "now_sha256.h"

uint_8bit padding_sha256[64]={
    0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

uint_32bit k[64]={
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void state_init_sha256(uint_32bit state[]){
    state[0]=0x6a09e667;
    state[1]=0xbb67ae85;
    state[2]=0x3c6ef372;
    state[3]=0xa54ff53a;
    state[4]=0x510e527f;
    state[5]=0x9b05688c;
    state[6]=0x1f83d9ab;
    state[7]=0x5be0cd19;
}

void padding_length_sha256(uint_8bit* ptr, uint_64bit length_64bit){
    for(uint_8bit i=0;i<8;i++){
        *(ptr+i)=(length_64bit>>(56-i*8))&0xFF;
    }
}

void generate_words(uint_32bit w_array[], uint_8bit raw_512bit[]){
    uint_8bit j;
    for(j=0;j<16;j++){
        w_array[j]=(raw_512bit[4*j]<<24)|(raw_512bit[4*j+1]<<16)|(raw_512bit[4*j+2]<<8)|(raw_512bit[4*j+3]);
    }
    for(j=16;j<64;j++){
        w_array[j]=sigma_small1(w_array[j-2])+w_array[j-7]+sigma_small0(w_array[j-15])+w_array[j-16];
    }
}

void now_sha256_core(uint_32bit state[], uint_8bit raw_512bit[]){
    uint_32bit words_array[64];
    uint_8bit j;
    uint_32bit a=state[0];
    uint_32bit b=state[1];
    uint_32bit c=state[2];
    uint_32bit d=state[3];
    uint_32bit e=state[4];
    uint_32bit f=state[5];
    uint_32bit g=state[6];
    uint_32bit h=state[7];
    uint_32bit T1,T2;
    generate_words(words_array,raw_512bit);
    for(j=0;j<64;j++){
        T1=h+sigma_big1(e)+ch(e,f,g)+k[j]+words_array[j];
        T2=sigma_big0(a)+maj(a,b,c);
        h=g;
        g=f;
        f=e;
        e=d+T1;
        d=c;
        c=b;
        b=a;
        a=T1+T2;
    }
    state[0]+=a;
    state[1]+=b;
    state[2]+=c;
    state[3]+=d;
    state[4]+=e;
    state[5]+=f;
    state[6]+=g;
    state[7]+=h;
}

int state_to_sha256_string(uint_32bit state[], char sha256_string[], uint_8bit sha256_len){
    uint_8bit i,j,k,temp;
    if(sha256_len<65){
        return -3;
    }
    memset(sha256_string,'\0',sha256_len);
    for(i=0;i<64;i++){
        j=i>>3;
        k=i%8;
        temp=(state[j]>>(28-4*k))&0x0F;
        if(temp>9){
            sha256_string[i]='a'+temp-10;
        }
        else{
            sha256_string[i]='0'+temp-0;
        }
    }
    return 0;
}

int now_sha256_for_file(char* input_file, char sha256_string[], int sha256_len){
    if(sha256_len<65){
        return -3;
    }
    FILE* file_p=fopen(input_file,"rb");
    if(file_p==NULL){
        return -1;
    }
    uint_32bit state[8];
    uint_8bit buffer_8bit[64];
    uint_8bit i;
    uint_8bit read_byte=0;
    int_64bit total_length_byte=0;
    uint_32bit buffer_blocks=0;
    uint_32bit buffer_block_length=0;
    uint_8bit* buffer_block_ptr=NULL;
    uint_32bit buffer_read;
    int final_flag=0;
    int break_flag;
    state_init_sha256(state);
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
        if(total_length_byte>FILEIO_BUFFER_SIZE_SHA*(buffer_blocks+1)){
            buffer_block_length=FILEIO_BUFFER_SIZE_SHA;
        }
        else{
            buffer_block_length=total_length_byte-FILEIO_BUFFER_SIZE_SHA*buffer_blocks;
            final_flag=1;
        }
        buffer_blocks++;
        buffer_block_ptr=(uint_8bit*)malloc(sizeof(uint_8bit)*buffer_block_length);
        if(buffer_block_ptr==NULL){
            fclose(file_p);
            return -7;
        }
        /* Initialize the break_flag to 0 (not final buffer block) or 1 (final buffer block)*/
        break_flag=final_flag; 
        /* Read a block < = 65536 byte */
        fread(buffer_block_ptr,sizeof(uint_8bit),buffer_block_length,file_p);
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
                padding_length_sha256(buffer_8bit+56,(total_length_byte<<3)&0xFFFFFFFF);
                now_sha256_core(state,buffer_8bit);
                break_flag=3; /* Exit without an extra round. */
                break;
            }
            else if(read_byte<64){
                buffer_8bit[read_byte]=0x80;
                for(i=read_byte+1;i<64;i++){
                    buffer_8bit[i]=0x00;
                }
                now_sha256_core(state,buffer_8bit);
                break_flag=5; /* Exit with an extra round. */
                break;
            }
            else{
                now_sha256_core(state,buffer_8bit);
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
        memcpy(buffer_8bit,padding_sha256,64);
        padding_length_sha256(buffer_8bit+56,(total_length_byte<<3)&0xFFFFFFFF);
        if(break_flag==5){
            buffer_8bit[0]=0x00;
        }
        now_sha256_core(state,buffer_8bit);
    }
    state_to_sha256_string(state,sha256_string,sha256_len);
    return 0;
}

/*
int main(){
    char a[65]="";
    now_sha256_for_file("test.txt",a,65);
    printf("%s\n",a);
    return 0;
}*/