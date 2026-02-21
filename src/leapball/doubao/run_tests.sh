#!/bin/bash

# 增强版跳棋程序测试脚本
# 支持多种测试模式

set -e  # 遇到错误时退出

echo "=================================="
echo "  跳棋程序自动化测试脚本"
echo "=================================="

# 配置变量
COMPILER="g++"
SOURCE_FILE="doubao-version.cpp"
OUTPUT_FILE="doubao_game.out"
TEST_INPUT="test_input.txt"

# 编译函数
compile_program() {
    echo "正在编译 $SOURCE_FILE..."
    $COMPILER -std=c++11 -O2 -Wall -Wextra -o $OUTPUT_FILE $SOURCE_FILE
    
    if [ $? -eq 0 ]; then
        echo "✅ 编译成功！"
        echo "生成文件: $OUTPUT_FILE ($(stat -f%z $OUTPUT_FILE 2>/dev/null || stat -c%s $OUTPUT_FILE) 字节)"
        echo ""
    else
        echo "❌ 编译失败！"
        exit 1
    fi
}

# 基本功能测试
basic_test() {
    echo "🧪 基本功能测试"
    echo "=================================="
    
    if [ ! -f "$TEST_INPUT" ]; then
        echo "❌ 测试输入文件 $TEST_INPUT 不存在"
        return 1
    fi
    
    echo "使用测试输入文件: $TEST_INPUT"
    echo "开始运行程序..."
    echo ""
    
    # 运行程序并传入测试输入
    time ./$OUTPUT_FILE < $TEST_INPUT
    
    echo ""
    echo "✅ 基本测试完成"
}

# 性能测试
performance_test() {
    echo "⚡ 性能测试"
    echo "=================================="
    
    echo "测量程序执行时间..."
    
    # 多次运行取平均值
    total_time=0
    runs=3
    
    for ((i=1; i<=runs; i++)); do
        echo "第 $i 次运行..."
        start_time=$(date +%s.%N)
        ./$OUTPUT_FILE < $TEST_INPUT > /dev/null 2>&1
        end_time=$(date +%s.%N)
        
        # 计算执行时间（秒）
        duration=$(echo "$end_time - $start_time" | bc -l)
        printf "执行时间: %.3f 秒\n" $duration
        
        total_time=$(echo "$total_time + $duration" | bc -l)
    done
    
    avg_time=$(echo "scale=3; $total_time / $runs" | bc -l)
    echo ""
    echo "📈 平均执行时间: ${avg_time} 秒 (${runs} 次运行)"
}

# 内存使用测试
memory_test() {
    echo "💾 内存使用测试"
    echo "=================================="
    
    if command -v valgrind &> /dev/null; then
        echo "使用 Valgrind 检测内存使用..."
        valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
                 ./$OUTPUT_FILE < $TEST_INPUT 2>&1 | grep -E "(definitely lost|indirectly lost|ERROR SUMMARY)"
    else
        echo "⚠️  Valgrind 未安装，跳过内存测试"
        echo "在 macOS 上可以使用: brew install valgrind"
    fi
}

# 显示帮助信息
show_help() {
    echo "使用方法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  all          运行所有测试 (默认)"
    echo "  compile      仅编译程序"
    echo "  basic        基本功能测试"
    echo "  perf         性能测试"
    echo "  memory       内存使用测试"
    echo "  clean        清理编译产物"
    echo "  help         显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0              # 运行所有测试"
    echo "  $0 basic        # 仅运行基本测试"
    echo "  $0 perf memory  # 运行性能和内存测试"
}

# 清理函数
clean_build() {
    echo "🧹 清理编译产物..."
    rm -f $OUTPUT_FILE
    echo "✅ 清理完成"
}

# 主逻辑
case "${1:-all}" in
    "compile")
        compile_program
        ;;
    "basic")
        compile_program
        basic_test
        ;;
    "perf")
        compile_program
        performance_test
        ;;
    "memory")
        compile_program
        memory_test
        ;;
    "clean")
        clean_build
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    "all"|*)
        compile_program
        basic_test
        performance_test
        memory_test
        echo ""
        echo "🎉 所有测试完成！"
        ;;
esac