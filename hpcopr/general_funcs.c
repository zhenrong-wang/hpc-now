/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <malloc.h>
#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#elif __APPLE__
#include <sys/time.h>
#endif

#include "now_macros.h"
#include "general_funcs.h"

void reset_string(char* orig_string){
    int length=strlen(orig_string);
    int i;
    for(i=0;i<length;i++){
        *(orig_string+i)='\0';
    }
}

int fgetline(FILE* file_p, char* line_string){
    char ch;
    int i=0;
    if(file_p==NULL){
        return -1;
    }
    reset_string(line_string);
    do{
        ch=fgetc(file_p);
        if(ch!=EOF&&ch!='\n'){
            *(line_string+i)=ch;
            i++;
        }
/*        else if(ch=='\n'){
            return 0;
        }
        else if(ch==EOF&&i==0){
            return 1;
        }*/
    }while(ch!=EOF&&ch!='\n');
    if(ch==EOF&&i==0){
        return 1;
    }
    else{
        return 0;
    }
}

int contain_or_not(const char* line, const char* findkey){
    int length_line=strlen(line);
    int length_findkey=strlen(findkey);
    int i,j;
    char* string_temp=(char *)malloc(sizeof(char)*(length_findkey+1));
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

int global_replace(char* filename, char* orig_string, char* new_string){
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
    char cmdline[CMDLINE_LENGTH]="";
    int getline_flag;
    int i,j,k,line_length;

    sprintf(filename_temp,"%s.tmp",filename);
    file_p_tmp=fopen(filename_temp,"w+");
    do{
        getline_flag=fgetline(file_p,single_line);
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
        if(getline_flag==1){
            fprintf(file_p_tmp,"%s",new_line);
        }
        else{
            fprintf(file_p_tmp,"%s\n",new_line);
        }
    }while(getline_flag==0);
    fclose(file_p);
    fclose(file_p_tmp);
#ifdef _WIN32
    sprintf(cmdline,"move /y %s %s > nul 2>&1",filename_temp,filename);
#else
    sprintf(cmdline,"mv %s %s >> /dev/null 2>&1",filename_temp,filename);
#endif
    system(cmdline);
    return 0;
}

int line_replace(char* orig_line, char* new_line, char* orig_string, char* new_string){
    int length=strlen(orig_line);
    int length_orig=strlen(orig_string);
    int length_new=strlen(new_string);
    char* temp_string=(char *)malloc(sizeof(char)*(length_orig+1));
    int i,j;
    int k=0,k2;
    for(i=0;i<length_orig+1;i++){
        *(temp_string+i)='\0';
    }
    if(length_orig==0){
        for(i=0;i<length;i++){
            *(new_line+i)=*(orig_line+i);
        }
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

int find_and_replace(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5, char* orig_string, char* new_string){
    if(strcmp(orig_string,new_string)==0){
        return -1;
    }
    int replace_count=0;
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -1;
    }
    char filename_temp[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
    char single_line[LINE_LENGTH]="";
    char new_single_line[LINE_LENGTH]="";
    sprintf(filename_temp,"%s.tmp",filename);
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
#ifdef _WIN32
    sprintf(cmdline,"del /f /q %s > nul 2>&1 && move /y %s %s > nul 2>&1",filename,filename_temp,filename);
#else
    sprintf(cmdline,"rm -rf %s >> /dev/null && mv %s %s >> /dev/null 2>&1",filename,filename_temp,filename);
#endif
    system(cmdline);
    return replace_count;
}

int find_multi_keys(char* filename, char* findkey1, char* findkey2, char* findkey3, char* findkey4, char* findkey5){
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

int calc_str_num(char* line, char split_ch){
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

int find_and_get(char* filename, char* findkey_primary1, char* findkey_primary2, char* findkey_primary3, int plus_line_num, char* findkey1, char* findkey2, char* findkey3, char split_ch, int string_seq_num, char* get_string){
    if(strlen(findkey_primary1)==0&&strlen(findkey_primary2)==0&&strlen(findkey_primary3)==0){
        return -1;
    }
    if(strlen(findkey1)==0&&strlen(findkey2)==0&&strlen(findkey3)==0){
        return -1;
    }
    FILE* file_p=fopen(filename, "r");
    if(file_p==NULL){
        return -1;
    }
    char single_line[LINE_LENGTH]="";
    int flag_primary1=0,flag_primary2=0,flag_primary3=0;
    int flag_primary=1;
    int flag_eof_or_not=0;
    int flag1=0,flag2=0,flag3=0;
    int i;
    do{
        flag_eof_or_not=fgetline(file_p,single_line);
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
    }while(flag_primary!=0&&flag_eof_or_not==0);
    if(flag_eof_or_not==1){
        fclose(file_p);
        return 1;
    }
    i=0;
    while(flag_eof_or_not!=1&&i<plus_line_num){
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
            flag_eof_or_not=fgetline(file_p,single_line);
            continue;
        }
        else{
            fclose(file_p);
            return get_seq_string(single_line,split_ch,string_seq_num,get_string);
        }
    }
    strcpy(get_string,"");
    return 1;
}

int file_exist_or_not(char* filename){
    FILE* file_p=fopen(filename,"r");
    if(file_p==NULL){
        return 1;
    }
    else{
        fclose(file_p);
        return 0;
    }
}

int file_empty_or_not(char* filename){
    FILE* file_p=fopen(filename,"r");
    char temp_line[LINE_LENGTH]="";
    int line_num=0;
    if(file_p==NULL){
        return -1;
    }
    else{
        while(fgetline(file_p,temp_line)!=1){
            line_num++;
        }
        fclose(file_p);
        if(strlen(temp_line)>0){
            line_num++;
        }
        return line_num;
    }
}

int folder_exist_or_not(char* foldername){
    char filename[FILENAME_LENGTH]="";
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    sprintf(filename,"%s\\testfile.txt",foldername);
#else
    sprintf(filename,"%s/testfile.txt",foldername);
#endif
    FILE* test_file=fopen(filename,"w+");
    if(test_file==NULL){
        return 1;
    }
    else{
        fclose(test_file);
#ifdef _WIN32
        sprintf(cmdline,"del /q /f %s > nul 2>&1",filename);
#else
        sprintf(cmdline,"rm -rf %s >> /dev/null 2>&1",filename);
#endif
        system(cmdline);
        return 0;
    }
}

int generate_random_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
#ifdef _WIN32
        mingw_gettimeofday(&current_time,NULL);
#else
        gettimeofday(&current_time,NULL);
#endif
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%72;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
}

int generate_random_db_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[62]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
#ifdef _WIN32
        mingw_gettimeofday(&current_time,NULL);
#else
        gettimeofday(&current_time,NULL);
#endif
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%62;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
}

int generate_random_string(char* random_string){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[36]="abcdefghijklmnopqrstuvwxyz0123456789";
    unsigned int seed_num;
#ifdef _WIN32
    mingw_gettimeofday(&current_time,NULL);
#else
    gettimeofday(&current_time,NULL);
#endif
    seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
    srand(seed_num);
    rand_num=rand()%26;
    *(random_string+0)=*(ch_table+rand_num);
    usleep(5000);
    for(i=1;i<RANDSTR_LENGTH_PLUS-1;i++){
#ifdef _WIN32
        mingw_gettimeofday(&current_time,NULL);
#else
        gettimeofday(&current_time,NULL);
#endif
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%36;
        *(random_string+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    *(random_string+RANDSTR_LENGTH_PLUS-1)='\0';
    return 0;  
}