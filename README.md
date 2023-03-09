# hpc-now-service
1. 项目介绍：通过本项目构建的两个组件——hpcopr和now-crypto，实现云超算平台的核心功能。
2. 构建环境需求：

- Windows：安装 mingw
- GNU/Linux：安装 gcc
- macOS：安装 clang

3. 如何构建：
    从本仓库下载源代码至本地目录，切换至本地目录之后
- Windows用户，请运行：gcc source_code_file.c -Wall
- GNU/Linux用户，请运行：gcc source_code_file.c -Wall -lm
- macOS用户，请运行：clang source_code_file.c -Wall
    即可构建出两个可执行文件，请将基于 hpcopr_windows.c | hpcopr_darwin.c | hpcopr_linux.c 构建形成的可执行文件命名为 hpcopr；将 now_crypto.c 构建形成的可执行文件命名为 now-crypto
4. 如何使用：
    请参阅部署手册：https://www.hpc-now.com/deploy
    请注意：如果您直接运行 hpcopr 本地安装，将会下载云上已经编译好的 now-crypto 至以下本地目录：

- Windows： C:\programdata\hpc-now\bin\now-crypto.exe
- GNU/Linux：/usr/.hpc-now/.bin/now-crypto
- macOS：/Applications/.hpc-now/.bin/now-crypto.exe

    您可以用自己构建的 now-crypto 替换掉安装时下载的文件，请注意文件名保持一致即可。
5. bug提交与技术沟通：
    欢迎随时联系 info@hpc-now.com
    或者加入我们的技术交流群：
![输入图片说明](Group_QR_Code.jpg)