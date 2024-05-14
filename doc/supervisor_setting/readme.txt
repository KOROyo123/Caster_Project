1、安装supervisor
sudo apt-get install supervisor
2、配置
Supervisor 的配置文件通常位于 /etc/supervisor/supervisord.conf。你可以编辑这个文件来配置 Supervisor 的行为
3、添加守护进程配置 
    通常是 /etc/supervisor/conf.d/ 目录下
    xxxx.conf：内容 

    [program:your_program]
    command=/path/to/your_program
    autostart=true
    autorestart=true
    stderr_logfile=/var/log/your_program.err.log
    stdout_logfile=/var/log/your_program.out.log

    将配置文件复制到/etc/supervisor/conf.d/目录下

4、重新加载 Supervisor 配置
    sudo supervisorctl reload

5、管理进程
    sudo supervisorctl start your_program
    sudo supervisorctl stop your_program
    sudo supervisorctl restart your_program
    sudo supervisorctl status




使用supervisorctl 命令报错unix:///var/run/supervisor.sock no such file时，往往是因为这个目录下的文件被清理了，只需要再新建一个空的文件然后修改一下权限即可：

sudo touch /var/run/supervisor.sock

sudo chmod 777 /var/run/supervisor.sock

sudo service supervisor restart

若此时

运行supervisorct 报如下错误：

supervisorctl unix:///var/run/supervisor.sock refused connection

解决办法：

unlink /var/run/supervisor.sock
unlink /tmp/supervisor.sock

这个错误的原因就是supervisor.sock这个文件会被系统自动删除或者其它原因不存在了，删除软连接就可以了。
supervisor.sock生成的位置可以去supervisor的配置文件中找到。
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
                        
原文链接：https://blog.csdn.net/weixin_41762173/article/details/88901970


