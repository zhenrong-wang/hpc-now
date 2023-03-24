### 1. 项目背景

云上的高性能计算（ Cloud High-Performance Computing, Cloud HPC ），与本地集群相比，有多方面的区别。其中最为显著的区别，就是云资源的弹性、动态与灵活性。理论上讲，您可以轻松使用多个云计算机房的计算、存储、网络资源，您的超算集群规模可以扩展到数百、数千核心，也可能根据实际情况减小到 0 个计算核心。这种动态特点就意味着，您在一定程度上需要关心底层资源 —— 例如集群的规模、集群的数量等。

然而，资源管理对于广大的 Cloud HPC 用户来说，是比较陌生的；尤其是云资源的管理，更是无从下手。云提供海量的底层资源，但是 Cloud HPC 用户首先需要让这些资源组成具备超级计算能力的集群。这也就意味着需要一个  **构建和运行** 的过程。这个过程，对于大多数用户来说，难度非常高，原因在于它涉及到多方面的 IT 应用知识和云计算技能，包括但不限于：

- 什么是网络、虚拟私有网络、子网、网段、ACL、公有和私有 IP ……
- 什么是云主机、主机镜像、安全组、公钥私钥 ……
- 什么是云硬盘、文件存储、对象存储、挂载不同类型的存储 ……
- 什么是任务调度器、调度器如何配置和运行、超算用户如何使用和提交任务 ……
- ……

以上种种，都成为了制约 Cloud HPC 走向更广阔应用的一道门槛。目前，这些工作要么是由云厂商以自研的方式、面向 HPC 用户推出自有的 Cloud HPC 服务，要么是由第三方服务商进行集成开发之后以类 SaaS 平台的方式提供给客户。无论是哪一种方案，都旨在将客户留在各自的平台上；尽管在商业方面是完全合理而且完全值得尊重的，但是无形之中会限制最终用户对于超算的掌控权以及选择 Cloud HPC 服务的自由度。

为了打造全面开放且超级简单的 Cloud HPC 平台，让用户能够以极低的门槛开启 Cloud HPC，加速科研创新，我们打造了开源的云超算平台 HPC-NOW。这里，NOW 有两层含义：

- 现在即可开始，无需等待
- No Operation Workload，无需繁重的运行维护工作，即可在云上轻松管理您的超算集群和超算服务

希望与您共同携手打造开源、开放的 Cloud HPC 生态！

### 2. 核心组件

