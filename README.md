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
-  **infra-as-code** ：模板文件是 IaC（基础设施及代码）的核心要素，我们已经针对 AWS、阿里云、腾讯云 三家公有云厂商制作了专用的资源模板.
-  **scripts**    ：启动脚本包含了集群各个节点启动过程的编排，包括各类必要组件的自动化安装。

### 3. 用法与功能

如前所述，**hpcopr** 是您操作和管理 Cloud HPC 集群的核心界面。 **hpcopr** 目前以命令行的形式提供多种功能。您可以运行 `hpcopr help` 命令查看详细的帮助信息，主要如下：

 **`hpcopr command_name PARAM1 PARAM2 ... `** 

##### 3.1 多集群管理

- envcheck ：快速检查和修复 hpcopr 的运行环境
- new-cluster ：创建集群。您需要输入 AWS | 阿里云 | 腾讯云 的访问密钥对。关于如何获取访问密钥，请参考各家云厂商的文档
- ls-clusters : 列出当前的所有集群
- switch ：切换集群，您需要提供目标集群的名字作为命令参数
- glance ：查看集群。您可以用 'all' 或者 目标集群的名字作为命令参数，以分别快速查看所有集群或者某个集群
- exit-current ：退出当前集群。
- remove ：彻底删除某个集群。您需要提供目标集群的名字作为命令参数

##### 3.2 全局管理

- help ：显示详细的帮助信息
- usage ：以文本方式查看集群的用量统计，该命令同样会将最新的用量统计导出到您的工作目录中
- syslog ：以文本方式查看 hpcopr 的命令运行情况记录，该命令会自动导出至您的工作目录中

###### 开发者选项：仅面向开发者，涉及到较为复杂的技术细节。具体文档将在后续补齐

- configloc ： 设置您自定义的 Terraform 相关包的位置。包括了 Terraform 的可执行程序以及云厂商的 provider
- showloc：查看目前所使用的 Terraform 相关包的位置
- resetloc：将上述位置重置为默认位置

##### 3.3 集群初始化：注意，以下所有命令都将要求您先切换至某一个集群。

- get-conf ：获取默认的集群创建配置，如有需要，您可以编辑并保存
- edit-conf：获取并编辑集群的创建配置
- init ： 初始化集群。如您未在初始化之前下载任何创建配置文件，该命令将自行使用默认的配置文件创建您的集群。无论对于开发者还是普通用户，我们都强烈推荐使用免配置创建的方式，即：您直接运行 hpcopr init 命令即可
- new-keypair ：建议您定期或不定期轮换云访问密钥对，您可以用该命令轮换当前集群所使用的密钥对

##### 3.4 单集群管理：注意，以下所有命令都将要求您先切换至某一个集群。

- ssh ： 免密 SSH 登录至您的当前集群
- vault ：查看当前集群的敏感信息，包括 master 节点的登录方式
- graph：绘制当前集群的配置和拓扑图

##### 3.5 集群操作：注意，以下所有命令都将要求您先切换至某一个集群。

- delc ：删除若干个（指定数字作为命令参数）或所有（指定 'all' 作为命令参数）计算节点
- addc ：添加若干个（指定数字作为命令参数）计算节点
- shutdownc ：关闭部分或者所有计算节点
- turnonc ：开启部分或者所有计算节点
- reconfc ：计算节点改配置，您可以指定目标配置，如 a4c8g 。您也可以不带参数用于查看当前所有的可用配置
- reconfm ：主节点改配置。您可以指定目标配置，如 a8c16g 。 您也可以不带参数用于查看当前所有的可用配置
- sleep ：休眠整个集群。该操作将使得集群所有节点处于关闭状态
- wakeup ：唤醒整个集群（all参数）或者只唤醒集群的管理节点（minimal参数）（计算节点仍保持关闭）
- destroy：永久销毁整个集群。销毁之后，您可以再次运行 init 命令创建，但是您之前的所有节点和数据已经全部销毁

##### 3.6 其他功能：注意，以下命令不要求您切换到任何集群。

- about ：关于本程序
- license ：阅读程序的 License，本程序使用 GNU Public License 作为主要 License。其余开源组件的 License 将在后续包含进来
- repair ： 快速修复 hpcopr，包括检查运行环境和核心组件

### 4. 构建环境需求

为了更方便的获取代码和管理分支，请使用 git。关于如何在不同的操作系统环境中安装 git，请参考公开教程。
我们使用 C 语言和 GNU/Linux Shell 脚本进行整个平台的构建。其中，三个核心程序  **installer** 、 **hpcopr** 、 **now-crypto**  均为纯 C 语言编写。C 语言跨平台和偏底层的特点，使得其适合用来进行核心程序的开发。对于这三个核心程序而言，构建过程仅需要 C 语言编译器即可，在三个主流操作系统方面，需要安装的 C 语言编译器略有不同。

