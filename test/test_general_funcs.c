/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include "../hpcopr/general_funcs.h"
#include "../hpcopr/now_macros.h"
#else
#include "..\\hpcopr\\general_funcs.h"
#include "..\\hpcopr\\now_macros.h"
#endif

int main(int argc, char** argv){
    FILE* file_p=fopen("test_gfuncs.txt","r");
    if(file_p==NULL){
        return 1;
    }
    char line_buffer[256]="";
    char get_str_buffer[64]="";
    printf("FNGETLINE:\n");
    /*while(fngetline(file_p,line_buffer,128)!=1){
        printf("%s\n",line_buffer);
    }*/
    printf("FNGETLINE_DONE\n");
    fclose(file_p);
    int contain_flag=contain_or_nnot("         \"${aws_s3_bucket.hpc_data_storage.arn}/*\"","ar");
    printf("CONTAINS: %d\n",contain_flag);
    for(int i=0;i<10;i++){
        int get_seq_flag=get_seq_nstring(line_buffer,'(',i+1,get_str_buffer,63);
        if(get_seq_flag==0){
            printf("GET_SEQ_STR: #%d:%d  -> %s %d\n",i+1,get_seq_flag,get_str_buffer,strlen(get_str_buffer));
        }
    }
    printf("GLOBAL_REPLACE: %d\n",global_nreplace("test_gfuncs.txt",128,"_storage.","global_replace_test"));
    int run_flag=get_key_nvalue("test_gfuncs.txt",256,"provider",' ',get_str_buffer,16);
    printf(">%d< >%s<\n",run_flag,get_str_buffer);
    find_and_nreplace("test_gfuncs.txt",256,"Shanghai","-NOW","","","","FIND_AND_REPLACE_TEST.","FNRT");
    printf(">FIND_MULTI_KEYS<   %d\n",find_multi_nkeys("test_gfuncs.txt",256,"","","","",""));
    find_and_nget("test_gfuncs.txt",256,"Effect","Allow\"","",20,"bucket","*","",'.',2,get_str_buffer,64);
    printf(">GET_AND_NGET<  %s\n",get_str_buffer);
    insert_nlines("test_gfuncs.txt",256,"\"${aws_s3_bucket.hpc_dataglobal_replace_testarn}/*\"","***********************inserted!");
    file_trunc_by_kwds("test_gfuncs.txt","*****inserted","]",0);
    file_ntrunc_by_kwds("test_gfuncs.txt",256,"*****inserted","]",0);
    delete_nlines_by_kwd("test_gfuncs.txt",256,"*inserted!",1);
    /*file_p=fopen("test_gfuncs.txt","r");
    if(file_p==NULL){
        return 1;
    }
    printf("FNGETLINE_AFTER:\n");
    while(fngetline(file_p,line_buffer,128)!=1){
        printf("%s\n",line_buffer);
    }
    printf("FNGETLINE_DONE_AFTER\n");
    fclose(file_p);*/
    return 0;
}