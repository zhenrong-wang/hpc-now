### 1. 项目介绍
通过本项目构建的两个组件——hpcopr和now-crypto，实现云超算平台的核心功能。
- 其中 hpcopr 为核心程序，驱动 Terraform 管理云资源，相关的tf模板文件库尚未迁移到本仓库，将于近期完成迁移。
- now-crypto 为辅助程序，主要作用是文件加密和解密，以确保敏感信息不以明文形式存放。

### 2. 构建环境需求：
仅需要 C 语言编译器即可，如下所示

- Microsoft Windows：安装最新版 mingw，具体请参考教程：https://blog.csdn.net/LawssssCat/article/details/103407137
- GNU/Linux：安装 gcc，版本一般为 8.x.x 及以上，请从自带的软件仓库中安装，如 yum 或者 apt，示例命令：sudo yum -y install gcc
- macOS：安装 clang，版本一般为 13.x.x 及以上，在 Terminal 输入 clang 之后，如果本机没有安装clang，macOS会询问是否安装，可根据提示进行自动安装

### 3. 如何构建：
从本仓库下载源代码至本地目录，切换至代码所在的本地目录之后
- Microsoft Windows用户，请运行：gcc source_code_file.c -Wall
- GNU/Linux用户，请运行：gcc source_code_file.c -Wall -lm
- macOS用户，请运行：clang source_code_file.c -Wall

分别从 hpcopr_OS.c 和 now_crypto.c 构建出两个可执行文件之后，请将基于 hpcopr_windows.c | hpcopr_darwin.c | hpcopr_linux.c 构建形成的可执行文件命名为 hpcopr；将 now_crypto.c 构建形成的可执行文件命名为 now-crypto

### 4. 如何使用：
请参阅部署手册：https://www.hpc-now.com/deploy 。请注意：如果您直接运行 hpcopr 本地安装，将会下载云上已经编译好的 now-crypto 至以下本地目录：

- Windows： C:\programdata\hpc-now\bin\now-crypto.exe
- GNU/Linux：/usr/.hpc-now/.bin/now-crypto
- macOS：/Applications/.hpc-now/.bin/now-crypto.exe

您可以用自己构建的 now-crypto 替换掉安装时下载的文件，请注意文件名保持一致即可。此外，在 GNU/Linux 和 macOS 下，注意要赋予可执行权限，示例命令： 
- sudo chmod +x now-crypto
### 5. bug提交与技术沟通：
欢迎随时联系 info@hpc-now.com
或者加入我们的技术交流群：
![输入图片说明](Group_QR_Code.jpg)