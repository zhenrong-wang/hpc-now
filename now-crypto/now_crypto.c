/*
This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
It is distributed under the license: GNU Public License - v2.0
Bug report: info@hpc-now.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int file_encryption(FILE* orig_file, FILE* target_file, int encrypt_key){
  int real_key=((encrypt_key%1000*17+1301)%100+19)*7%100;
  int origc=' ';
  int newc=' ';
  if(orig_file==NULL || target_file==NULL ){
    return 1;
  }
  origc=fgetc(orig_file);
  while(origc!=EOF){
    newc=origc+real_key;
    fputc(newc,target_file);
    origc=fgetc(orig_file);
  }
  return 0;
}

int file_decryption(FILE* orig_file, FILE* target_file, int encrypt_key){
  int real_key=((encrypt_key%1000*17+1301)%100+19)*7%100;
  int origc=' ';
  int newc=' ';
  if(orig_file==NULL || target_file==NULL ){
    return 1;
  }
  origc=fgetc(orig_file);
  while(origc!=EOF){
    newc=origc-real_key;
    fputc(newc,target_file);
    origc=fgetc(orig_file);
  }
  return 0;
}

int md5convert(char* md5string){
  int length=strlen(md5string);
  int i,sum=0;
  if(length!=32){
    return -1;
  }
  for(i=0;i<length;i++){
    sum+=*(md5string+i);
  }
  sum=((sum*13+17)*19+31)*37+41;
  return sum;
}

int main(int argc,char *argv[]){
  FILE* orig_file=NULL;
  FILE* target_file=NULL;
  if(argc!=5){
    printf("Error: Not Enough parameters.\n"); 
    return -1;
  }
  if(strcmp(argv[1],"encrypt")!=0&&strcmp(argv[1],"decrypt")!=0){
    printf("Error: Option Error.\n");
    return -2;
  }
  if(md5convert(argv[4])==-1){
    printf("Error: Not a valid crypto-key.\n");
    return -3;
  }
  orig_file=fopen(argv[2],"r");
  if(orig_file==NULL){
    printf("Error: Cannot open original file.\n");
    return -5;
  }
  target_file=fopen(argv[3],"w+");
  if(target_file==NULL){
    printf("Error: Cannot create target file.\n");
    return -5;
  }
  if(strcmp(argv[1],"encrypt")==0){
    file_encryption(orig_file,target_file,md5convert(argv[4]));
  }
  else{
    file_decryption(orig_file,target_file,md5convert(argv[4]));
  }
  fclose(orig_file);
  fclose(target_file);
  return 0; 
}