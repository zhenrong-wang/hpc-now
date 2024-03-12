/*
 * Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
 * This code is distributed under the license: MIT License
 * Originally written by Zhenrong WANG
 * mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com
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

#define HPCMGR_VERSION "0.2.0.0017"

int appstore_env_check(void){
  int apps_inst_scripts_flag=system("cat /usr/hpc-now/appstore_env.sh | grep APPS_INST_SCRIPTS_URL= >> /dev/null 2>&1");
  int apps_inst_pkgs_flag=system("cat /usr/hpc-now/appstore_env.sh | grep APPS_INST_PKGS_URL= >> /dev/null 2>&1");
  if(apps_inst_scripts_flag!=0||apps_inst_pkgs_flag!=0){
    return 1;
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
  printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd    LICENSE: MIT\n\n");
}

void print_tail_hpcmgr(void){
  printf("\n");
  printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

int main(int argc,char *argv[]){
  int env_flag=appstore_env_check();
  if(env_flag==1){
    printf("[ -WARN- ] The HPC appstore may not work properly.\n");
  }
  int i,j,ch;
  int position=0;
  int system_run_flag=0;
  char* cmd_cp="/bin/cp -r /usr/hpc-now/.hpcmgr_main.sh /tmp/.thread-";
  char* cmd_chmod="chmod +x /tmp/.thread-";
  char* cmd_base="/tmp/.thread-";
  char* cmd_dele="rm -rf /tmp/.thread-";
  char final_cmd_cp[256]="";
  char final_cmd_chmod[128]="";
  char cmd_run[128]="";
  char final_cmd_run[1024]="";
  char final_cmd_dele[64];
  char confirm[8]="";
  int real_argc;
  
  srand((unsigned)time(NULL));
  int rand_num=rand();
  if(rand_num>1000000){
    rand_num=rand_num%1000000;
  }

  snprintf(final_cmd_cp,255,"%s%d >> /dev/null 2>&1",cmd_cp,rand_num);
  snprintf(final_cmd_chmod,127,"%s%d",cmd_chmod,rand_num);
  snprintf(cmd_run,127,"%s%d",cmd_base,rand_num);
  snprintf(final_cmd_dele,63,"%s%d",cmd_dele,rand_num);
  
  for(i=0;i<strlen(cmd_run);i++){
    *(final_cmd_run+i)=*(cmd_run+i);
  }
  *(final_cmd_run+i)=' ';
  position=i+1;

  if(argc>8){
    real_argc=8;
  }
  else{
    real_argc=argc;
  }
  for(i=1;i<real_argc;i++){
    for(j=0;j<strlen(argv[i]);j++){
      *(final_cmd_run+position+j)=*(argv[i]+j);
    }
    *(final_cmd_run+position+j)=' ';
    position+=(j+1);
  }
  *(final_cmd_run+position+1)='\0';
  if(argc>4&&strcmp(argv[1],"users")==0&&strcmp(argv[2],"delete")==0&&strcmp(argv[4],"os")==0){
    printf("[ -WARN- ] You are deleting User %s from the cluster and the OS!\n", argv[3]);
    printf("[ INPUT: ] Please input 'y-e-s' to confirm: ");
    scanf("%7s",confirm);
    while((ch=getchar())!='\n'&&ch!=EOF){}
    if(ch==EOF){
        if(ferror(stdin)){
            clearerr(stdin);
        }
    }
    if(strcmp(confirm,"y-e-s")==0){
      printf("Operation confirmed.\n");
    }
    else{
      printf("[ -INFO- ]You denied the operation. Nothing changed.\n");
      return 1;
    }
  }
  system_run_flag=system(final_cmd_cp);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 1.\n");
    return 1;
  }
  system_run_flag=system(final_cmd_chmod);  
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 2.\n");
    system(final_cmd_dele);
    return 1;
  }
  system_run_flag=system(final_cmd_run);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 3.\n");
    system(final_cmd_dele);
    return 1;
  } 
  system(final_cmd_dele);
  return 0;
}