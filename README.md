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
-  **now-crypto** ：核心程序，主要作用是文本文件加密和解密，以确保敏感信息不以明文形式存放。请注意，该程序通过偏移字符的方式对文本信息进行随机修改，您的密文文件仍需要妥善保管。
-  **hpcmgr**    ：强大的集群内管理工具，包括集群的连接、SLURM 服务的启动、以及 HPC 软件包的自动化编译安装等。
-  **infra-as-code** ：模板文件是 IaC（基础设施及代码）的核心要素，我们已经针对 AWS、阿里云、腾讯云 三家公有云厂商制作了专用的资源模板。
-  **scripts**    ：启动脚本包含了集群各个节点启动过程的编排，包括各类必要组件的自动化安装。

### 3. 用法与功能

如前所述，**hpcopr** 是您操作和管理 Cloud HPC 集群的核心界面。 **hpcopr** 目前以命令行的形式提供多种功能。您可以运行 `hpcopr help` 命令查看详细的帮助信息，主要如下：

 **`hpcopr command_name PARAM1 PARAM2 ... -c=CLUSTER_NAME`** 

##### 3.0 快速环境检查

- envcheck ：快速检查和修复 hpcopr 的运行环境，强烈建议首次运行 hpcopr 时运行该命令

##### 3.1 多集群管理

- new-cluster ：创建集群。您需要输入 AWS | 阿里云 | 腾讯云 的访问密钥对。关于如何获取访问密钥，请参考各家云厂商的文档
- ls-clusters : 列出当前的所有集群
- switch ：切换集群，您需要提供目标集群的名字作为命令参数
- glance ：查看集群。您可以用 'all' 或者 目标集群的名字作为命令参数，以分别快速查看所有集群或者某个集群
- refresh：刷新集群。您可以不用参数以刷新当前集群（如果您已切换至一个集群）或者将目标集群的名字作为命令参数，以刷新目标集群
- exit-current ：退出当前集群。
- remove ：彻底删除某个集群。您需要提供目标集群的名字作为命令参数

##### 3.2 全局管理

- help ：显示详细的帮助信息
- usage ：以文本方式查看集群的用量统计，该命令同样会将最新的用量统计导出到您的工作目录中
- history ：以文本方式查看 hpcopr 的命令运行历史记录
- syserr ：以文本方式查看系统命令的报警和错误，一般用于故障排查
- ssh ： 免密 SSH 登录至您的当前集群

###### 开发者选项：仅面向开发者，涉及到较为复杂的技术细节。具体文档将在后续补齐

- syserr ：以文本方式查看 hpcopr 运行过程中调用系统命令时触发的报错信息
- configloc ： 设置您自定义的 Terraform 相关包的位置。包括了 Terraform 的可执行程序以及云厂商的 provider
- showloc：查看目前所使用的 Terraform 相关包的位置
- resetloc：将上述位置重置为默认位置

##### 3.3 集群初始化：

- get-conf ：获取默认的集群创建配置，如有需要，您可以编辑并保存
- edit-conf：获取并编辑集群的创建配置
- init ： 初始化集群。如您未在初始化之前下载任何创建配置文件，该命令将自行使用默认的配置文件创建您的集群。无论对于开发者还是普通用户，我们都强烈推荐使用免配置创建的方式，即：您直接运行 hpcopr init 命令即可
- new-keypair ：建议您定期或不定期轮换云访问密钥对，您可以用该命令轮换当前集群所使用的密钥对
- rebuild ：您可以重新构建集群的某些节点，适用于个别节点初始化失败的情况

##### 3.4 单集群管理：

- vault ：查看当前集群的敏感信息，包括 master 节点的登录方式
- graph：绘制当前集群的配置和拓扑图
- viewlog：以滚动方式显示集群当前的实时操作过程（例如正在创建的资源等）

##### 3.5 集群操作：

- delc ：删除若干个（指定数字作为命令参数）或所有（指定 'all' 作为命令参数）计算节点
- addc ：添加若干个（指定数字作为命令参数）计算节点
- shutdownc ：关闭部分或者所有计算节点
- turnonc ：开启部分或者所有计算节点
- reconfc ：计算节点改配置，您可以指定目标配置，如 a4c8g 。您也可以不带参数用于查看当前所有的可用配置
- reconfm ：主节点改配置。您可以指定目标配置，如 a8c16g 。 您也可以不带参数用于查看当前所有的可用配置
- sleep ：休眠整个集群。该操作将使得集群所有节点处于关闭状态
- wakeup ：唤醒整个集群（all参数）或者只唤醒集群的管理节点（minimal参数）（计算节点仍保持关闭）
- destroy：永久销毁整个集群。销毁之后，您可以再次运行 init 命令创建，但是您之前的所有节点和数据已经全部销毁

