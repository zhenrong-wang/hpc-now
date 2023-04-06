/*
This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
It is distributed under the license: GNU Public License - v2.0
Bug report: info@hpc-now.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


#define CMDLINE_LENGTH 2048
#define LINE_LENGTH 1024 //It has to be very long, because tfstate file may contain very long line
#define URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_windows_amd64.exe"
#define URL_LICENSE "https://gitee.com/zhenrong-wang/hpc-now/raw/dev/LICENSE"
#define PASSWORD_STRING_LENGTH 20
#define PASSWORD_LENGTH 19

void print_header(void){
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Welcome to the HPC-NOW Service Installer!     Version: 0.1.79                     |\n");
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd                         |\n");
    printf("| This is free software; see the source for copying conditions.  There is NO        |\n");
    printf("| warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.       |\n");
}

void print_tail(void){
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd                         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

void print_help(void){
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Usage: THIS_INSTALLER_FULL_PATH general_option advanced_option                    |\n");
    printf("| general_option:                                                                   |\n");
    printf("|        install          : Install or repair the HPC-NOW Services on your device.  |\n");
    printf("|        update           : Update the hpcopr to the latest version.                |\n");
    printf("|        uninstall        : Remove the HPC-NOW services and all relevant data.      |\n");
    printf("|        help             : Show this information.                                  |\n");
    printf("| advanced_option (for developers, optional):                                       |\n");
    printf("|        hpcopr_loc=LOC   : Provide your own location of hpcopr, both URL and local |\n");
    printf("|                           *absolute* path are accepted. You should guarantee that |\n");
    printf("|                           the location points to a valid hpcopr executable.       |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

int check_internet(void){    
    if(system("ping -n 2 www.baidu.com > nul 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service |\n");
        printf("|          or check your internet connectivity and retry later. Exit now.           |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    return 0;
}

int license_confirmation(void){
    char cmdline[CMDLINE_LENGTH]="";
    char confirmation[64]="";
    sprintf(cmdline,"curl -s %s | more",URL_LICENSE);
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Please read the following important information before continuing.       |\n");
    printf("|          You can press 'Enter' to continue reading, or press 'q' to quit reading. |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Currently the installer failed to download or print out the license.     |\n");
        printf("|          Please double check your internet connectivity and retry. If this issue  |\n");
        printf("|          still occurs, please report to us via info@hpc-now.com . Exit now.       |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] If you accept the terms and conditions above, please input 'accept',     |\n");
    printf("|          If you do not accept, this installation will exit immediately.           |\n");
    printf("[ INPUT: ] Please input ( case-sensitive ): ");
    fflush(stdin);
    scanf("%s",confirmation);
    if(strcmp(confirmation,"accept")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] This installation process is terminated because you didn't accept the    |\n");
        printf("|          terms and conditions in the license. Exit now.                           |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    return 0;
}

void reset_string(char* orig_string){
    int length=strlen(orig_string);
    int i;
    for(i=0;i<length;i++){
        *(orig_string+i)='\0';
    }
}

int generate_random_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
        mingw_gettimeofday(&current_time,NULL);
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%72;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
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
        else if(ch=='\n'){
            return 0;
        }
        else if(ch==EOF){
            return 1;
        }
    }while(ch!=EOF&&ch!='\n');
    if(ch==EOF){
        return 1;
    }
    else{
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
        return line_num;
    }
}

int check_current_user(void){
    system("whoami /groups | find \"S-1-16-12288\" > c:\\programdata\\check.txt.tmp 2>&1");
    if(file_empty_or_not("c:\\programdata\\check.txt.tmp")==0){
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please switch to Administrator or users with administration privilege:   |\n");
        printf("|          1. Run a CMD window with the Administrator role                          |\n");
        printf("|          2. Type the full path of this installer with an option, for example      |\n");
        printf("|             C:\\Users\\ABC\\installer_windows_amd64.exe install                      |\n");
        printf("|          to run this installer properly. Exit now.                                |\n");
        print_help();
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        return -1;    
    }
    system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
    return 0;
}

int install_services(int loc_flag, char* location){
    char cmdline[CMDLINE_LENGTH]="";
    char random_string[PASSWORD_STRING_LENGTH]="";
    FILE* file_p=NULL;
    if(system("net user hpc-now > nul 2>&1")==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] User 'hpc-now' found. It seems the HPC-NOW services have been installed. |\n");
        printf("|          If you'd like to reinstall, please uninstall first. Reinstallation       |\n");
        printf("|          is not permitted in order to protect your cloud clusters. In order to    |\n");
        printf("|          uninstall current HPC-NOW services, please run the command:              |\n");
        printf("|          1. Run a CMD window with Administrator role                              |\n");
        printf("|          2. Type the full path of this installer with an option, for example      |\n");
        printf("|             C:\\Users\\ABC\\installer_windows_amd64.exe uninstall                    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Checking and cleaning up current environment ...                         |\n");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
    system("attrib -h -s -r c:\\programdata\\hpc-now\\now_crypto_seed.lock > nul 2>&1");
    system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
    printf("[ -INFO- ] Adding the specific user 'hpc-now' to your OS ...                        |\n");   
    strcpy(cmdline,"net user hpc-now nowadmin2023~ /add /logonpasswordchg:yes > nul 2>&1");
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    printf("[ -INFO- ] Creating and configuring the running directory ...                       |\n");
    system("mkdir c:\\hpc-now > nul 2>&1");
    system("mkdir c:\\hpc-now\\LICENSES > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\bin\\ > nul 2>&1");
    system("icacls c:\\hpc-now /grant hpc-now:(OI)(CI)F /t > nul 2>&1");
    system("icacls c:\\hpc-now /deny hpc-now:(DE) /t > nul 2>&1");
    printf("[ -INFO- ] Creating a random file for encryption/decryption ...                     |\n");
    generate_random_passwd(random_string);
    file_p=fopen("c:\\programdata\\hpc-now\\now_crypto_seed.lock","w+");
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"%s\n",random_string);
    fclose(file_p);
    system("attrib +h +s +r c:\\programdata\\hpc-now\\now_crypto_seed.lock");
    system("attrib +h +s +r c:\\programdata\\hpc-now > nul 2>&1");

    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' now ...                            |\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' now ...                            |\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' now ...                                |\n");
        sprintf(cmdline,"copy %s C:\\hpc-now\\hpcopr.exe > nul 2>&1 ",location);
    }
    
    if(system(cmdline)==0){
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\LICENSES\\GPL-2",URL_LICENSE);
        system(cmdline);
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Congratulations! The HPC-NOW services are ready to run!                  |\n");
        printf("|          The user 'hpc-now' has been created with initial password: nowadmin2023~ |\n");
        printf("|          Please switch to the user 'hpc-now' by ctrl+alt+delete and then:         |\n");
        printf("|          1. Run CMD by typing cmd in the Windows Search box                       |\n");
        printf("|          2. cd c:\\hpc-now ( Change directory to the running directory )           |\n");
        printf("|          3. hpcopr help    ( Some core components will be downloaded )            |\n");
        printf("|          * You will be required to change the password of 'hpc-now'.              |\n");
        printf("|          Enjoy you Cloud HPC journey!                                             |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 0;
    }
    else{
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Failed to get the hpcopr executable. This installation process is        |\n");
        printf("|          terminated. If you specified the location of hpcopr executable, please   |\n");
        printf("|          make sure the location is correct. Exit now.                             |\n");
        printf("|          Please uninstall first and then install again.                           |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
}

int uninstall_services(void){
    char doubleconfirm[128]="";
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   YOU ARE UNINSTALLING THE HPC-NOW SERVICES, PLEASE CONFIRM THE ISSUES BELOW:   *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*   1. You have *DESTROYED* all the clusters managed by this device.              *|\n");
    printf("|*      This is * !!! EXTREMELY IMPORTANT !!! *                                    *|\n");
    printf("|*   2. You have *CHECKED* your cloud service account and all the resources        *|\n");
    printf("|*      created by the HPC-NOW services on this device have been destructed.       *|\n");
    printf("|*   3. You have *EXPORTED* the usage log and systemlog to a permenant directory,  *|\n");
    printf("|*      You can run 'hpcopr syslog' and 'hpcopr usage' to get the logs and save    *|\n");
    printf("|*      them to a directory such as C:\\                                            *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          *|\n");
    printf("|*                                                                                 *|\n");
    printf("|*                                C A U T I O N !                                  *|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.   |\n");
        printf("|          Nothing changed.                                                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] UNINSTALLING THE SERVICES AND REMOVING THE DATA NOW ...                  |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
    system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    system("rd /s /q c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
    system("net user hpc-now /delete > nul 2>&1");
    printf("[ -DONE- ] The HPC-NOW cluster services have been deleted from this OS and device.  |\n");
    printf("|          There are still remaining files for the specific user 'hpc-now'.         |\n");
    printf("|          Please mannually delete the folder C:\\Users\\hpc-now after reboot.        |\n");
    printf("|          Thanks a lot for using HPC-NOW services!                                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    return 0;
}

int update_services(int loc_flag, char* location){
    char doubleconfirm[128]="";
    char cmdline[CMDLINE_LENGTH]="";

    if(system("net user hpc-now > nul 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] User 'hpc-now' not found. It seems the HPC-NOW Services have not been    |\n");
        printf("|          installed. Please install it first in order to update.                   |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|* YOU ARE UPDATING THE HPC-NOW SERVICES TO THE LATEST VERSION.                    *|\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:         |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ INPUT: ]  ");
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.   |\n");
        printf("|          Nothing changed.                                                         |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] UPDATING THE SERVICES NOW ...                                            |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");

    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' now ...                            |\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' now ...                            |\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' now ...                                |\n");
        sprintf(cmdline,"copy %s C:\\hpc-now\\hpcopr.exe > nul 2>&1 ",location);
    }

    if(system(cmdline)==0){
        printf("[ -DONE- ] The HPC-NOW cluster services have been updated to your device and OS.    |\n");
        printf("|          Thanks a lot for using HPC-NOW services!                                 |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        return 0;
    }
    else{
        printf("[ FATAL: ] Failed to update the HPC-NOW services. Please check and make sure:       |\n");
        printf("|          1. The HPC-NOW Services have been installed previously.                  |\n");
        printf("|          2. Please make sure the specified location (if specified) is correct.    |\n");
        printf("|          3. Your device is connected to the internet.                             |\n");
        printf("|          4. Currently there is no 'hpcopr' thread(s) running.                     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        return 1;
    }
}

int main(int argc, char* argv[]){
    int run_flag=0;
    int i;
    int length;
    char advanced_option_head[12]="";
    char advanced_option_tail[256]="";
    int loc_flag=-1;

    print_header();
    if(check_current_user()!=0){
        print_tail();
        return -1;
    }

    if(check_internet()!=0){
        print_tail();
        return -3;
    }
    
    if(argc!=2&&argc!=3){
        print_help();
        print_tail();
        return 1;
    }
    
    if(strcmp(argv[1],"help")==0){
        print_help();
        print_tail();
        return 0;
    }

    if(strcmp(argv[1],"uninstall")!=0&&strcmp(argv[1],"update")!=0&&strcmp(argv[1],"install")!=0){
        print_help();
        print_tail();
        return 1;
    }

    if(argc==3){
        length=strlen(argv[2]);
        if(length<13){
            print_help();
            print_tail();
            return 1;
        }

        for(i=0;i<11;i++){
            advanced_option_head[i]=*(argv[2]+i);
        }
        advanced_option_head[11]='\0';
        for(i=0;i<length-11;i++){
            advanced_option_tail[i]=*(argv[2]+i+11);
        }
        advanced_option_tail[length-11]='\0';

        if(strcmp(advanced_option_head,"hpcopr_loc=")!=0){
            print_help();
            print_tail();
            return 1;
        }

        if(advanced_option_tail[1]==':'&&advanced_option_tail[2]=='\\'){
            loc_flag=0;
        }

        else{
            if(advanced_option_tail[0]=='h'&&advanced_option_tail[1]=='t'&&advanced_option_tail[2]=='t'&&advanced_option_tail[3]=='p'){
                loc_flag=1;
            }
        }

        if(loc_flag==-1){
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] The specified hpcopr location is invalid. Please refer to the formats:   |\n");
            printf("|            URL   : must start with http or https                                  |\n");
            printf("|            Local : must be an absolute path. Example: Z:\hpcopr.exe               |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            printf("[ FATAL: ] Exit now.                                                                |\n");
            printf("+-----------------------------------------------------------------------------------+\n");
            print_tail();
            return 1;
        }
    }

    run_flag=license_confirmation();
    if(run_flag!=0){
        print_tail();
        return run_flag;
    }

    if(strcmp(argv[1],"uninstall")==0){
        run_flag=uninstall_services();
        print_tail();
        return run_flag;
    }

    if(strcmp(argv[1],"update")==0){
        run_flag=update_services(loc_flag,advanced_option_tail);
        print_tail();
        return run_flag;
    }

    if(strcmp(argv[1],"install")==0){
        run_flag=install_services(loc_flag,advanced_option_tail);
        print_tail();
        return run_flag;
    }
    return 0;
}