#!/bin/bash

# 跳棋程序测试脚本
# 用于编译和运行 doubao-version.cpp

echo "=================================="
echo "  跳棋程序测试脚本"
echo "=================================="

# 设置编译器和输出文件名
COMPILER="g++"
SOURCE_FILE="doubao-version.cpp"
OUTPUT_FILE="doubao_game.out"

# 编译程序
echo "正在编译 $SOURCE_FILE..."
$COMPILER -std=c++11 -O2 -o $OUTPUT_FILE $SOURCE_FILE

# 检查编译是否成功
if [ $? -eq 0 ]; then
    echo "编译成功！生成可执行文件: $OUTPUT_FILE"
    echo ""
    
    # 询问是否立即运行
    read -p "是否现在运行程序？(y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "运行程序..."
        echo "=================================="
        ./$OUTPUT_FILE
        echo "=================================="
        echo "程序执行完毕"
    fi
else
    echo "编译失败！请检查代码是否有语法错误。"
    exit 1
fi

echo ""
echo "脚本执行完成"