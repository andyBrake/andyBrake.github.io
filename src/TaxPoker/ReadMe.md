# 系统总体框架
主要成员包含：
1. PokerTable : 牌桌，负责整场游戏的状态管理，与Player，Dealer交互等，推进游戏进行
2. Player : 玩家，至少需要2个Player，玩家根据PokerTable提供的信息，采取决策，包含：Call, Raise, Fold, Blind Bet
3. Dealer ： 荷官，负责洗牌，发牌，通知Player切牌，以及在show牌之后按照Rule确定Player的牌力大小，通知PokerTable分Bouns Pool
4. Ruler : 规则类，在show牌之后，根据公共牌以及每一个Player的手牌，计算该Player的牌力值
5. Channel ： PokerTable，Dealer，Rule等都运行在Server端，只有各个Player运行在不同的Client端，Client和Server之间的通信通道采用Channel，底层实现可以是TCP或者local queue等

主要流程：
最初，随机指定庄家（Host）位置，则Host + 1为Small Blind；下一局游戏Host位置向+1位置顺延，依次类推。
在下注过程中即Bet Loop 专门处理，负责将PokerTable信息发送给Player，等待Player决策，然后依次和下一位Player交互，直到达到Loop结束条件：只剩一个Player，或者所有人Bet持平或All in。


# PokerTable的主流程
1. 启动运行，完成系统资源初始化，确定当前玩家个数，并等待其他Player接入
2. 确定是否可以开始一局新游戏时
3. 游戏开始，确定Host位置，Small Blind 位置
4. 通知SB Player下Blind Bet，等待下注完成
5. 启动Bet Loop ： 从SB + 1 位置开始，到SB位置结束
6. 判断本局是否结束，即是否只剩一个Player，是则分Bouns Pool，否则继续下一步
7. 通知Dealer开始发牌：发翻牌，更新公共牌 Public Card 
8. 启动Bet Loop ： 从SB位置开始，到Host位置结束
9. 判断本局是否结束，即是否只剩一个Player，是则分Bouns Pool，否则继续下一步 （重复第 6 步）
10. 通知Dealer开始发牌：发转牌，更新公共牌 Public Card
11. 启动Bet Loop ： 从SB位置开始，到Host位置结束（重复第 8 步）
12. 判断本局是否结束，即是否只剩一个Player，是则分Bouns Pool，否则继续下一步（重复第 6 步）
13. 通知Dealer开始发牌：发河牌，更新公共牌 Public Card
14. 启动Bet Loop ： 从SB位置开始，到Host位置结束（重复第 8 步）
15. 判断本局是否结束，即是否只剩一个Player，是则分Bouns Pool，否则继续下一步（重复第 6 步）
16. Dealer通知Ruler依次计算每个Player的牌力，并得到牌力最大的N个Player
17. 根据牌力信息分Bouns Pool
18. 结束本局游戏，确定是否继续。是则跳到Step 3，否则结束进程。


# Dealer的主流程
荷官Dealer主要负责洗牌，发牌等和牌有关的操作。在一副牌中总共有4*13=52张，初始状态有序存储在一个数组中。

## Card Type
根据游戏进程变化，一副牌会拆分成几种类型，每一个类型可以使用Queue或者数组管理。
1. Public Card ：公共牌，所有Player可见，可用
2. Player Card ： 每一个Player起手的2张牌，其他玩家不可见
3. Remind Card ： 当前未使用的牌，当需要发牌时从该牌堆中依次发放，包括切牌
4. Useless Card ：无用牌，切牌被切走的部分Card

Dealer主要负责对Remind Card的处理，根据不同的功能有不同的实现流程。
## Card Operation
1. Wash Card ：洗牌，将所有的Card根据随机产生的顺序，Push到Remain Card Queue中。
2. Split Card : 切牌，选择切牌数量（位置），将之前的Card全部从Remain Card Queue中Pop出，Push到Useless Card Queue中。
3. Deal Card ： 发牌，有几种情况：
    * 发给每个Player 2张 Player Card
    * 发出3张翻牌到Public Card 
    * 发出1张转牌到Public Card 
    * 发出1张河牌到Public Card 

# Rule 的算法
Rule模块主要负责计算一副手牌的最终牌力。另外还可以提供计算非完整手牌的Outs （TBD）。
1. Card Set: 手牌, 一副手牌总共7张，其中5张Public Card，2张Player Card，从这7张中选择5张组成牌力最大的组合即这副手牌的最终牌力。
2. Card Set Power: 牌力,根据组合不同，分为几个层次，高等级组合牌力一定大于低等级。同等级组合情况下，比较具体Key值。
3. Key ：某种组合中，比如2对，每一个关键成员的具体值。比如一副牌为 （对K，对10，单A）则Key 1 为13，即K；Key 2 值为10， Key 3值为14，即A。一种组合内最多有5个有效Key，每一个key的定义跟具体组合类型有关。