##### 3.6 用户管理：

注意：用户管理功能要求您的集群处于（最小或完整）运行状态。

用法：hpcopr userman 管理选项 参数1 参数2 ...

- userman add ： 新增一个用户。该命令最多可以带两个字符串作为参数，分别是用户名和密码
- userman delete ：删除一个用户。该命令最多可以带一个字符串作为参数，用于指定用户名
- userman list ： 列出当前的所有用户。该命令无需携带参数
- userman enable ： 激活一个已存在的用户，处于 enabled 的用户可以提交超算任务。该命令最多可以携带一个字符串作为参数，用于指定用户名
- userman disable：注销一个已经活跃用户，处于 disabled 的用户仍然可以登录集群，但是无法提交超算任务。该命令最多可以携带一个字符串作为参数，用户指定用户名
- userman passwd ：重置用户的密码。该命令最多可以携带两个字符串作为参数，用于指定用户名（已存在）和新的密码字符串

##### 3.7 其他功能：

- about ：关于本程序
- license ：阅读程序的 License，本程序使用 GNU Public License 作为主要 License。其余开源组件的 License 将在后续包含进来
- version : 查看当前版本号
- repair ： 快速修复 hpcopr，包括检查运行环境和核心组件

### 4. 构建环境需求（针对开发者）

