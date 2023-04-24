/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#include <string.h>
#include <math.h>
#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#endif
#include "time_process.h"

void datetime_to_num(char* date_string, char* time_string, struct tm* datetime_num){
    int i;
    int year=0,month=0,day=0;
    int hour=0,min=0,sec=0;
    int position1=0;
    int position2=0;
    for(i=0;i<strlen(date_string);i++){
        if(*(date_string+i)=='-'){
            position1=i;
            break;
        }
    }
    for(i=position1+1;i<strlen(date_string);i++){
        if(*(date_string+i)=='-'){
            position2=i;
            break;
        }
    }
    for(i=0;i<position1;i++){
        year+=(*(date_string+i)-'0')*pow(10,position1-1-i);
    }
    for(i=position1+1;i<position2;i++){
        month+=(*(date_string+i)-'0')*pow(10,position2-i-1);
    }
    for(i=position2+1;i<strlen(date_string);i++){
        day+=(*(date_string+i)-'0')*pow(10,strlen(date_string)-i-1);
    }

    for(i=0;i<strlen(date_string);i++){
        if(*(time_string+i)==':'){
            position1=i;
            break;
        }
    }
    for(i=position1+1;i<strlen(date_string);i++){
        if(*(time_string+i)==':'){
            position2=i;
            break;
        }
    }
    for(i=0;i<position1;i++){
        hour+=(*(time_string+i)-'0')*pow(10,position1-1-i);
    }
    for(i=position1+1;i<position2;i++){
        min+=(*(time_string+i)-'0')*pow(10,position2-i-1);
    }
    for(i=position2+1;i<strlen(time_string);i++){
        sec+=(*(time_string+i)-'0')*pow(10,strlen(time_string)-i-1);
    }
    datetime_num->tm_year=year-1900;
    datetime_num->tm_mon=month-1;
    datetime_num->tm_mday=day;
    datetime_num->tm_hour=hour;
    datetime_num->tm_min=min;
    datetime_num->tm_sec=sec;
    datetime_num->tm_isdst=-1; // For Linux, this is essential. For Windows (mingw), it is not necessary
}

double calc_running_hours(char* prev_date, char* prev_time, char* current_date, char* current_time){
    time_t prev;
    time_t current;
    struct tm time_prev;
    struct tm time_current;
    datetime_to_num(prev_date,prev_time,&time_prev);
    datetime_to_num(current_date,current_time,&time_current);
    prev=mktime(&time_prev);
    current=mktime(&time_current);
    return difftime(current,prev)/3600;
}