首先，在此鸣谢卓越、开源、面向多云的 IaC（ 基础设施即代码 ）平台 [Terraform](http://www.terraform.io)。正是 Terraform 强大的功能和良好的生态为 HPC-NOW 提供了坚实的基础，我们可以不必重复造轮子。本项目的核心工作，在于如何驱动 Terraform 来构建和管理一个或多个 Cloud HPC 集群。

本项目的核心组件如下：

-  **installer**  ：HPC-NOW 服务的安装器。主要负责服务的安装、卸载、更新三项工作。该安装器被设计为必须由管理员权限执行。
-  **hpcopr**     ：意即 HPC Operator，是 HPC-NOW 的核心程序，也是用户需要执行的主程序。为了确保安全性和隔离性，该程序被设计为必须由专属 OS 用户 "hpc-now" 执行，其他用户，即使是管理员用户或者根用户也无法执行。由 hpcopr 管理基础设施代码，并调用 Terraform 对云资源进行全生命周期管理。
-  **now-crypto** ：辅助程序，主要作用是简单的文件加密和解密，以确保敏感信息不以明文形式存放。请注意，该程序不是严格的加解密程序，仅通过统一偏移字符的方式对文本信息进行修改，您的密文文件仍需要妥善保管。一旦密文文件泄露，他人可能通过穷举得到您的文本偏移量，从而反向偏移得到原文。
-  **hpcmgr**    ：强大的集群内管理工具，包括集群的连接、SLURM 服务的启动、以及 HPC 软件包的自动化编译安装等。
-  **templates** ：模板文件是 IaC（基础设施及代码）的核心要素，我们已经针对 AWS、阿里云、腾讯云 三家公有云厂商制作了专用的资源模板，后续将进一步接入微软 Azure、GCP、华为云等三家云资源
-  **scripts**    ：启动脚本包含了集群各个节点启动过程的编排，包括各类必要组件的自动化安装。

### 3. 构建环境需求

我们使用 C 语言和 GNU/Linux Shell 脚本进行整个平台的构建。其中，三个核心程序  **installer** 、 **hpcopr** 、 **now-crypto**  均为纯 C 语言编写。C 语言跨平台和偏底层的特点，使得其适合用来进行核心程序的开发。对于这三个核心程序而言，构建过程仅需要 C 语言编译器即可，在三个主流操作系统方面，需要安装的 C 语言编译器略有不同。

-  **Microsoft Windows** ：您需要安装最新版 mingw，具体请参考 CSDN 上的[教程](http://blog.csdn.net/LawssssCat/article/details/103407137)，或者参考其他公开来源的教程。请务必注意安装完 mingw 之后，需要将 mingw 安装目录下面的 bin 文件夹路径添加到系统环境变量 PATH 中 
-  **GNU/Linux** ：您需要安装 GNU Compiler Collections，也就是我们熟知的 gcc。目前主流的版本一般为 8.x.x 及以上。您可以从操作系统自带的软件仓库中安装，如 yum 或者 apt，示例命令：`sudo yum -y install gcc` 或者 `sudo apt-get install gcc`
-  **macOS** ：您需要安装 clang，版本一般为 13.x.x 及以上。如果您的 mac 设备未安装过 clang 编译器，您可以尝试打开 Terminal 并输入 clang，如果本机没有安装clang，macOS 会询问是否安装，您授权之后可根据提示进行自动安装

安装完之后，请您在命令提示符或者终端中输入 `gcc --version` 或者` clang --version` 来确认编译器正确安装。

### 4. 如何构建

本项目的源代码尚未做比较好的拆分、注释也正在逐步添加中。由于源代码以整文件的方式发布，其构建过程也比较基础，无需复杂的依赖和 make 工具链支持。您只需要运行 gcc 或 clang 命令即可完成构建。以核心程序 hpcopr 为例：

请从本项目的 dev 分支下载源代码至本地目录（ 例如 /home/ABC/hpc-now-dev/ ），使用 'cd' 命令切换至代码所在的本地目录之后：

-  **Microsoft Windows用户** ，请运行：`gcc hpcopr-windows.c -Wall -o hpcopr.exe`
-  **GNU/Linux用户** ，请运行：`gcc hpcopr-linux.c -Wall -lm -o hpcopr`
-  **macOS用户** ，请运行：`clang hpcopr-darwin.c -Wall -o hpcopr`

此外，还需以相似的方式编译 now-crypto.c 和 installer-OS-VERSION.c，并将生成的可执行文件命名为  **now-crypto.exe**  和  **installer_OS_VERSION.exe** 。

### 5. 如何使用

请参阅[部署手册](http://www.hpc-now.com/deploy)。请注意：如果您直接运行 installer 进行本地安装，将会下载云上已经编译好的 hpcopr 和 now-crypto 至以下本地目录：

- **Windows** ： C:\programdata\hpc-now\bin\now-crypto.exe 和 C:\hpc-now\hpcopr.exe
-  **GNU/Linu** x：/usr/.hpc-now/.bin/now-crypto.exe 和 /home/hpc-now/.bin/hpcopr
-  **macOS** ：/Applications/.hpc-now/.bin/now-crypto.exe 和 /Users/hpc-now/.bin/hpcopr

您可以用自己构建的  **hpcopr**  和  **now-crypto**  替换掉安装时下载的文件，请注意文件名保持一致即可。此外，在 GNU/Linux 和 macOS 下，注意要赋予可执行权限，示例命令： 
- `sudo chmod +x now-crypto`

### 6. 关键目录

 **hpcopr** 安装部署之后，将会对您的操作系统进行如下修改。具体的修改操作请阅读 installer 源代码的 `int install_services(void)` 函数内容。

- 创建一个 **名为 hpc-now 的操作系统用户** ，对于 Microsoft Windows，该用户将生成初始密码 nowadmin2023~ ，并且在首次以 hpc-now 用户登录时强制要求修改；对于其他操作系统，无初始密码，您需要按照说明为该用户设置密码
-  **创建工作目录** ，对于 Microsoft Windows，将创建 C:\hpc-now 目录作为关键工作目录，创建 C:\programdata\hpc-now 作为关键数据目录；对于 GNU/Linux，关键工作目录位于 hpc-now 的家目录下，即：/home/hpc-now，关键数据目录位于 /usr/.hpc-now ；对于 macOS，关键工作目录位于 hpc-now 的家目录下，即：/Users/hpc-now，关键数据目录位于 /Applications/.hpc-now 。

上述对操作系统的修改均不会无意或恶意破坏您的操作系统。同时， **您可以随时以管理员身份执行 `sudo YOUR_INSTALLER_FULL_PATH uninstall` 操作回滚上述对操作系统的改动** 。请注意，uninstall 操作可能会导致您对云上的集群失去管理权，请务必按照软件说明进行确认操作。

如有任何疑问，请您阅读源代码、或者联系我们进行技术沟通。

### 7. 核心功能

如前所述，**hpcopr** 是您操作和管理 Cloud HPC 集群的核心界面。 **hpcopr** 目前以命令行的形式提供多种功能。您可以运行 `hpcopr help` 命令查看详细的帮助信息，主要如下：

` **hpcopr command_name PARAM1 PARAM2** `

 **_初始化类：_** 
-  **hpcopr new PARAM**  – PARAM 为必选参数，且只能为 workdir 或者 keypair。当 PARAM 为 workdir 时，将创建一个新的工作目录，并加密保存云服务密钥对，后续可进入该目录创建您的集群；当 PARAM 为 keypair 时，需要首先 cd 到一个工作目录，并将轮换该目录对应的云账户密钥对
-  **hpcopr init PARAM**  – 初始化一个集群。其中，PARAM为可选项，用于指定集群的ID。如不指定，将使用默认的ID：hpcnow 。示例命令：hpcopr init my-first-cluster
-  **hpcopr conf**  – 生成默认的集群配置文件，位于工作目录的 conf 文件夹内，您可进行修改之后再运行 hpcopr init 命令创建自定义配置的集群

 **_一般管理类：_** 
-  **hpcopr help**  – 在当前窗口显示完整的帮助信息
-  **hpcopr usage**  – 查看最新的集群用量表，该命令将导出一份 CSV 格式的文本文件，您可以使用相关软件查看
-  **hpcopr syslog**  – 查看集群管理相关日志，包括扩容、缩容、销毁、创建等，包括操作时间、操作内容、是否成功等
-  **hpcopr vault**  – 将以加密方式存储的集群敏感信息以明文形式显示在当前窗口。包括集群登录 IP 地址、登录密码、网盘登录密码等
-  **hpcopr graph**  – 查看当前集群的拓扑和配置。通过 graph，您可以直观的看到该集群的节点配置、节点地址、运行状况等

 **_集群操作类：_** 
-  **hpcopr delc PARAM**  – 删除计算节点。您既可以指定 PARAM 为 all，用于删除所有计算节点；也可以指定 PARAM 为一个正整数，用于删除指定数量的计算节点。示例命令： hpcopr delc 1 或 hpcopr delc all
-  **hpcopr addc NUM**  – 增加计算节点，其中 NUM 为您指定要添加的节点数目。示例命令：hpcopr addc 3
-  **hpcopr shutdownc PARAM**  – 关闭计算节点。与 hpcopr delc 类似，您可以指定 PARAM 为 all 或者要关闭的计算节点数量
-  **hpcopr turnonc PARAM**  – 开启计算节点，与 hpcopr shutdownc 命令类似，您可以指定 PARAM 为 all 或者要启动的计算节点数量
-  **hpcopr reconfc PARAM1 PARAM2**  – 计算节点配置变更。您可以将 PARAM1 和 PARAM2 留空，用于查看支持的节点配置列表；PARAM1 是新配置的名称；PARAM2可选，为超线程开关（仅限于 AWS | 亚马逊云科技）。示例命令：hpcopr reconfc i4c8g
-  **hpcopr reconfm PARAM**  – 更改管理节点的配置。用法与 hpcopr reconfc 类似，但是不支持超线程选项
-  **hpcopr sleep**  – 关闭整个集群，包括所有的管理节点和计算节点
-  **hpcopr wakeup PARAM**  – 唤醒集群，您可以指定 PARAM 为 all 或者 minimal。当 PARAM 为 all 时，集群内所有节点都会打开；当 PARAM 为 minimal 时，仅打开管理节点
-  **hpcopr destroy**  –  销毁当前的集群，包括所有节点和数据。请务必在确认销毁前妥善保存好您的关键数据 

### 8. Bug与技术沟通

欢迎随时联系 info@hpc-now.com