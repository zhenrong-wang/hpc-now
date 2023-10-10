### 1. Project Background

Cloud High-Performance Computing, Cloud HPC differs from on-premise HPC significantly. Cloud service brings high scalability and flexibility to High-Performance Computing. However, most HPC users are not familiar with building and maintaining HPC services on the cloud. 

In order to make it super easy to start and manage HPC workloads on the cloud, we have been developing this project and named it as HPC-NOW. NOW represents:

- Start HPC journey on the cloud immediately, in minutes.
- (almost) No Operation Workload.


### 2. Core Components

Thanks to the Terraform[Terraform](http://www.terraform.io), a great cloud Infrastructure-as-Code platform, which makes it possible to orchestrate cloud resources in a unified and simple way.

In this project, we developed components as below:

-  **installer**  : HPC-NOW service installer, which requires administrator or root privilege to run.
-  **hpcopr**     : HPC Operator. The main component that manages the HPC clusters, users, jobs, data, monitoring, usage, etc.
-  **now-crypto** : A cryptography module that encrypts and decrypts the files containing sensitive information.
-  **hpcmgr**    : A utility running in every cluster to communicate with the operator.
-  **infra-as-code** : Infrastructure codes in HCL format
-  **scripts**    : Shell scripts to initialize the clusters, install applications, etc.


### 3. How-To: Build, Run, and Use

Please refer to Docs/UserManual-EN.pdf

### 4. Bug Reports

Please report issues to GitHub, Gitee, or 
mail to: zhenrongwang@live.com | wangzhenrong@hpc-now | info@hpc-now.com