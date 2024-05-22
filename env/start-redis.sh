#!/bin/bash

# 获取脚本文件所在的目录
script_dir=$(dirname "$(realpath "$0")")

# 切换到脚本文件所在的目录
cd "$script_dir" || { echo "Failed to change directory to $script_dir"; exit 1; }

# 执行当前目录下的程序
./redis-server redis.conf
