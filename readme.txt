工程的结构


app 可执行文件

build 生成文件夹

lib 静态库的源文件


rely 依赖的第三方库（需要单独编译生成）
    对于每个依赖库，依次执行 mkdir build 、 cd build 、cmake ..  、 make    、make install 【make install（可以不执行）】    

third 依赖的第三方源码（不需要单独生成库，仅包含即可）



其他注意事项

 CMake 进行项目配置时，找不到 OpenSSL 库，可以下载一个开发套件
sudo apt-get update
sudo apt-get install libssl-dev