/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

/* 
 * This file contains fundamental and general functions to be used by the 
 * HPC-NOW Project.
 * Including file operations, directory operations, string operations, etc.
 * Any updates of these functions should be carefully checked and 
 * validated because the whole project would be affected.
 */

#ifndef _WIN32
#define _GNU_SOURCE 1
#include <termios.h> /* For terminal operation of the getpass function*/
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h> /* Mingw-w64 with pthread is required. */

#ifdef _WIN32
/* Windows Socket-related headers */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windef.h>
#include <mswsock.h>

#include <windows.h>
#include <io.h>
#include <share.h>
#include <direct.h>
#include <fcntl.h>
#include <fileapi.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <conio.h>
#include <Shlobj.h>

#elif __linux__
#include <features.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>

#elif __APPLE__
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
#endif

/* POSIX Socket-related headers */
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "now_macros.h"
#include "now_md5.h"
#include "now_sha256.h"
#include "general_funcs.h"

typedef struct {
    char* req_domain;
    char* req_port;
    struct addrinfo req_hints;
    int req_result;
} req_info; /* This is for multi-thread network connectivity check. */

char command_flags[CMD_FLAG_NUM][16]={
    "-b", /* batch mode, skip every confirmation */
    "-r", /* recursive */
    "-rf", /* recursive + force */
    "-f", /* force */
    "--all", 
    "--list",
    "--force",
    "--recursive",
    "--print", /* print contents */
    "--installed",
    "--verbose",
    "--read", /* read contents */
    "--edit",
    "--std", /* standard info */
    "--err", /* error info */
    "--dbg", /* tf dbg info */
    "--this", /* this */
    "--hist", /* historical */
    "--mc", /* rebuild mc */
    "--mcdb", /* rebuild mcdb */
    "--bkey", /* display bucket passwd */
    "--rkey", /* display root passwd */
    "--admin", /* export admin privilege */
    "--accept", /* accept license terms */
    "--echo", /* echo_flag */
    "--od",
    "--month",
    "--gcp",
    "--rdp",
    "--copypass"
};

char command_keywords[CMD_KWDS_NUM][32]={
    "-c", /* cluster */
    "-u", /* user */
    "-p", /* password */
    "-s", /* Source  | start */
    "-e", /* End */
    "-d", /* Destination */
    "-t", /* target */
    "-n", /* node_name */
    "--cmd",
    "--dcmd",
    "--ucmd",
    "--acmd",
    "--app",
    "--jcmd",
    "--jname", /* Job Name */
    "--jid",
    "--jtime",
    "--jexec",
    "--jdata",
    "--level",
    "--log",
    "--vol",
    "--cname", /* cluster_name */
    "--ak",
    "--sk",
    "--az-sid",
    "--az-tid",
    "--ul", /* user list */
    "--key", /* key file */
    "--rg",
    "--az",
    "--nn", /* node_num */
    "--tn", /* tasks per node */
    "--un", /* user_num */
    "--mi",
    "--ci",
    "--os",
    "--ht",
    "--inst",
    "--repo",
    "--conf",
    "--hloc",
    "--cloc",
    "--hver",
    "--dbg-level",
    "--max-time",
    "--tf-run",
    "--pass"
};

void sleep_func(unsigned int time){
#ifdef _WIN32
    Sleep(time*1000);
#else
    sleep(time);
#endif
}

int string_to_positive_num(char* string){
    if(string==NULL){
        return NULL_PTR_ARG;
    }
    int i,sum=0;
    int length=strlen(string);
    if(length<1){
        return -1;
    }
    for(i=0;i<length;i++){
        if(*(string+i)<'0'||*(string+i)>'9'){
            return -1;
        }
    }
    for(i=0;i<length;i++){
        sum=sum*10+(*(string+i)-'0');
    }
    return sum;
}

void fflush_stdin(void){
    int ch;
    while((ch=getchar())!='\n'&&ch!=EOF){}
    if(ch==EOF){
        if(ferror(stdin)){
            clearerr(stdin);
        }
    }
}

/*
 * This function is more secure than get_key_value.
 * return  0: get_succeeded
 * return -1: param error
 * return -5: memory allocation failed
 * return -3: file I/O error
 * return  1: failed to get
 */
int get_key_nvalue(char* filename, unsigned int linelen_max, char* key, char ch, char value[], unsigned int valen_max){
    if(filename==NULL||key==NULL){
        return NULL_PTR_ARG;
    }
    if(linelen_max<1||valen_max<1||strlen(key)==0||ch=='\0'){
        memset(value,'\0',valen_max);
        return -1;
    }
    char* line_buffer=(char*)malloc(sizeof(char)*linelen_max);
    if(line_buffer==NULL){
        return -5;
    }
    char* key_buffer=(char*)malloc(sizeof(char)*(strlen(key)+8));
    if(key_buffer==NULL){
        free(line_buffer);
        return -5;
    }
    char* value_buffer=(char*)malloc(sizeof(char)*valen_max);
    if(value_buffer==NULL){
        free(line_buffer);
        free(key_buffer);
        return -5;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        memset(value,'\0',valen_max);
        free(line_buffer);
        free(key_buffer);
        free(value_buffer);
        return -3;
    }
    while(fngetline(file_p,line_buffer,linelen_max)!=1){
        reset_nstring(key_buffer,strlen(key)+8);
        reset_nstring(value_buffer,valen_max);
        get_seq_nstring(line_buffer,ch,1,key_buffer,strlen(key)+8);
        get_seq_nstring(line_buffer,ch,2,value_buffer,valen_max);
        if(strcmp(key,key_buffer)==0){
            strncpy(value,value_buffer,valen_max-1);
            fclose(file_p);
            free(line_buffer);
            free(key_buffer);
            free(value_buffer);
            return 0;
        }
    }
    memset(value,'\0',valen_max);
    fclose(file_p);
    free(line_buffer);
    free(key_buffer);
    free(value_buffer);
    return 1;
}

/* 
 * This function is not secure. Going to deprecate
 * Please use get_key_nvalue()
 */
int get_key_value(char* filename, char* key, char ch, char* value){
    char line_buffer[LINE_LENGTH_SHORT]="";
    char head[128]="";
    char tail[256]="";
    if(filename==NULL||key==NULL){
        return NULL_PTR_ARG;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        strcpy(value,"");
        return -1;
    }
    if(strlen(key)==0||ch=='\0'){
        fclose(file_p);
        strcpy(value,"");
        return -3;
    }
    while(!feof(file_p)){
        fgetline(file_p,line_buffer);
        reset_string(tail);
        get_seq_string(line_buffer,ch,1,head);
        get_seq_string(line_buffer,ch,2,tail);
        if(strcmp(key,head)==0){
            fclose(file_p);
            strcpy(value,tail);
            return 0;
        }
    }
    fclose(file_p);
    strcpy(value,"");
    return 1;
}

/* This function is going to be deprecated */
void reset_string(char* orig_string){
    if(orig_string==NULL){
        return;
    }
    int length=strlen(orig_string);
    int i;
    for(i=0;i<length;i++){
        *(orig_string+i)='\0';
    }
}

void reset_nstring(char string[], unsigned int string_length){
    if(string==NULL){
        return;
    }
    memset(string,'\0',string_length);
}
/* 
 * Potential risk: If the line_string array is not long enough, there might be overflow! The users should make sure the defined line_string is long enough
 * and the actual file doesn't contain lines longer than the LINE_LENGTH macro!
 * This function works, but not general or perfect at all! 
 * SEGMENT FAULT MAY OCCUR IF YOU DO NOT USE THIS FUNCTION PROPERLY!
 */
int fgetline(FILE* file_p, char* line_string){
    int ch='\0';
    int i=0;
    if(file_p==NULL||line_string==NULL){
        return NULL_PTR_ARG;
    }
    reset_string(line_string);
    do{
        ch=fgetc(file_p);
        if(ch!=EOF&&ch!='\n'){
            *(line_string+i)=ch;
            i++;
        }
    }while(ch!=EOF&&ch!='\n'&&i!=LINE_LENGTH); /* Be careful! This function can only handle lines <= 4096 chars. Extra chars will be ommited */
    if(i==LINE_LENGTH){
        return -7; /* When returns this value, the outcome will be unpredictable. */
    }
    *(line_string+i)='\0'; /* This is very dangerous. You need to guarantee the length of line_string is long enough! */
    if(ch==EOF&&i==0){
        return 1;
    }
    else{
        return 0;
    }
}

/*
 * This is a slightly-secure implementation of fgetline()
 * Aims to replace the fgetline() if validated.
 * Users must make sure that the max_length equals or smaller than the size of the array
 * Otherwise, overflow will definately occur.
 * Automatically add '\0' to the line_string, therefore, the max_length equals to the real width of the array
 * 
 * Return Vals:
 * return 0: get successed.
 * return -3: length invalid.
 * return -1: FILE not exist
 * return  1: EOF found and read nothing.
 * return  2: maxlength
 */
int fngetline(FILE* file_p, char line_string[], unsigned int max_length){
    int ch='\0';
    int i=0;
    if(file_p==NULL||line_string==NULL){
        return NULL_PTR_ARG;
    }
    memset(line_string,'\0',max_length);
    if(max_length<1){
        return -3;
    }
    do{
        ch=fgetc(file_p);
        if(ch!=EOF&&ch!='\n'&&ch!='\r'){
            *(line_string+i)=ch;
            i++;
        }
    }while(ch!=EOF&&ch!='\n'&&ch!='\r'&&i<max_length-1); /* Reserve a char for '\0' */
    if(i==max_length-1){
        return 2; /* When returns this value, the line is not read completely. */
    }
    if(ch==EOF&&i==0){
        return 1; /* Read nothing */
    }
    else{
        return 0; /* Finished reading */
    }
}

/* 
 * contain: return 0
 * not contain: return 1
 * return -1: MEM ERROR!
 */
int contain_or_not(const char* line, const char* findkey){
    if(line==NULL||findkey==NULL){
        return NULL_PTR_ARG;
    }
    int length_line=strlen(line);
    int length_findkey=strlen(findkey);
    int i,j;
    char* string_temp=(char *)malloc(sizeof(char)*(length_findkey+1));
    if(string_temp==NULL){
        return -1;
    }
    for(i=0;i<length_findkey;i++){
        *(string_temp+i)='\0';
    }
    if(length_line<length_findkey){
        free(string_temp);
        return 1;
    }
    for(i=0;i<length_line;i++){
        if(*(line+i)==*(findkey)){
            for(j=0;j<length_findkey;j++){
                *(string_temp+j)=*(line+i+j);
            }
            *(string_temp+length_findkey)='\0';
            if(strcmp(findkey,string_temp)==0){
                free(string_temp);
                return 0;
            }
        }
        else{
            continue;
        }
    }
    free(string_temp);
    return 1;
}

/*
 * This function is more secure than contain_or_not
 * Return Values:
 * return  0: not contain
 * return >0: contain N keys
 */
int contain_or_nnot(char line[], char findkey[]){
    int count=0;
    size_t line_len=0;
    size_t key_len=0;
    if(line==NULL||findkey==NULL){
        return NULL_PTR_ARG;
    }
    line_len=strlen(line);
    key_len=strlen(findkey);
    if(line_len<key_len){
        return 0;
    }
    if(key_len==0){ /* If findkey = '\0', then return 1 because the line contains at least one '\0' */
        return 1;
    }
    int i=0;
    while(i<line_len-key_len+1){
        if(memcmp(findkey,line+i,key_len)==0){
            count++;
            i+=key_len;
        }
        else{
            i++;
        }
    }
    return count;
}

/* This function is deprecated by global_nreplace() */
int global_replace(char* filename, char* orig_string, char* new_string){
    if(filename==NULL||orig_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    if(strcmp(orig_string,new_string)==0){
        return 1;
    }
    if(strlen(orig_string)==0){
        return -1;
    }
    FILE* file_p=fopen(filename, "r");
    FILE* file_p_tmp=NULL;
    if(file_p==NULL){ 
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    char new_line[LINE_LENGTH]="";
    char head=*(orig_string);
    int length_orig=strlen(orig_string);
    char temp_string[LINE_LENGTH]="";
    char filename_temp[FILENAME_LENGTH]="";
    int i,j,k,line_length;
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    while(fgetline(file_p,single_line)==0){
        line_length=strlen(single_line);
        if(contain_or_not(single_line,orig_string)!=0||line_length<length_orig){
            fprintf(file_p_tmp,"%s\n",single_line);
            continue;
        }
        i=0;
        j=0;
        reset_string(new_line);
        reset_string(temp_string);
        do{
            if(*(single_line+i)!=head){
                *(new_line+j)=*(single_line+i);
                i++;
                j++;
                continue;
            }
            for(k=0;k<length_orig;k++){
                *(temp_string+k)=*(single_line+i+k);
            }
            if(strcmp(temp_string,orig_string)!=0){
                *(new_line+j)=*(single_line+i);
                i++;
                j++;
                continue;
            }
            else{
                for(k=0;k<strlen(new_string);k++){
                    *(new_line+j+k)=*(new_string+k);
                }
                j=j+k;
                i=i+length_orig;
                continue;
            }
        }while(i<line_length);
        fprintf(file_p_tmp,"%s\n",new_line);
    }
    fclose(file_p);
    fclose(file_p_tmp);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)==0){
        return -3;
    }
    return 0;
}

