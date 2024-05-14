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

4、重新加载 Supervisor 配置
    sudo supervisorctl reload

5、管理进程
    sudo supervisorctl start your_program
    sudo supervisorctl stop your_program
    sudo supervisorctl restart your_program
    sudo supervisorctl status



