/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

/*
 * This is an AES implementation for this project. The method here is ECB (Electronic Codebook Book).
 * ECB is not quite secure, compare to CBC.
 * Encryption/Decryption is very important in any scenario processing sensitive infomation
 * The code here is implemented based on FIPS-197 https://csrc.nist.gov/pubs/fips/197/final
 * Without comprehensive validation, please *DO NOT* use the code in your project!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define CRYPTO_VERSION "0.3.0"

typedef unsigned char uint_8bit;
typedef unsigned short uint_16bit;
typedef unsigned int uint_32bit;
typedef unsigned long int uint_64bit;

//Each expanded key is in format of 0xAABBCCDD, so 4X8bit=32bit
typedef struct{
    uint_32bit encryption_key[44];
    int expansion_round;
} now_aes_key;

const uint_8bit s_box[256]={
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

const uint_8bit inv_s_box[256]={
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

const uint_32bit round_con[10]={
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000
};

const uint_8bit MixColumnMatrix[4][4] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
};

const uint_8bit InvMixColumnMatrix[4][4] = {
    {0x0E, 0x0B, 0x0D, 0x09},
    {0x09, 0x0E, 0x0B, 0x0D},
    {0x0D, 0x09, 0x0E, 0x0B},
    {0x0B, 0x0D, 0x09, 0x0E}
};

//Get an 8-bit(1 byte) from a given 32 bit number.
uint_8bit get_byte(uint_32bit a, unsigned int n){
    return (a>>(8*n))&0xFF;
}

//key=128bit, stored in an array width 16; so the element of the key array is 8bit each;
//key_length should be less than 16;
int key_expansion(uint_8bit* key, uint_8bit key_length, now_aes_key* AES_key){
    int i,j;
    uint_32bit a,b;
    uint_8bit c,d,e,f;
    if(key_length!=16){
        return -1; //Illegal length
    }
    for(i=0;i<4;i++){
        AES_key->encryption_key[i]&=0x00000000;
        for(j=0;j<4;j++){
            AES_key->encryption_key[i]+=((*(key+i*4+j)&0xFF)<<(24-j*8)); //Push the 4 0x numbers to a 32bit expanded key.
        }
    }

    //Generate the other 40 expanded keys
    for(i=4;i<44;i++){
        AES_key->encryption_key[i]&=0x00000000;
        a=AES_key->encryption_key[i-4];
        b=AES_key->encryption_key[i-1];
        if(i%4!=0){
            AES_key->encryption_key[i]=a^b;
        }
        else{
            c=s_box[get_byte(b,3)]; //Get the numbers from S-Box
            d=s_box[get_byte(b,2)];
            e=s_box[get_byte(b,1)];
            f=s_box[get_byte(b,0)];
            AES_key->encryption_key[i]=a^(((d<<24)&0xFF000000)^((e<<16)&0xFF0000)^((f<<8)&0xFF00)^(c&0xFF))^round_con[i/4-1]; //Push the numbers to 32bit number with rotation.
        }
    }
    return 0;
}

//The key is an array with 4 32-bit numbers, aka, w[i]
void AddRoundKey(uint_8bit (*state)[4], uint_32bit *key){
    int i,j;
    for(j=0;j<4;j++){
        for(i=0;i<4;i++){
            state[i][j]^=get_byte(key[j],3-i);
        }
    }
}

//Assemble a 32bit number from 4 8bit numbers
uint_32bit assem_row(uint_8bit* short_nums){
    return ((short_nums[0]<<24)&0xFF000000)^((short_nums[1]<<16)&0xFF0000)^((short_nums[2]<<8)&0xFF00)^((short_nums[3])&0xFF); //Push 4 8-bit integers to a 32-bit integer.
}

//Deassemble a 32bit number into 4 8bit numbers
void deassem_row(uint_32bit a, uint_8bit* short_nums){
    for(int i=0;i<4;i++){
        short_nums[i]=get_byte(a,3-i);
    }
}

//Rotate the 32-bit number a to another 32-bit number b, direction: left
int rot_left(uint_32bit a, uint_8bit n, uint_32bit* b){
    uint_8bit a3=get_byte(a,3);
    uint_8bit a2=get_byte(a,2);
    uint_8bit a1=get_byte(a,1);
    uint_8bit a0=get_byte(a,0);
    if(n==0){
        *b=a; //If no rotation, just copy the number and return 0
        return 0;
    }
    else if(n==1){
        *b=((a2<<24)&0xFF000000)^((a1<<16)&0xFF0000)^((a0<<8)&0xFF00)^(a3&0xFF); //Push the bytes to b, with rotation 1
        return 0;
    }
    else if(n==2){
        *b=((a1<<24)&0xFF000000)^((a0<<16)&0xFF0000)^((a3<<8)&0xFF00)^(a2&0xFF); //Push the bytes to b, with rotation 2
        return 0;
    }
    else if(n==3){
        *b=((a0<<24)&0xFF000000)^((a3<<16)&0xFF0000)^((a2<<8)&0xFF00)^(a1&0xFF); //Push the bytes to b, with rotation 3
        return 0;
    }
    else{
        return -1; //Make sure the rotation rounds<3
    }
}

//Right-direction rotation is a reverse rotation to left.
int rot_right(uint_32bit a, uint_8bit n, uint_32bit* b){
    return rot_left(a,4-n,b);   
}

int ShiftRows(uint_8bit (*state)[4]){
    int i;
    uint_32bit a,b;
    for(i=1;i<4;i++){
        a=assem_row(state[i]);
        if(rot_left(a,i,&b)!=0){
            return -1;
        }
        deassem_row(b,state[i]);
    }
    return 0;
}

int InvShiftRows(uint_8bit (*state)[4]){
    int i;
    uint_32bit a,b;
    for(i=1;i<4;i++){
        a=assem_row(state[i]);
        if(rot_right(a,i,&b)!=0){
            return -1;
        }
        deassem_row(b,state[i]);
    }
    return 0;
}

uint_8bit GaloisMultiple2(uint_8bit a){
    uint_8bit flag=(a>>7)&0x01;
    if(flag==1){
        return ((a<<1)&0xFF)^0x1B;
    }
    else{
        return (a<<1)&0xFF;
    }
}

uint_8bit GaloisMultipleGeneral(uint_8bit a, uint_8bit b){
    uint_8bit c=0x00;
    int i;
    uint_8bit flag;
    for(i=0;i<8;i++){
        flag=b&0x01; //If the last bit is 1, then xor a, otherwise keep the result unchanged.
        if(flag==1){
            c^=a;
        }
        a=GaloisMultiple2(a);
        b>>=1;
    }
    return c;
}

void SubBytes(uint_8bit (*state)[4]){
    int i,j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            state[i][j]=s_box[state[i][j]];
        }
    }
}

void InvSubBytes(uint_8bit (*state)[4]){
    int i,j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            state[i][j]=inv_s_box[state[i][j]];
        }
    }
}

void MixColumns(uint_8bit (*state)[4]){
    uint_8bit temp[4][4];
    uint_8bit s0,s1,s2,s3;
    int i,j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            temp[i][j]=state[i][j];
        }
    }
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            s0=GaloisMultipleGeneral(MixColumnMatrix[i][0],temp[0][j]);
            s1=GaloisMultipleGeneral(MixColumnMatrix[i][1],temp[1][j]);
            s2=GaloisMultipleGeneral(MixColumnMatrix[i][2],temp[2][j]);
            s3=GaloisMultipleGeneral(MixColumnMatrix[i][3],temp[3][j]);
            state[i][j]=s0^s1^s2^s3;
        }
    }
}

void InvMixColums(uint_8bit (*state)[4]){
    int i,j;
    uint_8bit temp[4][4];
    uint_8bit s0,s1,s2,s3;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            temp[i][j]=state[i][j];
        }
    }
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            s0=GaloisMultipleGeneral(InvMixColumnMatrix[i][0],temp[0][j]);
            s1=GaloisMultipleGeneral(InvMixColumnMatrix[i][1],temp[1][j]);
            s2=GaloisMultipleGeneral(InvMixColumnMatrix[i][2],temp[2][j]);
            s3=GaloisMultipleGeneral(InvMixColumnMatrix[i][3],temp[3][j]);
            state[i][j]=s0^s1^s2^s3;
        }
    }
}

void print_state(uint_8bit (*state)[4]){
    int i,j;
    printf("\n");
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            printf("%x ",state[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

//Improved, move the key_expansion out of the encryption process
int now_AES_ECB_encryption(uint_8bit (*state)[4], uint_8bit (*out)[4], now_aes_key* AES_key){
    if(state==NULL||out==NULL||AES_key==NULL){
        return -1;
    }
    int i,j;
    uint_32bit* key_pointer=AES_key->encryption_key;
    AddRoundKey(state,key_pointer);
    for(i=1;i<10;i++){
        key_pointer+=4;
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state,key_pointer);
    }
    key_pointer+=4;
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state,key_pointer);
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            out[i][j]=state[i][j];
        }
    }
    return 0;
}

int now_AES_ECB_decryption(uint_8bit (*state)[4], uint_8bit (*out)[4], now_aes_key* AES_key){
    if(AES_key==NULL||state==NULL||out==NULL){
        return -1;
    }
    int i,j;
    uint_32bit* key_pointer=AES_key->encryption_key+40;
    AddRoundKey(state,key_pointer);
    for(i=1;i<10;i++){
        key_pointer-=4;
        InvShiftRows(state);
        InvSubBytes(state);
        AddRoundKey(state,key_pointer);
        InvMixColums(state);
    }
    key_pointer-=4;
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state,key_pointer);
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            out[i][j]=state[i][j];
        }
    }
    return 0;
}

int get_file_size(char* filename){
    int fd=-1;
    struct stat file_stat;
    fd=open(filename,O_RDONLY);
    if(fd==-1){
        return -1;
    }
    if(fstat(fd,&file_stat)==-1){
        close(fd);;
        return -1;
    }
    close(fd);
    return file_stat.st_size;
}

uint_8bit get_padding_num(char* filename){
    int flag=get_file_size(filename);
    if(flag==-1){
        return -1;
    }
    return 16-flag%16;
}

//Read 128bit per time.
//return -1: FILE I/O error
//return 1: read 0 bytes
//return 3: Padding and file end
//return 0: continue reading
int byte_read_encryption(FILE* file_p, uint_8bit (*state)[4]){
    uint_8bit buffer[16]={0x00};
    uint_8bit read_flag;
    uint_8bit padding_num;
    uint_8bit flag=0;
    uint_8bit* pt_head=state[0];
    uint_8bit* pt;
    int i,j;
    if(file_p==NULL){
        return -1;
    }
    read_flag=fread(buffer,sizeof(uint_8bit),16,file_p)^0x00; // Read 128 bit
    if(read_flag==0){
        return 1;
    }
    padding_num=16-read_flag;
    i=0;
    flag=0;
    pt=pt_head+i;
    while(flag+padding_num<16){
        *(pt)=*(buffer+flag);
        flag++;
        pt+=4;
        if(flag%4==0){
            i++;
            pt=pt_head+i;
        }
    }
    for(j=0;j<padding_num;j++){
        *(pt)=padding_num;
        pt+=4;
        flag++;
        if(flag%4==0){
            i++;
            pt=pt_head+i;
        }
    }
    if(padding_num!=0){
        return 3;
    }
    return 0;
}

//Write to an encrypted file with paddling 0x00
int byte_write_encryption(FILE* file_p, uint_8bit (*state)[4]){
    int i,j;
    uint_8bit* pt_head=state[0];
    uint_8bit* pt;
    if(file_p==NULL){
        return -1;
    }
    for(i=0;i<4;i++){
        pt=pt_head+i;
        for(j=0;j<4;j++){
            fwrite(pt,1,1,file_p);
            pt+=4;
        }
        pt=state[0]+i;
    }
    return 0;
}

//Read an decrypted file. 128bit per time.
//The decrypted file is already 128bit width
//return -1: FILE I/O error, faild to read the file
//return -3: Abnormal. Not supposed to happen.
//return 0: continue reading without eof
//return 1: Find end found.
int byte_read_decryption(FILE* file_p, uint_8bit (*state)[4]){
    uint_8bit buffer[16]={0x00};
    int i,j;
    int read_flag=0;
    if(file_p==NULL){
        return -1;
    }
    read_flag=fread(buffer,sizeof(uint_8bit),16,file_p); // Read 128 bit
    if(read_flag!=0&&read_flag!=16){
        return -3;
    }
    else if(read_flag==0){
        return 1;
    }
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            state[j][i]=*(buffer+i*4+j);
        }
    }
    return 0; // Continue reading.
}

//Write to an decrypted file
//If last_block_flag==0, then write last block and check padding; else write normal block.
//When the paddling 0x00 found, will exit and report to the upper-level function
//return -1: FILE I/O output error
//return 1*: Write end exit the loop
//return 0: Continue writing. 
int byte_write_decryption(FILE* file_p, uint_8bit (*state)[4], uint_8bit last_block_flag){
    int i,j;
    uint_8bit* pt_head=state[0];
    uint_8bit* pt;
    uint_8bit padding_num;
    uint_8bit flag=0;
    if(file_p==NULL){
        return -1;
    }
    if(last_block_flag==0){
        padding_num=state[3][3]; //Getting the padding number
        i=0;
        pt=pt_head+i;  //Put the pointer to the top of the next column
        while(flag<16-padding_num){
            fwrite(pt,1,1,file_p);
            pt+=4;
            flag++;
            if(flag%4==0){
                i++; //every 4 bytes written, i++
                pt=pt_head+i;
            }
        }
        return 1; // Skip the extra bytes.
    }
    // Output the buffer 4x4 to file_p
    for(i=0;i<4;i++){
        pt=pt_head+i;
        for(j=0;j<4;j++){
            fwrite(pt,1,1,file_p);
            pt+=4;
        }
        pt=state[0]+i;
    }
    return 0;
}

uint_8bit char_to_hex(char x){
    if(x=='0'||x=='9'){
        return x-='0';
    }
    else if(x>'0'&&x<'9'){
        return x-='0';
    }
    else if(x=='A'||x=='F'){
        return x-'A'+10;
    }
    else if(x>'A'&&x<'F'){
        return x-'A'+10;
    }
    else if(x=='a'||x=='f'){
        return x-'a'+10;
    }
    else if(x>'a'&&x<'f'){
        return x-'a'+10;
    }
    else{
        return 255;
    }
}

//convert an MD5(char [32]) to a 128-bit AES key.
int md5convert(char* md5string, uint_8bit* key, uint_8bit key_length){
    int length=strlen(md5string);
    int i;
    uint_8bit a,b;
    if(length!=32){ //If the md5string width is not 32, stop and exit.
        return -1;
    }
    for(i=0;i<32;i++){
        if(md5string[i]<'0'||md5string[i]>'f'){
            return -1;
        }
        else if(md5string[i]>'9'&&md5string[i]<'A'){
            return -1;
        }
        else if(md5string[i]>'F'&&md5string[i]<'a'){
            return -1;
        }
        else{
            continue;
        }
    }
    if(key_length!=16){ //If the key length is incorrect, stop and exit.
        return -3;
    }
    for(i=0;i<16;i++){
        a=char_to_hex(md5string[i<<1])&0x0F; //Get the lower 4 bits
        b=char_to_hex(md5string[(i<<1)^0x01])&0x0F; //Get the lower 4 bits
        if(a==255||b==255){
            return -1;
        }
        key[i]=(a<<4)^b;
    }
    return 0;
}


//return 1: Option incorrect
//return 3: Not a valid MD5 string
//return -1: FILE READ ERROR
//return -3: FILE WRITE ERROR
//return 5: AES failed
//return 0: Normal Exit.
//return -5: Attempt to decrypt a file that is not encrypted
//return 7: Failed to expand the key.
int file_encryption_decryption(char* option, char* orig_file, char* target_file, char* md5_string){
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return 1; //Option incorrect.
    }
    uint_8bit key[16]={0x00};
    int read_flag,write_flag;
    uint_8bit state[4][4]={0x00};
    uint_8bit output[4][4]={0x00};
    now_aes_key AES_key;
    unsigned long decrypt_read_blocks;
    unsigned long i;
    int padding_num=get_padding_num(orig_file);
    if(md5convert(md5_string,key,16)!=0){
        return 3; //Not a valid MD5 String
    }
    if(padding_num==-1){
        return -1;
    }
    if(strcmp(option,"decrypt")==0&&padding_num!=16){ //An AES encrypted file's padding num should be zero.
        return -5;
    }
    FILE* file_p=fopen(orig_file,"rb");
    if(file_p==NULL){
        return -1;
    }
    FILE* file_p_out=fopen(target_file,"wb+");
    if(file_p_out==NULL){
        fclose(file_p);
        return -3;
    }
    if(key_expansion(key,16,&AES_key)!=0){
        fclose(file_p);
        fclose(file_p_out);
        return 7;
    }
    if(strcmp(option,"encrypt")==0){
        while(1){
            read_flag=byte_read_encryption(file_p,state);
            if(read_flag==-1){
                fclose(file_p_out);
                return -1;
            }
            if(read_flag==1){ //If read 0 && padding_num=16, keep going, otherwise, exit
                if(padding_num==16){
                    memset(state,padding_num&0x10,16);
                }
                else{
                    printf("[ -INFO- ] Encryption Done from file %s to file %s.\n",orig_file,target_file);
                    break;
                }
            }
            if(now_AES_ECB_encryption(state,output,&AES_key)!=0){
                fclose(file_p);
                fclose(file_p_out);
                return 5;
            }
            if(byte_write_encryption(file_p_out,output)!=0){
                fclose(file_p);
                return -3;
            }
            if(read_flag==1||read_flag==3){
                printf("[ -INFO- ] Encryption Done from file %s to file %s.\n",orig_file,target_file);
                break;
            }
        }
    }
    else{
        decrypt_read_blocks=get_file_size(orig_file)>>4; //Get the block number, use >> 4 instead of /16
        for(i=0;i<decrypt_read_blocks;i++){
            read_flag=byte_read_decryption(file_p,state);
            if(read_flag==-1||read_flag==-3){
                fclose(file_p_out);
                return -1;
            }
            if(now_AES_ECB_decryption(state,output,&AES_key)!=0){
                fclose(file_p);
                fclose(file_p_out);
                return 5;
            }
            if(i==decrypt_read_blocks-1){
                write_flag=byte_write_decryption(file_p_out,output,0);
            }
            else{
                write_flag=byte_write_decryption(file_p_out,output,1);
            }
            if(write_flag==-1){
                fclose(file_p);
                return -3;
            }
            else if(write_flag==1){
                printf("[ -INFO- ] Decryption Done from file %s to file %s.\n",orig_file,target_file);
                break;
            }
        }
    }
    fclose(file_p);
    fclose(file_p_out);
    return 0;
}

/* 
 * return 1: Not enough parameters: 
 *    +-> Format: now-crypto.exe OPTION ORIGINAL_FILE_PATH TARGET_FILE_PATH MD5_STRING
 * return 5: Option is invalid
 * return 3: Not a valid MD5 string
 * return 7: FILE I/O: read error
 * return 9: FILE I/O: write error
 * return 11: AES error, probably a bug
 * return 13: Not an AES encrypted file.
 * return 15: Failed to expand the key. Key may be invalid.
 * return 0: Normally exit.
 */