/*
 * This function is more secure and strict than global_replace
 */
int global_nreplace(char* filename, unsigned int linelen_max, char* orig_string, char* new_string){
    if(filename==NULL||orig_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    unsigned int orig_str_len=(unsigned int)strlen(orig_string);
    unsigned int new_str_len=(unsigned int)strlen(new_string);
    if(linelen_max<1||linelen_max<orig_str_len){
        return -1;
    }
    if(strcmp(orig_string,new_string)==0){
        return 1;
    }
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -3;
    }
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -3;
    }
    int i,j,contain_count;
    char* single_line=NULL;
    char* new_line=NULL;
    unsigned int new_line_length=0;
    single_line=(char*)malloc(sizeof(char)*linelen_max);
    if(single_line==NULL){
        fclose(file_p);
        fclose(file_p_tmp);
        return -5;
    }
    while(fngetline(file_p,single_line,linelen_max)!=1){
        contain_count=contain_or_nnot(single_line,orig_string);
        if(contain_count<1){
            fprintf(file_p_tmp,"%s\n",single_line);
            continue;
        }
        new_line_length=linelen_max+contain_count*(new_str_len-orig_str_len)+1;
        new_line=(char*)malloc(sizeof(char)*new_line_length);
        if(new_line==NULL){
            free(single_line);
            fclose(file_p);
            fclose(file_p_tmp);
            return -5;
        }
        i=0;
        j=0;
        reset_nstring(new_line,new_line_length);
        do{
            if(strncmp(single_line+i,orig_string,orig_str_len)!=0){
                *(new_line+j)=*(single_line+i);
                i++;
                j++;
                continue;
            }
            strncpy(new_line+j,new_string,new_str_len);
            i+=orig_str_len;
            j+=new_str_len;
        }while(i<linelen_max);
        fprintf(file_p_tmp,"%s\n",new_line);
    }
    free(single_line);
    free(new_line);
    fclose(file_p);
    fclose(file_p_tmp);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -7;
    }
    return 0;
}

int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string){
    if(orig_line==NULL||new_line==NULL||orig_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    int length=strlen(orig_line);
    int length_orig=strlen(orig_string);
    int length_new=strlen(new_string);
    char* temp_string=(char *)malloc(sizeof(char)*(length_orig+1));
    int i,j;
    int k=0,k2;
    if(temp_string==NULL){
        return -1;
    }
    for(i=0;i<length_orig+1;i++){
        *(temp_string+i)='\0';
    }
    if(length_orig==0){
        for(i=0;i<length;i++){
            *(new_line+i)=*(orig_line+i);
        }
        free(temp_string);
        return length;
    }
    reset_string(new_line);
    i=0;
    do{
        if(*(orig_line+i)==*(orig_string)&&i+length_orig<length+1){
            for(j=0;j<length_orig;j++){
                *(temp_string+j)=*(orig_line+i+j);
            }
            if(strcmp(temp_string,orig_string)==0){
                for(j=0;j<length_new;j++){
                    *(new_line+k)=*(new_string+j);
                    k++;
                }
                *(new_line+k)='\0';
                i=i+length_orig;
                for(k2=0;k2<length_orig+1;k2++){
                    *(temp_string+k2)='\0';
                }
                continue;
            }
            else{
                *(new_line+k)=*(orig_line+i);
                i++;
                k++;
                for(k2=0;k2<length_orig+1;k2++){
                    *(temp_string+k2)='\0';
                }
                continue;
            }
        }
        else{
            *(new_line+k)=*(orig_line+i);
            i++;
            k++;
            continue;
        }
    }while(i<length);
    free(temp_string);
    return k;
}

/* 
 * Memory Allocated here. Please free the memory after use.
 * return NULL: Failed to replace
 * return char*: Replaced
 */
char* line_nreplace(char* orig_line, int contain_count, char* orig_string, char* new_string){
    if(orig_line==NULL||orig_string==NULL||new_string==NULL){
        return NULL;
    }
    unsigned int orig_line_len=(unsigned int)strlen(orig_line);
    if(orig_line_len==0||contain_count<1||strlen(orig_string)<1){
        return NULL;
    }
    char* new_line=NULL;
    unsigned int new_line_len=orig_line_len+contain_count*(strlen(new_string)-strlen(orig_string))+1;
    new_line=(char*)malloc(sizeof(char)*new_line_len);
    if(new_line==NULL){
        return NULL;
    }
    memset(new_line,'\0',new_line_len);
    int i=0;
    int j=0;
    do{
        if(strncmp(orig_line+i,orig_string,strlen(orig_string))==0){
            strncpy(new_line+j,new_string,strlen(new_string));
            i+=strlen(orig_string);
            j+=strlen(new_string);
            continue;
        }
        *(new_line+j)=*(orig_line+i);
        i++;
        j++;
    }while(i<orig_line_len);
    return new_line;
}

int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string){
    if(filename==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||findkey4==NULL||findkey5==NULL||orig_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    if(strcmp(orig_string,new_string)==0){
        return -1;
    }
    int replace_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    char new_single_line[LINE_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0,flag6=0;
    FILE* file_temp_p=fopen(filename_temp,"w+");
    if(file_temp_p==NULL){
        fclose(file_p);
        return -1;
    }
    while(fgetline(file_p,single_line)!=1){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(strlen(findkey4)!=0){
            flag4=contain_or_not(single_line,findkey4);
        }
        if(strlen(findkey5)!=0){
            flag5=contain_or_not(single_line,findkey5);
        }
        if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0){
            fprintf(file_temp_p,"%s%c",single_line,'\n');
            flag1=0;
            flag2=0;
            flag3=0;
            flag4=0;
            flag5=0;
            continue;
        }
        else{
            flag6=contain_or_not(single_line,orig_string);
            if(flag6!=0){
                fprintf(file_temp_p,"%s%c",single_line,'\n');
            }
            else{
                replace_count++;
                if(strcmp(orig_string,new_string)==0){
                    fprintf(file_temp_p,"%s%c",single_line,'\n');
                    continue;
                }
                line_replace(single_line,new_single_line,orig_string,new_string);
                fprintf(file_temp_p,"%s%c",new_single_line,'\n');
                reset_string(new_single_line);
            }
            flag6=0;
        }
    }
    fprintf(file_temp_p,"%s",single_line);
    fclose(file_p);
    fclose(file_temp_p);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -3;
    }
    return replace_count;
}

int find_and_nreplace(char* filename, unsigned int linelen_max, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string){
    if(filename==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||findkey4==NULL||findkey5==NULL||orig_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    if(strcmp(orig_string,new_string)==0||strlen(orig_string)<1||linelen_max<strlen(orig_string)){
        return -1;
    }
    int replace_count=0;
    int contain_flag;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -3;
    }
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_temp_p=fopen(filename_temp,"w+");
    if(file_temp_p==NULL){
        fclose(file_p);
        return -3;
    }
    char* single_line=(char*)malloc(sizeof(char)*(linelen_max));
    char* new_line=NULL;
    if(single_line==NULL){
        fclose(file_p);
        fclose(file_temp_p);
        free(single_line);
        return -5;
    }
    while(fngetline(file_p,single_line,linelen_max)!=1){
        if(contain_or_nnot(single_line,findkey1)<1||contain_or_nnot(single_line,findkey2)<1||contain_or_nnot(single_line,findkey3)<1||contain_or_nnot(single_line,findkey4)<1||contain_or_nnot(single_line,findkey5)<1){
            fprintf(file_temp_p,"%s\n",single_line);
            continue;
        }
        contain_flag=contain_or_nnot(single_line,orig_string);
        if(contain_flag<1){
            fprintf(file_temp_p,"%s\n",single_line);
            continue;
        }
        new_line=line_nreplace(single_line,contain_flag,orig_string,new_string);
        if(new_line==NULL){
            fprintf(file_temp_p,"%s\n",single_line);
        }
        else{
            replace_count++;
            fprintf(file_temp_p,"%s\n",new_line);
            free(new_line);
        }
    }
    fprintf(file_temp_p,"%s",single_line);
    fclose(file_p);
    fclose(file_temp_p);
    free(single_line);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -3;
    }
    return replace_count;
}

int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5){
    if(filename==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||findkey4==NULL||findkey5==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(findkey1)==0&&strlen(findkey2)==0&&strlen(findkey3)==0&&strlen(findkey4)==0&&strlen(findkey5)==0){
        return -1;
    }
    int find_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
    while(fgetline(file_p,single_line)!=1){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(strlen(findkey4)!=0){
            flag4=contain_or_not(single_line,findkey4);
        }
        if(strlen(findkey5)!=0){
            flag5=contain_or_not(single_line,findkey5);
        }
        if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0){
            flag1=0;
            flag2=0;
            flag3=0;
            flag4=0;
            flag5=0;
            continue;
        }
        else{
            find_count++;
        }
    }
    fclose(file_p);
    return find_count;
}

int find_multi_nkeys(char* filename, unsigned int linelen_max, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5){
    if(linelen_max<1){
        return -1;
    }
    if(filename==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||findkey4==NULL||findkey5==NULL){
        return NULL_PTR_ARG;
    }
    int find_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -3;
    }
    char* single_line=(char*)malloc(sizeof(char)*linelen_max);
    if(single_line==NULL){
        fclose(file_p);
        return -5;
    }
    while(fngetline(file_p,single_line,linelen_max)!=1){
        if(contain_or_nnot(single_line,findkey1)>0&&contain_or_nnot(single_line,findkey2)>0&&contain_or_nnot(single_line,findkey3)>0&&contain_or_nnot(single_line,findkey4)>0&&contain_or_nnot(single_line,findkey5)>0){
            find_count++;
        }
    }
    free(single_line);
    fclose(file_p);
    return find_count;
}

int calc_str_num(char* line, char split_ch){
    if(line==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(line)==0){
        return 0;
    }
    int i=0,j=0;
    int str_num=0;
    if(split_ch==' '){
        do{
            if(*(line+i)!=' '&&*(line+i)!='\t'){
                do{
                    j++;
                }while(*(line+i+j)!=' '&&*(line+i+j)!='\t'&&j<strlen(line)-i);
                if(j==(strlen(line)-i)){
                    str_num++;
                    return str_num;
                }
                if(*(line+i+j)==' '||*(line+i+j)=='\t'){
                    str_num++;
                    i=i+j;
                    j=0;
                }
            }
            else{
                i++;
            }
        }while(i<strlen(line));
        return str_num;
    }
    else{
        str_num=0;
        if(*(line)!=split_ch){
            str_num++;
        }
        do{
            if(*(line+i)==split_ch&&*(line+i+1)!=split_ch){
                str_num++;
            }
            i++;
        }while(i<strlen(line));
        return str_num;
    }
}

int calc_str_nnum(char* line, char split_ch){
    if(line==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(line)<1||split_ch=='\0'){
        return 0;
    }
    int i=0,j=0;
    int str_num=0;
    unsigned int line_len=(unsigned int)strlen(line);
    if(split_ch==' '){
        do{
            if(*(line+i)!=' '&&*(line+i)!='\t'){
                do{
                    j++;
                }while(*(line+i+j)!=' '&&*(line+i+j)!='\t'&&j<line_len-i);
                if(j==(line_len-i)){
                    str_num++;
                    return str_num;
                }
                if(*(line+i+j)==' '||*(line+i+j)=='\t'){
                    str_num++;
                    i+=j;
                    j=0;
                }
            }
            else{
                i++;
            }
        }while(i<line_len);
        return str_num;
    }
    else{
        if(*(line)!=split_ch){
            str_num++;
        }
        do{
            if(*(line+i)==split_ch&&*(line+i+1)!=split_ch){
                str_num++;
            }
            i++;
        }while(i<line_len);
        return str_num;
    }
}

/* 
 * return -1: get finished
 * return 0: get_successed
 */
int get_seq_string(char* line, char split_ch, int string_seq, char* get_string){
    int total_string_num=calc_str_num(line,split_ch);
    int i=0,j=0;
    int string_seq_current;
    if(string_seq>total_string_num){
        strcpy(get_string,"");
        return -1;
    }
    reset_string(get_string);
    if(split_ch==' '){
        string_seq_current=0;
        if(*(line)!=' '&&*(line)!='\t'){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==' '||*(line+i)=='\t'){
                if(*(line+i+1)!=' '&&*(line+i+1)!='\t'){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==' '||*(line+j)=='\t'){
                break;
            }
            else{
                *(get_string+j-i)=*(line+j);
            }
        }
        return 0;
    }
    else{
        string_seq_current=0;
        if(*(line)!=split_ch){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==split_ch){
                if(*(line+i+1)!=split_ch){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==split_ch){
                break;
            }
            else{
                *(get_string+j-i)=*(line+j);
            }
        }
        return 0;
    }
}

/* 
 * This function is more secure than get_seq_string
 * Make sure the get_string_length <= length of get_string[] array
 * Return Values:
 * return -1: get_string_length=0
 * return  3: get_empty because get_string_length is too small
 * return  1: get empty
 * return  0: get_successfully
 */
