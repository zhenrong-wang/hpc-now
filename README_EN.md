### 1. Project Background

Cloud High-Performance Computing, Cloud HPC differs from on-premise HPC significantly. Cloud service brings high scalability and flexibility to High-Performance Computing. However, most HPC users are not familiar with building and maintaining HPC services on the cloud. 

In order to make it super easy to start and manage HPC workloads on the cloud, we have been developing this project and named it as HPC-NOW. NOW stands for:

- Start HPC journey on the cloud immediately, in minutes.
- (almost) No Operation Workload.

Currently, the HPC-NOW platform supports the cloud services as below:

- [Alibaba Cloud](https://www.alibabacloud.com/en), HPC-NOW Internal Code: CLOUD_A
- [Tencent Cloud](https://www.tencentcloud.com/), HPC-NOW Internal Code: CLOUD_B
- [Amazon Web Services](https://aws.amazon.com/), HPC-NOW Internal Code: CLOUD_C
- [Huawei Cloud](https://www.huaweicloud.com/), HPC-NOW Internal Code: CLOUD_D
- [Baidu BCE](https://cloud.baidu.com/), HPC-NOW Internal Code: CLOUD_E
- [Microsoft Azure](https://azure.microsoft.com/en-us/), HPC-NOW Internal Code: CLOUD_F
- [Google Cloud Platform](https://cloud.google.com/), HPC-NOW Internal Code: CLOUD_G

### 2. Core Components

Thanks to the [Terraform](http://www.terraform.io), a great cloud Infrastructure-as-Code platform, which makes it possible to orchestrate cloud resources in a unified and simple way.

In this project, we developed components as below:

-  **installer**  : HPC-NOW service installer, which requires administrator or root privilege to run.
-  **hpcopr**     : HPC Operator. The main component that manages the HPC clusters, users, jobs, data, monitoring, usage, etc.
-  **now-crypto** : A cryptography module that encrypts and decrypts the files containing sensitive information.
-  **hpcmgr**    : A utility running in every cluster to communicate with the operator.
-  **infra-as-code** : Infrastructure codes in HCL format
-  **scripts**    : Shell scripts to initialize the clusters, install applications, etc.


### 3. How-To: Build, Run, and Use

The HPC-NOW platform is very easy to build, run, and use. It is also cross-platform, which means you can run the HPC-NOW on Windows, GNU/Linux, or macOS. Note: Currently only x86_64 platform is supported.

#### 3.1 Build

Prerequisites: git, gcc (for GNU/Linux) | clang (for macOS) | mingw (for Microsoft Windows)

Step 1. Clone this repository: git clone https://github.com/zhenrong-wang/hpc-now

Step 2. Change the directory : cd hpc-now

Step 3. Run the build script : 

    - For Microsoft Windows users: .\make_windows.bat build
    - For GNU/Linux Distro users : ./make_linux.sh build
    - For macOS users            : ./make_darwin.sh build

If everything goes well, the binaries will be built to the 'build' folder.

#### 3.2 Run

Step 1. Run the installer:

- For Microsoft Windows users: open a cmd windows as Administrator, and change the direcroty to the 'build' folder. Then run: 

    .\installer-win-INSTALLER_VERSION_CODE.exe install --hloc hpcopr-win-HPCOPR_VERSION_CODE.exe --cloc now-crypto-win.exe 

    Please replace the INSTALLER_VERSION_CODE and HPCOPR_VERSION_CODE to the real codes.

- For GNU/Linux Distro users: 

    sudo ./installer-lin-INSTALLER_VERSION_CODE.exe install --hloc hpcopr-lin-HPCOPR_VERSION_CODE.exe --cloc now-crypto-lin.exe 

- For macOS users:

    sudo ./installer-dwn-INSTALLER_VERSION_CODE.exe install --hloc hpcopr-dwn-HPCOPR_VERSION_CODE.exe --cloc now-crypto-dwn.exe 

Step 2. Run the hpcopr.exe

- For Microsoft Windows users: 

- For GNU/Linux Distros and macOS users:


For more information, please refer to Docs/UserManual-EN.pdf

### 4. Bug Reports

Please submit issues to this repo. Or
mailto: zhenrongwang@live.com | wangzhenrong@hpc-now | info@hpc-now.com