为了更方便的获取代码和管理分支，请使用 [git](https://git-scm.com/)。关于如何在不同的操作系统环境中安装 git，请参考公开教程。

我们使用 C 语言和 GNU/Linux Shell 脚本进行整个平台的构建。其中，三个核心程序  **installer** 、 **hpcopr** 、 **now-crypto**  均为纯 C 语言编写。C 语言跨平台和偏底层的特点，使得其适合用来进行核心程序的开发。对于这三个核心程序而言，构建过程仅需要 C 语言编译器即可，在三个主流操作系统方面，需要安装的 C 语言编译器略有不同。

-  **Microsoft Windows** ：您需要安装最新版 mingw，具体请参考 CSDN 上的[教程](http://blog.csdn.net/LawssssCat/article/details/103407137)，或者参考其他公开来源的教程。请务必注意安装完 mingw 之后，需要将 mingw 安装目录下面的 bin 文件夹路径添加到系统环境变量 PATH 中。 
-  **GNU/Linux** ：您需要安装 GNU Compiler Collections，也就是我们熟知的 gcc。目前主流的版本一般为 8.x.x 及以上。您可以从操作系统自带的软件仓库中安装，如 yum 或者 apt，示例命令：`sudo yum -y install gcc` 或者 `sudo apt-get install gcc`
-  **macOS** ：您需要安装 clang，版本一般为 13.x.x 及以上。如果您的 mac 设备未安装过 clang 编译器，您可以尝试打开 Terminal 并输入 clang，如果本机没有安装clang，macOS 会询问是否安装，您授权之后可根据提示进行自动安装

安装完之后，请您在命令提示符或者终端中输入 `gcc --version` 或者` clang --version` 来确认编译器正确安装。

### 5. 如何构建（针对开发者）

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
- now-crypto-win.exe | now-crypto-lin.exe | now-crypto-dwn.exe：如上所述，该组件是 HPC-NOW 服务的核心依赖组件。
- libgfuncs.a：如上所述，该组件后续无用，可自行删除和丢弃。
- hpcmgr.exe：该程序是一个非常轻量化的 hpcmgr 本地程序，您可以在创建好的任意集群中，将该组件放置在 /usr/bin 目录下。用于替换默认的 hpcmgr.exe 。**一般情况下不需要进行该操作。**


### 6. 如何安装

对于开发者，构建完成后，请参考以下步骤安装。

对于一般用户，无需构建，请直接下载安装器。开发版的安装器下载链接请点击：[Microsoft Windows](https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/installer-dev/installer-win-0.2.0.0120.exe) | [macOS](https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/installer-dev/installer-dwn-0.2.0.0120.exe) | [GNU/Linux](https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/installer-dev/installer-lin-0.2.0.0120.exe)

注意：在安装之前，请确保您的安装器文件可执行。对于 macOS 和 GNU/Linux 用户，请使用 chmod +x 命令赋权。

#### Microsoft Windows 用户：

假设您在步骤 5 中构建的程序位于 C:\Users\ABC\hpc-now\build 目录中。

- 请在 Windows 搜索框（一般位于您桌面上的 Windows 徽标旁）中输入三个字母 cmd
- 在弹出的 “命令提示符” 图标上**右击 -> 以管理员身份运行**（**非常重要！请不要直接单击命令提示符图标**），将有一个黑色底色的”命令提示符“窗口弹出在您的桌面上
- cd C:\Users\ABC\hpc-now\build
- 您可以选择不同的安装选项：
    - 如您想要使用在步骤 5 中自行构建的 hpcopr-win.exe，请运行：
        - .\installer-win.exe install hpcoprloc=hpcopr-win.exe （建议开发者）
    - 如您想使用默认的最新版本 hpcopr-win.exe，请运行：
        - .\installer-win.exe install （建议一般用户）
    - 同样的，如您想要使用自行构建的 now-crypto-lin.exe，可参考如上方式添加 cryptoloc=now-crypto-lin.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
- 此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户，其初始密码为 nowadmin2023~
- 完成之后，请依次输入以下命令：
- net user hpc-now YOUR_COMPLEX_PASSWORD 请为 hpc-now 用户设置具备一定复杂度的密码
- runas /profile /savecred /user:mymachine\hpc-now powershell
- 此时会弹出一个新的 powershell 窗口，请在**新的窗口**中运行：
    - hpcopr envcheck

此时，hpcopr 将开始下载必要的组件并部署在您的系统之中，部件的总大小约 150 MB；该过程视您的网络情况而定，可能需要 1 ~ 5 分钟。如您看到如下回显，则说明 hpcopr 已经可以运行。

[ -INFO- ] Running environment successfully checked.

后续，请务必在运行 hpcopr 之前首先通过上述 runas 命令调出以 **hpc-now** 用户身份运行的 powershell 窗口。该步骤至关重要，否则您将无权运行 hpcopr 命令。

#### GNU/Linux 用户：

假设您在步骤 5 中构建的程序位于 /home/ABC/hpc-now/build 目录中。

- cd /home/ABC/hpc-now/build
- 您可以选择不同的安装选项：
    - 如您想要使用在步骤 5 中自行构建的 hpcopr-lin.exe，请运行：
        - sudo ./installer-lin.exe install hpcoprloc=hpcopr-lin.exe （建议开发者）
    - 如您想使用默认的最新版本 hpcopr-lin.exe，请运行：
        - sudo ./installer-lin.exe install （建议一般用户）
    - 同样的，如您想要使用自行构建的 now-crypto-lin.exe，可参考如上方式添加 cryptoloc=now-crypto-lin.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
    此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户。

**hpcopr 全面支持 sudo 。如果您当前用户拥有 sudo 权限，您也可以无需设置 hpc-now 的密码，直接使用 sudo -u hpc-now 作为前缀，来执行 hpcopr 的所有命令而无需 su hpc-now 切换用户，使用更加简单。例如：sudo -u hpc-now hpcopr envcheck 。我们建议您使用 sudo 模式运行 hpcopr。**

**如您不使用 sudo 模式**，则需要按照以下步骤为 hpc-now 用户设置密码并手动切换用户：
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
        - sudo ./installer-dwn.exe install （建议一般用户）
    - 同样的，如您想要使用自行构建的 now-crypto-dwn.exe，可参考如上方式添加 cryptoloc=now-crypto-dwn.exe 参数。
    - 如您想跳过 License 阅读（不建议），可添加 skiplic=y 参数。
    此时，安装器将开始安装过程。安装过程将在几秒内完成。在该过程中，您的操作系统新增了名为 hpc-now 的用户。

**hpcopr 全面支持 sudo 。如果您当前用户拥有 sudo 权限，您也可以无需设置 hpc-now 的密码，直接使用 sudo -u hpc-now 作为前缀，来执行 hpcopr 的所有命令而无需 su hpc-now 切换用户，使用更加简单。例如：sudo -u hpc-now hpcopr envcheck 。我们建议您使用 sudo 模式运行 hpcopr。**

**如您不使用 sudo 模式**，则需要按照以下步骤为 hpc-now 用户设置密码并手动切换用户：
- sudo dscl . -passwd /Users/hpc-now **mycomplexpasswd** （请将加粗字体替换成为您的自定义密码！）
- su hpc-now （请输入刚刚设置好的密码）
- cd ~
- hpcopr envcheck

此时，hpcopr 将开始下载必要的组件并部署在您的系统之中，部件的总大小约 150 MB；该过程视您的网络情况而定，可能需要 1 ~ 5 分钟。如您看到如下回显，则说明 hpcopr 已经可以运行。

[ -INFO- ] Running environment successfully checked.

如果您因各种原因无法完成下载，不必担心。您后续可以随时运行 hpcopr repair 命令来修复所有组件。

### 7. 创建并登录您的第一个集群

在顺利完成上一个步骤之后，您现在已经可以使用强大的 hpcopr 创建、登录并管理您的第一个集群了。请在命令行窗口中依次输入以下命令：

- hpcopr new-cluster 您将被要求依次输入集群名字、对应的云厂商密钥对。建议您依次复制并粘贴云厂商的密钥对，并按回车确认。
- hpcopr init 此时，将创建默认的集群。该集群将内置 3 个 HPC 用户，并拥有 1 个计算节点。创建过程大约需要 7 分钟，在此期间，您仅需等待即可，无需任何操作。
- hpcopr ssh user1 以 user1 的身份登录您的集群
- hpcmgr install fftw3 尝试部署您的第一个 HPC 应用 - fftw3

**建议您使用 hpcopr vault 命令获取用户名和密码，并使用远程桌面连接（RDP）工具连接至您的集群，集群已经内置了用户友好和易于使用的桌面环境，您仅需要运行 gini 命令即可完成桌面环境的初始化。**

您还可以退出 ssh 环境，并尝试运行 hpcopr 的其他命令，例如增加/删除节点、激活/注销用户、修改节点配置、集群休眠及唤醒等。

### 8. 关键目录

 **hpcopr** 安装部署之时，将会对您的操作系统进行如下修改。具体的修改操作请阅读 installer 源代码的 `int install_services(void)` 函数内容。

- 创建一个 **名为 hpc-now 的操作系统用户** ，对于 Microsoft Windows，该用户将生成初始密码 nowadmin2023~ ，并且在首次以 hpc-now 用户登录时强制要求修改；对于其他操作系统，无初始密码，您需要按照步骤 5 为该用户设置密码。
-  **创建工作目录** ，对于 Microsoft Windows，将创建 C:\programdata\hpc-now 作为关键目录；对于 GNU/Linux，关键目录位于 /usr/.hpc-now ；对于 macOS，关键数据目录位于 /Applications/.hpc-now 。

上述对操作系统的修改均不会无意或恶意破坏您的操作系统。同时， **您可以随时以管理员身份执行 `sudo YOUR_INSTALLER_PATH uninstall` 操作回滚上述对操作系统的改动** 。请注意，uninstall 操作可能会导致您对云上的集群失去管理权，请务必按照软件说明进行确认操作。

如有任何疑问，请您阅读源代码、或者联系我们进行技术沟通。

### 9. 如何卸载

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

### 10. 异常处理

云资源的使用和管理受到多种因素的影响，包括但不限于：

- 密钥对过期或失效
- 本地网络突然中断
- 账号购买的资源达到限额
- 当前区域无法购买您所需要的配置或数量
- 开机/关机失败（为了成本考虑，云资源的模板之中默认购买的是**停机不收费**机型，对于停机不收费的云主机，云厂商不保障再次开机时能够成功）
- 其他未知或不可抗的因素
- ……

HPC-NOW 平台在设计时，尽可能的考虑到上述异常，并作出一定的处理（详见代码的 cluster_operation.c 模块 和 cluter_init.c 模块），但是仍然无法保证完全不出差错。在极端情况下，如果发现 hpcopr 已经失去了管理您的集群的能力，为了避免产生意外的资源账单和成本，您**必须**在云平台提供的控制台上进行数据备份、资源监控和手动销毁。

假设您已经登录 AWS | 阿里云 | 腾讯云 的控制台。参考以下建议进行操作：

##### 10.1 数据备份：

您的部分关键数据位于共享文件存储 hpc_data 中。在 AWS 上对应的产品为 EFS，在腾讯云上对应的产品为 CFS， 在阿里云上对应的产品为文件存储 NAS。您可以另购云主机，将被损坏的集群之中的文件存储挂载至新的云主机上。建议您配置一个专用的对象存储桶（AWS - S3，阿里云 - OSS，腾讯 COS），将文件存储中的关键数据备份至该存储桶中。

您的另外部分关键数据可能位于集群所对应的专属存储桶中。每个集群都有一个专属的存储桶用于您做数据中转和持久存储，该桶与集群的生命周期相同。因此，您在手动销毁该桶之前，请务必确认您的关键数据已经下载或转储。

##### 10.2 资源销毁：

**非常重要**：我们**强烈不建议**您在集群中各个节点的系统盘（除了 /hpc_apps 和 /hpc_data 之外的任何目录）中存放任何关键数据！**请务必将所有的关键数据都放置在 /hpc_data 对应的共享存储中**。

因此，通常情况下，您对计算节点的销毁不会影响您的关键数据。在按照 9.1 步骤备份和转存完毕所有关键数据之后，您即可手动销毁该集群所针对的所有资源。此外，针对每个集群，HPC-NOW 都自动生成一个随机的唯一识别码，并将该识别码作为标签打在 VPC、云主机、对象存储、文件存储等关键资源上。您可以在控制台上非常直观的看到该集群所对应的资源。

- 请在创建完任一集群之后，使用 hpcopr usage 命令来查看每个集群对应的唯一识别码

请依次销毁如下资源：

1. 本集群所对应的所有节点，包括 nat 节点、database 节点、 master 节点、compute 节点
2. 本集群所对应的 VPC 及其内部的所有资源
   - 对于阿里云，建议您直接在控制台上打开 VPC，查看其中的所有资源，并依次删除 文件存储、文件存储对应的权限组、ACL、交换机、安全组
   - 对于腾讯云，建议您在控制台上尝试删除 VPC，并按照相应的提示删除文件存储、权限组、安全组等所有资源
   - 对于 AWS，建议您在控制台上尝试删除 VPC，并按照相应的提示删除 VPC 内的残留资源之后再确认删除 VPC
3. 本集群所对应的对象存储桶，名字为本集群的唯一识别码
4. 本集群所对应的对象存储专属用户：
    - 在 AWS 上，请打开 IAM 功能，查看并删除该无用用户
    - 在 阿里云，请打开 RAM 功能，查看并删除该无用用户
    - 在 腾讯云，请打开 CAM 功能，查看并删除该无用用户
5. 本集群所对应的对象存储桶访问策略：
    - 对于 AWS，您在上一个步骤中已经自动删除，无需本操作
    - 对于阿里云，请在 RAM 中打开 策略 - 自定义策略，查看并删除该策略
    - 对于腾讯云，请在 CAM 中打开 策略 - 自定义策略，查看并删除该策略
6. 本集群所对应的资源组：
    - 对于 AWS，请搜索产品 Resource Group，找到并删除该资源组标签
    - 对于阿里云，请搜索 资源管理，找到并删除该资源组的标签
    - 腾讯云无需本操作。

**以上操作非常重要！后续会进一步的细化图文说明，以确保您在云上不再残留本集群所对应的任何资源、后续不会再因本集群而产生任何费用。**

在完成以上步骤之后，请在本地运行 hpcopr remove 命令将该集群从本地 Registry 和文件目录之中彻底删除。

### 11. Bug与技术沟通

欢迎随时联系 info@hpc-now.com

### 12. 开发路线图

HPC-NOW 的开发将遵循以下总体路线（其中的优先级为暂定）：

资源接入层：接入 GCP 与 Azure （最高优先级）。接入 华为云 （一般优先级）

调度器层：接入 openPBS（一般优先级）

软件仓库优化：

- HPCMGR 操作逻辑优化（最高优先级）
- 提供PreBuild 仓库与源代码仓库（高优先级）
- 本地软件仓库功能（低优先级）

UI/UE：（高优先级）

- hpcopr、installer 的GUI客户端（一般优先级）
- 基于 Web 的在线平台（高优先级）

平台核心层：（以下皆为最高优先级）

- 命令输入错误之后模糊匹配（不会统一弹出完整的 help 文档，而是弹出最相近的命令选项）
- 自动生成 RDP 远程连接配置文件
- 平台普通用户生成密码之后同步至 operator 进行统一管理
- 日志清理功能（清理 error archive、logtrash、tf_prep.log.archive）
- 带参数导出系统日志、用量统计
- main 函数返回值整理以及最终错误的重定向
- 默认 Disable root ssh 和远程桌面连接