int get_seq_nstring(char line[], char split_ch, int string_seq, char get_str[], unsigned int getstr_len_max){
    if(getstr_len_max<1||split_ch=='\0'){
        return -5;
    }
    if(line==NULL||get_str==NULL){
        return NULL_PTR_ARG;
    }
    int total_string_num=calc_str_nnum(line,split_ch);
    int i=0,j=0;
    int break_flag=0;
    int string_seq_current;
    if(string_seq>total_string_num){
        reset_nstring(get_str,getstr_len_max);
        return 1;
    }
    reset_nstring(get_str,getstr_len_max);
    if(split_ch==' '){
        string_seq_current=0;
        if(*(line)!=' '&&*(line)!='\t'){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==' '||*(line+i)=='\t'){
                if(*(line+i+1)!=' '&&*(line+i+1)!='\t'){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==' '||*(line+j)=='\t'){
                break;
            }
            if((j-i)==(getstr_len_max-1)){
                break_flag=1;
                break;
            }
            *(get_str+j-i)=*(line+j);
        }
        if(break_flag==1&&j<strlen(line)-1){
            if(*(line+j+1)!=' '&&*(line+j+1)!='\t'){
                reset_nstring(get_str,getstr_len_max);
                return 3; 
            }
        }
        return 0;
    }
    else{
        string_seq_current=0;
        if(*(line)!=split_ch){
            string_seq_current++;
        }
        while(string_seq_current<string_seq){
            if(*(line+i)==split_ch){
                if(*(line+i+1)!=split_ch){
                    string_seq_current++;
                    i++;
                }
                else{
                    i++;
                }
            }
            else{
                i++;
            }
        }
        for(j=i;j<strlen(line);j++){
            if(*(line+j)==split_ch){
                break;
            }
            if((j-i)==(getstr_len_max-1)){
                break_flag=1;
                break;
            }
            *(get_str+j-i)=*(line+j);
        }
        if(break_flag==1&&j<strlen(line)-1){
            if(*(line+j+1)!=split_ch){
                reset_nstring(get_str,getstr_len_max);
                return 3;
            }
        }
        return 0;
    }
}

int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string){
    if(filename==NULL||findkey_primary1==NULL||findkey_primary2==NULL||findkey_primary3==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||get_string==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(findkey_primary1)==0&&strlen(findkey_primary2)==0&&strlen(findkey_primary3)==0){
        return -1;
    }
    if(strlen(findkey1)==0&&strlen(findkey2)==0&&strlen(findkey3)==0){
        return -1;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -1;
    }
    char single_line[LINE_LENGTH_EXT]="";
    char get_string_buffer[LINE_LENGTH]="";
    int flag_primary1=0,flag_primary2=0,flag_primary3=0;
    int flag_primary=1;
    int flag1=0,flag2=0,flag3=0;
    int i;
    while(flag_primary!=0&&!feof(file_p)){
        fgetline(file_p,single_line);
        if(strlen(findkey_primary1)!=0){
            flag_primary1=contain_or_not(single_line,findkey_primary1);
        }
        if(strlen(findkey_primary2)!=0){
            flag_primary2=contain_or_not(single_line,findkey_primary2);
        }
        if(strlen(findkey_primary3)!=0){
            flag_primary3=contain_or_not(single_line,findkey_primary3);
        }
        if(flag_primary1==0&&flag_primary2==0&&flag_primary3==0){
            flag_primary=0;
            break;
        }
        else{
            flag_primary=1;
            flag_primary1=0;
            flag_primary2=0;
            flag_primary3=0;
            continue;
        }
    }
    if(feof(file_p)){
        fclose(file_p);
        strcpy(get_string,"");
        return 1;
    }
    i=0;
    while(!feof(file_p)&&i<plus_line_num){
        if(strlen(findkey1)!=0){
            flag1=contain_or_not(single_line,findkey1);
        }
        if(strlen(findkey2)!=0){
            flag2=contain_or_not(single_line,findkey2);
        }
        if(strlen(findkey3)!=0){
            flag3=contain_or_not(single_line,findkey3);
        }
        if(flag1!=0||flag2!=0||flag3!=0){
            flag1=0;
            flag2=0;
            flag3=0;
            i++;
            fgetline(file_p,single_line);
            continue;
        }
        else{
            fclose(file_p);
            get_seq_string(single_line,split_ch,string_seq_num,get_string_buffer);
            strcpy(get_string,get_string_buffer);
            return 0;
        }
    }
    strcpy(get_string,"");
    return 1;
}

/* This function force empty the string */
int find_and_nget(char* filename, unsigned int linelen_max, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char get_str[], int get_strlen_max){
    if(filename==NULL||findkey_primary1==NULL||findkey_primary2==NULL||findkey_primary3==NULL||findkey1==NULL||findkey2==NULL||findkey3==NULL||get_str==NULL){
        return NULL_PTR_ARG;
    }
    reset_nstring(get_str,get_strlen_max); /* force empty */
    if(linelen_max<1||split_ch=='\0'||string_seq_num<1||get_strlen_max<1){
        return -1;
    }
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -3;
    }
    char* single_line=(char*)malloc(sizeof(char)*linelen_max);
    if(single_line==NULL){
        fclose(file_p);
        return -5;
    }
    int j;
    while(fngetline(file_p,single_line,linelen_max)!=1){
        if(contain_or_nnot(single_line,findkey_primary1)>0&&contain_or_nnot(single_line,findkey_primary2)>0&&contain_or_nnot(single_line,findkey_primary3)>0){
            if(contain_or_nnot(single_line,findkey1)>0&&contain_or_nnot(single_line,findkey2)>0&&contain_or_nnot(single_line,findkey3)>0){
                get_seq_nstring(single_line,split_ch,string_seq_num,get_str,get_strlen_max);
                free(single_line);
                fclose(file_p);
                return 0;
            }
            break;
        }
    }
    j=0;
    while(!feof(file_p)&&j<plus_line_num){
        fngetline(file_p,single_line,linelen_max);
        j++;
        if(contain_or_nnot(single_line,findkey1)>0&&contain_or_nnot(single_line,findkey2)>0&&contain_or_nnot(single_line,findkey3)>0){
            get_seq_nstring(single_line,split_ch,string_seq_num,get_str,get_strlen_max);
            free(single_line);
            fclose(file_p);
            return 0;
        }
    }
    reset_nstring(get_str,get_strlen_max);
    free(single_line);
    fclose(file_p);
    return 1;
}

/* 
 * return 0: file exists and open successfully
 * return 1: not exist or open failed
 */
