#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <iomanip>

using namespace std;

enum CardColor // change to eCardColor
{
    Spade = 0,   // 黑桃    \u2664
    Hearts = 1,  // 红桃    \u2661
    Club = 2,    // 梅花    \u2663
    Diamond = 3, // 方片    \u2662

    ColorNum = 4
};

enum CardValue
{
    CV_INV = 0,

    CV_1 = 1,
    CV_2,
    CV_3,
    CV_4,
    CV_5,
    CV_6,
    CV_7,
    CV_8,
    CV_9,
    CV_10,

    CV_J,  // 11
    CV_Q,  // 12
    CV_K,  // 13
    CV_A   // 14

};


static const string s[] = {"\u2664", "\u2661", "\u2663", "\u2662"};
static const string v[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

struct Card
{
    friend ostream &operator<<(ostream &, const Card &);

    Card()
    {
        this->color = Spade;
        this->value = CV_INV;
    }

    Card(CardColor color, CardValue value)
    {
        this->color = color;
        this->value = value;
    }

    Card(const Card &card)
    {
        this->color = card.color;
        this->value = card.value;
    }

    Card &operator=(const Card &card)
    {
        if (this == &card)
        {
            return *this;
        }
        this->color = card.color;
        this->value = card.value;

        return *this;
    }

    bool operator==(const Card &card)
    {
        if (this->color == card.color && this->value == card.value)
        {
            return true;
        }

        return false;
    }

    void display()
    {
        cout << " Color " << this->color << ", Value " << setw(2) << this->value << ".  "\
             << s[(int)this->color] << setw(2) << left << v[int(this->value) - 2];
    }

    ~Card() {}

    CardColor color;      // enum of Card color, total 4 color
    CardValue value; // from 1 to 14. J means 11, Q means 12, K means 13， A means 14 or 1.
};

ostream &operator<<(ostream &os, const Card &ob)
{
    os << s[(int)ob.color] << " "<<setw(2)<< v[int(ob.value) - 2]<<"   ";

    return os;
}

enum CardNumber
{
    ePublicCardNum = 5,
    ePrivateCardNum = 2,

    eFlopCardNum = 3,
    eTurnCardNum = 1,
    eRiverCardNum = 1,

    eCardSetNum = ePublicCardNum + ePrivateCardNum
};

enum CardLevel
{
    Unkown = 0,
    HighCardLevel = 1,
    OnePairsLevel = 2,
    TwoPairsLevel = 3,
    SetLevel = 4,
    StraightLevel = 5,
    FlushLevel = 6,
    WholeHouseLevel = 7,    // 3 + 2
    FourKindLevel = 8,      // Four of a Kind
    StraightFlushLevel = 9, // same color, sort

    TotalLevelCount = 9        
};

// Action : Fold, Check, Call, Raise, Allin
enum PlayerAction
{
    cPLAYER_FOLD     = 0,
    cPLAYER_CHECK   = 1,
    cPLAYER_CALL      = 2,
    cPLAYER_RAISE    = 3,
    cPLAYER_ALLIN    = 4,

    cPLAYER_INV        = 10
};


enum MsgType
{
    cMSG_ASSIGN_ID       = 0,     // To try to connect with Server, assign a Player ID
    cMSG_SYNC_STATUS = 1,     // Sync the status to 1 ready
    cMSG_ACQ_ACTION   = 2,     // acquire and response player action
    cMSG_INV                    = 10

};

enum GameStatus
{
    GS_Init = 0,

    GS_preFlop = 1,
    GS_postFlop = 2,

    GS_Turn = 3,
    GS_River = 4,
    GS_Final
};

#if 0
// Prefix increment operator.
GameStatus& operator++()       
{
    
}

// Postfix increment operator.
GameStatus  operator++(int)
{

}
#endif
// Declare prefix and postfix decrement operators.
// Point& operator--();       // Prefix decrement operator.
// Point operator--(int);     // Postfix decrement operator.

void operator++(GameStatus &con)
{
    int i = (int)con;
    con = GameStatus(++i);
}

class Server2ClientMsg
{
public:
    int playerId;
    int bonusPool;
    int curBet;
    int behindPlayerCount;
    bool isBlind;

    Server2ClientMsg()
    {
    }

    ~Server2ClientMsg()
    {
    }

    Server2ClientMsg(const Server2ClientMsg &msg)
    {
        this->playerId = msg.playerId;
        this->bonusPool = msg.bonusPool;
        this->curBet = msg.curBet;
        this->behindPlayerCount = msg.behindPlayerCount;
        this->isBlind = msg.isBlind;
    }

    bool operator==(const Server2ClientMsg &msg)
    {
        if (this->playerId == msg.playerId && this->bonusPool == msg.bonusPool && this->curBet == msg.curBet && this->behindPlayerCount == msg.behindPlayerCount && this->isBlind == msg.isBlind)
        {
            return true;
        }

        return false;
    }
/*
{
    Type : 0               MsgType::cMSG_ASSIGN_ID
    Player Name: Fa
    Player ID: 1
}

{
    Type : 1
    Player ID: 1
    Status:1         # Status 分类：0 表示Ready， 1 表示 Start
}

{
    Type : 2
    Player ID: 1
    Option:0         # 0 Blind, 1 Bet
    Bet:5            # 表示最低需要支付的筹码值
    Behind: 7        # 表示在你之后，还有多少位Player决策。例如Blind消息，Behind则为0，因为只需要一个Player支付盲注
    Bonus: 100       # 表示当前底池总共有多少价值
}
*/
    void fillAssignMsg(char msg[], char name[], int id)
    {
        const char * type0format = "Type:%u;\nPlayer Name:%s;\nPlayer ID:%u\n";

        sprintf(msg, type0format, MsgType::cMSG_ASSIGN_ID, name, id);

        return;
    }

    /* status : 1 means Ready, 0 means Wait */
    void fillSyncMsg(char msg[], int id, int status)
    {
        const char * type1format = "Type:%u;\nPlayer ID:%u;\nStatus:%u\n";

        sprintf(msg, type1format, MsgType::cMSG_SYNC_STATUS, id, status);

        return;
    }

    void fillActionMsg(char msg[], int id, int option, int bet, int behind, int bonus)
    {
        const char * type2format = "Type:%u;\nPlayer ID:%u;\nOption:%u;\nBet:%u;\nBehind:%u;\nBonus:%u\n";

        sprintf(msg, type2format, MsgType::cMSG_ACQ_ACTION, id, option, bet, behind, bonus);

        return;
    }
};

ostream &operator<<(ostream &os, const Server2ClientMsg &ob)
{
    os << "  Server2ClientMsg " << endl;

    return os;
}

class Client2ServerMsg
{
public:
    int PlayerId;
    bool isFold;
    bool isAllIn;
    int bet;

    Client2ServerMsg()
    {
    }

    ~Client2ServerMsg()
    {
    }

    Client2ServerMsg(const Client2ServerMsg &msg)
    {
        this->PlayerId = msg.PlayerId;
        this->isFold = msg.isFold;
        this->isAllIn = msg.isAllIn;
        this->bet = msg.bet;
    }

    bool operator==(const Client2ServerMsg &msg)
    {
        if (this->PlayerId == msg.PlayerId && this->isFold == msg.isFold && this->isAllIn == msg.isAllIn && this->bet == msg.bet)
        {
            return true;
        }
        return false;
    }

    // Action : Fold, Check, Call, Raise, Allin
    void fillActionMsg(char msg[], int action, int bet, int remindBet)
    {
        const char *msgFormat = "Action:%s;\nBet:%u;\nRemindBet:%u;\n";

        sprintf(msg, msgFormat, (0 == action)?"Fold":"None", bet, remindBet);
        
    }
};

ostream &operator<<(ostream &os, const Client2ServerMsg &ob)
{
    os << "  Client2ServerMsg " << endl;

    return os;
}

#endif
