# 条件
1. 系统中一开始指定N个不重复的终点
2. 系统中一开始指定N个不重复的起点
3. 要求N个起点的ball填满N个终点，跳棋的走法规则
4. 整个系统中除了N个起点在开始时刻有ball之外，其他所有位置都不存在ball

# 基本算法思路
1. 计算当前N个ball到N个终点的Trend值：X，Y轴坐标差的绝对值之和。每个ball对N个终点求trend值，取Min
2. N个ball得到N个Trend值，O（N*N）时间复杂度
3. N个Trend值求和，为系统Trend值
4. 当前系统状态下，有M个下一步策略，求每一种策略下得到的新的系统状态的Trend值
5. 取Trend值最小的状态为系统下一步状态
6. 直到N个终点填满，迭代计算结束
7. 最优解可以通过回溯计算所有可能分支得到系统总的最优解，但是在不做优化的情况下，时间复杂度，空间复杂度太大



# 如何判断一个ball的可达点位
1. 首先判断ball所有方向上+1位置，如果可用（没有越界，没有其他ball占用）则计入可达list
2. ball的每一个方向上超过+1的位置，进行搜索。如果达到边界则结束；如果没达到边界，且发现一个ball，则计算可Skip这个ball
3. 如果有可Skip的ball，则改为新的位置，重复step 2. 不可以重复step 1. 这里是深度优先的搜索。
4. 得到ball的可达点位的list，计入系统下一步策略中
5. 对每一个ball计算可达点位list，得到系统总体的下一步策略
6. 评估每一个策略的Trend值，得到最小的为下一步策略 （如果考虑回溯求解，这里就要去遍历穷举所有策略了）