int file_exist_or_not(char* filename){
    if(filename==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    if(GetFileAttributes(filename)&FILE_ATTRIBUTE_DIRECTORY){
        return 1;
    }
#else
    struct stat file_stat;
    if(stat(filename,&file_stat)!=0){
        return 1;
    }
    if(S_ISDIR(file_stat.st_mode)){
        return 1;
    }
#endif
    if(access(filename,4)!=0){
        return 1;
    }
    return 0;
}

/*
 * Make a *PRIVATE* directory recursively
 * For Windows, the permission inherates from upper-level dir. 
 * For *nix, the permission is set to 0700
 * 
 * return  0: create the directory successfully
 * return  1: directory already exists
 * return -7: illegal path
 * return -5: memory error
 * return -3: create failed
 * return -1: Failed
 */
int mk_pdir(char* pathname){
    if(pathname==NULL){
        return NULL_PTR_ARG;
    }
    unsigned int length=strlen(pathname);
    unsigned int i=0;
    unsigned int exist_flag=0;
    char* sub_path=NULL;
    if(length<1){
        return -7;
    }
    while(i<length){
        if(*(pathname+i)!=PATH_SLASH[0]&&i!=length-1){
            i++;
            continue;
        }
        sub_path=(char*)malloc(sizeof(char)*(i+2));
        if(sub_path==NULL){
            return -5;
        }
        memset(sub_path,'\0',i+2);
        strncpy(sub_path,pathname,i+1);
        if(access(sub_path,0)==0){
            exist_flag=1;
            i++;
            free(sub_path);
            continue;
        }
        exist_flag=0;
#ifdef _WIN32
        if(CreateDirectory(sub_path,NULL)==0){
            free(sub_path);
            return -3;
        }
#else
        if(mkdir(sub_path,0700)!=0){
            free(sub_path);
            return -3;
        }
#endif
        i++;
        free(sub_path);
    }
    if(folder_exist_or_not(pathname)==0){
        return exist_flag; /* Return 0 or 1 */
    }
    return -1; /* Return < 0*/
}

/* 
 * This function is risky because it delete a folder recursively.
 * Please be aware of the risk!
 */
int rm_pdir(char* pathname){
    if(pathname==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(pathname)<1){
        return -5;
    }
#ifdef _WIN32
    HANDLE handle_find;
    WIN32_FIND_DATA find_data;
    char sub_path[MAX_PATH]=""; /* Windows max path length is 260, which is MAX_PATH*/
    char sub_search_pattern[MAX_PATH]="";
    if(!(GetFileAttributes(pathname)&FILE_ATTRIBUTE_DIRECTORY)){
        if(!DeleteFile(pathname)){
            return 1;
        }
        return 0;
    }
    snprintf(sub_search_pattern,MAX_PATH-1,"%s\\*",pathname);
    handle_find=FindFirstFile(sub_search_pattern,&find_data);
    if(handle_find==INVALID_HANDLE_VALUE){
        return -3;
    }
    do{
        if(strcmp(find_data.cFileName,".")==0||strcmp(find_data.cFileName,"..")==0){
            continue;
        }
        else if(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
            snprintf(sub_path,MAX_PATH-1,"%s\\%s",pathname,find_data.cFileName);
            if(rm_pdir(sub_path)!=0){
                FindClose(handle_find);
                return -1;
            }
        }
        else{
            snprintf(sub_path,MAX_PATH-1,"%s\\%s",pathname,find_data.cFileName);
            if(!DeleteFile(sub_path)){
                FindClose(handle_find);
                return -1;
            }  
        }  
    }while(FindNextFile(handle_find,&find_data)!=0);
    FindClose(handle_find); /* Close the handle */
    _chmod(pathname,_S_IWRITE);
    if(!RemoveDirectory(pathname)){
        return 1;
    }
    return 0;
#else
    DIR* dir=NULL;
    struct dirent* entry=NULL;
    char sub_path[DIR_LENGTH_EXT]="";
    struct stat sub_stat;
    if(lstat(pathname,&sub_stat)!=0){
        return -1;
    }
    if(!(S_ISDIR(sub_stat.st_mode))){
        if(unlink(pathname)!=0){
            return 1;
        }
        return 0;
    }
    dir=opendir(pathname);
    if(dir==NULL){
        return -3;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        snprintf(sub_path,DIR_LENGTH_EXT-1,"%s/%s",pathname,entry->d_name);
        if(lstat(sub_path,&sub_stat)==-1){
            closedir(dir);
            return -1;
        }
        /* If it is a softlink */
        if((sub_stat.st_mode&S_IFMT)==S_IFLNK){
            if(unlink(sub_path)!=0){
                closedir(dir);
                return -1;
            }
        }
        /* If it is a normal directory */
        else if(S_ISDIR(sub_stat.st_mode)){
            if(rm_pdir(sub_path)!=0){
                closedir(dir);
                return -1;
            }
        }
        /* All other cases */
        else{
            if(unlink(sub_path)!=0){
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);
    if(rmdir(pathname)!=0){
        return 1;
    }
    return 0;
#endif
}

/*
 * force_flag  =0: force
 * force_flag !=0: not force
 */
int cp_file(char* current_filename, char* new_filename, int force_flag){
#ifdef _WIN32
    char filebase_win[FILENAME_BASE_LENGTH]="";
    char ext_win[FILENAME_EXT_LENGTH]="";
#endif
    char filebase_full[FILENAME_BASE_FULL_LENGTH]="";
    char new_filename_temp[FILENAME_LENGTH]="";
    uint8_t* buffer=NULL;
    int64_t i,filesize_byte;
#ifndef _WIN32
    struct stat file_stat;
#endif
    if(current_filename==NULL||new_filename==NULL){
        return NULL_PTR_ARG;
    }
    if(file_exist_or_not(current_filename)!=0){
        return -1;
    }
#ifdef _WIN32
    _splitpath(current_filename,NULL,NULL,filebase_win,ext_win);
    snprintf(filebase_full,FILENAME_BASE_FULL_LENGTH-1,"%s%s",filebase_win,ext_win);
#else
    strncpy(filebase_full,basename(current_filename),FILENAME_BASE_FULL_LENGTH-1);
#endif
    if(folder_exist_or_not(new_filename)==0){
        snprintf(new_filename_temp,FILENAME_LENGTH-1,"%s%s%s",new_filename,PATH_SLASH,filebase_full);
#ifdef _WIN32
        if(GetFileAttributes(new_filename_temp)!=INVALID_FILE_ATTRIBUTES){
            if(force_flag!=0||GetFileAttributes(new_filename_temp)&FILE_ATTRIBUTE_DIRECTORY){
                return -1;
            }
            if(!DeleteFile(new_filename_temp)){
                return -1;
            }
        }
#else
        if(stat(new_filename_temp,&file_stat)==0){
            if(force_flag!=0||S_ISDIR(file_stat.st_mode)){
                return -1;
            }
            if(unlink(new_filename_temp)!=0){
                return -1;
            }
        }
#endif
    }
    else{
        /* If the new_filename is a file, it must not exist. */
#ifdef _WIN32
        if(GetFileAttributes(new_filename)!=INVALID_FILE_ATTRIBUTES){
            if(force_flag!=0){
                return -1;
            }
            if(!DeleteFile(new_filename)){
                return -1;
            }
        }
#else
        if(stat(new_filename,&file_stat)==0){
            if(force_flag!=0){
                return -1;
            }
            if(unlink(new_filename)!=0){
                return -1;
            }
        }
#endif
    }
    /* Make sure the file can be opened and created */
    FILE* file_p_curr=fopen(current_filename,"rb");
    if(file_p_curr==NULL){
        return -5; /* File I/O failed. */
    }
    filesize_byte=get_filesize_byte(file_p_curr);
    if(filesize_byte<0){
        fclose(file_p_curr);
        return -5; /* File I/O failed. */
    }
    FILE* file_p_new=NULL;
    if(strlen(new_filename_temp)>0){
        file_p_new=fopen(new_filename_temp,"wb+");
    }
    else{
        file_p_new=fopen(new_filename,"wb+");
    }
    if(file_p_new==NULL){
        fclose(file_p_curr);
        return -3; /* File I/O failed. */
    }
    for(i=0;i<filesize_byte/FILE_IO_BLOCK;i++){
        buffer=(uint8_t*)malloc(sizeof(uint8_t)*FILE_IO_BLOCK);
        if(buffer==NULL){
            fclose(file_p_curr);
            fclose(file_p_new);
            return -7; /* Memory Allocation Failed. */
        }
        if(fread(buffer,FILE_IO_BLOCK,sizeof(uint8_t),file_p_curr)==0){
            free(buffer);
            if(ferror(file_p_curr)){
                fclose(file_p_curr);
                fclose(file_p_new);
                return 1;
            }
            break;
        }
        if(fwrite(buffer,FILE_IO_BLOCK,sizeof(uint8_t),file_p_new)<sizeof(uint8_t)){
            free(buffer);
            fclose(file_p_curr);
            fclose(file_p_new);
            return 3; /* File write error. */
        }
        free(buffer);
    }
    buffer=(uint8_t*)malloc(sizeof(uint8_t)*(filesize_byte%FILE_IO_BLOCK));
    if(buffer==NULL){
        fclose(file_p_curr);
        fclose(file_p_new);
        return -7; /* Memory Allocation Failed. */
    }
    if(fread(buffer,sizeof(uint8_t),filesize_byte%FILE_IO_BLOCK,file_p_curr)==0){
        if(ferror(file_p_curr)){
            free(buffer);
            fclose(file_p_curr);
            fclose(file_p_new);
            return 1;
        }
    }
    if(fwrite(buffer,sizeof(uint8_t),filesize_byte%FILE_IO_BLOCK,file_p_new)<filesize_byte%FILE_IO_BLOCK){
        free(buffer);
        fclose(file_p_curr);
        fclose(file_p_new);
        return 3; /* File write error. */
    }
    free(buffer);
    fclose(file_p_curr);
    fclose(file_p_new);
    return 0;
}

/* 
 * Function: compare a string that contains wildcard '*' with a normal string.
 *           E.g. 1: fuzzy_strcmp("wangzhenrong", "w*") should return 0(match);
 *           E.g. 2: fuzzy_strcmp("wangzhenrong", "*rong") should return 0(match);
 *           E.g. 3: fuzzy_strcmp("wangzhenrong", "we*rong") should return non-zero(not match);
 * 
 * Return values: -1 - invalid string or buffer size
 *                -3 - memory allocation failed
 *                 1 - not match
 *                 0 - match
 */
int fuzzy_strcmp(char* target_string, char* fuzzy_string){
    size_t i=0,j=0,k,buffer_length;
    char* buffer=NULL;
    if(target_string==NULL||fuzzy_string==NULL){
        return NULL_PTR_ARG;
    }
    size_t target_str_len=strlen(target_string);
    size_t fuzzy_str_len=strlen(fuzzy_string);
    int memcmp_flag;
    if(target_str_len<1||fuzzy_str_len<1){
        return -1;
    }
    buffer=(char*)malloc(sizeof(char)*fuzzy_str_len);
    if(buffer==NULL){
        return -3;
    }
    while(i<fuzzy_str_len){
        if(*(fuzzy_string+i)=='*'){
            memset(buffer,'\0',fuzzy_str_len);
            for(k=i+1;k<fuzzy_str_len&&*(fuzzy_string+k)!='*';k++){
                *(buffer+k-i-1)=*(fuzzy_string+k);
            }
            if(strlen(buffer)==0){
                i=k;
                continue;
            }
            buffer_length=k-i-1;
            i=k;
            memcmp_flag=0;
            for(k=j;k<target_str_len-buffer_length+1;k++){
                if(memcmp(target_string+k,buffer,buffer_length)==0){
                    memcmp_flag=1;
                    break;
                }
            }
            if(memcmp_flag==1){
                if(i==fuzzy_str_len&&(k+buffer_length)<target_str_len){
                    free(buffer);
                    return 1;
                }
                j=k+buffer_length;
                continue;
            }
            free(buffer);
            return 1;    
        }
        if(*(fuzzy_string+i)!=*(target_string+j)){
            free(buffer);
            return 1;
        }
        i++;
        j++;
    }
    free(buffer);
    return 0;
}

/*
 * Memory allocated here. Please free the memory after use.
 * buffer_size should be >= 8! 
 */
char* get_first_fuzzy_subpath(char* pathname, char* fuzzy_name, unsigned int buffer_size){
#ifdef _WIN32
    HANDLE handle_find;
    WIN32_FIND_DATA find_data;
    char sub_search_pattern[MAX_PATH]="";
#else
    DIR* dir=NULL;
    struct dirent* entry=NULL;
#endif
    char* get_path=NULL;
    if(pathname==NULL||fuzzy_name==NULL){
        return NULL;
    }
    if(strlen(pathname)<1||strlen(fuzzy_name)<1||buffer_size<8){
        return NULL;
    }
#ifdef _WIN32
    snprintf(sub_search_pattern,MAX_PATH-1,"%s\\*",pathname);
    handle_find=FindFirstFile(sub_search_pattern,&find_data);
    if(handle_find==INVALID_HANDLE_VALUE){
        return NULL;
    }
    do{
        if(strcmp(find_data.cFileName,".")==0||strcmp(find_data.cFileName,"..")==0){
            continue;
        }
        if(fuzzy_strcmp(find_data.cFileName,fuzzy_name)==0){
            get_path=(char*)malloc(sizeof(char)*buffer_size);
            if(get_path==NULL){
                return NULL;
            }
            memset(get_path,'\0',buffer_size);
            snprintf(get_path,buffer_size-1,"%s\\%s",pathname,find_data.cFileName);
            return get_path;
        }
    }while(FindNextFile(handle_find,&find_data)!=0);
    FindClose(handle_find); /* Close the handle */ 
    return NULL;
#else
    dir=opendir(pathname);
    if(dir==NULL){
        return NULL;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(fuzzy_strcmp(entry->d_name,fuzzy_name)==0){
            get_path=(char*)malloc(sizeof(char)*buffer_size);
            if(get_path==NULL){
                return NULL;
            }
            memset(get_path,'\0',buffer_size);
            snprintf(get_path,buffer_size-1,"%s/%s",pathname,entry->d_name);
            return get_path;
        }
    }
    closedir(dir);
    return NULL;
#endif
}

/*
 * force_flag  =0: force
 * force_flag !=0: not force
 * cp: copy | mv: move | rm: remove
 */
int batch_file_operation(char* source_dir, char* fuzzy_filename, char* target_dir, char* option, int force_flag){
    char filename_temp[FILENAME_LENGTH]="";
    char filename_temp2[FILENAME_LENGTH]="";
#ifdef _WIN32
    HANDLE handle_find;
    WIN32_FIND_DATA find_data;
#else
    DIR* dir=NULL;
    struct dirent* entry=NULL;
#endif
    if(source_dir==NULL||fuzzy_filename==NULL||target_dir==NULL||option==NULL){
        return NULL_PTR_ARG;
    }
    if(strcmp(option,"cp")!=0&&strcmp(option,"mv")!=0&&strcmp(option,"rm")!=0){
        return -7;
    }
    if(strcmp(option,"mv")==0||strcmp(option,"rm")==0){
        if(folder_check_general(source_dir,6)!=0){
            return -5;
        }
    }
    else{
        if(folder_check_general(source_dir,4)!=0){
            return -5;
        }
    }
    if(strcmp(option,"rm")!=0){
        if(folder_check_general(target_dir,6)!=0){
            return -5;
        }
    }
    if(strlen(fuzzy_filename)<1){
        return -1;
    }
#ifdef _WIN32
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s\\*",source_dir);
    handle_find=FindFirstFile(filename_temp,&find_data);
    if(handle_find==INVALID_HANDLE_VALUE){
        return -3;
    }
    do{
        if(strcmp(find_data.cFileName,".")==0||strcmp(find_data.cFileName,"..")==0){
            continue;
        }
        if(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
            continue; /* ONLY copy files, not folders. */
        }
        if(fuzzy_strcmp(find_data.cFileName,fuzzy_filename)!=0){
            continue;
        }
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s\\%s",source_dir,find_data.cFileName);
        if(strcmp(option,"rm")==0){
            if(!DeleteFile(filename_temp)){
                FindClose(handle_find);
                return 1;
            }
        }
        else if(strcmp(option,"mv")==0){
            snprintf(filename_temp2,FILENAME_LENGTH-1,"%s\\%s",target_dir,find_data.cFileName);
            if(rename(filename_temp,filename_temp2)!=0){
                FindClose(handle_find);
                return 1;
            }
        }
        else{
            if(cp_file(filename_temp,target_dir,force_flag)!=0){
                FindClose(handle_find);
                return 1;
            }
        }
    }while(FindNextFile(handle_find,&find_data)!=0);
    FindClose(handle_find); /* Close the handle */ 
    return 0;
#else
    dir=opendir(source_dir);
    if(dir==NULL){
        return -3;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(entry->d_type==DT_DIR){
            continue;
        }
        if(fuzzy_strcmp(entry->d_name,fuzzy_filename)!=0){
            continue;
        }
        snprintf(filename_temp,FILENAME_LENGTH-1,"%s/%s",source_dir,entry->d_name);
        if(strcmp(option,"rm")==0){
            if(unlink(filename_temp)!=0){
                closedir(dir);
                return 1;
            }
        }
        else if(strcmp(option,"mv")==0){
            snprintf(filename_temp2,FILENAME_LENGTH-1,"%s/%s",target_dir,entry->d_name);
            if(rename(filename_temp,filename_temp2)!=0){
                closedir(dir);
                return 1;
            }
        }
        else{
            if(cp_file(filename_temp,target_dir,force_flag)!=0){
                closedir(dir);
                return 1;
            }
        }
    }
    closedir(dir);
    return 0;
#endif
}

/* 
 * Risky: if the file is not plain text, this function may cause infinite
 *        loop because EOF is absent! 
 * return <1, empty
 * return >1, with content
 */
int file_empty_or_not(char* filename){
    if(filename==NULL){
        return NULL_PTR_ARG;
    }
    FILE* file_p=fopen(filename,"r");
    char temp_line[8]="";
    int line_num=0;
    if(file_p==NULL){
        return -1;
    }
    else{
        while(fngetline(file_p,temp_line,8)!=1){
            line_num++;
        }
        fclose(file_p);
        return line_num;
    }
}

/* 
 * return 0: exists
 * return non-zero: not exists (or not writable)
 */
int folder_exist_or_not(char* foldername){
    if(foldername==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    if(GetFileAttributes(foldername)&FILE_ATTRIBUTE_DIRECTORY){
        if(access(foldername,6)==0){
            return 0;
        }
    }
    return 1;
#else
    struct stat dirstat;
    if(stat(foldername,&dirstat)==-1){
        return 1;
    }
    if(S_ISDIR(dirstat.st_mode)){
        if(access(foldername,6)==0){
            return 0;
        }
    }
    return 1;
#endif
}

/* 
 * return 0: exists
 * return 1: not exists (or not writable)
 * return -1: premission code is invalid (0-exist, 2-writable, 4-readable, 6-r&W) 
 */
int folder_check_general(char* foldername, int rw_flag){
    if(rw_flag!=0&&rw_flag!=2&&rw_flag!=4&&rw_flag!=6){
        return -1;
    }
    if(foldername==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    if(GetFileAttributes(foldername)&FILE_ATTRIBUTE_DIRECTORY){
        if(access(foldername,rw_flag)==0){
            return 0;
        }
    }
    return 1;
#else
    struct stat dirstat;
    if(stat(foldername,&dirstat)==-1){
        return 1;
    }
    if(S_ISDIR(dirstat.st_mode)){
        if(access(foldername,rw_flag)==0){
            return 0;
        }
    }
    return 1;
#endif
}

/* 
 * Return:  0 - Successfully checked and empty;
 *          1 - Successfully checked and NOT empty
 *         -1 - Failed to check 
 */
int folder_empty_or_not(char* foldername){
    if(foldername==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    HANDLE handle_find;
    WIN32_FIND_DATA find_data;
    char sub_search_pattern[MAX_PATH]=""; /* Windows max path length is 260, which is MAX_PATH*/
    snprintf(sub_search_pattern,MAX_PATH-1,"%s\\*",foldername);
    handle_find=FindFirstFile(sub_search_pattern,&find_data);
    if(handle_find==INVALID_HANDLE_VALUE){
        return -1; /* Failed to open the dir */
    }
    do{
        if(strcmp(find_data.cFileName,".")!=0&&strcmp(find_data.cFileName,"..")!=0){
            FindClose(handle_find);
            return 1; /* If iterm(s) found, not empty*/
        }
    }while(FindNextFile(handle_find,&find_data)!=0);
    FindClose(handle_find); /* Close the handle */
    return 0; /* Folder is empty. */
#else
    DIR* dir=NULL;
    struct dirent* entry=NULL;
    dir=opendir(foldername);
    if(dir==NULL){
        return -1; /* Failed to open the dir. */
    }
    errno=0;
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")!=0&&strcmp(entry->d_name,"..")!=0){
            closedir(dir);
            return 1; /* If iterm(s) found, not empty*/
        }
    }
    closedir(dir);
    if(errno!=0){
        return -1; /* readdir() reports error. */
    }
    return 0; /* Folder is empty. */
#endif
}

/* 
 * Delete a file or a folder(if it is a folder) *by force!!!* 
 * This function is going to be deprecated. Use rm_file_or_dir instead.
 */
int delete_file_or_dir(char* file_or_dir){
    char cmdline[CMDLINE_LENGTH]="";
    if(file_or_dir==NULL){
        return NULL_PTR_ARG;
    }
    if(file_exist_or_not(file_or_dir)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FILE_CMD,file_or_dir,SYSTEM_CMD_REDIRECT_NULL);
        return system(cmdline);
    }
    if(folder_exist_or_not(file_or_dir)==0){
        snprintf(cmdline,CMDLINE_LENGTH-1,"%s %s %s",DELETE_FOLDER_CMD,file_or_dir,SYSTEM_CMD_REDIRECT_NULL);
        return system(cmdline);
    }
    return -1;
}

int64_t get_filesize_byte(FILE* file_p){
    int64_t current,length;
    if(file_p==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    current=_ftelli64(file_p);
    fseek(file_p,0L,SEEK_END);
    length=_ftelli64(file_p);
    _fseeki64(file_p,current,SEEK_SET);
#else
    current=ftello(file_p);
    fseeko(file_p,0L,SEEK_END);
    length=ftello(file_p);
    fseeko(file_p,current,SEEK_SET);
#endif
    return length;
}

int64_t get_filesize_byte_byname(char* filename){
    FILE *file_p=fopen(filename,"rb");
    if(file_p){
        int64_t size=get_filesize_byte(file_p);
        fclose(file_p);
        return size;
    }
    return NULL_PTR_ARG;
}

/* 
 * This function is to replace the delete_file_or_dir function 
 */
int rm_file_or_dir(char* file_or_dir){
    if(file_or_dir==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(file_or_dir)<1){
        return -3;
    }
    if(file_exist_or_not(file_or_dir)==0){
#ifdef _WIN32
        if(DeleteFile(file_or_dir)!=0){
            return 0;
        }
        return 1;
#else
        if(unlink(file_or_dir)==0){
            return 0;
        }
        return 1;
#endif
    }
    if(folder_exist_or_not(file_or_dir)==0){
        if(rm_pdir(file_or_dir)==0){
            return 0;
        }
        return 1;
    }
    return -1;
}

/*
 * return 0: The password is complex enough
 * return 1: The password is not complex enough
 */
int password_complexity_check(char* password, char* special_chars){
    if(password==NULL||special_chars==NULL){
        return NULL_PTR_ARG;
    }
    int i,length=strlen(password);
    int uppercase_flag=0;
    int lowercase_flag=0;
    int number_flag=0;
    int special_ch_flag=0;
    char ch_temp[2]={'\0','\0'};
    
    for(i=0;i<length;i++){
        if(*(password+i)=='A'||*(password+i)=='Z'){
            uppercase_flag=1;
        }
        else if(*(password+i)>'A'&&*(password+i)<'Z'){
            uppercase_flag=1;
        }
        else if(*(password+i)=='a'||*(password+i)=='z'){
            lowercase_flag=1;
        }
        else if(*(password+i)>'a'&&*(password+i)<'z'){
            lowercase_flag=1;
        }
        else if(*(password+i)=='0'||*(password+i)=='9'){
            number_flag=1;
        }
        else if(*(password+i)>'0'&&*(password+i)<'9'){
            number_flag=1;
        }
        else{
            *(ch_temp)=*(password+i);
            if(contain_or_nnot(special_chars,ch_temp)>0){
                special_ch_flag=1;
            }
            else{
                return 1;
            }
        }
    }
    if((uppercase_flag+lowercase_flag+number_flag+special_ch_flag)<3){
        return 1;
    }
    else{
        return 0;
    }
}

/*
 * This function is *strict*. The password_array_len must <= the actual length of password_password[]
 * The actual length of the generated password is array_len-1;
 * The special_chars_array_len should <= actual length of special_chars_array
 * The minimum length is 4
 * return -1: array length is too short
 * return -3: special_chars_string is invalid
 * return -5: memory allocation error
 * return  1: max loop 
 * return  0: good password generated
 */
int generate_random_npasswd(char password_array[], unsigned int password_array_len, char special_chars_array[], unsigned int special_chars_array_len){
    int i;
    char special_ch;
    const char* ch_table_base="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int total_times,rand_num;
    struct timespec current_time;
    unsigned int seed_num;

    if(password_array==NULL||special_chars_array==NULL){
        return NULL_PTR_ARG;
    }
    memset(password_array,'\0',password_array_len);
    if(password_array_len<5){
        return -1;
    }
    if(special_chars_array_len<1){
        return -3;
    }
    /* The special_chars_string should not contain base chars '0'~'9' 'A'-'B' 'a'-'z' and other unprintable chars (0~32)*/
    for(i=0;i<special_chars_array_len;i++){
        special_ch=*(special_chars_array+i);
        if(special_ch>32&&special_ch<48){ /* !"#$%&,()*+,-./ */
            continue;
        }
        else if(special_ch>57&&special_ch<65){ /* :;<=>?@ */
            continue;
        }
        else if(special_ch>90&&special_ch<97){ /* [\]^_` */
            continue;
        }
        else if(special_ch>122&&special_ch<127){ /* {|}~ */
            continue;
        }
        else{
            return -3;
        }
    }
    /*Added a '\0' to use the complexity check function*/
    char* special_chars_string=(char*)malloc(sizeof(char)*(special_chars_array_len+1));
    if(special_chars_string==NULL){
        return -5;
    } 
    memcpy(special_chars_string,special_chars_array,special_chars_array_len);
    *(special_chars_string+special_chars_array_len)='\0';
    int ch_table_length=62+strlen(special_chars_string);
    char* ch_table_final=(char*)malloc(sizeof(char)*ch_table_length);
    if(ch_table_final==NULL){
        free(special_chars_string);
        return -5;
    }
    memset(ch_table_final,'\0',ch_table_length);
    memcpy(ch_table_final,ch_table_base,62);
    memcpy(ch_table_final+62,special_chars_string,strlen(special_chars_string));
    char* password_temp=(char*)malloc(sizeof(char)*password_array_len);
    if(password_temp==NULL){
        free(special_chars_string);
        free(ch_table_final);
        return -5;
    }
    memset(password_temp,'\0',password_array_len);
    for(total_times=0;total_times<16;total_times++){
        i=0;
        clock_gettime(CLOCK_MONOTONIC,&current_time);
            /* create a random seed num using the time */
        seed_num=(unsigned int)(current_time.tv_sec*1000000000+current_time.tv_nsec);
        srand(seed_num+i);
        while(i<password_array_len-1){
            /* generate a random number as the index of the char table for password */
            rand_num=rand()%ch_table_length;
            /* get a char to paddle the password */
            *(password_temp+i)=*(ch_table_final+rand_num);
            i++;
        }
        /* If the password is complex enough, then copy the password, free mem, and exit 0*/
        if(password_complexity_check(password_temp,special_chars_string)==0){
            memcpy(password_array,password_temp,password_array_len);
            free(special_chars_string);
            free(ch_table_final);
            free(password_temp);
            return 0;
        }
        if(total_times!=15){
            memset(password_temp,'\0',password_array_len);
        }
        else{
            /* If max loop, then export the last loop of password, but the complexity is not guaranteed */
            memcpy(password_array,password_temp,password_array_len);
        }
    }
    free(special_chars_string);
    free(ch_table_final);
    free(password_temp);
    return 1;
}

/* Make sure the password[] is 20 width */
int generate_random_passwd(char* password){
    int i,total_times,rand_num;
    struct timespec current_time;
    unsigned int seed_num;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    char password_temp[PASSWORD_STRING_LENGTH]="";

    if(password==NULL){
        return NULL_PTR_ARG;
    }
    for(total_times=0;total_times<16;total_times++){
        clock_gettime(CLOCK_MONOTONIC,&current_time);
        /* create a random seed num using the time */
        seed_num=(unsigned int)(current_time.tv_sec*1000000000+current_time.tv_nsec);
        srand(seed_num+i);
        for(i=0;i<PASSWORD_LENGTH;i++){
            rand_num=rand()%72;
            *(password_temp+i)=*(ch_table+rand_num);
        }
        if(password_complexity_check(password_temp,"~@&(){}[]=")==0){
            strcpy(password,password_temp);
            return 0;
        }
        if(total_times!=15){
            reset_nstring(password_temp,20);
        }
        else{
            strcpy(password,password_temp);
        }
    }
    return 1;
}

/* 
 * Minimum length is 9 with a '\0' 
 * It resets the password string
 */
int generate_random_db_passwd(char password[], unsigned int len_max){
    int i,rand_num;
    struct timespec current_time;
    unsigned int seed_num;
    char ch_table[62]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    if(password==NULL){
        return NULL_PTR_ARG;
    }
    if(len_max<9){
        strcpy(password,"");
        return -1;
    }
    reset_nstring(password,len_max);
    clock_gettime(CLOCK_MONOTONIC,&current_time);
        /* create a random seed num using the time */
    seed_num=(unsigned int)(current_time.tv_sec*1000000000+current_time.tv_nsec);
    srand(seed_num);
    i=0;
    while(i<len_max-1){
        rand_num=rand()%62;
        *(password+i)=*(ch_table+rand_num);
        i++;
    }
    return 0;
}

/* 
 * generate a random string with length RANDSTR_LENGTH(10) and starting with a letter 
 */
int generate_random_string(char* random_string){
    int i,rand_num;
    char ch_table[36]="abcdefghijklmnopqrstuvwxyz0123456789";
    struct timespec current_time;
    unsigned int seed_num;

    if(random_string==NULL){
        return NULL_PTR_ARG;
    }
    clock_gettime(CLOCK_MONOTONIC,&current_time);
    /* create a random seed num using the time */
    seed_num=(unsigned int)(current_time.tv_sec*1000000000+current_time.tv_nsec);
    srand(seed_num);
    rand_num=rand()%26; /* Start with a letter, not 0~9*/
    *(random_string+0)=*(ch_table+rand_num);
    i=1;
    while(i<RANDSTR_LENGTH_PLUS-1){
        rand_num=rand()%36;
        *(random_string+i)=*(ch_table+rand_num);
        i++;
    }
    *(random_string+RANDSTR_LENGTH_PLUS-1)='\0';
    return 0;
}

/* 
 * generate a random string with actual length len_max-1
 * start_flag=0, start only with a letter
 * start_flag!=0,start with letter or number
 */
int generate_random_nstring(char random_string[], unsigned int len_max, int start_flag){
    int i,rand_num;
    struct timespec current_time;
    unsigned int seed_num;
    char ch_table[36]="abcdefghijklmnopqrstuvwxyz0123456789";
    if(random_string==NULL){
        return NULL_PTR_ARG;
    }
    if(len_max<2){
        strcpy(random_string,"");
        return -1;
    }
    reset_nstring(random_string,len_max);
    clock_gettime(CLOCK_MONOTONIC,&current_time);
    /* create a random seed num using the time */
    seed_num=(unsigned int)(current_time.tv_sec*1000000000+current_time.tv_nsec);
    srand(seed_num);
    if(start_flag==0){
        rand_num=rand()%26; /* Start with a letter, not 0~9*/
    }
    else{
        rand_num=rand()%36; /* Start with a letter, not 0~9*/
    }
    *(random_string+0)=*(ch_table+rand_num);
    i=1;
    while(i<len_max-1){
        rand_num=rand()%36;
        *(random_string+i)=*(ch_table+rand_num);
        i++;
    }
    /*printf("#%s\n",random_string);*/
    return 0;
}

/* 
 * Get a password from stdin securely (without echo)
 * CAUTION: The pass_length includes a '\0' 
 */
int getpass_stdin(char* prompt, char pass_string[], unsigned int pass_length){
    char BACKSPACE='\b';
    char ch='\0';
    int i=0;
#ifndef _WIN32
    struct termios prev,new;
    char ENTER='\n';
#else
    char ENTER='\r';
#endif
    if(pass_string==NULL){
        return NULL_PTR_ARG;
    }
    memset(pass_string,'\0',pass_length);
    if(pass_length<2){ /* There should at least be one char for input*/
        return -3;
    }
    if(prompt!=NULL&&strlen(prompt)>0){
        printf("%s" GENERAL_BOLD "[s]" RESET_DISPLAY ,prompt);
    }
    else{
        printf(GENERAL_BOLD "[s]" RESET_DISPLAY);
    }
#ifdef _WIN32
    while((ch=_getch())!=ENTER&&i<pass_length-1){
        if(ch!=BACKSPACE&&ch!='\t'&&ch!=' '){
            pass_string[i]=ch;
            putchar('*');
            i++;
        }
        else if(ch==BACKSPACE){
            if(i==0){
                continue;
            }
            else{
                printf("\b \b");
                i--;
                pass_string[i]='\0';
            }
        }
    }
#else
    if(tcgetattr(fileno(stdin),&prev)!=0){
        return -1;
    }
    new=prev;
    new.c_lflag&=~ECHO;
    if(tcsetattr(fileno(stdin),TCSAFLUSH,&new)!=0){
        return -1;
    }
    while((ch=getchar())!=ENTER&&i<pass_length-1){
        if(ch!=BACKSPACE&&ch!='\t'&&ch!=' '){
            pass_string[i]=ch;
            i++;
        }
        else if(ch==BACKSPACE){
            if(i==0){
                continue;
            }
            else{
                i--;
                pass_string[i]='\0';
            }
        }
    }
    tcsetattr(fileno(stdin),TCSAFLUSH,&prev);
#endif
    printf("\n");
    return 0;
}

/*
 * CAUTION: THIS IS NOT SUITABLE FOR *NIX 
 */
#ifdef _WIN32
char* getpass_win(char* prompt){
    static char passwd[AKSK_LENGTH];
    char ch='\0';
    int i=0;
    if(prompt!=NULL){
        printf("%s",prompt);
    }
    else{
        printf(":");
    }
    char BACKSPACE='\b';
    char ENTER='\r';
    while((ch=_getch())!=ENTER&&i!=AKSK_LENGTH-1){
        if(ch!=BACKSPACE&&ch!='\t'&&ch!=' '){
            passwd[i]=ch;
            putchar('*');
            i++;
        }
        else if(ch==BACKSPACE){
            if(i==0){
                continue;
            }
            else{
                printf("\b \b");
                i--;
                passwd[i]='\0';
            }
        }
    }
    passwd[i]='\0';
    printf("\n");
    return passwd;
}
#endif

int insert_lines(char* filename, char* keyword, char* insert_string){
    if(filename==NULL||keyword==NULL||insert_string==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(keyword)==0||strlen(insert_string)==0){
        return -1;
    }
    if(file_exist_or_not(filename)!=0){
        return -3;
    }
    FILE* file_p=fopen(filename,"r");
    FILE* file_p_2=NULL;
    char filename_temp[FILENAME_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    int line_num=0;
    int i;
    while(fgetline(file_p,single_line)==0){
        if(contain_or_not(single_line,keyword)==0){
            break;
        }
        else{
            line_num++;
        }
    }
    fseek(file_p,0,SEEK_SET);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    file_p_2=fopen(filename_temp,"w+");
    if(file_p_2==NULL){
        fclose(file_p);
        return -1;
    }
    for(i=0;i<line_num;i++){
        fgetline(file_p,single_line);
        fprintf(file_p_2,"%s\n",single_line);
    }
    fprintf(file_p_2,"%s\n",insert_string);
    while(fgetline(file_p,single_line)==0){
        fprintf(file_p_2,"%s\n",single_line);
    }
    fclose(file_p);
    fclose(file_p_2);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -5;
    }
    return 0;
}

int insert_nlines(char* filename, unsigned int linelen_max, char* keyword, char* insert_string){
    if(filename==NULL||keyword==NULL||insert_string==NULL){
        return NULL_PTR_ARG;
    }
    if(linelen_max<1||strlen(keyword)==0||strlen(insert_string)==0){
        return -1;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -3;
    }
    FILE* file_p_2=NULL;
    int contain_flag=0;
    char filename_temp[FILENAME_LENGTH]="";
    char* single_line=(char*)malloc(sizeof(char)*linelen_max);
    if(single_line==NULL){
        fclose(file_p);
        return -5;
    }
    int line_num=0;
    int i;
    while(fngetline(file_p,single_line,linelen_max)!=1){
        if(contain_or_nnot(single_line,keyword)>0){
            contain_flag=1;
            break;
        }
        else{
            line_num++;
        }
    }
    if(contain_flag==0){
        fclose(file_p);
        free(single_line);
        return 1;
    }
    fseek(file_p,0,SEEK_SET);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    file_p_2=fopen(filename_temp,"w+");
    if(file_p_2==NULL){
        fclose(file_p);
        free(single_line);
        return -3;
    }
    for(i=0;i<line_num;i++){
        fngetline(file_p,single_line,linelen_max);
        fprintf(file_p_2,"%s\n",single_line);
    }
    fprintf(file_p_2,"%s\n",insert_string);
    while(fngetline(file_p,single_line,linelen_max)==0){
        fprintf(file_p_2,"%s\n",single_line);
    }
    fclose(file_p);
    fclose(file_p_2);
    free(single_line);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -7;
    }
    return 0;
}

/* This function is deprecated. */
int local_path_parser(char* path_string, char* path_final){
    if(path_string==NULL||path_final==NULL){
        return NULL_PTR_ARG;
    }
#ifdef _WIN32
    strcpy(path_final,path_string);
    return 0;
#endif
    int i;
    char path_temp[DIR_LENGTH]="";
    if(strlen(path_string)==0){
        strcpy(path_final,"");
        return 0;
    }
    if(*(path_string+0)=='~'){
        for(i=1;i<strlen(path_string);i++){
            *(path_temp+i-1)=*(path_string+i);
        }
#ifdef __linux__
        sprintf(path_final,"/home/hpc-now%s",path_temp);
#elif __APPLE__
        sprintf(path_final,"/Users/hpc-now%s",path_temp);
#else
        strcpy(path_final,path_string);
        return 1;
#endif
    }
    else{
        strcpy(path_final,path_string);
    }
    return 0;
}

int local_path_nparser(char* path_string, char path_final[], unsigned int path_final_len_max){
    if(path_string==NULL||path_final==NULL){
        return NULL_PTR_ARG;
    }
    if(path_final_len_max<2){
        strcpy(path_final,"");
        return 1;
    }
#ifdef _WIN32
    strncpy(path_final,path_string,path_final_len_max-1);
    return 0;
#endif
    int i;
    char path_temp[DIR_LENGTH]="";
    if(strlen(path_string)==0){
        reset_nstring(path_final,path_final_len_max);
        return 0;
    }
    if(*(path_string+0)=='~'){
        for(i=1;i<strlen(path_string)&&i<DIR_LENGTH-1;i++){
            *(path_temp+i-1)=*(path_string+i);
        }
#ifdef __linux__
        snprintf(path_final,path_final_len_max-1,"/home/hpc-now%s",path_temp);
#elif __APPLE__
        snprintf(path_final,path_final_len_max-1,"/Users/hpc-now%s",path_temp);
#else
        strncpy(path_final,path_string,path_final_len_max-1);
        return 0;
#endif
    }
    else{
        strncpy(path_final,path_string,path_final_len_max-1);
    }
    return 0;
}

/* This function is deprecated. */
int direct_path_check(char* path_string, char* hpc_user, char* real_path){
    char header[256]="";
    char tail[DIR_LENGTH]="";
    int i=0;
    int j;
    if(path_string==NULL||hpc_user==NULL||real_path==NULL){
        return NULL_PTR_ARG;
    }
    while(*(path_string+i)!='/'&&i<strlen(path_string)){
        *(header+i)=*(path_string+i);
        i++;
    }
    for(j=i+1;j<strlen(path_string);j++){
        *(tail+j-i-1)=*(path_string+j);
    }
    if(strcmp(header,"@h")==0){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/root/%s",tail);
        }
        else{
            sprintf(real_path,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@d")==0){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/hpc_data/%s",tail);
        }
        else{
            sprintf(real_path,"/hpc_data/%s_data/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@a")==0){
        if(strcmp(hpc_user,"root")==0){
            sprintf(real_path,"/hpc_apps/%s",tail);
        }
        else{
            sprintf(real_path,"/hpc_apps/%s_apps/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@p")==0){
        sprintf(real_path,"/hpc_data/public/%s",tail);
        return 0;
    }
    else if(strcmp(header,"@R")==0){
        if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
            sprintf(real_path,"/%s",tail);
        }
        else{
            sprintf(real_path,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@t")==0){
        sprintf(real_path,"/tmp/%s",tail);
        return 0;
    }
    else{
        strcpy(real_path,path_string);
        return 1;
    }
}

/* real_path_len=width-1*/
int direct_path_ncheck(char* path_string, char* hpc_user, char* real_path, unsigned int real_path_len_max){
    char header[8]="";
    char tail[DIR_LENGTH]="";
    int i=0;
    int j;
    if(real_path_len_max<1){
        strcpy(real_path,"");
        return -1;
    }
    if(path_string==NULL||hpc_user==NULL||real_path==NULL){
        return NULL_PTR_ARG;
    }
    while(*(path_string+i)!='/'&&i<strlen(path_string)&&i<7){
        *(header+i)=*(path_string+i);
        i++;
    }
    if(i>4){
        strncpy(real_path,path_string,real_path_len_max-1);
        return 1;
    }
    for(j=i+1;j<strlen(path_string);j++){
        if(j-i-1<DIR_LENGTH){
            *(tail+j-i-1)=*(path_string+j);
        }
    }
    if(strcmp(header,"@h")==0){
        if(strcmp(hpc_user,"root")==0){
            snprintf(real_path,real_path_len_max-1,"/root/%s",tail);
        }
        else{
            snprintf(real_path,real_path_len_max-1,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@d")==0){
        if(strcmp(hpc_user,"root")==0){
            snprintf(real_path,real_path_len_max-1,"/hpc_data/%s",tail);
        }
        else{
            snprintf(real_path,real_path_len_max-1,"/hpc_data/%s_data/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@a")==0){
        if(strcmp(hpc_user,"root")==0){
            snprintf(real_path,real_path_len_max-1,"/hpc_apps/%s",tail);
        }
        else{
            snprintf(real_path,real_path_len_max-1,"/hpc_apps/%s_apps/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@p")==0){
        snprintf(real_path,real_path_len_max-1,"/hpc_data/public/%s",tail);
        return 0;
    }
    else if(strcmp(header,"@R")==0){
        if(strcmp(hpc_user,"root")==0||strcmp(hpc_user,"user1")==0){
            snprintf(real_path,real_path_len_max-1,"/%s",tail);
        }
        else{
            snprintf(real_path,real_path_len_max-1,"/home/%s/%s",hpc_user,tail);
        }
        return 0;
    }
    else if(strcmp(header,"@t")==0){
        snprintf(real_path,real_path_len_max-1,"/tmp/%s",tail);
        return 0;
    }
    else{
        strncpy(real_path,path_string,real_path_len_max-1);
        return 1;
    }
}

int file_creation_test(char* filename){
    if(filename==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(filename)==0){
        return -1;
    }
    if(file_exist_or_not(filename)==0){
        return 1;
    }
    FILE* file_p=fopen(filename,"w+");
    if(file_p==NULL){
        return 3;
    }
    fclose(file_p);
    rm_file_or_dir(filename);
    return 0;
}

int cmd_flg_or_not(char* argv){
    int i;
    if(argv==NULL){
        return NULL_PTR_ARG;
    }
    for(i=0;i<CMD_FLAG_NUM;i++){
        if(strcmp(argv,command_flags[i])==0){
            return 0;
        }
    }
    return 1;
}

int cmd_key_or_not(char* argv){
    int i;
    if(argv==NULL){
        return NULL_PTR_ARG;
    }
    for(i=0;i<CMD_KWDS_NUM;i++){
        if(strcmp(argv,command_keywords[i])==0){
            return 0;
        }
    }
    return 1;
}

int cmd_flag_check(int argc, char** argv, char* flag_string){
    int i;
    if(argv==NULL||flag_string==NULL){
        return NULL_PTR_ARG;
    }
    for(i=2;i<argc;i++){
        if(argv[i]==NULL){
            continue;
        }
        if(strcmp(argv[i],flag_string)==0){
            return 0;
        }
    }
    return 1;
}

/*
 * return 0: found the keyword
 * return 1: not found
 * make sure the dest array has 128 width.
 */
int cmd_keyword_check(int argc, char** argv, char* key_word, char* kwd_string){
    int i,j;
    if(argv==NULL||key_word==NULL||kwd_string==NULL){
        return NULL_PTR_ARG;
    }
    for(i=2;i<argc-1;i++){
        if(argv[i]==NULL){
            continue;
        }
        if(strcmp(argv[i],key_word)==0){
            j=i+1;
            if(cmd_flg_or_not(argv[j])!=0&&cmd_key_or_not(argv[j])!=0){
                strncpy(kwd_string,argv[j],127);
                return 0;
            }
            else{
                strcpy(kwd_string,"");
                return 1;
            }
        }
    }
    strcpy(kwd_string,"");
    return 1;
}

/*
 * return 0: found the keyword
 * return 1: not found
 */
int cmd_keyword_ncheck(int argc, char** argv, char* key_word, char* kwd_string, unsigned int n){
    int i,j;
    if(argv==NULL||key_word==NULL||kwd_string==NULL){
        return NULL_PTR_ARG;
    }
    if(n<8){
        return 1;
    }
    for(i=2;i<argc-1;i++){
        if(argv[i]==NULL){
            continue;
        }
        if(strcmp(argv[i],key_word)==0){
            j=i+1;
            if(cmd_flg_or_not(argv[j])!=0&&cmd_key_or_not(argv[j])!=0){
                strncpy(kwd_string,argv[j],n-1);
                return 0;
            }
            else{
                strcpy(kwd_string,"");
                return 1;
            }
        }
    }
    strcpy(kwd_string,"");
    return 1;
}

int include_string_or_not(int cmd_c, char** cmds, char* string){
    int i;
    if(cmds==NULL||string==NULL){
        return NULL_PTR_ARG;
    }
    for(i=0;i<cmd_c;i++){
        if(cmds[i]==NULL){
            continue;
        }
        if(strcmp(cmds[i],string)==0){
            return 0;
        }
    }
    return 1;
}

int file_cr_clean(char* filename){
#ifndef _WIN32 /* For Windows 32 environment, there is no need to clean the \r char. */
    if(file_exist_or_not(filename)!=0){
        return -1;
    }
    FILE* file_p=fopen(filename,"r");
    char filename_temp[FILENAME_LENGTH]="";
    int ch;
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    do{
        ch=fgetc(file_p);
        if(ch==EOF){
            break;
        }
        else{
            if(ch!='\r'){
                fputc(ch,file_p_tmp);
            }
            else{
                fputc('\0',file_p_tmp);
            }
        }
    }while(!feof(file_p));
    fclose(file_p);
    fclose(file_p_tmp);
    rm_file_or_dir(filename);
    if(rename(filename_temp,filename)!=0){
        return -3;
    }
#endif
    return 0;
}

/* This function is risky! It overwrites the original file*/
int file_trunc_by_kwds(char* filename, char* start_key, char* end_key, int overwrite_flag){
    char filename_temp[FILENAME_LENGTH]="";
    char line_buffer[LINE_LENGTH]="";
    int start_flag=0;
    int contain_start_flag;
    int contain_end_flag;
    if(file_exist_or_not(filename)!=0){
        return -1;
    }
    if(start_key==NULL||end_key==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(start_key)==0&&strlen(end_key)==0){
        return 3;
    }
    if(strcmp(start_key,end_key)==0){
        return 5;
    }
    FILE* file_p=fopen(filename,"r");
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.trunc%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    while(!feof(file_p)){
        fgetline(file_p,line_buffer);
        if(strlen(start_key)==0){
            if(contain_or_not(line_buffer,end_key)!=0){
                fprintf(file_p_tmp,"%s\n",line_buffer);
            }
            else{
                break;
            }
        }
        else{
            contain_start_flag=contain_or_not(line_buffer,start_key);
            if(strlen(end_key)!=0){
                contain_end_flag=contain_or_not(line_buffer,end_key);
            }
            else{
                contain_end_flag=-1;
            }
            if(contain_start_flag!=0&&start_flag==0){
                continue;
            }
            else if(contain_start_flag==0&&start_flag==0){
                if(contain_end_flag==0){
                    break;
                }
                fprintf(file_p_tmp,"%s\n",line_buffer);
                start_flag=1;
            }
            else{
                if(contain_end_flag==0){
                    break;
                }
                fprintf(file_p_tmp,"%s\n",line_buffer);
            }
        }
    }
    fclose(file_p);
    fclose(file_p_tmp);
    if(overwrite_flag!=0){
        rm_file_or_dir(filename);
        if(rename(filename_temp,filename)!=0){
            return 1;
        }
    }
    return 0;
}

/*
 * If overwrite_flag =0: not overwrite
 * If overwrite_flag!=0: overwrite
 */
int file_ntrunc_by_kwds(char* filename, unsigned int linelen_max, char* start_key, char* end_key, int overwrite_flag){
    char filename_temp[FILENAME_LENGTH]="";
    int start_flag=0;
    int contain_start_flag;
    int contain_end_flag;
    if(filename==NULL||start_key==NULL||end_key==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(start_key)==0&&strlen(end_key)==0){
        return -3;
    }
    if(linelen_max<1){
        return -3;
    }
    if(strcmp(start_key,end_key)==0){
        return -3;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.trunc%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    char* line_buffer=(char*)malloc(sizeof(char)*linelen_max);
    if(line_buffer==NULL){
        fclose(file_p);
        fclose(file_p_tmp);
        return -5;
    }
    while(fngetline(file_p,line_buffer,linelen_max)!=1){
        if(strlen(start_key)==0){
            if(contain_or_nnot(line_buffer,end_key)<1){
                fprintf(file_p_tmp,"%s\n",line_buffer);
            }
            else{
                break;
            }
        }
        else{
            contain_start_flag=contain_or_nnot(line_buffer,start_key);
            if(strlen(end_key)!=0){
                contain_end_flag=contain_or_nnot(line_buffer,end_key);
            }
            else{
                contain_end_flag=0;
            }
            if(contain_start_flag<1&&start_flag==0){
                continue;
            }
            else if(contain_start_flag>0&&start_flag==0){
                if(contain_end_flag>0){
                    break;
                }
                fprintf(file_p_tmp,"%s\n",line_buffer);
                start_flag=1;
            }
            else{
                if(contain_end_flag>0){
                    break;
                }
                fprintf(file_p_tmp,"%s\n",line_buffer);
            }
        }
    }
    free(line_buffer);
    fclose(file_p);
    fclose(file_p_tmp);
    if(overwrite_flag!=0){
        rm_file_or_dir(filename);
        if(rename(filename_temp,filename)!=0){
            return 1;
        }
    }
    return 0;
}

/*
 * overwrite flag =0, not replace
 * overwrite flag !=0, replace.
 */
int delete_lines_by_kwd(char* filename, char* key, int overwrite_flag){
    char filename_temp[FILENAME_LENGTH]="";
    char line_buffer[LINE_LENGTH]="";
    int getline_flag=0;
    if(filename==NULL||key==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(key)==0){
        return -3;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -1;
    }
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.dline%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    while(!feof(file_p)){
        getline_flag=fgetline(file_p,line_buffer);
        if(contain_or_not(line_buffer,key)==0){
            continue;
        }
        if(getline_flag==0){
            fprintf(file_p_tmp,"%s\n",line_buffer);
        }
    }
    fclose(file_p);
    fclose(file_p_tmp);
    if(overwrite_flag!=0){
        rm_file_or_dir(filename);
        if(rename(filename_temp,filename)!=0){
            return 1;
        }
    }
    return 0;
}

/*
 * overwrite_flag!=0: overwrite, ==0: not overwrite
 */
int delete_nlines_by_kwd(char* filename, unsigned int linelen_max, char* key, int overwrite_flag){
    if(filename==NULL||key==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(key)==0){
        return -3;
    }
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s.dline%s",filename,GFUNC_FILE_SUFFIX);
    FILE* file_p_tmp=fopen(filename_temp,"w+");
    if(file_p_tmp==NULL){
        fclose(file_p);
        return -1;
    }
    char* line_buffer=(char*)malloc(sizeof(char)*linelen_max);
    if(line_buffer==NULL){
        fclose(file_p);
        fclose(file_p_tmp);
        return -5;
    }
    int getline_flag=0;
    while(!feof(file_p)){
        getline_flag=fngetline(file_p,line_buffer,linelen_max);
        if(contain_or_nnot(line_buffer,key)>0){
            continue;
        }
        if(getline_flag==0){
            fprintf(file_p_tmp,"%s\n",line_buffer);
        }
    }
    free(line_buffer);
    fclose(file_p);
    fclose(file_p_tmp);
    if(overwrite_flag!=0){
        rm_file_or_dir(filename);
        if(rename(filename_temp,filename)!=0){
            return 1;
        }
    }
    return 0;
}

/*
 * return 0: successfully get md5sum
 * return -1: failed to get the md5sum
 * This function is deprecated. Please use get_nmd5sum()
 */

int get_crypto_key(char* crypto_key_filename, char* md5sum){
    char cmdline[CMDLINE_LENGTH]="";
    FILE* md5_tmp=NULL;
#ifdef _WIN32
    char buffer[256]="";
#endif
    if(crypto_key_filename==NULL||md5sum==NULL){
        return NULL_PTR_ARG;
    }
#ifdef __APPLE__
    snprintf(cmdline,CMDLINE_LENGTH-1,"md5 '%s' | awk '{print $NF}' > /tmp/md5.txt.tmp",crypto_key_filename);
#elif __linux__
    snprintf(cmdline,CMDLINE_LENGTH-1,"md5sum '%s' | awk '{print $1}' > /tmp/md5.txt.tmp",crypto_key_filename);
#elif _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"certutil -hashfile \"%s\" md5 > C:\\ProgramData\\md5.txt.tmp",crypto_key_filename);
#endif
    if(system(cmdline)!=0){
        return -1;
    }
#ifdef _WIN32
    md5_tmp=fopen("C:\\ProgramData\\md5.txt.tmp","r");
#else
    md5_tmp=fopen("/tmp/md5.txt.tmp","r");
#endif
    if(md5_tmp==NULL){
        return -1;
    }
#ifdef _WIN32
    fgetline(md5_tmp,buffer);
#endif
    fgetline(md5_tmp,md5sum);
    fclose(md5_tmp);
#ifdef _WIN32
    snprintf(cmdline,CMDLINE_LENGTH-1,"del /f /q C:\\ProgramData\\md5.txt.tmp %s",SYSTEM_CMD_REDIRECT_NULL);
#else
    snprintf(cmdline,CMDLINE_LENGTH-1,"rm -rf /tmp/md5.txt.tmp %s",SYSTEM_CMD_REDIRECT_NULL);
#endif
    system(cmdline);
    return 0;
}

/* 
 * Please *DO* make sure the md5sum_length equals to the md5sum_string array length
 * return -3: the given length is invalid
 * return 1: get_md5 failed
 * return 0: get_md5 succeeded
 */
int get_nmd5sum(char* filename, char md5sum_string[], int md5sum_length){
    if(filename==NULL||md5sum_string==NULL){
        return NULL_PTR_ARG;
    }
    memset(md5sum_string,'\0',md5sum_length);
    if(md5sum_length<33){
        return -3;
    }
    int run_flag=now_md5_for_file(filename,md5sum_string,md5sum_length);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

int password_md5_hash(char* password, char md5_hash[], int md5_length){
    if(password==NULL||md5_hash==NULL){
        return NULL_PTR_ARG;
    }
    if(strlen(password)==0){
        return -3;
    }
    if(md5_length<33){
        return -5;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[16]="";
    int run_flag;
    generate_random_string(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tmp%spass%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,GFUNC_FILE_SUFFIX);
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        strcpy(md5_hash,"");
        return -1;
    }
    fprintf(file_p,"%s%s",password,CRLF_PASSWORD_HASH);
    fclose(file_p);
    run_flag=get_nmd5sum(filename_temp,md5_hash,md5_length);
    rm_file_or_dir(filename_temp);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

/* Generate the SHA-256 string of the file and cut the [1-32] chars */
int get_file_sha_hash(char* filename, char hash_string[], int hash_length){
    char sha256[65]="";
    if(filename==NULL||hash_string==NULL){
        return NULL_PTR_ARG;
    }
    strcpy(hash_string,"");
    if(hash_length<33){
        return -3;
    }
    int run_flag=now_sha256_for_file(filename,sha256,65);
    if(run_flag!=0){
        return 1;
    }
    /* Cut the 1-32 chars */
    strncpy(hash_string,sha256,32);
    return 0;
}

int get_file_sha_hash_full(char* filename, char hash_string_full[], int hash_length){
    if(filename==NULL||hash_string_full==NULL){
        return NULL_PTR_ARG;
    }
    strcpy(hash_string_full,"");
    if(hash_length<65){
        return -1;
    }
    int run_flag=now_sha256_for_file(filename,hash_string_full,hash_length);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

/* Generate the SHA-256 string of the file and cut the [1-32] chars */
int password_sha_hash(char* password, char hash[], int hash_length){
    char filename_temp[FILENAME_LENGTH]="";
    char string_temp[16]="";
    int run_flag;
    if(password==NULL||hash==NULL){
        return NULL_PTR_ARG;
    }
    strcpy(hash,"");
    if(strlen(password)==0){
        return -3;
    }
    if(hash_length<33){
        return -5;
    }
    generate_random_string(string_temp);
    snprintf(filename_temp,FILENAME_LENGTH-1,"%s%s.tmp%spass%s",HPC_NOW_ROOT_DIR,PATH_SLASH,PATH_SLASH,GFUNC_FILE_SUFFIX);
    FILE* file_p=fopen(filename_temp,"w+");
    if(file_p==NULL){
        return -1;
    }
    fprintf(file_p,"%s%s",password,CRLF_PASSWORD_HASH);
    fclose(file_p);
    run_flag=get_file_sha_hash(filename_temp,hash,hash_length);
    rm_file_or_dir(filename_temp);
    if(run_flag!=0){
        return 1;
    }
    return 0;
}

int windows_path_to_nstring(char* input_string, char new_string[], unsigned int maxlen){
    if(input_string==NULL||new_string==NULL){
        return NULL_PTR_ARG;
    }
    if(maxlen<strlen(input_string)+16){ /* MAXIMUM ADD 15 slashes */
        strcpy(new_string,"");
        return -1;
    }
    reset_nstring(new_string,maxlen);
#ifndef _WIN32
    strncpy(new_string,input_string,maxlen);
    return 0;
#else
    int i,j=0;
    char ch_curr,ch_prev,ch_next;
    int length=strlen(input_string);
    for(i=0;i<length;i++){
        ch_curr=*(input_string+i);
        if(i!=0){
            ch_prev=*(input_string+i-1);
        }
        else{
            ch_prev='\0';
        }
        if(i!=length-1){
            ch_next=*(input_string+i+1);
        }
        else{
            ch_next='\0';
        }
        if(j>maxlen-2){
            strcpy(new_string,"");
            return -1;
        }
        if(ch_curr=='\\'&&ch_prev!='\\'&&ch_next!='\\'){
            new_string[j]='\\';
            new_string[j+1]='\\';
            j=j+2;
        }
        else{
            new_string[j]=ch_curr;
            j++;
        }
    }
    return 0;
#endif
}

/* memory allocated! */
char* base64_clear_CRLF(char orig[], int length){
    if(orig==NULL){
        return NULL;
    }
    char* new_string=(char*)malloc(sizeof(char)*(length+1));
    if(new_string==NULL){
        return NULL;
    }
    memset(new_string,'\0',length+1);
    int j=0;
    for(int i=0;i<length;i++){
        if(orig[i]!='\r'&&orig[i]!='\n'){
            *(new_string+j)=orig[i];
            j++;
        }
    }
    return new_string;
}

/* Convert a base64 char to an unsigned char */
unsigned char get_base64_index(char base64_char){
    if(base64_char=='='){
        return 253;
    }
    else if(base64_char=='+'){
        return 62;
    }
    else if(base64_char=='/'){
        return 63;
    }
    else if(base64_char>64&&base64_char<91){
        return base64_char-65;
    }
    else if(base64_char>47&&base64_char<58){
        return base64_char+4;
    }
    else if(base64_char>96&&base64_char<123){
        return base64_char-71;
    }
    else{
        return 255; /* Illegal! */
    }
}

/* 
 * decode a base64 string and print to a file.
 * return -1: MEM ALLOC FAILED
 * return -3: length invalid
 * return 1: Format invalid
 * return -5: FILE Output error
 * return 0: Normal exit
 */
int base64decode(char* encoded_string, char* export_path){
    if(encoded_string==NULL||export_path==NULL){
        return NULL_PTR_ARG;
    }
    char* encoded_string_new=base64_clear_CRLF(encoded_string,strlen(encoded_string));
    if(encoded_string_new==NULL){
        return -1;
    }
    unsigned long length=strlen(encoded_string_new);
    if(length%4!=0||length<4){
        free(encoded_string_new);
        return -3;
    }
    unsigned char* decoded_string=(unsigned char*)malloc(sizeof(unsigned char)*length);
    if(decoded_string==NULL){
        free(encoded_string_new);
        return -1;
    }
    memset(decoded_string,'\0',length);
    
    unsigned long group=length>>2;
    unsigned long i,j=0;
    unsigned char ch0,ch1,ch2,ch3;
    
    for(i=0;i<group-1;i++){
        ch0=get_base64_index(encoded_string_new[i*4]);
        ch1=get_base64_index(encoded_string_new[i*4+1]);
        ch2=get_base64_index(encoded_string_new[i*4+2]);
        ch3=get_base64_index(encoded_string_new[i*4+3]);
        if(ch0>63||ch1>63||ch2>63||ch3>63){
            free(encoded_string_new);
            free(decoded_string);
            return 1; /* Illegal format */
        }
        *(decoded_string+j)=(ch0<<2)|(ch1>>4);
        *(decoded_string+j+1)=(ch1<<4)|(ch2>>2);
        *(decoded_string+j+2)=(ch2<<6)|ch3;
        j+=3;
    }
    ch0=get_base64_index(encoded_string_new[i*4]);
    ch1=get_base64_index(encoded_string_new[i*4+1]);
    ch2=get_base64_index(encoded_string_new[i*4+2]);
    ch3=get_base64_index(encoded_string_new[i*4+3]);
    if(ch0>63||ch1>63||ch2==255||ch3==255){
        free(encoded_string_new);
        free(decoded_string);
        return 1; /* Illegal format */
    }
    if(ch3!=253&&ch2==253){
        free(encoded_string_new);
        free(decoded_string);
        return 1; /* Illegal format */
    }
    *(decoded_string+j)=(ch0<<2)|(ch1>>4);
    if(ch3==253){
        if(ch2!=253){
            *(decoded_string+j+1)=(ch1<<4)|(ch2>>2);
        }
    }
    else{
        *(decoded_string+j+1)=(ch1<<4)|(ch2>>2);
        *(decoded_string+j+2)=ch2<<6|ch3;
    }
    free(encoded_string_new);
    FILE* file_p=fopen(export_path,"w+");
    if(file_p==NULL){
        free(decoded_string);
        return -5;
    }
    fprintf(file_p,"%s",decoded_string);
    fclose(file_p);
    free(decoded_string);
    return 0;
}

/*
 * return -1: MEM ALLOC FAILED
 * return -3: Original length invalid
 * return -5: FILE output Error
 * return 0: Normal exit
 */
int base64encode(char* plain_string, char* export_path){
    char encode_chars[64]={
        'A','B','C','D','E','F','G','H',
        'I','J','K','L','M','N','O','P',
        'Q','R','S','T','U','V','W','X',
        'Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k','l','m','n',
        'o','p','q','r','s','t','u','v',
        'w','x','y','z','0','1','2','3',
        '4','5','6','7','8','9','+','/'
    };
    if(plain_string==NULL||export_path==NULL){
        return NULL_PTR_ARG;
    }
    unsigned long length=strlen(plain_string);
    if(length<1){
        return -3; /* The original length should be at least 1; */
    }
    char* encoded_string=(char*)malloc(sizeof(char)*((length*3)>>1)); /* Alloc 1.5x mem */
    if(encoded_string==NULL){
        return -1; 
    }
    unsigned long i,j=0;
    unsigned long group=length/3;
    unsigned short extra=length%3;
    memset(encoded_string,'\0',(length*3)>>1);
    for(i=0;i<group;i++){
        *(encoded_string+j)=encode_chars[plain_string[i*3]>>2];
        *(encoded_string+j+1)=encode_chars[((plain_string[i*3]&0x03)<<4)|(plain_string[i*3+1]>>4)];
        *(encoded_string+j+2)=encode_chars[((plain_string[i*3+1]&0x0F)<<2)|(plain_string[i*3+2]>>6)];
        *(encoded_string+j+3)=encode_chars[plain_string[i*3+2]&0x3F];
        j=j+4;
    }
    if(extra==1){
        *(encoded_string+j)=encode_chars[plain_string[i*3]>>2];
        *(encoded_string+j+1)=encode_chars[(plain_string[i*3]&0x03)<<4];
        *(encoded_string+j+2)='=';
        *(encoded_string+j+3)='=';
    }
    else if(extra==2){
        *(encoded_string+j)=encode_chars[plain_string[i*3]>>2];
        *(encoded_string+j+1)=encode_chars[((plain_string[i*3]&0x03)<<4)|(plain_string[i*3+1]>>4)];
        *(encoded_string+j+2)=encode_chars[(plain_string[i*3+1]&0x0F)<<2];
        *(encoded_string+j+3)='=';
    }
    FILE* file_p=fopen(export_path,"w+");
    if(file_p==NULL){
        free(encoded_string);
        return -5;
    }
    fprintf(file_p,"%s",encoded_string);
    free(encoded_string);
    fclose(file_p);
    return 0;
}

/* 
 * This function resets Windows display to make sure the print is good.
 * For *nix, do nothing
 */
int reset_windows_cmd_display(void){
#ifdef _WIN32
    return system("Color");
#else
    return 0;
#endif
}

/*
 * The dir_lenmax should be 128 or greater
 * Althought this is not standard in Windows (MAX_PATH=260)
 * It should be OK.
 */
int get_win_appdata_dir(char appdata[], unsigned int dir_lenmax){
#ifdef _WIN32
    if(appdata==NULL){
        return NULL_PTR_ARG;
    }
    if(dir_lenmax<MAX_PATH){
        return -3;
    }
    if(SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,appdata)!=S_OK){
        return 1;
    }
#endif
    return 0;
}

/* Socket-related functions. */
void* thread_getaddrinfo(void* arg){
    req_info* request=(req_info*)arg;
    struct addrinfo* server_info=NULL;
    int getaddr_result=getaddrinfo(request->req_domain,request->req_port,&(request->req_hints),&server_info);
    if(getaddr_result==0){
        request->req_result=0;
        pthread_exit((void*)server_info);
    }
    if(server_info!=NULL){
        freeaddrinfo(server_info);
    }
    pthread_exit(NULL);
    return NULL;
}

void close_socket(int socket_fd){
#ifdef _WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif
}

/* Check the connect() status */
int sock_connect_errno_check(void){
#ifdef _WIN32
    if(WSAGetLastError()==WSAEWOULDBLOCK){
        return 0;
    }
#else
    if(errno==EINPROGRESS){
        return 0;
    }
#endif
    return 1;
}

/* 
 * Check connectivity using socket.
 * Return -127: NULL Pointer
 *        -3  : WINDOWS Socket initialization error
 *        -1  : Failed to get address
 *         1  : Failed to connect
 *         0  : Connectivity checked successfully
 */
int check_connectivity(const char* domain, const char* port, const unsigned int max_wait_sec){
    int socket_fd=-1;
    struct addrinfo hints;
    struct addrinfo *server_info=NULL;
    struct addrinfo *addr_ptr=NULL;
    int sockopt_error=1;
    socklen_t sockopt_error_len=sizeof(sockopt_error);
    req_info dns_request;
    pthread_t thread_id;
    int thread_c=-1;

    unsigned long long thread_timer_nsec=0;
    void* thread_result=NULL;
    struct timespec sleep_time;
    sleep_time.tv_sec=0;
    sleep_time.tv_nsec=100000000; /* 100 millisecond */

#ifndef _WIN32
    int block_flag=0;
#endif
    fd_set write_fds;
    struct timeval timeout;
    int connectivity_flag=1;

    if(domain==NULL||port==NULL){
        return NULL_PTR_ARG; /* Reject null pointer(s) */
    }
    if(max_wait_sec>10||max_wait_sec<1){
        timeout.tv_sec=1;
    }
    else{
        timeout.tv_sec=max_wait_sec;
    }
    timeout.tv_usec=0;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;

#ifdef _WIN32
    WSADATA win_socket_start;
    WORD version_requested=MAKEWORD(2,2);
    unsigned long non_blocking=1;
    int winsock_result=0;
    if(WSAStartup(version_requested,&win_socket_start)!=0){
        return -3;
    }
#endif
    dns_request.req_domain=(char*)domain;
    dns_request.req_port=(char*)port;
    memcpy(&(dns_request.req_hints),&hints,sizeof(hints));
    dns_request.req_result=-1;
    thread_c=pthread_create(&thread_id,NULL,thread_getaddrinfo,(void*)&dns_request);
    if(thread_c!=0){
        return -3;
    }
    while(thread_timer_nsec<(unsigned long long)max_wait_sec*1000000000){
        if(dns_request.req_result==0){
            pthread_join(thread_id,&thread_result);
            break;
        }
        if((thread_timer_nsec%500000000)==0){
            printf("[ -INFO- ] Checking network connectivity: " GREY_LIGHT "%.1lf" RESET_DISPLAY " / %d s\r",(double)thread_timer_nsec/1000000000,max_wait_sec);
            fflush(stdout);
        }
        thread_timer_nsec+=sleep_time.tv_nsec;
#ifdef _WIN32
        Sleep(sleep_time.tv_nsec/1000000);
#else
        nanosleep(&sleep_time,NULL);
#endif
    }
    printf("                                                      \r");
    fflush(stdout);
    if(thread_result!=NULL){
        server_info=(struct addrinfo*)thread_result;
    }
    else{
        pthread_cancel(thread_id);
#ifdef _WIN32
        WSACleanup();
#endif
        return -1;
    }

    for(addr_ptr=server_info;addr_ptr!=NULL;addr_ptr=addr_ptr->ai_next){
        socket_fd=socket(addr_ptr->ai_family,addr_ptr->ai_socktype,addr_ptr->ai_protocol);
        if(socket_fd==-1){
            continue;
        }
#ifdef _WIN32
        winsock_result=ioctlsocket(socket_fd,FIONBIO,&non_blocking);
        if(winsock_result==SOCKET_ERROR){
            closesocket(socket_fd);
            continue;
        }
#else
        block_flag=fcntl(socket_fd,F_GETFL,0);
        fcntl(socket_fd,F_SETFL,block_flag|O_NONBLOCK);
        errno=0;
#endif
        if(connect(socket_fd,addr_ptr->ai_addr,addr_ptr->ai_addrlen)!=0){
            if(sock_connect_errno_check()!=0){
                close_socket(socket_fd);
                continue;
            }
        }
        FD_ZERO(&write_fds);
        FD_SET(socket_fd,&write_fds);
        if(select(socket_fd+1,NULL,&write_fds,NULL,&timeout)>0){
            if(FD_ISSET(socket_fd,&write_fds)){
                if(getsockopt(socket_fd,SOL_SOCKET,SO_ERROR,(char*)&sockopt_error,&sockopt_error_len)==0){
                    if(sockopt_error==0){
                        connectivity_flag=0;
                        close_socket(socket_fd);
                        break;
                    }
                }
            }
        }
        close_socket(socket_fd);
    }
    freeaddrinfo(server_info);
#ifdef _WIN32
    WSACleanup();
#endif
    return connectivity_flag;
}