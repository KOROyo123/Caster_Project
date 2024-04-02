工程的结构

app 可执行程序源文件

build 生成文件夹

dev 功能验证和工具

lib 静态库的源文件

rely 依赖的第三方库 

third 依赖的第三方源码（不需要单独生成库，仅包含即可）


其他注意事项

CMake 进行项目配置时，找不到 OpenSSL 库，可以下载一个开发套件
sudo apt-get update
sudo apt-get install libssl-dev
