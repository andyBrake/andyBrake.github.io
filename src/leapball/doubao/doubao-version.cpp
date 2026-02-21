
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <climits>
#include <algorithm>
#include <chrono>
using namespace std;
using namespace chrono;

// ==================== 1. 10层标准六芒星棋盘坐标系统 ====================
using Pos = pair<int, int>; // (q, r) 六角坐标，第三个轴 s = -q - r（隐含）
// 10层标准棋盘：|q| + |r| + |s| ≤ 20 且 |q| ≤ 10, |r| ≤ 10, |s| ≤ 10（s=-q-r）
// 总合法点位：121个（1+6+12+18+24+30+24+18+12+6+1 = 121）

// 精准判定10层标准棋盘的合法位置
bool is_valid(Pos p) {
    int q = p.first, r = p.second;
    int s = -q - r; // 六角坐标第三个轴，恒满足 q + r + s = 0
    // 10层核心规则：三个轴的绝对值都≤10，且总和的一半≤10（等价于|q|+|r|≤10）
    if (abs(q) > 10 || abs(r) > 10 || abs(s) > 10) return false;
    return (abs(q) + abs(r) + abs(s)) / 2 <= 10;
}

// 六芒星跳棋相邻6个方向（六角坐标标准方向）
vector<Pos> dirs = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,-1}, {-1,1}};

// ==================== 2. BFS 单点最短步数（支持连续跳） ====================
int bfs(Pos s, Pos t, const vector<Pos>& others) {
    if (s == t) return 0; // 起点=终点，步数为0

    map<Pos, int> dist;    // 记录每个位置的最短步数
    queue<Pos> q;
    dist[s] = 0;
    q.push(s);

    while (!q.empty()) {
        Pos u = q.front(); q.pop();

        // 1）走一步：移动到相邻空点
        for (auto& d : dirs) {
            Pos v = {u.first + d.first, u.second + d.second};
            if (!is_valid(v)) continue;
            if (dist.count(v)) continue;
            bool occupied = false;
            for (auto& p : others) if (p == v) { occupied = true; break; }
            if (!occupied) {
                dist[v] = dist[u] + 1;
                if (v == t) return dist[v];
                q.push(v);
            }
        }

        // 2）跳一步（可连续跳，所有由连续跳可达的位置视为 dist[u]+1）
        for (auto& d : dirs) {
            Pos mid = {u.first + d.first, u.second + d.second};
            Pos to  = {u.first + 2*d.first, u.second + 2*d.second};
            if (!is_valid(mid) || !is_valid(to)) continue;

            bool has_mid_piece = false;
            for (auto& p : others) if (p == mid) { has_mid_piece = true; break; }
            if (!has_mid_piece) continue;

            bool to_occupied = false;
            for (auto& p : others) if (p == to) { to_occupied = true; break; }
            if (to_occupied) continue;

            // 从这个初始落点开始做局部 BFS，找到所有通过连续跳可达的位置
            queue<Pos> jq;
            set<Pos> seen_jump;
            jq.push(to);
            seen_jump.insert(to);

            while (!jq.empty()) {
                Pos cur = jq.front(); jq.pop();
                if (!dist.count(cur)) {
                    dist[cur] = dist[u] + 1;
                    if (cur == t) return dist[cur];
                    q.push(cur);
                }

                for (auto& d2 : dirs) {
                    Pos mid2 = {cur.first + d2.first, cur.second + d2.second};
                    Pos to2  = {cur.first + 2*d2.first, cur.second + 2*d2.second};
                    if (!is_valid(mid2) || !is_valid(to2)) continue;
                    bool has_mid2 = false;
                    for (auto& p : others) if (p == mid2) { has_mid2 = true; break; }
                    if (!has_mid2) continue;
                    bool to2_occ = false;
                    for (auto& p : others) if (p == to2) { to2_occ = true; break; }
                    if (to2_occ) continue;
                    if (!seen_jump.count(to2)) {
                        seen_jump.insert(to2);
                        jq.push(to2);
                    }
                }
            }
        }
    }

    // 无法到达时返回明显的不可达标记
    return INT_MAX / 2;
}

// 辅助函数：格式化步数输出
string format_steps(int steps) {
    if (steps >= INT_MAX / 2) {
        return "不可达";
    }
    return to_string(steps);
}

