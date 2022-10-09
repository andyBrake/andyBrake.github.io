lsof -i:9999


 g++  -pthread  -O testServer.cpp



# 在游戏开始前的消息交互
## Client2Server Msg:
1. 在connect成功后，第一个发送Player Name, Server收到后返回player ID
{
    Type : 0
    Player Name: Fa
}
2. Client收到Player ID后，发送Ready并进入等待状态，等待Server通知下一局比赛开始
{
    Type : 1         # type 分类:  0 表示connect消息，即完成链接之后发送Player Name的消息
                     #            1 表示Status消息
                     #            2 表示Option/Action消息
    Player ID: 1
    Status:0         # Status 分类：0 表示Ready， 1 表示 Start
}
3. 在Client收到Server的Start后，回复一个确认Start的消息，消息内容和Server发过来的一样
{
    Type : 1
    Player ID: 1
    Status:1         # Start 
}

## Server2Client Msg:
1. 在Server收到Player Name信息后，在Server创建Player对象分配Player ID，并发送给Client
{
    Type : 0
    Player Name: Fa
    Player ID: 1
}
2. Server收到Client的Ready消息后，等待下一局游戏开始的时候，发送Start消息给Client
{
    Type : 1
    Player ID: 1
    Status:1         # Status 分类：0 表示Ready， 1 表示 Start
}
3. Server 收到Client的Start确认消息，则开始本局游戏

## 游戏开始前主要流程为：
Client 1 -> Server 1 -> Client 2 ->Server 2 -> Client 3 -> Server 3

# 在游戏开始后的消息交互 
## Client2Server Msg:
1. Player收到option的消息后，根据option的值做处理。如果是Blind则必须支付盲注，否则可以选择check或者raise，
或者AllIn，或者Fold。决策后发送Action消息给Server
{
    Type : 2
    Player ID: 1
    Action:1         # 0 Fold, 1 Check (此时Bet一定为0), 2 Raise（此时Bet一定比require要求的大）, 
                     # 3 AllIn（此时Bet不一定比require要求的大，但是Player的剩余筹码归0） 
    Bet: 10          # Player选择支付的筹码，可能比require要求的大，即表示raise了
}

## Server2Client Msg:
1. 如果Player是Blind，则会首先发送Blind消息，如果不是Blind玩家，则会发送 Bet 消息，即Option为1
{
    Type : 2
    Player ID: 1
    Option:0         # 0 Blind, 1 Bet
    Bet:5            # 表示最低需要支付的筹码值
    Behind: 7        # 表示在你之后，还有多少位Player决策。例如Blind消息，Behind则为0，因为只需要一个Player支付盲注
    Bonus: 100       # 表示当前底池总共有多少价值
}


# 在游戏开始后主要流程为:
Server 1 -> Client 1 -> Server 1 -> Client 1 .... Loop