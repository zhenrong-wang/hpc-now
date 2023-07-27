/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
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

#define HPCMGR_VERSION "0.2.0.0015"

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
  printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd    LICENSE: MIT\n\n");
}

void print_tail_hpcmgr(void){
  printf("\n");
  printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

int main(int argc,char *argv[]){
  int env_flag=env_ready_or_not();
  if(env_flag==-1||env_flag==3){
    printf("[ FATAL: ] The environment variables are absent.\n");
    return 3;
  }
  else if(env_flag==1){
    printf("[ -WARN- ] The HPC appstore may not work properly.\n");
  }
  int i,j;
  int position=0;
  int system_run_flag=0;
  char* cmd_dl="wget -q $HPCMGR_SCRIPT_URL -O /tmp/.thread-";
  char* cmd_chmod="chmod +x /tmp/.thread-";
  char* cmd_base="/tmp/.thread-";
  char* cmd_dele="rm -rf /tmp/.thread-";
  char final_cmd_dl[256]="";
  char final_cmd_chmod[128]="";
  char cmd_run[128]="";
  char final_cmd_run[512]="";
  char final_cmd_dele[64];
  char confirm[64];
  int real_argc;
  
  //print_header_hpcmgr(); The hpcmgr has been internalized. No need to print header or tail.
  srand((unsigned)time(NULL));
  int rand_num=rand();
  if(rand_num>1000000){
    rand_num=rand_num%1000000;
  }

  sprintf(final_cmd_dl,"%s%d",cmd_dl,rand_num);
  sprintf(final_cmd_chmod,"%s%d",cmd_chmod,rand_num);
  sprintf(cmd_run,"%s%d",cmd_base,rand_num);
  sprintf(final_cmd_dele,"%s%d",cmd_dele,rand_num);
  
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
    fflush(stdin);
    scanf("%s",confirm);
    getchar();
    if(strcmp(confirm,"y-e-s")==0){
      printf("Operation confirmed.\n");
    }
    else{
      printf("[ -INFO- ]You denied the operation. Nothing changed.\n");
//      print_tail_hpcmgr();
      return 1;
    }
  }
  system_run_flag=system(final_cmd_dl);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 1.\n");
//    print_tail_hpcmgr();
    return 1;
  }
  system_run_flag=system(final_cmd_chmod);  
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 2.\n");
    system(final_cmd_dele);
//    print_tail_hpcmgr();
    return 1;
  }
  system_run_flag=system(final_cmd_run);
  if(system_run_flag!=0){
    printf("[ FATAL: ] ERROR CODE 3.\n");
    system(final_cmd_dele);
//    print_tail_hpcmgr();
    return 1;
  } 
  system(final_cmd_dele);
//  print_tail_hpcmgr();
  return 0;
}