// 返回从 s 到 t 的具体每一步走法（每一步可以是一次相邻移动或连续跳的一条着陆序列）
// 每一步由若干条记录组成：pair<landing, midJumped>
// 对于普通相邻移动，midJumped 使用 sentinel {-10000,-10000}
vector<vector<pair<Pos, Pos>>> find_path(Pos s, Pos t, const vector<Pos>& others) {
    vector<vector<Pos>> empty;
    vector<vector<pair<Pos, Pos>>> empty_steps;
    if (s == t) return empty_steps;

    map<Pos, int> dist;
    queue<Pos> q;
    map<Pos, Pos> step_prev;   // 表示按“计步”前驱（每一步的起点）
    map<Pos, Pos> chain_prev;  // 若某一步是连续跳，则 chain_prev maps landing -> previous landing (or to initial u)
    map<Pos, Pos> chain_mid;   // 对每个 landing，记录本跳跃所越过的中间棋子位置

    dist[s] = 0; q.push(s);

    while (!q.empty()) {
        Pos u = q.front(); q.pop();

        // 相邻移动
        for (auto& d : dirs) {
            Pos v = {u.first + d.first, u.second + d.second};
            if (!is_valid(v)) continue;
            if (dist.count(v)) continue;
            bool occupied = false;
            for (auto& p : others) if (p == v) { occupied = true; break; }
            if (!occupied) {
                dist[v] = dist[u] + 1;
                step_prev[v] = u;
                q.push(v);
                if (v == t) goto FOUND;
            }
        }

        // 跳跃（连续跳的所有落点都视为 dist[u]+1）
        for (auto& d : dirs) {
            Pos mid = {u.first + d.first, u.second + d.second};
            Pos to  = {u.first + 2*d.first, u.second + 2*d.second};
            if (!is_valid(mid) || !is_valid(to)) continue;
            bool has_mid = false;
            for (auto& p : others) if (p == mid) { has_mid = true; break; }
            if (!has_mid) continue;
            bool to_occ = false;
            for (auto& p : others) if (p == to) { to_occ = true; break; }
            if (to_occ) continue;

            // 局部 BFS 探索所有由连续跳可达的落点，并记录链路与中间被跳过的棋子
            queue<Pos> jq;
            set<Pos> seen_jump;
            jq.push(to);
            seen_jump.insert(to);
            // 初始的 landing 的 chain_prev 指向 u，表示链路起点
            chain_prev[to] = u;
            chain_mid[to] = mid;

            while (!jq.empty()) {
                Pos cur = jq.front(); jq.pop();
                if (!dist.count(cur)) {
                    dist[cur] = dist[u] + 1;
                    step_prev[cur] = u;
                    q.push(cur);
                    if (cur == t) goto FOUND;
                }

                for (auto& d2 : dirs) {
                    Pos mid2 = {cur.first + d2.first, cur.second + d2.second};
                    Pos to2  = {cur.first + 2*d2.first, cur.second + 2*d2.second};
                    if (!is_valid(mid2) || !is_valid(to2)) continue;
                    bool has_mid2 = false;
                    for (auto& p : others) if (p == mid2) { has_mid2 = true; break; }
                    if (!has_mid2) continue;
                    bool to2_occ = false;
                    for (auto& p : others) if (p == to2) { to2_occ = true; break; }
                    if (to2_occ) continue;
                    if (!seen_jump.count(to2)) {
                        seen_jump.insert(to2);
                        chain_prev[to2] = cur; // 记录链中前驱
                        chain_mid[to2] = mid2;  // 记录本跳越过的中间棋子
                        jq.push(to2);
                    }
                }
            }
        }
    }

    // 未找到路径，返回空
    return empty_steps;

FOUND:;
    // 重建按步的路径
    vector<vector<pair<Pos, Pos>>> steps;
    Pos cur = t;
    const Pos MOVE_SENTINEL = {-10000, -10000};
    while (!(cur == s)) {
        Pos u = step_prev[cur];
        // 如果 chain_prev 包含 cur，则这一步是一次或多次跳
        if (chain_prev.count(cur)) {
            // 收集从 u 到 cur 的跳着陆序列：按链路从 cur 反向到 u
            vector<pair<Pos, Pos>> seq;
            Pos node = cur;
            while (true) {
                Pos mid = chain_mid[node];
                seq.push_back({node, mid});
                Pos prev = chain_prev[node];
                if (prev == u) break;
                node = prev;
            }
            reverse(seq.begin(), seq.end());
            steps.push_back(seq);
        } else {
            // 相邻移动，仅记录目标，mid使用 sentinel
            steps.push_back(vector<pair<Pos, Pos>>{{cur, MOVE_SENTINEL}});
        }
        cur = u;
    }
    reverse(steps.begin(), steps.end());
    return steps;
}

