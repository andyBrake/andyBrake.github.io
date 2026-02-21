
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

    // 无法到达时返回明显的不可达标记，匈牙利算法中会把 >= INT_MAX/2 视为不可用边
    return INT_MAX / 2;
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
    cout << "    跳棋最优路径规划程序 (最终稳定版)" << endl;
    cout << "=========================================" << endl;
    
    auto start_time = high_resolution_clock::now();
    print_board_range();

    vector<Pos> S(10), T(10); // 10个起点、10个终点
    cout << "\n请输入10个起点坐标（格式：q r，每行1个）：" << endl;
    for (int i = 0; i < 10; i++) {
        cin >> S[i].first >> S[i].second;
        // 校验输入坐标合法性
        if (!is_valid(S[i])) {
            cout << "警告：起点" << i+1 << " (" << S[i].first << "," << S[i].second << ") 不是合法棋盘位置！" << endl;
        }
    }

    cout << "\n请输入10个终点坐标（格式：q r，每行1个）：" << endl;
    for (int i = 0; i < 10; i++) {
        cin >> T[i].first >> T[i].second;
        // 校验输入坐标合法性
        if (!is_valid(T[i])) {
            cout << "警告：终点" << i+1 << " (" << T[i].first << "," << T[i].second << ") 不是合法棋盘位置！" << endl;
        }
    }

    // 构造代价矩阵：计算每个起点到每个终点的最短步数
    cout << "\n🧮 计算各棋子到各终点的最短步数..." << endl;
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
            cout << cost_matrix[i][j] << " ";
        }
        cout << endl;
    }

    // 使用匈牙利算法求最小权完美匹配（替代有问题的KM算法）
    cout << "\n🎯 运行匈牙利算法求最优匹配..." << endl;
    int total_min_steps = hungarian_algorithm(cost_matrix);
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    // 输出结果
    cout << "\n=========================================" << endl;
    cout << "           📊 计算结果" << endl;
    cout << "=========================================" << endl;
    cout << "⏱️  计算耗时: " << duration.count() << " 毫秒" << endl;
    cout << "🔢 10枚棋子的最少总步数：" << total_min_steps << endl;
    
    // 获取详细匹配结果
    vector<int> match(10, -1);
    for (int x = 0; x < 10; x++) {
        vector<bool> used(10, false);
        hungarian_dfs(x, cost_matrix, used, match);
    }
    
    cout << "\n📋 最优分配方案:" << endl;
    cout << "-----------------------------------------" << endl;
    for (int j = 0; j < 10; j++) {
        if (match[j] != -1) {
            cout << "🏁 终点" << j << " (" << T[j].first << "," << T[j].second << ")" << endl;
            cout << "   ↖️  ← 起点" << match[j] << " (" << S[match[j]].first << "," << S[match[j]].second << ")" << endl;
            cout << "   📏 步数: " << cost_matrix[match[j]][j] << endl;
            cout << "-----------------------------------------" << endl;
        }
    }

    cout << "\n✅ 程序执行完成!" << endl;
    return 0;
}