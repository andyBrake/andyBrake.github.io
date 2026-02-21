#!/bin/bash

# 增强版测试脚本 - 包含超时检测和调试功能

set -e

echo "=================================="
echo "  跳棋程序增强测试脚本"
echo "=================================="

# 配置变量
COMPILER="g++"
SOURCE_FILE="doubao-version.cpp"
OUTPUT_FILE="doubao_game.out"
TIMEOUT_DURATION=10  # 10秒超时

# 编译函数
compile_program() {
    echo "正在编译 $SOURCE_FILE..."
    $COMPILER -std=c++11 -O2 -Wall -Wextra -g -o $OUTPUT_FILE $SOURCE_FILE
    
    if [ $? -eq 0 ]; then
        echo "✅ 编译成功！"
    else
        echo "❌ 编译失败！"
        exit 1
    fi
}

# 带超时的运行函数
run_with_timeout() {
    local input_file=$1
    local description=$2
    
    echo "🧪 测试: $description"
    echo "输入文件: $input_file"
    echo "超时设置: ${TIMEOUT_DURATION}秒"
    echo "----------------------------------"
    
    if [ ! -f "$input_file" ]; then
        echo "❌ 输入文件不存在: $input_file"
        return 1
    fi
    
    # 检查文件内容
    echo "文件内容预览:"
    head -20 "$input_file"
    echo "----------------------------------"
    
    # 使用 timeout 命令运行（Linux）或 gtimeout（macOS with Homebrew）
    if command -v timeout &> /dev/null; then
        timeout_cmd="timeout"
    elif command -v gtimeout &> /dev/null; then
        timeout_cmd="gtimeout"
    else
        echo "⚠️  timeout 命令不可用，将无超时保护运行"
        timeout_cmd=""
    fi
    
    # 运行程序
    start_time=$(date +%s)
    
    if [ -n "$timeout_cmd" ]; then
        if $timeout_cmd $TIMEOUT_DURATION ./$OUTPUT_FILE < "$input_file"; then
            end_time=$(date +%s)
            duration=$((end_time - start_time))
            echo "✅ 程序正常结束，耗时: ${duration}秒"
        else
            exit_code=$?
            if [ $exit_code -eq 124 ]; then
                echo "⏰ 程序超时（>${TIMEOUT_DURATION}秒），可能存在死循环"
            else
                echo "❌ 程序异常退出，退出码: $exit_code"
            fi
            return 1
        fi
    else
        # 无超时保护的运行
        if ./$OUTPUT_FILE < "$input_file"; then
            end_time=$(date +%s)
            duration=$((end_time - start_time))
            echo "✅ 程序正常结束，耗时: ${duration}秒"
        else
            exit_code=$?
            echo "❌ 程序异常退出，退出码: $exit_code"
            return 1
        fi
    fi
    
    echo ""
}

# 主测试流程
main() {
    compile_program
    
    echo ""
    echo "🚀 开始测试..."
    echo ""
    
    # 基本功能测试
    run_with_timeout "test_input.txt" "基本功能测试"
    
    echo "🎉 测试完成！"
}

# 运行主测试
main "$@"