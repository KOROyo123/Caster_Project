#!/bin/bash

# 要停止的程序名称
program_name="redis-server"

# 获取所有匹配的 PID
pids=$(pgrep -f "$program_name")

# 检查是否找到任何进程
if [ -z "$pids" ]; then
    echo "No processes found for $program_name"
    exit 0
fi

# 逐一停止所有找到的进程
for pid in $pids; do
    if kill "$pid" 2>/dev/null; then
        echo "Process $pid stopped successfully"
    else
        echo "Failed to stop process $pid or process is not running"
    fi
done