-  **Microsoft Windows** ：您需要安装最新版 mingw，具体请参考 CSDN 上的[教程](http://blog.csdn.net/LawssssCat/article/details/103407137)，或者参考其他公开来源的教程。请务必注意安装完 mingw 之后，需要将 mingw 安装目录下面的 bin 文件夹路径添加到系统环境变量 PATH 中。 
-  **GNU/Linux** ：您需要安装 GNU Compiler Collections，也就是我们熟知的 gcc。目前主流的版本一般为 8.x.x 及以上。您可以从操作系统自带的软件仓库中安装，如 yum 或者 apt，示例命令：`sudo yum -y install gcc` 或者 `sudo apt-get install gcc`
-  **macOS** ：您需要安装 clang，版本一般为 13.x.x 及以上。如果您的 mac 设备未安装过 clang 编译器，您可以尝试打开 Terminal 并输入 clang，如果本机没有安装clang，macOS 会询问是否安装，您授权之后可根据提示进行自动安装

安装完之后，请您在命令提示符或者终端中输入 `gcc --version` 或者` clang --version` 来确认编译器正确安装。

### 5. 如何构建


- Step 1. git clone https://gitee.com/zhenrong-wang/hpc-now.git
- Step 2. cd hpc-now
- Step 3. git checkout development
- Step 4. 
    - Micorsoft Windows 用户： .\make_windows.bat build （也可以不带参数，查看该脚本的说明）
    - GNU/Linux 用户：chmod +x ./make_linux.sh && ./make_linux.sh build
    - macOS 用户：chmod +x ./make_darwin.sh && ./make_darwin.sh build

构建完成之后，会创建一个 build 目录，并在其中生成若干可执行程序和一个静态库 libgfuncs.a 。静态库已经链接到 installer 之中，后续不会再使用，可以丢弃。对于 GNU/Linux 用户，还会生成 hpcmgr.exe。构建出的组件说明如下：

- hpcopr-win.exe | hpcopr-lin.exe | hpcopr-dwn.exe ：如上所述，该组件是 HPC-NOW 的最核心组件。
- installer-win.exe | installer-lin.exe | installer-dwn.exe：如上所述，该组件是安装器，您后续需要该安装器来在您的本地环境中安装、更新、卸载 HPC-NOW 服务。
- now-crypto-win.exe | now-crypto-lin.exe | now-crypto-dwn.exe：如上所述，该组件是**对称**加密和解密程序。是 HPC-NOW 服务的依赖组件。
- libgfuncs.a：如上所述，该组件后续无用，可自行删除和丢弃。
- hpcmgr.exe：该程序是一个非常轻量化的 hpcmgr 本地程序，您可以在创建好的任意集群中，将该组件放置在 /usr/bin 目录下。用于替换默认的 hpcmgr.exe 。**一般情况下不需要进行该操作。**


### 6. 如何安装

构建完成后，请参考以下步骤安装。

#### Microsoft Windows 用户：

假设您在步骤 5 中构建的程序位于 C:\Users\ABC\hpc-now\build 目录中。

- 请在 Windows 搜索框（一般位于您桌面上的 Windows 徽标旁）中输入三个字母 cmd
- 在弹出的 “命令提示符” 图标上**右击 -> 以管理员身份运行**（**非常重要！请不要直接单击命令提示符图标**），将有一个黑色底色的”命令提示符“窗口弹出在您的桌面上
- cd C:\Users\ABC\hpc-now\build
- 您可以选择不同的安装选项：
    - 如您想要使用在步骤 5 中自行构建的 hpcopr-win.exe，请运行：
        - .\installer-win.exe install hpcoprloc=hpcopr-win.exe （建议开发者）
    - 如您想使用默认的最新版本 hpcopr-win.exe，请运行：
        - .\installer-win.exe install （建议普通用户）
    - 同样的，如您想要使用自行构建的 now-crypto-lin.exe，可参考如上方式添加 cryptoloc=now-crypto-lin.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
- 此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户，其初始密码为 nowadmin2023~
- 现在，按下键盘上的 Ctrl + Alt + Delete 组合按键，点击 “ 切换用户 ” ，请切换至用户 hpc-now，输入初始密码之后，需要按照要求更改登陆密码
- 切换到 hpc-now 用户之后，您同样需要启动 命令提示符，但是此时您**无需**以管理员身份运行，**请务必直接单击图标**
- 依次运行如下两条命令：
    - cd c:\hpc-now
    - hpcopr envcheck

此时，hpcopr 将开始下载必要的组件并部署在您的系统之中，部件的总大小约 150 MB；该过程视您的网络情况而定，可能需要 1 ~ 5 分钟。如您看到如下回显，则说明 hpcopr 已经可以运行。您可以跳至步骤 7 。

[ -INFO- ] Running environment successfully checked.

#### GNU/Linux 用户：

假设您在步骤 5 中构建的程序位于 /home/ABC/hpc-now/build 目录中。

- cd /home/ABC/hpc-now/build
- 您可以选择不同的安装选项：
    - 如您想要使用在步骤 5 中自行构建的 hpcopr-lin.exe，请运行：
        - sudo ./installer-lin.exe install hpcoprloc=hpcopr-lin.exe （建议开发者）
    - 如您想使用默认的最新版本 hpcopr-lin.exe，请运行：
        - sudo ./installer-lin.exe install （建议普通用户）
    - 同样的，如您想要使用自行构建的 now-crypto-lin.exe，可参考如上方式添加 cryptoloc=now-crypto-lin.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
    此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户。您需要运行如下命令来为 hpc-now 用户创建密码：
- sudo passwd hpc-now
- 创建完成后，请依次运行如下命令：
- su hpc-now （请输入刚刚设置好的密码）
- cd ~
- hpcopr envcheck

此时，hpcopr 将开始下载必要的组件并部署在您的系统之中，部件的总大小约 150 MB；该过程视您的网络情况而定，可能需要 1 ~ 5 分钟。如您看到如下回显，则说明 hpcopr 已经可以运行。

[ -INFO- ] Running environment successfully checked.

#### macOS 用户：

假设您在步骤 5 中构建的程序位于 /Users/ABC/hpc-now/build 目录中。

- cd /Users/ABC/hpc-now/build
- 您可以选择不同的安装选项：
    - 如您想要使用在步骤 5 中自行构建的 hpcopr-dwn.exe，请运行：
        - sudo ./installer-dwn.exe install hpcoprloc=hpcopr-dwn.exe （建议开发者）
    - 如您想使用默认的最新版本 hpcopr-dwn.exe，请运行：
        - sudo ./installer-dwn.exe install （建议普通用户）
    - 同样的，如您想要使用自行构建的 now-crypto-dwn.exe，可参考如上方式添加 cryptoloc=now-crypto-dwn.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
    此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户。您需要运行如下命令来为 hpc-now 用户创建密码：
- sudo dscl . -passwd /Users/hpc-now **mycomplexpasswd** （请将加粗字体替换成为您的自定义密码！）
- 创建完成后，请依次运行如下命令：
- su hpc-now （请输入刚刚设置好的密码）
- cd ~
- hpcopr envcheck

此时，hpcopr 将开始下载必要的组件并部署在您的系统之中，部件的总大小约 150 MB；该过程视您的网络情况而定，可能需要 1 ~ 5 分钟。如您看到如下回显，则说明 hpcopr 已经可以运行。

[ -INFO- ] Running environment successfully checked.

如果您因各种原因无法完成下载，不必担心。您后续可以随时运行 hpcopr repair 命令来修复所有组件。

### 7. 关键目录

 **hpcopr** 安装部署之时，将会对您的操作系统进行如下修改。具体的修改操作请阅读 installer 源代码的 `int install_services(void)` 函数内容。

- 创建一个 **名为 hpc-now 的操作系统用户** ，对于 Microsoft Windows，该用户将生成初始密码 nowadmin2023~ ，并且在首次以 hpc-now 用户登录时强制要求修改；对于其他操作系统，无初始密码，您需要按照步骤 5 为该用户设置密码。
-  **创建工作目录** ，对于 Microsoft Windows，将创建 C:\programdata\hpc-now 作为关键目录；对于 GNU/Linux，关键目录位于 /usr/.hpc-now ；对于 macOS，关键数据目录位于 /Applications/.hpc-now 。

上述对操作系统的修改均不会无意或恶意破坏您的操作系统。同时， **您可以随时以管理员身份执行 `sudo YOUR_INSTALLER_PATH uninstall` 操作回滚上述对操作系统的改动** 。请注意，uninstall 操作可能会导致您对云上的集群失去管理权，请务必按照软件说明进行确认操作。

如有任何疑问，请您阅读源代码、或者联系我们进行技术沟通。

### 8. 如何卸载

您可以随时以管理员身份完全卸载 HPC-NOW 服务，卸载过程将完全抹除 HPC-NOW 对您的设备和操作系统的更改，并删除您的云集群管理数据。关于如何以管理员身份运行，请参考本文档的第 6 章 “ 如何安装 ”。

对于 Microsoft Windows 用户，假设您的安装器位于如下位置：

C:\Users\ABC\Downloads\installer-win.exe

则您需要在**由管理员运行***的 CMD 窗口中运行如下命令：

C:\Users\ABC\Downloads\installer-win.exe uninstall
对于 GNU/Linux 或者 macOS 用户，假设您已经切换至安装器所在的目录，则您需要分别运行如下命令：

sudo ./installer-lin uninstall
（GNU/Linux 用户）
sudo ./installer-dwn uninstall
（macOS 用户）
**非常重要：请在卸载之前，务必确认销毁在当前设备上管理的所有集群。您可以用 hpc-now 用户运行 hpcopr glance all 命令确认您的所有集群已经销毁。否则，您将永久失去对当前尚未销毁的集群的管理能力！**

### 9. 异常处理

在特殊情况下，hpcopr 可能会运行失败。此时，您可能有残留的云资源为

### 10. Bug与技术沟通

欢迎随时联系 info@hpc-now.com