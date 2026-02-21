#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP="$SCRIPT_DIR/doubao-version.cpp"
BIN="$SCRIPT_DIR/doubao_test_bin"

echo "== Automated test for doubao-version =="

if ! command -v g++ >/dev/null 2>&1; then
  echo "ERROR: g++ not found in PATH"
  exit 2
fi

echo "Compiling $CPP..."
COMPILE_LOG="$SCRIPT_DIR/compile.log"
if ! g++ -std=c++17 -O2 "$CPP" -o "$BIN" >"$COMPILE_LOG" 2>&1; then
  echo "ERROR: 编译失败，详情： $COMPILE_LOG"
  cat "$COMPILE_LOG"
  exit 2
fi
echo "Compile OK -> $BIN"

# 测试统计
tests_total=0
tests_pass=0
tests_fail=0

echo "=== 测试1: sanity — 程序应能完成并输出最少总步数字段 ==="
tests_total=$((tests_total+1))

SANITY_IN="$SCRIPT_DIR/sanity_input.txt"
cat >"$SANITY_IN" <<'EOF'
0 0
1 0
0 1
-1 1
-1 0
0 -1
1 -1
2 0
0 2
-2 0
0 0
1 0
0 1
-1 1
-1 0
0 -1
1 -1
2 0
0 2
-2 0
EOF

tmp_out="$SCRIPT_DIR/automated_test_output.txt"
# 使用临时文件捕获输出，避免子进程返回非0触发 set -e
$BIN <"$SANITY_IN" >"$tmp_out" 2>&1
BIN_RC=$?
OUTPUT="$(cat "$tmp_out")"

# 检查程序是否打印了“最少总步数”和完成标识
sanity_ok=1
if ! echo "$OUTPUT" | grep -E "最少总步数： *[0-9]+" >/dev/null; then
  echo "FAIL: 未找到 '最少总步数' 字段或其值"
  echo "---- Program output start ----"
  echo "$OUTPUT"
  echo "---- Program output end ----"
  sanity_ok=0
fi

if ! echo "$OUTPUT" | grep -F "程序执行完成!" >/dev/null; then
  echo "FAIL: 未找到程序完成标识 '程序执行完成!'"
  sanity_ok=0
fi

if [ $sanity_ok -eq 1 ]; then
  echo "PASS: 程序成功执行且输出包含总步数和值"
  tests_pass=$((tests_pass+1))
else
  tests_fail=$((tests_fail+1))
fi
if [ $BIN_RC -ne 0 ]; then
  echo "WARNING: 程序退出码非0: $BIN_RC (但已捕获输出)"
fi

## 额外：运行严格的 identity 测试，期望总步数为 0
echo "\n=== 测试2: identity — 起点等于终点，期望总步数 = 0 ==="
tests_total=$((tests_total+1))
ID_IN="$SCRIPT_DIR/identity_input.txt"
cat >"$ID_IN" <<'EOF'
0 0
1 0
0 1
-1 1
-1 0
0 -1
1 -1
2 0
0 2
-2 0
0 0
1 0
0 1
-1 1
-1 0
0 -1
1 -1
2 0
0 2
-2 0
EOF

ID_OUT_TMP="$SCRIPT_DIR/automated_test_id_output.txt"
$BIN <"$ID_IN" >"$ID_OUT_TMP" 2>&1
ID_RC=$?
ID_OUT="$(cat "$ID_OUT_TMP")"

echo "$ID_OUT" > "$SCRIPT_DIR/automated_test_id_output.txt"

identity_ok=1
if ! echo "$ID_OUT" | grep -E "最少总步数： *0" >/dev/null; then
  echo "FAIL: identity test 未找到总步数为0的结果"
  echo "---- identity output start ----"
  echo "$ID_OUT"
  echo "---- identity output end ----"
  identity_ok=0
fi

if [ $identity_ok -eq 1 ]; then
  echo "PASS: identity test 总步数为 0"
  tests_pass=$((tests_pass+1))
else
  tests_fail=$((tests_fail+1))
fi
if [ $ID_RC -ne 0 ]; then
  echo "WARNING: identity test 程序退出码非0: $ID_RC (但输出中包含/可能包含预期结果)"
fi

echo "\n=== 测试3: boundary — 边界坐标（角部）期望总步数 = 0 ==="
tests_total=$((tests_total+1))
BOUND_IN="$SCRIPT_DIR/boundary_input.txt"
cat >"$BOUND_IN" <<'EOF'
10 0
9 -1
8 -2
7 -3
6 -4
5 -5
4 -4
3 -3
2 -2
1 -1
10 0
9 -1
8 -2
7 -3
6 -4
5 -5
4 -4
3 -3
2 -2
1 -1
EOF

BOUND_OUT="$SCRIPT_DIR/automated_test_boundary_output.txt"
$BIN <"$BOUND_IN" >"$BOUND_OUT" 2>&1
BOUND_RC=$?
BOUND_TEXT="$(cat "$BOUND_OUT")"
echo "$BOUND_TEXT" > "$BOUND_OUT"
boundary_ok=1
if ! echo "$BOUND_TEXT" | grep -E "最少总步数： *0" >/dev/null; then
  echo "FAIL: boundary test 未得到总步数0"
  echo "---- boundary output start ----"
  echo "$BOUND_TEXT"
  echo "---- boundary output end ----"
  boundary_ok=0
fi
if [ $boundary_ok -eq 1 ]; then
  echo "PASS: boundary test 通过"
  tests_pass=$((tests_pass+1))
else
  tests_fail=$((tests_fail+1))
fi

echo "\n=== 测试4: 随机用例 — 5 个不同合法布局，每个 20 个坐标 ==="
for i in 1 2 3 4 5; do
  tests_total=$((tests_total+1))
  echo "--- 随机测试 $i: 生成20个合法坐标并运行程序 ---"
  RAND_IN="$SCRIPT_DIR/random_input_${i}.txt"
  python3 - <<PY > "$RAND_IN"
import random
def valid(q,r):
    s = -q-r
    return abs(q)<=10 and abs(r)<=10 and abs(s)<=10 and (abs(q)+abs(r)+abs(s))//2 <= 10
vals = set()
while len(vals) < 20:
    q = random.randint(-10,10)
    r = random.randint(-10,10)
    if valid(q,r):
        vals.add((q,r))
vals = list(vals)
for x,y in vals:
    print(x, y)
PY

  RAND_OUT="$SCRIPT_DIR/automated_test_random_${i}_output.txt"
  $BIN <"$RAND_IN" >"$RAND_OUT" 2>&1
  RC=$?
  TXT="$(cat "$RAND_OUT")"
  random_ok=1
  if ! echo "$TXT" | grep -E "最少总步数： *[0-9]+" >/dev/null; then
    echo "FAIL: random test $i 未找到最少总步数字段"
    echo "---- random $i output start ----"
    echo "$TXT"
    echo "---- random $i output end ----"
    random_ok=0
  fi
  if ! echo "$TXT" | grep -F "程序执行完成!" >/dev/null; then
    echo "FAIL: random test $i 未找到完成标识"
    random_ok=0
  fi
  if [ $random_ok -eq 1 ]; then
    echo "PASS: random test $i 通过 (exit $RC)"
    tests_pass=$((tests_pass+1))
  else
    tests_fail=$((tests_fail+1))
  fi
done

echo "\n测试汇总："
echo "总用例: $tests_total，已通过: $tests_pass，失败: $tests_fail"
if [ $tests_fail -eq 0 ]; then
  echo "全部测试通过 ✅"
  exit 0
else
  echo "部分测试失败 ❌"
  exit 1
fi