int main(int argc,char *argv[]){
    printf("[ -INFO- ] AES-128 ECB crypto module for HPC-NOW. Version: %s\n",CRYPTO_VERSION);
    printf("|          Shanghai HPC-NOW Technologies. All right reserved. License: MIT\n");
    int run_flag=0;
    if(argc!=5){
        printf("[ FATAL: ] Not Enough parameters.\n"); 
        return 1;
    }
    run_flag=file_encryption_decryption(argv[1],argv[2],argv[3],argv[4]);
    if(run_flag==1){
        printf("[ FATAL: ] Option is invalid.\n");
        return 5;
    }
    else if(run_flag==3){
        printf("[ FATAL: ] Not a valid crypto-key.\n");
        return 3;
    }
    else if(run_flag==-1){
        printf("[ FATAL: ] File read error.\n");
        return 7;
    }
    else if(run_flag==-3){
        printf("[ FATAL: ] File write error.\n");
        return 9;
    }
    else if(run_flag==5){
        printf("[ FATAL: ] AES error. Please report this bug.\n");
        return 11;
    }
    else if(run_flag==-5){
        printf("[ FATAL: ] Probably you are not decrypting an AES encrypted file.\n");
        return 13;
    }
    else if(run_flag==7){
        printf("[ FATAL: ] Failed to expand the key.\n");
        return 15;
    }
    else{
        return 0; 
    }
}