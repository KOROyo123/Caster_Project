#!/bin/bash

# 获取脚本所在路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 指定可执行程序的名字
PROGRAM_NAME="your_program_name"  # 修改为你的可执行程序名称

# 检查可执行程序是否存在
if [ ! -x "$SCRIPT_DIR/$PROGRAM_NAME" ]; then
    echo "Error: Executable '$PROGRAM_NAME' not found or not executable in $SCRIPT_DIR"
    exit 1
fi

# 定义服务单元文件路径
SERVICE_FILE="/etc/systemd/system/${PROGRAM_NAME}.service"

# 生成服务单元文件内容
cat <<EOF > "/tmp/${PROGRAM_NAME}.service"
[Unit]
Description=$PROGRAM_NAME
After=network.target

[Service]
Type=simple
ExecStart=$SCRIPT_DIR/$PROGRAM_NAME
Restart=always
RestartSec=3
User=root
Group=root

[Install]
WantedBy=multi-user.target
EOF

# 移动服务单元文件到指定位置
sudo mv "/tmp/${PROGRAM_NAME}.service" "$SERVICE_FILE"
echo "Service file created and moved to $SERVICE_FILE"

# 重新加载systemd配置
sudo systemctl daemon-reload

# 启动并启用服务
sudo systemctl start "${PROGRAM_NAME}.service"
sudo systemctl enable "${PROGRAM_NAME}.service"

echo "Service ${PROGRAM_NAME} started and enabled."
#!/bin/bash

# 获取脚本所在路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 指定可执行程序的名字
PROGRAM_NAME="your_program_name"  # 修改为你的可执行程序名称

# 检查可执行程序是否存在
if [ ! -x "$SCRIPT_DIR/$PROGRAM_NAME" ]; then
    echo "Error: Executable '$PROGRAM_NAME' not found or not executable in $SCRIPT_DIR"
    exit 1
fi

# 定义服务单元文件路径
SERVICE_FILE="/etc/systemd/system/${PROGRAM_NAME}.service"

# 生成服务单元文件内容
cat <<EOF > "/tmp/${PROGRAM_NAME}.service"
[Unit]
Description=$PROGRAM_NAME
After=network.target

[Service]
Type=simple
ExecStart=$SCRIPT_DIR/$PROGRAM_NAME
Restart=always
RestartSec=3
User=root
Group=root

[Install]
WantedBy=multi-user.target
EOF

# 移动服务单元文件到指定位置
sudo mv "/tmp/${PROGRAM_NAME}.service" "$SERVICE_FILE"
echo "Service file created and moved to $SERVICE_FILE"

# 重新加载systemd配置
sudo systemctl daemon-reload

# 启动并启用服务
# sudo systemctl start "${PROGRAM_NAME}.service"
# sudo systemctl enable "${PROGRAM_NAME}.service"

# echo "Service ${PROGRAM_NAME} started and enabled."
