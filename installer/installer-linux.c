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
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define CMDLINE_LENGTH 2048
#define URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_linux_amd64"
#define PASSWORD_STRING_LENGTH 20
#define PASSWORD_LENGTH 19

void print_help(void){
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Welcome to HPC-NOW Service Installer! There are 3 options:                        |\n");
    printf("| Usage: sudo THIS_INSTALLER_FULL_PATH option                                       |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  install          : Install or repair the HPC-NOW Services on your device.        |\n");
    printf("|  update           : Update the hpcopr to the latest version.                      |\n");
    printf("|  uninstall        : Remove the HPC-NOW services and all relevant data.            |\n");
    printf("|  help             : Show this information.                                        |\n");      
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Version: 0.1.61   * This software is licensed under GPLv2, with NO WARRANTY! *    |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("|  HPC NOW, start now ... to infinity!            | H - igh         | N - o         |\n");
    printf("|                                                 | P - erformance  + O - perating  |\n");
    printf("|  https://www.hpc-now.com   |  info@hpc-now.com  | C - omputing    | W - orkload   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
}

int check_internet(void){    
    if(system("ping -c 2 www.baidu.com >> /dev/null 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("| Welcome to HPC-NOW Service Installer!                                             |\n");
        printf("| Version: 0.1.61   * This software is licensed under GPLv2, with NO WARRANTY! *    |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service |\n");
        printf("|          or check your internet connectivity and retry later.                     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
    return 0;
}

int generate_random_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
        gettimeofday(&current_time,NULL);
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec);
        srand(seed_num);
        rand_num=rand()%72;
        *(password+i)=*(ch_table+rand_num);
        usleep(5000);
    }
    return 0;
}

int install_services(void){
    char cmdline[CMDLINE_LENGTH]="";
    char random_string[PASSWORD_STRING_LENGTH]="";
    FILE* file_p=NULL;

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Welcome to HPC-NOW Service Installer!                                             |\n");
    printf("| Version: 0.1.61   * This software is licensed under GPLv2, with NO WARRANTY! *    |\n");

    if(system("whoami | grep -w root >> /dev/null 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please switch to the root user or users with administration privilege    |\n");
        printf("|          and run the installer *WITH* 'sudo' to install the HPC-NOW services.     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;    
    }

    if(system("id hpc-now >> /dev/null 2>&1")==0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] User 'hpc-now' found. It seems the HPC-NOW services have been installed. |\n");
        printf("|          If you'd like to reinstall, please uninstall first. Reinstallation       |\n");
        printf("|          is not permitted in order to protect your cloud clusters. In order to    |\n");
        printf("|          uninstall current HPC-NOW services, please run the command:              |\n");
        printf("|          sudo THIS_INSTALLER_FULL_PATH uninstall (Double confirm is needed)       |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Checking and cleaning up current environment ...                         |\n");
    system("rm -rf /home/hpc-now/ >> /dev/null 2>&1");
    system("chattr -i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now/ >> /dev/null 2>&1");
    printf("[ -INFO- ] Adding the specific user 'hpc-now' to your OS ...                        |\n");
    strcpy(cmdline,"useradd hpc-now -m -s /bin/bash >> /dev/null 2>&1");
    if(system(cmdline)!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;
    }
    
    printf("[ -INFO- ] Creating and configuring the running directory ...                       |\n");
    system("mkdir -p /usr/.hpc-now && chmod 777 /usr/.hpc-now");
    printf("[ -INFO- ] Creating a random file for encryption/decryption ...                     |\n");
    generate_random_passwd(random_string);
    file_p=fopen("/usr/.hpc-now/.now_crypto_seed.lock","w+");
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"%s\n",random_string);
    fclose(file_p);
    system("chown -R root:root /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("chattr +i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    printf("[ -INFO- ] Setting up environment variables for 'hpc-now' ...                       |\n");    
    system("mkdir -p /home/hpc-now/.bin >> /dev/null 2>&1");
    if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ >> /dev/null 2>&1")!=0){
        strcpy(cmdline,"echo \"export PATH=/home/hpc-now/.bin/:$PATH\" >> /home/hpc-now/.bashrc");
        system(cmdline);
    }
    printf("[ -INFO- ] Downloading the main program 'hpcopr' now ...                            |\n");
    sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr && chmod +x /home/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    system("mkdir -p /home/hpc-now/.now-ssh/ >> /dev/null 2>&1");
    system("mkdir -p /home/hpc-now/.now-lic/ >> /dev/null 2>&1");
    system("chown -R hpc-now:hpc-now /home/hpc-now/");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Congratulations! The HPC-NOW services are ready to run!                  |\n");
    printf("|          The user 'hpc-now' has been created *WITHOUT* an initial password.       |\n");
    printf("|          You *MUST* run 'sudo passwd hpc-now' command to set a password.          |\n");
    printf("|          Please ensure the complexity of the new password!                        |\n");
    printf("|          After setting password, please switch to the user 'hpc-now' and run      |\n");
    printf("|          the command 'hpcopr help' to get started. Enjoy you Cloud HPC journey!   |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("[ -INFO- ] Exit now.                                                                |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    return 0;
}


int uninstall_services(void){
    char doubleconfirm[128]="";
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Welcome to HPC-NOW Service Installer!                                             |\n");
    printf("| Version: 0.1.61   * This software is licensed under GPLv2, with NO WARRANTY! *    |\n");

    if(system("whoami | grep -w root >> /dev/null 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please switch to the root user or users with administration privilege    |\n");
        printf("|          and run the installer with 'sudo' to uninstall the HPC-NOW services.     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;    
    }

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
    printf("|*      them to a directory such as /home/ANOTHER_USER                             *|\n");
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
    system("chattr -i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now >> /dev/null 2>&1");
    system("userdel -f -r hpc-now >> /dev/null 2>&1");
    printf("[ -DONE- ] The HPC-NOW cluster services have been deleted from this OS and device.  |\n");
    printf("|          Thanks a lot for using HPC-NOW services!                                 |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    return 0;
}


int update_services(void){
    char doubleconfirm[128]="";
    char cmdline[CMDLINE_LENGTH]="";

    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| Welcome to HPC-NOW Service Installer!                                             |\n");
    printf("| Version: 0.1.61   * This software is licensed under GPLv2, with NO WARRANTY! *    |\n");

    if(system("whoami | grep -w root >> /dev/null 2>&1")!=0){
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Please switch to the root user or users with administration privilege    |\n");
        printf("|          and run the installer *WITH* 'sudo' to update the HPC-NOW services.      |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        printf("[ FATAL: ] Exit now.                                                                |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return -1;    
    }

    if(system("id hpc-now >> /dev/null 2>&1")!=0){
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
    sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr && chmod +x /home/hpc-now/.bin/hpcopr && chown -R hpc-now:hpc-now /home/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    if(system(cmdline)==0){
        printf("[ -DONE- ] The HPC-NOW cluster services have been updated to your device and OS.    |\n");
        printf("|          Thanks a lot for using HPC-NOW services!                                 |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 0;
    }
    else{
        printf("[ FATAL: ] Failed to update the HPC-NOW services. Please check and make sure:       |\n");
        printf("|          1. The HPC-NOW Services have been installed previously.                  |\n");
        printf("|          2. Your device is connected to the internet.                             |\n");
        printf("|          3. Currently there is no 'hpcopr' thread(s) running.                     |\n");
        printf("+-----------------------------------------------------------------------------------+\n");
        return 1;
    }
}

int main(int argc, char* argv[]){
    int run_flag=0;
    if(argc!=2){
        print_help();
        return 1;
    }
    
    if(strcmp(argv[1],"uninstall")!=0&&strcmp(argv[1],"update")!=0&&strcmp(argv[1],"install")!=0){
        print_help();
        return 1;
    }

    if(check_internet()!=0){
        return -3;
    }

    if(strcmp(argv[1],"uninstall")==0){
        run_flag=uninstall_services();
        return run_flag;
    }

    if(strcmp(argv[1],"update")==0){
        run_flag=update_services();
        return run_flag;
    }

    if(strcmp(argv[1],"install")==0){
        run_flag=install_services();
        return run_flag;
    }

    if(strcmp(argv[1],"help")==0){
        print_help();
        return 0;
    }
    return 0;
}