## 各种组合等级
从最高级向下，依次为：
1. 同花顺 ： 同时符合同花和顺子的规则。 Key 1 为顺子中最大的牌的value，其余 Key 都为0.
2. 四条：有四张牌的value一样，第五张牌不考虑，不需要参与计算牌力. Key 1为四张相同牌的Value，其余 Key 都为0.
3. 葫芦：首先有三张牌value一样，然后另外有两张牌的value一样。Key 1为三张相同牌的Value， Key 2 为两张相同牌的Value， 其余 Key 都为0.
4. 同花: 五张牌的color一样. Key 1 为同花中最大的牌Value，其他 Key都为0.
5. 顺子 ： 有五张牌的Value是连续的。 Key 1 为顺子中最大牌的Value，其余 Key 都为0.
6. 三条 ： 有三张牌的Value是一样的。Key 1 为三张牌的Value， 其余 Key 都为0.
7. 两对 ： 首先有两张牌的Value一样，然后另外还有2张牌的Value也一样。 Key 1， Key 2 为两对的Value， Key 3 为剩下的3张牌中Value最大的。其余 Key 都为0
8. 一对 ： 只有2张牌的Value一样，Key 1 为相同牌的Value，Key 2，Key 3, Key 4 为剩下牌中依次最大的。
9. 单牌 ： 上诉都不满足，只能比较单张大小。 Key 1 到Key 5 依次为牌Value最大的，降序排列。

## 如何计算牌力
高等级的组合，其等级值越大，最高等级同花顺设为9，依次减1.
为了保证高等级组合牌力一定大于低等级，对于不同等级需要给予不同的权重值。
同时在相同等级内比较时，Key 1 大的牌力，一定比后续Key大的牌力强，所以不同的Key也需要给予不同的权重值。
当前设计如下：Card Set Power = (等级值 * 100000) + (Key1 * 10000) + (Key2 * 1000) + (Key3 * 100) + (Key4 * 10) + Key5

## 不同等级的组合比较逻辑
由于在一个等级内的组合值，差异不会大于100000， 所以上诉计算公式保证高等级的牌力值一定大于低等级。

## 同等级的组合比较逻辑
由于在同等级内各个Key值的最小差异为13 - 2 = 11. 同时Key 4 的最小值是2，乘以10之后为20，大于11，所以上诉公式也可以保证同等级内比较逻辑的正确。

=======================================================================================================================================================
# Major Class

## Card
struct Card
{
    Color color; // enum of Card color, total 4 color
    int   value; // from 1 to 14. J means 11, Q means 12, K means 13， A means 14 or 1.
}

enum eLevel
{
    HighCardLevel = 1,
    OnePairsLevel,
    TwoPairsLevel,
    SetLevel,
    Straight,
    FlushLevel,
    WholeHouseLevel, // 3 + 2
    FourKindLevel, // Four of a Kind
    StraightFlushLevel, // same color, sort 
};


class Dealer
{
    public:
    void washCard(); // enqueue the remain card queue in random order
    void splitCard(const int splitCount); // set the split count, must set before each deal operation
    void dealCard(const int dealCount, Card *outCards); // get dealCount Cards from remain card

    private:
    Card initCard[52]; // in order card
    
    int splitCount; // how many card should be splited, should be reset once dealed card
    Queue<Card> remainCardQueue;
    Queue<Card> uselessCardQueue;
};



class Player
{
    public:
    int Id; // the sequence in table
    bool isStay;
    int totalBet;
    bool isAllIn;

    private:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];
};

class PokerTable
{
    public:

    int betLoop(); // return true if the game finish
    void moveBouns(); // give bonus to winner
    int acquirePlayerAction(Player player, int currentLoopBet, int currentBounsPool, int behindPlayerCount); //
    int waitPlayerAction(Player &player); // dead loop for waiting for player Id action


    private:
    Player allPlayer[];
    int hostPos;
    int sbPos;
    int bounsPool;
    int currentLoopBet; // how many Bet should call in current loop, if any Player raise, should increase it
    GameStatus status; // the enum of game status, to indicate which state of the game, for example, before flop.

};

class Ruler
{
    struct CardPower
    {
        int key1;
        int key2;
        int key3;
        int key4;
        int key5;
    };

    /* the Ruler function must be staic */
    static int calCardSetPower(const Card cardSet[], const int cardCount); // the card count should be 7

    private:
    Ruler() = default; // we don't need Ruler instance
    ~Ruler() = default;
}

=====================================================================================================================================================


class PlayerClient
{
    public:
    void process(); // Loop to receive message from server
    int betAction(); // call, raise, all-in, or fold 

    int totalBet;

    private:
    Card cards[2];

};