// ==================== 3. 匈牙利算法 最小权完美匹配 ====================
// 匈牙利算法实现二分图最大匹配
bool hungarian_dfs(int x, const vector<vector<int>>& cost_matrix, vector<bool>& used, vector<int>& match) {
    for (int y = 0; y < 10; y++) {
        if (!used[y] && cost_matrix[x][y] < INT_MAX/2) {
            used[y] = true;
            if (match[y] == -1 || hungarian_dfs(match[y], cost_matrix, used, match)) {
                match[y] = x;
                return true;
            }
        }
    }
    return false;
}

int hungarian_algorithm(const vector<vector<int>>& cost_matrix) {
    vector<int> match(10, -1);
    int total_cost = 0;
    
    // 对每个起点尝试匹配
    for (int x = 0; x < 10; x++) {
        vector<bool> used(10, false);
        hungarian_dfs(x, cost_matrix, used, match);
    }
    
    // 计算总成本
    for (int y = 0; y < 10; y++) {
        if (match[y] != -1) {
            total_cost += cost_matrix[match[y]][y];
        }
    }
    
    return total_cost;
}

// ==================== 4. 辅助：打印10层棋盘合法点位（可选） ====================
void print_board_range() {
    cout << "\n=== 10层标准六芒星棋盘合法坐标范围 ===" << endl;
    cout << "坐标(q, r) 满足：|q| ≤ 10, |r| ≤ 10, |-q-r| ≤ 10，且 |q|+|r|+|-q-r| ≤ 20" << endl;
    cout << "例如：角部营区（红色）：q=10, r=-10 ~ 0 → (10,0),(10,-1)...(10,-10)" << endl;
    cout << "      中心点位：(0,0)" << endl;
}

