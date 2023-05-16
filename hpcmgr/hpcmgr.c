/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

/* 
* This code is only for GNU/Linux distributions to build the HPC Manager toolset. It 
* is not necessary to consider cross-platform, because the HPC Manager will only run
* on the Master nodes of the clusters.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HPCMGR_VERSION "0.2.0.0013"

int env_ready_or_not(void){
  int mgr_flag=system("cat /etc/profile | grep HPCMGR_SCRIPT_URL= >> /dev/null 2>&1");
  int appstore_flag=system("cat /etc/profile | grep APPS_INSTALL_SCRIPTS_URL= >> /dev/null 2>&1");
  if(mgr_flag!=0&&appstore_flag!=0){
    return -1;
  }
  else if(mgr_flag==0&&appstore_flag!=0){
    return 1;
  }
  else if(mgr_flag!=0&&appstore_flag!=0){
    return 3;
  }
  else{
    return 0;
  }
}

void print_header_hpcmgr(void){
  time_t current_time_long;
  struct tm* time_p=NULL;
  time(&current_time_long);
  time_p=localtime(&current_time_long);
  printf("|   /HPC->  Welcome to HPC-NOW Cluster Manager! Version: %s\n",HPCMGR_VERSION);
  printf("|\\\\/ ->NOW  %d-%d-%d %d:%d:%d\n",time_p->tm_year+1900,time_p->tm_mon+1,time_p->tm_mday,time_p->tm_hour,time_p->tm_min,time_p->tm_sec);
  printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd LICENSE: GPL-2.0\n\n");
}

void print_tail_hpcmgr(void){
  printf("\n");
  printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

int main(int argc,char *argv[]){
  int env_flag=env_ready_or_not();
  if(env_flag==-1||env_flag==3){
    return -1;
  }
  else if(env_flag==1){
    printf("[ -WARN- ] The HPC appstore may not work properly.\n");
  }
  int i,start,end;
  int system_run_flag=0;
  int param1_length=0;
  int param2_length=0;
  int param3_length=0;
  int param4_length=0; 
  int base_length=0;
  char* param1=argv[1];
  char* param2=argv[2];
  char* param3=argv[3];
  char* param4=argv[4];
  char* cmd_dl="wget -q $HPCMGR_SCRIPT_URL -O /tmp/.thread-";
  char* cmd_chmod="chmod +x /tmp/.thread-";
  char* cmd_base="/tmp/.thread-";
  char* cmd_dele="rm -rf /tmp/.thread-";
  char final_cmd_dl[128];
  char final_cmd_chmod[64];
  char cmd_run[64];
  char final_cmd_run[64]="";
  char final_cmd_dele[64];
  char confirm[64];
  int param_number=argc-1;
  
  print_header_hpcmgr();

  char rand_num_string[7]="";
  srand((unsigned)time(NULL));
  int rand_num=rand();
  if(rand_num>1000000){
    rand_num=rand_num%1000000;
  }
  sprintf(rand_num_string,"%d",rand_num);
  sprintf(final_cmd_dl,"%s%s",cmd_dl,rand_num_string);
  sprintf(final_cmd_chmod,"%s%s",cmd_chmod,rand_num_string);
  sprintf(cmd_run,"%s%s",cmd_base,rand_num_string);
  sprintf(final_cmd_dele,"%s%s",cmd_dele,rand_num_string);
  
  if(param_number==1){
    param1_length=strlen(param1);
  }
  else if(param_number==2){
    param1_length=strlen(param1);
    param2_length=strlen(param2);
  }
  else if(param_number==3){
    param1_length=strlen(param1);
    param2_length=strlen(param2);
    param3_length=strlen(param3);
  }
  else if(param_number>3){
    param1_length=strlen(param1);
    param2_length=strlen(param2);
    param3_length=strlen(param3);
    param4_length=strlen(param4);
  }
  
  for(i=0;i<63;i++){
    *(final_cmd_run+i)=' ';
  }
  *(final_cmd_run+63)='\0';  
  base_length=strlen(cmd_run);
  
  for(i=0;i<base_length;i++){
    *(final_cmd_run+i)=*(cmd_run+i);
  }

  if(param1_length!=0&&param1_length<8){
    start=base_length+1;
    end=base_length+1+param1_length;
    for(i=start;i<end;i++){
      *(final_cmd_run+i)=*(param1+i-start);
    }
  }

  if(param2_length!=0&&param2_length<8){
    start=base_length+1+param1_length+1;
    end=base_length+1+param1_length+1+param2_length;
    for(i=start;i<end;i++){
      *(final_cmd_run+i)=*(param2+i-start);
    }
  }

  if(param3_length!=0&&param3_length<16){
    start=base_length+param1_length+1+param2_length+2;
    end=base_length+param1_length+1+param2_length+2+param3_length;
    for(i=start;i<end;i++){
      *(final_cmd_run+i)=*(param3+i-start);
    }
  }

  if(param4_length!=0&&param4_length<16){
    start=base_length+param1_length+1+param2_length+1+param3_length+2;
    end=base_length+param1_length+1+param2_length+1+param3_length+2+param4_length;
    for(i=start;i<end;i++){
      *(final_cmd_run+i)=*(param4+i-start);
    }
  }

  if(param1_length!=0&&param2_length!=0&&param3_length!=0&&param4_length!=0){
    if(strcmp(param1,"users")==0&&strcmp(param2,"delete")==0&&strcmp(param4,"os")==0){
      printf("[ -WARN- ] You are deleting User %s from the cluster and the OS!\n", param3);
      printf("[ INPUT: ] Please input 'y-e-s' to confirm: ");
      fflush(stdin);
      scanf("%s",confirm);
      getchar();
      if(strcmp(confirm,"y-e-s")==0){
        printf("Operation confirmed.\n");
      }
      else{
        printf("[ -INFO- ]You denied the operation. Nothing changed.\n");
        print_tail_hpcmgr();
        return 1;
      }
    }
  }
  system_run_flag=system(final_cmd_dl);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 1.\n");
    print_tail_hpcmgr();
    return 1;
  }
  system_run_flag=system(final_cmd_chmod);  
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 2.\n");
    system(final_cmd_dele);
    print_tail_hpcmgr();
    return 1;
  }    
  system_run_flag=system(final_cmd_run);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 3.\n");
    system(final_cmd_dele);
    print_tail_hpcmgr();
    return 1;
  } 
  system(final_cmd_dele);
  print_tail_hpcmgr();
  return 0;
}