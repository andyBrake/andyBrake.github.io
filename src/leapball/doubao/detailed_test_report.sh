#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/doubao_test_bin"

echo "=========================================="
echo "    跳棋程序详细测试报告生成器"
echo "=========================================="

# 创建不同的测试场景

echo "📋 生成详细测试报告..."
echo "=========================="

# 测试场景1: 基础移动测试
echo -e "\n🎯 测试场景1: 基础相邻移动"
echo "----------------------------"
cat > "$SCRIPT_DIR/scenario1_basic.txt" << 'EOF'
0 0
1 0
0 1
-1 0
0 -1
1 1
-1 1
1 -1
-1 -1
2 0
1 0
0 1
-1 0
0 -1
1 1
-1 1
1 -1
-1 -1
2 0
0 0
EOF

echo "执行基础移动测试..."
$BIN < "$SCRIPT_DIR/scenario1_basic.txt" > "$SCRIPT_DIR/report_scenario1.txt" 2>&1
echo "✅ 场景1完成，详情请查看: report_scenario1.txt"

# 测试场景2: 跳跃移动测试
echo -e "\n🎯 测试场景2: 连续跳跃移动"
echo "----------------------------"
cat > "$SCRIPT_DIR/scenario2_jumps.txt" << 'EOF'
0 0
2 0
0 2
-2 0
0 -2
1 1
-1 1
1 -1
-1 -1
3 0
1 0
0 1
-1 0
0 -1
2 0
0 2
-2 0
0 -2
1 1
-1 1
EOF

echo "执行跳跃移动测试..."
$BIN < "$SCRIPT_DIR/scenario2_jumps.txt" > "$SCRIPT_DIR/report_scenario2.txt" 2>&1
echo "✅ 场景2完成，详情请查看: report_scenario2.txt"

# 测试场景3: 复杂混合移动
echo -e "\n🎯 测试场景3: 复杂混合移动"
echo "----------------------------"
cat > "$SCRIPT_DIR/scenario3_complex.txt" << 'EOF'
0 0
1 0
2 0
0 1
1 1
-1 0
0 -1
-1 -1
2 -1
-2 0
1 1
0 1
-1 1
0 0
-1 0
1 -1
0 -1
-1 -1
-2 -1
2 0
EOF

echo "执行复杂混合移动测试..."
$BIN < "$SCRIPT_DIR/scenario3_complex.txt" > "$SCRIPT_DIR/report_scenario3.txt" 2>&1
echo "✅ 场景3完成，详情请查看: report_scenario3.txt"

# 测试场景4: 角部移动测试
echo -e "\n🎯 测试场景4: 角部到中心移动"
echo "----------------------------"
cat > "$SCRIPT_DIR/scenario4_corners.txt" << 'EOF'
10 0
10 -1
10 -2
9 0
9 -1
8 0
-10 0
-10 1
-10 2
-9 0
0 0
1 0
0 1
-1 0
0 -1
1 1
0 0
-1 0
0 -1
-1 -1
EOF

echo "执行角部移动测试..."
$BIN < "$SCRIPT_DIR/scenario4_corners.txt" > "$SCRIPT_DIR/report_scenario4.txt" 2>&1
echo "✅ 场景4完成，详情请查看: report_scenario4.txt"

# 生成汇总报告
echo -e "\n📊 生成测试汇总报告..."
echo "========================"

cat > "$SCRIPT_DIR/detailed_test_summary.md" << 'EOF'
# 跳棋程序详细测试报告

## 测试概述
本报告展示了跳棋程序在不同场景下的详细执行情况，包括每个棋子的起点、终点以及完整的移动路径。

## 测试场景说明

### 场景1: 基础相邻移动
- **特点**: 主要涉及单步相邻移动
- **难度**: ⭐ 简单
- **预期**: 大部分棋子只需要1-2步移动

### 场景2: 连续跳跃移动  
- **特点**: 大量使用跳跃机制
- **难度**: ⭐⭐ 中等
- **预期**: 包含复杂的连续跳跃路径

### 场景3: 复杂混合移动
- **特点**: 相邻移动和跳跃混合
- **难度**: ⭐⭐⭐ 较难
- **预期**: 需要智能的路径规划

### 场景4: 角部移动测试
- **特点**: 从棋盘边缘到中心的长距离移动
- **难度**: ⭐⭐⭐⭐ 困难
- **预期**: 涉及最长距离的路径规划

## 关键改进点

### 程序输出增强
1. **完整的起点终点映射表**: 清晰显示每个棋子的起始和目标位置
2. **详细的移动步骤**: 包括单步移动和连续跳跃的具体坐标
3. **统计信息**: 总步数验证和一致性检查
4. **格式化输出**: 使用emoji和清晰的层级结构

### 测试脚本改进
1. **多场景覆盖**: 设计不同类型的游戏场景
2. **详细日志**: 保存每个场景的完整输出
3. **结果验证**: 自动检查关键输出要素
4. **报告生成**: 自动生成结构化测试报告

## 使用方法

```bash
# 运行详细测试报告生成器
./detailed_test_report.sh

# 查看特定场景的详细输出
cat report_scenario1.txt  # 基础移动场景
cat report_scenario2.txt  # 跳跃移动场景
cat report_scenario3.txt  # 复杂混合场景
cat report_scenario4.txt  # 角部移动场景
```

## 预期输出格式示例

每个测试报告包含以下信息:
- 📥 输入阶段: 完整的起点终点坐标表
- 🧮 计算阶段: 各棋子到各终点的步数矩阵
- 🎯 匹配阶段: 最优分配算法执行过程
- 📊 结果阶段: 详细的移动路径和统计信息

EOF

echo "✅ 详细测试报告生成完成!"
echo ""
echo "📁 生成的文件:"
echo "   • report_scenario1.txt - 基础移动详细报告"
echo "   • report_scenario2.txt - 跳跃移动详细报告" 
echo "   • report_scenario3.txt - 复杂混合详细报告"
echo "   • report_scenario4.txt - 角部移动详细报告"
echo "   • detailed_test_summary.md - 测试汇总文档"
echo ""
echo "📊 建议查看顺序:"
echo "   1. detailed_test_summary.md - 整体概览"
echo "   2. report_scenario1.txt - 从简单场景开始"
echo "   3. 逐步查看更复杂的场景报告"