/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

/*
 * This is a *psedo* and very *simple* symmetric crypto program! Not a real or strict one. If you'd like to use a real
 * crypto program, try AES, or at least DES. Maybe we will switch to one of them in the future.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CH_BUFFER_MAX 1048576
                      
int read_file(char* filename, int* ch_array){
    int i=0;
    int ch='\0';
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -1;
    }
    while((ch=fgetc(file_p))!=EOF){
        *(ch_array+i)=ch;
        i++;
    }
    *(ch_array+i)='\0';
    fclose(file_p);
    return i;
}

int write_file(char* filename, int* ch_array, int file_length){
    int i;
    FILE* file_p=fopen(filename,"w+");
    if(file_p==NULL){
        return -1;
    }
    for(i=0;i<file_length;i++){
        fputc(*(ch_array+i),file_p);
    }
    fclose(file_p);
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
    sum=(sum*13+17)*37+41;
    return sum;
}

int file_encryption_decryption(char* option, char* orig_file, char* target_file, int encrypt_key){
    if(strcmp(option,"encrypt")!=0&&strcmp(option,"decrypt")!=0){
        return 1;
    }
    int real_key=((encrypt_key%1000*17+1301)%100+19)*7%41;
    int salt_position=real_key%11+2;
    int i=0,j=0,file_length;
    int ch_array_buffer[CH_BUFFER_MAX]={'\0',};
    FILE* filep_orig=fopen(orig_file,"r");
    if(filep_orig==NULL){
        return -1;
    }
    FILE* filep_target=fopen(target_file,"w+");
    if(filep_target==NULL){
        fclose(filep_orig);
        return -1;
    }
    fclose(filep_target);
    file_length=read_file(orig_file,ch_array_buffer);
//    printf("%d,%d,%d,%d\n",file_length,encrypt_key,salt_position,real_key);
    do{
        if(i%salt_position==0){
            if(strcmp(option,"encrypt")==0){
                *(ch_array_buffer+i)+=((i*j)%17+real_key);
            }
            else{
                *(ch_array_buffer+i)-=((i*j)%17+real_key);
            }
            i++;
            j++;
            continue;
        }
        else if(i%2==0){
            if(strcmp(option,"encrypt")==0){
                *(ch_array_buffer+i)+=real_key;
            }
            else{
                *(ch_array_buffer+i)-=real_key;
            }
            i++;
            continue;
        }
        else{
            if(strcmp(option,"encrypt")==0){
                *(ch_array_buffer+i)+=(i%17+real_key);
            }
            else{
                *(ch_array_buffer+i)-=(i%17+real_key);
            }
            i++;
            continue;
        }
    }while(i<file_length);
    fclose(filep_orig);
    write_file(target_file,ch_array_buffer,file_length);
    return 0;
}

int main(int argc,char *argv[]){
    FILE* orig_file=NULL;
    FILE* target_file=NULL;
    if(argc!=5){
        printf("Error: Wrong Parameter Numbers.\n"); 
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
    fclose(orig_file);
    target_file=fopen(argv[3],"w+");
    if(target_file==NULL){
        printf("Error: Cannot create target file.\n");
        return -5;
    }
    fclose(target_file);
    return file_encryption_decryption(argv[1],argv[2],argv[3],md5convert(argv[4]));
}