#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc,char *argv[]) 
{
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
  char* cmd_dl="wget -q https://now-codes-1308065454.cos.ap-nanjing.myqcloud.com/scripts/hpcmgr.sh -O /tmp/.thread-";
  char* cmd_chmod="chmod +x /tmp/.thread-";
  char* cmd_base="/tmp/.thread-";
  char* cmd_dele="rm -rf /tmp/.thread-";
  char final_cmd_dl[128];
  char final_cmd_chmod[64];
  char cmd_run[64];
  char final_cmd_run[64]="";
  char final_cmd_dele[64];
  char confirm[3];
  int param_number=argc-1;
  
  printf("\nHign Performance Computing - start NOW!\n\nHPC-NOW Cluster Manager\n\nShanghai HPC-NOW Technologies Co., Ltd\nAll rights reserved (2022)\ninfo@hpc-now.com\n\n");
  
  char rand_num_string[7]="";
  srand((unsigned)time(NULL));
  int rand_num = rand();
  if(rand_num > 1000000){
    rand_num=rand_num%1000000;
  }
  sprintf(rand_num_string,"%d",rand_num);
  
  sprintf(final_cmd_dl,"%s%s",cmd_dl,rand_num_string);
  sprintf(final_cmd_chmod,"%s%s",cmd_chmod,rand_num_string);
  sprintf(cmd_run,"%s%s",cmd_base,rand_num_string);
  sprintf(final_cmd_dele,"%s%s",cmd_dele,rand_num_string);

//  printf("%s\n%s\n%s\n%s\n",final_cmd_dl,final_cmd_chmod,cmd_run,final_cmd_dele);
  
  for(i=0;i<3;i++){
    *(confirm+i)=' ';
  }
  
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
//  printf("%s,,,,,,,,,,,,,,,\n",final_cmd_run);
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
      printf("WARNING: You are deleting User%s from the cluster and the Operating System! Please input 'yes' to confirm: ", param3);
      scanf("%s",confirm);
      if(strcmp(confirm,"yes")==0){
        printf("Operation confirmed.\n");
      }
      else{
        printf("You denied the operation. Nothing changed.\n");
        return 1;
      }
    }
  }
//  printf("%s\n%s\n%s\n%s\n",final_cmd_dl,final_cmd_chmod,final_cmd_run,final_cmd_dele);
  system_run_flag=system(final_cmd_dl);
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