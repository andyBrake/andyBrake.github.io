# 德州扑克系统架构文档

## 系统架构图

```mermaid
graph TB
    subgraph "Server端 (C++)"
        A[testServer.cpp] --> B[Server类]
        B --> C[PokerTable]
        C --> D[Dealer发牌器]
        C --> E[Ruler规则引擎]
        C --> F1[Robot Player 0]
        C --> F2[Robot Player 1]
        C --> F3[Remote Player 2]
        B --> G[Socket监听线程]
        G --> H[端口10002]
    end
    
    subgraph "Client端 (Python)"
        I[ClientMain.py] --> J[ClientController]
        J --> K[ClientView GUI]
        J --> L[ClientModule]
        J --> M[Socket连接]
        K --> N[Tkinter窗口]
    end
    
    H -->|TCP/IP| M
    M -->|消息协议| H
```

## 通信流程图

```mermaid
graph LR
    A[客户端启动] --> B[连接服务器]
    B --> C[发送玩家名称]
    C --> D[接收玩家ID]
    D --> E[同步玩家状态]
    E --> F[接收私人牌]
    F --> G[接收下注请求]
    G --> H[用户操作]
    H --> I[发送操作响应]
    I --> J[下一轮]
    J --> G
```

## 状态机图

```mermaid
graph TB
    S1[cINIT] --> S2[cCONNECTED]
    S2 --> S3[cWAIT_PLAYER_ID]
    S3 --> S4[cRESP_PLAYER_ID]
    S4 --> S5[cWAIT_START_STATUS]
    S5 --> S6[cRESP_START_STATUS]
    S6 --> S7[cPLAYING_SYNC]
    S7 --> S7
```

## 消息协议类型

```mermaid
graph TB
    A[消息类型] --> B[cMSG_ASSIGN_ID]
    A --> C[cMSG_SYNC_STATUS]
    A --> D[cMSG_ACQ_ACTION]
    A --> E[cMSG_ADJUST_BET]
    A --> F[cMSG_SEND_CARD]
```

## 类关系图

```mermaid
graph TB
    Server --> PokerTable
    PokerTable --> Dealer
    PokerTable --> Ruler
    PokerTable --> Player
    Player --> Robot
    Player --> RemotePlayer
    PokerTable --> SimpleChannel
    RemotePlayer --> Socket
```

## 游戏流程图

```mermaid
graph TB
    A[初始化游戏] --> B[设置庄家位置]
    B --> C[小盲注]
    C --> D[发私人牌]
    D --> E[Pre-Flop下注]
    E --> F[翻牌Flop]
    F --> G[Post-Flop下注]
    G --> H[转牌Turn]
    H --> I[Turn下注]
    I --> J[河牌River]
    J --> K[River下注]
    K --> L[比牌决胜]
    L --> M[分配奖金]
```

## 线程模型

```mermaid
graph LR
    A[主线程] --> B[游戏逻辑]
    A --> C[监听线程]
    C --> D[接受客户端连接]
    
    E[客户端主线程] --> F[GUI事件循环]
    E --> G[接收线程]
    G --> H[处理服务器消息]
```

## 数据流向

```mermaid
graph LR
    A[Server] -->|Request消息| B[Client]
    B -->|Response消息| A
    
    C[PokerTable] -->|游戏状态| D[RemotePlayer]
    D -->|Socket| E[ClientController]
    E -->|更新界面| F[ClientView]
    F -->|用户操作| E
    E -->|Socket| D
```

## 内存管理模型

```mermaid
graph TB
    A[Server对象] --> B[unique_ptr PokerTable]
    B --> C[原始指针 Player数组]
    C --> D[new Robot]
    C --> E[new RemotePlayer]
    
    F[PokerTable析构] --> G[自动删除所有Player]
    H[Server析构] --> I[自动删除PokerTable]
```

## 核心类职责

### Server类
- Socket监听和连接管理
- 多线程控制
- PokerTable生命周期管理

### PokerTable类
- 游戏流程控制
- 玩家管理
- 下注循环
- 获胜判定

### Dealer类
- 洗牌
- 发牌(私人牌、公共牌)
- 牌堆管理

### Ruler类
- 牌力计算
- 规则判定
- 大小比较

### Player类
- 玩家状态
- 筹码管理
- 操作接口

### ClientController类
- 网络通信
- 状态管理
- 消息处理
- View控制

### ClientView类
- GUI渲染
- 用户交互
- 状态显示

## 技术栈

### 服务器端
- **语言**: C++11
- **网络**: POSIX Socket
- **线程**: pthread
- **内存管理**: unique_ptr, atomic
- **编译**: g++/clang++

### 客户端
- **语言**: Python 3
- **GUI**: Tkinter
- **网络**: socket模块
- **线程**: threading模块

## 配置参数

| 参数 | 值 | 说明 |
|------|---|------|
| 服务器IP | 127.0.0.1 | 本地测试 |
| 服务器端口 | 10002 | TCP监听端口 |
| 最大玩家数 | 8 | PokerTable::cMaxPlayerCount |
| 小盲注 | 5 | PokerTable::cBlindBet |
| 初始筹码 | 500 | gPlayerInitBet |
| 消息缓冲区 | 1024 | Server::MsgBufferLen |

## 部署方式

1. 编译服务器: `cd Server && make`
2. 启动服务器: `./server.out`
3. 启动客户端: `python3 ClientMain.py`
4. 在GUI界面进行游戏操作
