# 跳棋程序 Makefile

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra
DEBUG_FLAGS = -g -DDEBUG

# 目标文件
TARGET = doubao_game.out
SOURCE = doubao-version.cpp

# 测试相关
TEST_SCRIPT = run_tests.sh
TEST_INPUT = test_input.txt

.PHONY: all clean test debug help

# 默认目标
all: $(TARGET)

# 编译主程序
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ $<

# 调试版本
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# 运行测试
test: $(TARGET)
	./$(TEST_SCRIPT)

# 快速测试（使用预设输入）
quick-test: $(TARGET)
	./$(TARGET) < $(TEST_INPUT)

# 清理编译产物
clean:
	rm -f $(TARGET)
	@echo "清理完成"

# 显示帮助信息
help:
	@echo "可用目标:"
	@echo "  all        - 编译程序 (默认)"
	@echo "  debug      - 编译调试版本"
	@echo "  test       - 运行完整测试套件"
	@echo "  quick-test - 快速测试 (使用预设输入)"
	@echo "  clean      - 清理编译产物"
	@echo "  help       - 显示此帮助信息"
	@echo ""
	@echo "使用示例:"
	@echo "  make          # 编译程序"
	@echo "  make test     # 运行测试"
	@echo "  make clean    # 清理文件"

# 文件依赖关系
$(SOURCE):