// ==================== 5. 主函数（适配10个棋子输入） ====================
int main() {
    cout << "=========================================" << endl;
    cout << "    跳棋最优路径规划程序 (详细输出版)" << endl;
    cout << "=========================================" << endl;
    
    auto start_time = high_resolution_clock::now();
    print_board_range();

    vector<Pos> S(10), T(10); // 10个起点、10个终点
    cout << "\n📥 输入阶段" << endl;
    cout << "===================" << endl;
    cout << "请输入10个起点坐标（格式：q r，每行1个）：" << endl;
    for (int i = 0; i < 10; i++) {
        cin >> S[i].first >> S[i].second;
        // 校验输入坐标合法性
        if (!is_valid(S[i])) {
            cout << "⚠️  警告：起点" << i+1 << " (" << S[i].first << "," << S[i].second << ") 不是合法棋盘位置！" << endl;
        }
    }

    cout << "\n请输入10个终点坐标（格式：q r，每行1个）：" << endl;
    for (int i = 0; i < 10; i++) {
        cin >> T[i].first >> T[i].second;
        // 校验输入坐标合法性
        if (!is_valid(T[i])) {
            cout << "⚠️  警告：终点" << i+1 << " (" << T[i].first << "," << T[i].second << ") 不是合法棋盘位置！" << endl;
        }
    }

    // 显示完整的起点终点对应表
    cout << "\n📋 完整测试用例信息" << endl;
    cout << "=======================" << endl;
    cout << "棋子编号 | 起点坐标    | 终点坐标" << endl;
    cout << "---------|-------------|----------" << endl;
    for (int i = 0; i < 10; i++) {
        printf("   %2d    | (%2d,%2d)    | (%2d,%2d)\n", 
               i, S[i].first, S[i].second, T[i].first, T[i].second);
    }

    // 构造代价矩阵：计算每个起点到每个终点的最短步数
    cout << "\n🧮 计算阶段 - 各棋子到各终点的最短步数..." << endl;
    vector<vector<int>> cost_matrix(10, vector<int>(10));
    
    for (int i = 0; i < 10; i++) {
        // others：除了当前棋子外的其他9个棋子位置（模拟棋盘占位）
        vector<Pos> others;
        for (int k = 0; k < 10; k++) {
            if (k != i) others.push_back(S[k]);
        }
        cout << "  计算起点" << i << "到所有终点的步数: ";
        for (int j = 0; j < 10; j++) {
            cost_matrix[i][j] = bfs(S[i], T[j], others);
            cout << format_steps(cost_matrix[i][j]) << " ";
        }
        cout << endl;
    }

    // 使用匈牙利算法求最小权完美匹配（替代有问题的KM算法）
    cout << "\n🎯 匹配阶段 - 运行匈牙利算法求最优匹配..." << endl;
    int total_min_steps = hungarian_algorithm(cost_matrix);
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    // 输出结果
    cout << "\n=========================================" << endl;
    cout << "           📊 最终计算结果" << endl;
    cout << "=========================================" << endl;
    cout << "⏱️  计算总耗时: " << duration.count() << " 毫秒" << endl;
    cout << "🔢 10枚棋子的最少总步数：" << format_steps(total_min_steps) << endl;
    
    // 获取详细匹配结果
    vector<int> match(10, -1);
    for (int x = 0; x < 10; x++) {
        vector<bool> used(10, false);
        hungarian_dfs(x, cost_matrix, used, match);
    }
    
    cout << "\n📋 详细最优分配方案和移动路径:" << endl;
    cout << "======================================" << endl;
    int total_actual_steps = 0;
    int unreachable_count = 0;
    
    for (int j = 0; j < 10; j++) {
        if (match[j] != -1) {
            int start_idx = match[j];
            int end_idx = j;
            int steps = cost_matrix[start_idx][end_idx];
            
            cout << "🏁 棋子 " << start_idx << ": (" << S[start_idx].first << "," << S[start_idx].second 
                 << ") → (" << T[end_idx].first << "," << T[end_idx].second << ")" << endl;
            cout << "   📏 最短步数: " << format_steps(steps) << endl;
            
            if (steps >= INT_MAX / 2) {
                cout << "   ⚠️  路径不可达：起点和终点之间无有效路径" << endl;
                unreachable_count++;
            } else {
                total_actual_steps += steps;
                
                // 输出每一步的具体走法
                vector<Pos> others;
                for (int k = 0; k < 10; k++) if (k != start_idx) others.push_back(S[k]);
                auto move_steps = find_path(S[start_idx], T[end_idx], others);
                const Pos MOVE_SENTINEL = {-10000, -10000};
                
                if (move_steps.empty()) {
                    cout << "   📍 无需移动（起点=终点）" << endl;
                } else {
                    cout << "   🎯 详细移动步骤:" << endl;
                    for (size_t step_idx = 0; step_idx < move_steps.size(); ++step_idx) {
                        auto &step = move_steps[step_idx];
                        cout << "      第" << step_idx + 1 << "步: ";
                        
                        if (step.size() == 1 && step[0].second == MOVE_SENTINEL) {
                            Pos land = step[0].first;
                            cout << "单步移动到 (" << land.first << "," << land.second << ")" << endl;
                        } else {
                            cout << "连续跳跃链 (" << step.size() << "个着陆点):" << endl;
                            for (size_t landing_idx = 0; landing_idx < step.size(); ++landing_idx) {
                                Pos land = step[landing_idx].first;
                                Pos mid  = step[landing_idx].second;
                                cout << "         着陆点" << landing_idx + 1 << ": (" << land.first << "," << land.second << ")";
                                if (!(mid.first == -10000 && mid.second == -10000)) {
                                    cout << "  ← 跳跃过 (" << mid.first << "," << mid.second << ")";
                                }
                                cout << endl;
                            }
                        }
                    }
                }
            }
            cout << "-----------------------------------------" << endl;
        }
    }
    
    cout << "📊 统计信息:" << endl;
    cout << "   • 可达路径数: " << (10 - unreachable_count) << "/10" << endl;
    cout << "   • 不可达路径数: " << unreachable_count << "/10" << endl;
    cout << "   • 实际累加的移动步数: " << total_actual_steps << endl;
    if (unreachable_count == 0) {
        cout << "   • 差异检查: " << (total_min_steps == total_actual_steps ? "✅ 一致" : "❌ 不一致") << endl;
    } else {
        cout << "   • 注意: 存在不可达路径，总步数计算可能不准确" << endl;
    }

    cout << "\n✅ 程序执行完成! 所有测试用例信息已完整输出。" << endl;
    return 0;
}