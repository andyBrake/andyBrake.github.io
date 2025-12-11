#!/bin/bash
# 德州扑克系统自动化测试脚本

echo "=================================="
echo "德州扑克系统测试脚本"
echo "=================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查编译环境
check_environment() {
    echo -e "${YELLOW}[1/5] 检查编译环境...${NC}"
    
    if ! command -v g++ &> /dev/null; then
        echo -e "${RED}✗ g++未安装${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ g++已安装: $(g++ --version | head -n 1)${NC}"
    
    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}✗ Python3未安装${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Python3已安装: $(python3 --version)${NC}"
    echo ""
}

# 清理旧文件
clean_old_files() {
    echo -e "${YELLOW}[2/5] 清理旧文件...${NC}"
    cd Server
    make clean > /dev/null 2>&1
    cd ..
    echo -e "${GREEN}✓ 清理完成${NC}"
    echo ""
}

# 编译服务器
compile_server() {
    echo -e "${YELLOW}[3/5] 编译服务器...${NC}"
    cd Server
    if make > compile.log 2>&1; then
        echo -e "${GREEN}✓ 编译成功${NC}"
        echo "  生成文件: server.out"
        
        # 检查警告数量
        warning_count=$(grep -c "warning:" compile.log)
        if [ $warning_count -gt 0 ]; then
            echo -e "  ${YELLOW}⚠ 编译警告: $warning_count 个${NC}"
        fi
    else
        echo -e "${RED}✗ 编译失败${NC}"
        cat compile.log
        exit 1
    fi
    cd ..
    echo ""
}

# 检查Python依赖
check_python_deps() {
    echo -e "${YELLOW}[4/5] 检查Python依赖...${NC}"
    
    python3 -c "import tkinter" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ tkinter已安装${NC}"
    else
        echo -e "${RED}✗ tkinter未安装${NC}"
        echo "  请安装: pip3 install tk"
        exit 1
    fi
    
    python3 -c "import socket" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ socket模块可用${NC}"
    fi
    
    python3 -c "import threading" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ threading模块可用${NC}"
    fi
    echo ""
}

# 运行提示
run_instructions() {
    echo -e "${YELLOW}[5/5] 系统就绪!${NC}"
    echo ""
    echo "=================================="
    echo "运行说明:"
    echo "=================================="
    echo ""
    echo "步骤1: 启动服务器"
    echo "  cd Server"
    echo "  ./server.out"
    echo ""
    echo "步骤2: 在新终端启动客户端"
    echo "  cd Client"
    echo "  python3 ClientMain.py"
    echo ""
    echo "步骤3: 在弹出的GUI窗口中进行游戏"
    echo "  - 点击Check/Call/Fold等按钮进行操作"
    echo "  - 观察公共牌和私人牌的变化"
    echo "  - 查看底池和其他玩家状态"
    echo ""
    echo "=================================="
    echo "快速测试命令:"
    echo "=================================="
    echo ""
    echo "# 终端1: 启动服务器"
    echo "cd Server && ./server.out"
    echo ""
    echo "# 终端2: 启动客户端"
    echo "cd Client && python3 ClientMain.py"
    echo ""
    echo "=================================="
    echo "测试文档:"
    echo "=================================="
    echo "  - 测试报告: TEST_REPORT.md"
    echo "  - 架构文档: ARCHITECTURE.md"
    echo ""
}

# 主流程
main() {
    check_environment
    clean_old_files
    compile_server
    check_python_deps
    run_instructions
}

# 执行
main
