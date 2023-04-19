/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define VERSION_CODE "0.2.1.0001"

#ifdef _WIN32
#include <malloc.h>
#define LINE_LENGTH 1024
#define URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_windows_amd64.exe"
#elif __linux__
#include <malloc.h>
#include <sys/time.h>
#define URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_linux_amd64"
#elif __APPLE__
#include <sys/time.h>
#define URL_HPCOPR_LATEST "https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/now-installers/hpcopr_darwin_amd64"
#endif

#define CMDLINE_LENGTH 2048
#define URL_LICENSE "https://gitee.com/zhenrong-wang/hpc-now/raw/master/LICENSE"
#define PASSWORD_STRING_LENGTH 20
#define PASSWORD_LENGTH 19

void print_header(void){
    printf("| Welcome to the HPC-NOW Service Installer! Version: %s\n",VERSION_CODE);
    printf("| Copyright (c) 2023 Shanghai HPC-NOW Technologies Co., Ltd\n");
    printf("| This is free software; see the source for copying conditions.  There is NO\n");
    printf("| warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
}

void print_tail(void){
    printf("\n");
    printf("<> visit: https://www.hpc-now.com <> mailto: info@hpc-now.com\n");
}

// Print out help info for this installer
void print_help(void){
    printf("| Usage: sudo THIS_INSTALLER_FULL_PATH general_option advanced_option\n");
    printf("| general_option:\n");
    printf("|        install          : Install or repair the HPC-NOW Services on your device.\n");
    printf("|        update           : Update the hpcopr to the latest or your own version.\n");
    printf("|        uninstall        : Remove the HPC-NOW services and all relevant data.\n");
    printf("|        help             : Show this information.\n");
    printf("| advanced_option (for developers, optional):\n");
    printf("|        hpcopr_loc=LOC   : Provide your own location of hpcopr, both URL and local\n");
    printf("|                           filesystem path are accepted. You should guarantee that\n");
    printf("|                           the location points to a valid hpcopr executable.\n");
}

// check the internet connectivity by pinging Baidu's url. If connected, return 0; otherwise, return 1
int check_internet(void){
#ifdef _WIN32
    if(system("ping -n 2 www.baidu.com > nul 2>&1")!=0){
#else
    if(system("ping -c 2 www.baidu.com >> /dev/null 2>&1")!=0){
#endif
        printf("[ FATAL: ] Internet connectivity check failed. Please either check your DNS service\n");
        printf("|          or check your internet connectivity and retry later. Exit now.\n");
        return 1;
    }
    return 0;
}

// Generate a randome string, length = 19.
int generate_random_passwd(char* password){
    int i,rand_num;
    struct timeval current_time;
    char ch_table[72]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~@&(){}[]=";
    unsigned int seed_num;
    for(i=0;i<PASSWORD_LENGTH;i++){
#ifdef _WIN32
        mingw_gettimeofday(&current_time,NULL);
#else
        gettimeofday(&current_time,NULL); //Get the precise time
#endif
        seed_num=(unsigned int)(current_time.tv_sec+current_time.tv_usec); //Calculate the random seed
        srand(seed_num);
        rand_num=rand()%72; //Get the random character from the string
        *(password+i)=*(ch_table+rand_num);
        usleep(5000); // Must sleep in order to make the timeval different enough
    }
    return 0;
}

#ifdef _WIN32
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
#endif

int check_current_user(void){
#ifdef _WIN32
    system("whoami /groups | find \"S-1-16-12288\" > c:\\programdata\\check.txt.tmp 2>&1");
    if(file_empty_or_not("c:\\programdata\\check.txt.tmp")==0){
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        printf("[ FATAL: ] Please switch to Administrator or users with administration privilege:\n");
        printf("|          1. Run a CMD window with the Administrator role\n");
        printf("|          2. Type the full path of this installer with an option, for example\n");
        printf("|             C:\\Users\\ABC\\installer_windows_amd64.exe install\n");
        printf("|          to run this installer properly. Exit now.\n");
        print_help();
        system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
        return -1;    
    }
    system("del /f /q /s c:\\programdata\\check.txt.tmp > nul 2>&1");
#else
    if(system("whoami | grep -w root >> /dev/null 2>&1")!=0){
        printf("[ FATAL: ] Please either switch to users with admin privilege and run the installer\n");
        printf("|          with 'sudo', or switch to the root user. Exit now.\n");
        print_help();
        return -1;    
    }
#endif
    return 0;
}

int license_confirmation(void){
    char cmdline[CMDLINE_LENGTH]="";
    char confirmation[64]="";
    sprintf(cmdline,"curl -s %s | more",URL_LICENSE);
    printf("[ -INFO- ] Please read the following important information before continuing.\n");
    printf("|          You can press 'Enter' to continue reading, or press 'q' to quit reading.\n");
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Currently the installer failed to download or print out the license.\n");
        printf("|          Please double check your internet connectivity and retry. If this issue\n");
        printf("|          still occurs, please report to us via info@hpc-now.com . Exit now.\n");
        return 1;
    }
    printf("[ -INFO- ] If you accept the terms and conditions above, please input 'accept',\n");
    printf("|          If you do not accept, this installation will exit immediately.\n");
    printf("[ INPUT: ] Please input ( case-sensitive ): ");
    fflush(stdin);
    scanf("%s",confirmation);
    if(strcmp(confirmation,"accept")!=0){
        printf("[ -INFO- ] This installation process is terminated because you didn't accept the\n");
        printf("|          terms and conditions in the license. Exit now.\n");
        return -1;
    }
    return 0;
}

// Install HPC-NOW Services
// If everything goes well, return 0; otherwise return non-zero value
// 1. Check and add the dedicated user 'hpc-now'
// 2. Create necessary directories, including /Applications/.hpc-now 
// 3. Create the crypto key file for further encryption and decryption
// 4. Manage the folder permissions

int install_services(int loc_flag, char* location){
    char cmdline[CMDLINE_LENGTH]="";
    char random_string[PASSWORD_STRING_LENGTH]="";
    FILE* file_p=NULL;
#ifdef _WIN32
    if(system("net user hpc-now > nul 2>&1")==0){
        printf("[ FATAL: ] User 'hpc-now' found. It seems the HPC-NOW services have been installed.\n");
        printf("|          If you'd like to reinstall, please uninstall first. Reinstallation\n");
        printf("|          is not permitted in order to protect your cloud clusters. In order to\n");
        printf("|          uninstall current HPC-NOW services, please run the command:\n");
        printf("|          1. Run a CMD window with Administrator role\n");
        printf("|          2. Type the full path of this installer with an option, for example\n");
        printf("|             C:\\Users\\ABC\\installer_windows_amd64.exe uninstall\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }
    printf("[ -INFO- ] Checking and cleaning up current environment ...\n");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
    system("attrib -h -s -r c:\\programdata\\hpc-now\\now_crypto_seed.lock > nul 2>&1");
    system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
    printf("[ -INFO- ] Adding the specific user 'hpc-now' to your OS ...\n");   
    strcpy(cmdline,"net user hpc-now nowadmin2023~ /add /logonpasswordchg:yes > nul 2>&1");
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
    printf("[ -INFO- ] Creating and configuring the running directory ...\n");
    system("mkdir c:\\hpc-now > nul 2>&1");
    system("mkdir c:\\hpc-now\\LICENSES > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\.destroyed\\ > nul 2>&1");
    system("mkdir c:\\programdata\\hpc-now\\bin\\ > nul 2>&1");
    system("icacls c:\\hpc-now /grant hpc-now:(OI)(CI)F /t > nul 2>&1");
    system("icacls c:\\hpc-now /deny hpc-now:(DE) /t > nul 2>&1");
    printf("[ -INFO- ] Creating a random file for encryption/decryption ...\n");
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
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL ...\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"copy %s C:\\hpc-now\\hpcopr.exe > nul 2>&1 ",location);
    }
    
    if(system(cmdline)==0){
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\LICENSES\\GPL-2",URL_LICENSE);
        system(cmdline);
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        printf("[ -INFO- ] Congratulations! The HPC-NOW services are ready to run!\n");
        printf("|          The user 'hpc-now' has been created with initial password: nowadmin2023~\n");
        printf("|          Please switch to the user 'hpc-now' by ctrl+alt+delete and then:\n");
        printf("|          1. Run CMD by typing cmd in the Windows Search box\n");
        printf("|          2. cd c:\\hpc-now ( Change directory to the running directory )\n");
        printf("|          3. hpcopr help    ( Some core components will be downloaded )\n");
        printf("|          * You will be required to change the password of 'hpc-now'.\n");
        printf("|          Enjoy you Cloud HPC journey!\n");
        printf("[ -INFO- ] Exit now.\n");
        return 0;
    }
    else{
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        printf("[ FATAL: ] Failed to get the hpcopr executable. This installation process is\n");
        printf("|          terminated. If you specified the location of hpcopr executable, please\n");
        printf("|          make sure the location is correct. Rolling back and exit now.\n");
        system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
        system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
        system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
        system("rd /s /q c:\\hpc-now > nul 2>&1");
        system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
        system("net user hpc-now /delete > nul 2>&1");
        return -1;
    }
#elif __APPLE__
    int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0,flag6=0;

    if(system("id hpc-now >> /dev/null 2>&1")==0){
        printf("[ FATAL: ] User 'hpc-now' found. It seems the HPC-NOW services have been installed.\n");
        printf("|          If you'd like to reinstall, please uninstall first. Reinstallation\n");
        printf("|          is not permitted in order to protect your cloud clusters. In order to\n");
        printf("|          uninstall current HPC-NOW services, please run the command:\n");
        printf("|          sudo THIS_INSTALLER_FULL_PATH uninstall (Double confirm is needed)\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }

    printf("[ -INFO- ] Checking and cleaning up current environment ...\n");
    system("rm -rf /Users/hpc-now/ >> /dev/null 2>&1");
    system("chflags noschg /Applications/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /Applications/.hpc-now/ >> /dev/null 2>&1");
    printf("[ -INFO- ] Adding the specific user 'hpc-now' to your OS ...\n");
    flag1=system("dscl . -create /Users/hpc-now >> /dev/null 2>&1");
    flag2=system("dscl . -create /Users/hpc-now UserShell /bin/bash >> /dev/null 2>&1");
    flag3=system("dscl . -create /Users/hpc-now RealName hpc-now >> /dev/null 2>&1");
    flag4=system("dscl . -create /Users/hpc-now UniqueID 1988 >> /dev/null 2>&1");
    flag5=system("dscl . -create /Groups/hpc-now PrimaryGroupID 1988 >> /dev/null 2>&1");
    flag5=system("dscl . -create /Users/hpc-now PrimaryGroupID 1988 >> /dev/null 2>&1");
    flag6=system("dscl . -create /Users/hpc-now NFSHomeDirectory /Users/hpc-now >> /dev/null 2>&1");

    if(flag1!=0||flag2!=0||flag3!=0||flag4!=0||flag5!=0||flag6!=0){
        printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
    
    printf("[ -INFO- ] Creating and configuring the running directory ...\n");
    system("mkdir -p /Applications/.hpc-now >> /dev/null 2>&1 && chmod 777 /Applications/.hpc-now >> /dev/null 2>&1");
    system("mkdir -p /Users/hpc-now >> /dev/null 2>&1");
    printf("[ -INFO- ] Creating a random file for encryption/decryption ...\n");
    generate_random_passwd(random_string);
    file_p=fopen("/Applications/.hpc-now/.now_crypto_seed.lock","w+");
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"%s\n",random_string);
    fclose(file_p);
    system("chown -R root:root /Applications/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("chflags schg /Applications/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("mkdir -p /Users/hpc-now/.bin >> /dev/null 2>&1");
    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o /Users/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL ...\n");
        sprintf(cmdline,"curl -s %s -o /Users/hpc-now/.bin/hpcopr",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"/bin/cp -r %s /Users/hpc-now/.bin/hpcopr >> /dev/null 2>&1 ",location);
    }

    if(system(cmdline)==0){
        printf("[ -INFO- ] Setting up environment variables for 'hpc-now' ...\n");
        strcpy(cmdline,"echo \"export PATH=/Users/hpc-now/.bin/:$PATH\" >> /Users/hpc-now/.bashrc");
        system(cmdline);
        sprintf(cmdline,"chmod +x /Users/hpc-now/.bin/hpcopr");
        system(cmdline);
    }
    else{
        printf("[ FATAL: ] Failed to get the hpcopr executable. This installation process is\n");
        printf("|          terminated. If you specified the location of hpcopr executable, please\n");
        printf("|          make sure the location is correct. Rolling back and exit now.\n");
        system("chflags noschg /Applications/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
        system("rm -rf /Applications/.hpc-now/ >> /dev/null 2>&1");
        system("dscl . -delete /Users/hpc-now >> /dev/null 2>&1");
        system("dscl . -delete /Groups/hpc-now >> /dev/null 2>&1");
        system("rm -rf /Users/hpc-now >> /dev/null 2>&1");
        return -1;
    }
    
    printf("[ -INFO- ] Creating other key running directories ...\n");
    system("mkdir -p /Users/hpc-now/.now-ssh/ >> /dev/null 2>&1");
    system("mkdir -p /Users/hpc-now/LICENSES/ >> /dev/null 2>&1");
    sprintf(cmdline,"curl -s %s -o /home/hpc-now/LICENSES/GPL-2",URL_LICENSE);
    system(cmdline);
    system("chown -R hpc-now:hpc-now /Users/hpc-now/ >> /dev/null 2>&1");
    printf("[ -INFO- ] Congratulations! The HPC-NOW services are ready to run!\n");
    printf("|          The user 'hpc-now' has been created *WITHOUT* an initial password.\n");
    printf("|          You *MUST* run 'sudo dscl . -passwd /Users/hpc-now PASSWORD' command\n");
    printf("|          to set a password. Please ensure the complexity of the new password!\n");
    printf("|          After setting password, please switch to the user 'hpc-now' and run\n");
    printf("|          the command 'hpcopr help' to get started. Enjoy you Cloud HPC journey!\n");
    printf("[ -INFO- ] Exit now.\n");
#elif __linux__
    if(system("id hpc-now >> /dev/null 2>&1")==0){
        printf("[ FATAL: ] User 'hpc-now' found. It seems the HPC-NOW services have been installed.\n");
        printf("|          If you'd like to reinstall, please uninstall first. Reinstallation\n");
        printf("|          is not permitted in order to protect your cloud clusters. In order to\n");
        printf("|          uninstall current HPC-NOW services, please run the command:\n");
        printf("|          sudo THIS_INSTALLER_FULL_PATH uninstall (Double confirm is needed)\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }

    printf("[ -INFO- ] Checking and cleaning up current environment ...\n");
    system("rm -rf /home/hpc-now/ >> /dev/null 2>&1");
    system("chattr -i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now/ >> /dev/null 2>&1");
    printf("[ -INFO- ] Adding the specific user 'hpc-now' to your OS ...\n");
    strcpy(cmdline,"useradd hpc-now -m -s /bin/bash >> /dev/null 2>&1");
    if(system(cmdline)!=0){
        printf("[ FATAL: ] Internal Error. Please contact info@hpc-now.com for truble shooting.\n");
        printf("[ FATAL: ] Exit now.\n");
        return -1;
    }
    
    printf("[ -INFO- ] Creating and configuring the running directory ...\n");
    system("mkdir -p /usr/.hpc-now && chmod 777 /usr/.hpc-now");
    printf("[ -INFO- ] Creating a random file for encryption/decryption ...\n");
    generate_random_passwd(random_string);
    file_p=fopen("/usr/.hpc-now/.now_crypto_seed.lock","w+");
    fprintf(file_p,"THIS FILE IS GENERATED AND MAINTAINED BY HPC-NOW SERVICES.\n");
    fprintf(file_p,"PLEASE DO NOT HANDLE THIS FILE MANNUALLY! OTHERWISE THE SERVICE WILL BE CORRUPTED!\n");
    fprintf(file_p,"SHANGHAI HPC-NOW TECHNOLOGIES CO., LTD | info@hpc-now.com | https://www.hpc-now.com\n\n");
    fprintf(file_p,"%s\n",random_string);
    fclose(file_p);
    system("chown -R root:root /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("chattr +i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");   
    system("mkdir -p /home/hpc-now/.bin >> /dev/null 2>&1");
    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL ...\n");
        sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"/bin/cp -r %s /home/hpc-now/.bin/hpcopr >> /dev/null 2>&1 ",location);
    }
    if(system(cmdline)==0){
        printf("[ -INFO- ] Setting up environment variables for 'hpc-now' ...\n"); 
        if(system("cat /home/hpc-now/.bashrc | grep PATH=/home/hpc-now/.bin/ >> /dev/null 2>&1")!=0){
            strcpy(cmdline,"echo \"export PATH=/home/hpc-now/.bin/:$PATH\" >> /home/hpc-now/.bashrc");
            system(cmdline);
        }
        sprintf(cmdline,"chmod +x /home/hpc-now/.bin/hpcopr");
        system(cmdline);
    }
    else{
        printf("[ FATAL: ] Failed to get the hpcopr executable. This installation process is\n");
        printf("|          terminated. If you specified the location of hpcopr executable, please\n");
        printf("|          make sure the location is correct. Rolling back and exit now.\n");
        system("chattr -i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
        system("rm -rf /usr/.hpc-now >> /dev/null 2>&1");
        system("userdel -f -r hpc-now >> /dev/null 2>&1");
        return -1;
    }
    system("mkdir -p /home/hpc-now/.now-ssh/ >> /dev/null 2>&1");
    system("mkdir -p /home/hpc-now/LICENSES/ >> /dev/null 2>&1");
    sprintf(cmdline,"curl -s %s -o /home/hpc-now/LICENSES/GPL-2",URL_LICENSE);
    system(cmdline);
    system("chown -R hpc-now:hpc-now /home/hpc-now/");
    printf("[ -INFO- ] Congratulations! The HPC-NOW services are ready to run!\n");
    printf("|          The user 'hpc-now' has been created *WITHOUT* an initial password.\n");
    printf("|          You *MUST* run 'sudo passwd hpc-now' command to set a password.\n");
    printf("|          Please ensure the complexity of the new password!\n");
    printf("|          After setting password, please switch to the user 'hpc-now' and run\n");
    printf("|          the command 'hpcopr help' to get started. Enjoy you Cloud HPC journey!\n");
    printf("[ -INFO- ] Exit now.\n");
#endif
    return 0;
}

// Forcely uninstall the HPC-NOW services
int uninstall_services(void){
    char doubleconfirm[128]="";
    // Double confirmation is needed.
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|*                                                                                 *\n");
    printf("|*   YOU ARE UNINSTALLING THE HPC-NOW SERVICES, PLEASE CONFIRM THE ISSUES BELOW:   *\n");
    printf("|*                                                                                 *\n");
    printf("|*   1. You have *DESTROYED* all the clusters managed by this device.              *\n");
    printf("|*      This is * !!! EXTREMELY IMPORTANT !!! *                                    *\n");
    printf("|*   2. You have *CHECKED* your cloud service account and all the resources        *\n");
    printf("|*      created by the HPC-NOW services on this device have been destructed.       *\n");
    printf("|*   3. You have *EXPORTED* the usage log and systemlog to a permenant directory,  *\n");
    printf("|*      You can run 'hpcopr syslog' and 'hpcopr usage' to get the logs and save    *\n");
    printf("|*      them to a directory such as /Users/ANOTHER_USER                            *\n");
    printf("|*                                                                                 *\n");
    printf("|*                       THIS OPERATION IS UNRECOVERABLE!                          *\n");
    printf("|*                                                                                 *\n");
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|  ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:\n");
    printf("[ INPUT: ]  ");
    fflush(stdin);
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    printf("[ -INFO- ] UNINSTALLING THE SERVICES AND REMOVING THE DATA NOW ...\n");
#ifdef _WIN32
    system("attrib -h -s -r c:\\programdata\\hpc-now > nul 2>&1");
    system("attrib -h -s -r c:\\hpc-now > nul 2>&1");
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    system("rd /s /q c:\\hpc-now > nul 2>&1");
    system("rd /s /q c:\\programdata\\hpc-now > nul 2>&1");
    system("net user hpc-now /delete > nul 2>&1");
#elif __APPLE__
    system("chflags noschg /Applications/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /Applications/.hpc-now/ >> /dev/null 2>&1");
    system("dscl . -delete /Users/hpc-now >> /dev/null 2>&1");
    system("dscl . -delete /Groups/hpc-now >> /dev/null 2>&1");
    system("rm -rf /Users/hpc-now >> /dev/null 2>&1");
#elif __linux__
    printf("[ -INFO- ] UNINSTALLING THE SERVICES AND REMOVING THE DATA NOW ...\n");
    system("chattr -i /usr/.hpc-now/.now_crypto_seed.lock >> /dev/null 2>&1");
    system("rm -rf /usr/.hpc-now >> /dev/null 2>&1");
    system("userdel -f -r hpc-now >> /dev/null 2>&1");
#endif
    printf("[ -DONE- ] The HPC-NOW cluster services have been deleted from this OS and device.\n");
#ifdef _WIN32
    printf("|          There are still remaining files for the specific user 'hpc-now'.\n");
    printf("|          Please mannually delete the folder C:\\Users\\hpc-now after reboot.\n");
#endif
    printf("|          Thanks a lot for using HPC-NOW services!\n");
    return 0;
}

int update_services(int loc_flag, char* location){
    char doubleconfirm[128]="";
    char cmdline[CMDLINE_LENGTH]="";
#ifdef _WIN32
    if(system("net user hpc-now > nul 2>&1")!=0){
#else
    if(system("id hpc-now >> /dev/null 2>&1")!=0){
#endif
        printf("[ FATAL: ] User 'hpc-now' not found. It seems the HPC-NOW Services have not been\n");
        printf("|          installed. Please install it first in order to update.\n");
        printf("[ FATAL: ] Exit now.\n");
        return 1;
    }
    printf("|*                                C A U T I O N !                                  *\n");
    printf("|*                                                                                 *\n");
    printf("|*     YOU ARE UPDATING THE HPC-NOW SERVICES. THE CURRENT hpcopr BINARY WILL BE    *\n");
    printf("|*     REPLACED. IF YOU UPDATE WITH THE hpcopr_loc= PARAMETER, PLEASE MAKE SURE    *\n");
    printf("|*     THAT THE LOCATION POINTS TO A VALID hpcopr EXECUTABLE.                      *\n");
    printf("|*                                                                                 *\n");
    printf("| ARE YOU SURE? Only 'y-e-s' is accepted to double confirm this operation:\n");
    printf("[ INPUT: ]  ");
    fflush(stdin);
    scanf("%s",doubleconfirm);
    if(strcmp(doubleconfirm,"y-e-s")!=0){
        printf("[ -INFO- ] Only 'y-e-s' is accepted to confirm. You chose to deny this operation.\n");
        printf("|          Nothing changed.\n");
        return 1;
    }
    printf("[ -INFO- ] UPDATING THE SERVICES NOW ...\n");

#ifdef _WIN32
    system("icacls c:\\hpc-now /remove Administrators > nul 2>&1");
    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL ...\n");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\hpcopr.exe",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"copy /y %s C:\\hpc-now\\hpcopr.exe > nul 2>&1 ",location);
    }
    if(system(cmdline)==0){
        printf("[ -DONE- ] The HPC-NOW cluster services have been updated to your device and OS.\n");
        printf("|          Thanks a lot for using HPC-NOW services!\n");
        system("mkdir c:\\hpc-now\\LICENSES > nul 2>&1");
        sprintf(cmdline,"curl -s %s -o C:\\hpc-now\\LICENSES\\GPL-2",URL_LICENSE);
        system(cmdline);
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
        return 0;
    }
#elif __APPLE__
    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o /Users/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL...\n");
        sprintf(cmdline,"curl -s %s -o /Users/hpc-now/.bin/hpcopr",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"/bin/cp -r %s /Users/hpc-now/.bin/hpcopr >> /dev/null 2>&1 ",location);
    }
    if(system(cmdline)==0){
        sprintf(cmdline,"chmod +x /Users/hpc-now/.bin/hpcopr && chown -R hpc-now:hpc-now /Users/hpc-now/.bin/hpcopr");
        system(cmdline);
        system("mkdir -p /Users/hpc-now/LICENSES/ >> /dev/null 2>&1");
        sprintf(cmdline,"curl -s %s -o /Users/hpc-now/LICENSES/GPL-2",URL_LICENSE);
        system(cmdline);
        printf("[ -DONE- ] The HPC-NOW cluster services have been updated to your device and OS.\n");
        printf("|          Thanks a lot for using HPC-NOW services!\n");
        return 0;
    }
#elif __linux__
    if(loc_flag==-1){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from the default URL ...\n");
        sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr",URL_HPCOPR_LATEST);
    }
    else if(loc_flag==0){
        printf("[ -INFO- ] Downloading the main program 'hpcopr' from your own URL...\n");
        sprintf(cmdline,"curl -s %s -o /home/hpc-now/.bin/hpcopr",location);
    }
    else{
        printf("[ -INFO- ] Copying the main program 'hpcopr' ...\n");
        sprintf(cmdline,"/bin/cp -r %s /home/hpc-now/.bin/hpcopr >> /dev/null 2>&1 ",location);
    }
    if(system(cmdline)==0){
        sprintf(cmdline,"chmod +x /home/hpc-now/.bin/hpcopr && chown -R hpc-now:hpc-now /home/hpc-now/.bin/hpcopr");
        system(cmdline);
        system("mkdir -p /Users/hpc-now/LICENSES/ >> /dev/null 2>&1");
        sprintf(cmdline,"curl -s %s -o /home/hpc-now/LICENSES/GPL-2",URL_LICENSE);
        system(cmdline);
        printf("[ -DONE- ] The HPC-NOW cluster services have been updated to your device and OS.\n");
        printf("|          Thanks a lot for using HPC-NOW services!\n");
        return 0;
    }
#endif
    else{
        printf("[ FATAL: ] Failed to update the HPC-NOW services. Please check and make sure:\n");
        printf("|          1. The HPC-NOW Services have been installed previously.\n");
        printf("|          2. The specified location (if specified) is correct.\n");
        printf("|          3. Your device is connected to the internet.\n");
        printf("|          4. Currently there is no 'hpcopr' thread(s) running.\n");
#ifdef _WIN32
        system("icacls c:\\hpc-now /deny Administrators:F > nul 2>&1");
#endif
        return 1;
    }
}

int valid_loc_format_or_not(char* loc_string){
    int length;
    length=strlen(loc_string);
    if(length==0){
        return -1;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)==':'&&*(loc_string+5)=='/'&&*(loc_string+6)=='/'){
        return 0;
    }
    if(*(loc_string+0)=='h'&&*(loc_string+1)=='t'&&*(loc_string+2)=='t'&&*(loc_string+3)=='p'&&*(loc_string+4)=='s'&&*(loc_string+5)==':'&&*(loc_string+6)=='/'&&*(loc_string+7)=='/'){
        return 0;
    }
    return 1;
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
        loc_flag=valid_loc_format_or_not(advanced_option_tail);
        if(loc_flag==-1){
            printf("[ -WARN- ] The location specified in the command has invalid format.\n");
            printf("           Will use the default location to download.\n");
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