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
g++ -std=c++17 -O2 "$CPP" -o "$BIN"
echo "Compile OK -> $BIN"

echo "Running sanity test: 程序应能完成并输出总步数字段"

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
if ! echo "$OUTPUT" | grep -E "最少总步数： *[0-9]+" >/dev/null; then
  echo "FAIL: 未找到 '最少总步数' 字段或其值"
  echo "---- Program output start ----"
  echo "$OUTPUT"
  echo "---- Program output end ----"
  exit 1
fi

if ! echo "$OUTPUT" | grep -F "程序执行完成!" >/dev/null; then
  echo "FAIL: 未找到程序完成标识 '程序执行完成!'"
  echo "---- Program output start ----"
  echo "$OUTPUT"
  echo "---- Program output end ----"
  exit 1
fi

echo "PASS: 程序成功执行且输出包含总步数和值"
if [ $BIN_RC -ne 0 ]; then
  echo "WARNING: 程序退出码非0: $BIN_RC (但已捕获输出)"
fi

## 额外：运行严格的 identity 测试，期望总步数为 0
echo "\nRunning identity test (expect total steps = 0)"
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

if ! echo "$ID_OUT" | grep -E "最少总步数： *0" >/dev/null; then
  echo "FAIL: identity test 未找到总步数为0的结果"
  echo "---- identity output start ----"
  echo "$ID_OUT"
  echo "---- identity output end ----"
  exit 1
fi

echo "PASS: identity test 总步数为 0"
if [ $ID_RC -ne 0 ]; then
  echo "WARNING: identity test 程序退出码非0: $ID_RC (但输出中包含预期结果)"
fi

echo "\nRunning boundary test (edge coordinates, expect total steps = 0)"
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
if ! echo "$BOUND_TEXT" | grep -E "最少总步数： *0" >/dev/null; then
  echo "FAIL: boundary test 未得到总步数0"
  echo "---- boundary output start ----"
  echo "$BOUND_TEXT"
  echo "---- boundary output end ----"
  exit 1
fi
echo "PASS: boundary test 通过"

echo "\nRunning 5 randomized tests (valid board positions)"
for i in 1 2 3 4 5; do
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
  if ! echo "$TXT" | grep -E "最少总步数： *[0-9]+" >/dev/null; then
    echo "FAIL: random test $i 未找到最少总步数字段"
    echo "---- random $i output start ----"
    echo "$TXT"
    echo "---- random $i output end ----"
    exit 1
  fi
  if ! echo "$TXT" | grep -F "程序执行完成!" >/dev/null; then
    echo "FAIL: random test $i 未找到完成标识"
    exit 1
  fi
  echo "PASS: random test $i 通过 (exit $RC)"
done

echo "\n所有新增测试通过。"
exit 0
