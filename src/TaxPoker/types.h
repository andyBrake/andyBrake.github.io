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

enum ClientStatus
{
    cSTATUS_WAIT        = 0 ,
    cSTATUS_READY     = 1,

    cSTATUS_INV           =10
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
    cMSG_ADJUST_BET    = 3,     // adjust the player total bet
    cMSG_SEND_CARD    = 4,     // Send Card to Client
    
    cMSG_INV                    = 10

};

enum GameStatus
{
    GS_Init = 0,               //下盲注

    GS_preFlop = 1,        //翻牌前
    GS_postFlop = 2,     //翻牌后

    GS_Turn = 3,           //转牌后
    GS_River = 4,          //河牌后
    GS_Final                  //最后比大小
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

    void fillCardMsg(char msg[], int id, int cardCnt, Card cards[])
    {
        const char * type4format = "Type:%u;\nPlayer ID:%d;\nCardCnt:%u;\n"\
                                                            "Card0Type:%d;\nCard0Value:%d;\n"\
                                                            "Card1Type:%d;\nCard1Value:%d;\n"\
                                                            "Card2Type:%d;\nCard2Value:%d;\n";  // at most 3 card once send
        
        /* send public card */
        if (id < 0)
        {
            cout<<"Send public Card"<<endl;
        }
        /* send player private card */
        else
        {
            cout<<"Send private Card"<<endl;
        }
        sprintf(msg, type4format, MsgType::cMSG_SEND_CARD, id, cardCnt,
                int(cards[0].color), int(cards[0].value),
                int(cards[1].color), int(cards[1].value),
                int(cards[2].color), int(cards[2].value));
        return;
    }

    /* status : 1 means Ready, 0 means Wait */
    void fillSyncMsg(char msg[], int id, int status, char desc[8][50] )  
    {
        const char * type1format = "Type:%u;\nPlayer ID:%u;\nStatus:%u\n";
        const char * allPlayerInfoFormat = "Type:%u;\nPlayer ID:%u;\nStatus:%u;\nCount:%u;\n"  \
                                "P0:%s;\nP1:%s;\nP2:%s;\nP3:%s;\nP4:%s;\nP5:%s;\nP6:%s;\nP7:%s;\n";
        
        /* Start game should append all player info */
        if (0 == status)
        {
            sprintf(msg, type1format, MsgType::cMSG_SYNC_STATUS, id, status);
        }
        else if (1 == status)
        {
            sprintf(msg, allPlayerInfoFormat, MsgType::cMSG_SYNC_STATUS, id, status, 3, // total player count
                desc[0],   // P0
                desc[1],   // P1
                desc[2],   // P2
                desc[3],   // P3
                desc[4],   // P4
                desc[5],   // P5
                desc[6],   // P6
                desc[7]    // P7
                );
        }
        else
        {
            cout<<"Invalid Status "<<status<<endl;
            exit(1);
        }
        

        return;
    }

    void fillActionMsg(char msg[], int id, int option, int bet, int behind, int bonus, GameStatus stage) // GameStatus
    {
        const char * type2format = "Type:%u;\nPlayer ID:%u;\nOption:%u;\nBet:%u;\nBehind:%u;\nBonus:%u;\nStage:%u;\nCount:%u;\n" \
                                                           "P0:%s;\nP1:%s;\nP2:%s;\nP3:%s;\nP4:%s;\nP5:%s;\nP6:%s;\nP7:%s;\n";

        sprintf(msg, type2format, MsgType::cMSG_ACQ_ACTION, id, option, bet, behind, bonus, (int)stage, 3, // total player count
                                                            "None",   // P0
                                                            "None",   // P1
                                                            "None",   // P2
                                                            "None",   // P3
                                                            "None",   // P4
                                                            "None",   // P5
                                                            "None",   // P6
                                                            "None"    // P7
                                                            );

        return;
    }

    void fillAdjustBetMsg(char msg[], int id, int adj)
    {
        const char * type3format = "Type:%u;\nPlayer ID:%u;\nAdjust:%u\n";

        sprintf(msg, type3format, MsgType::cMSG_ADJUST_BET, id, adj);
        
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
    const int cRespMsgItemCount = 5;

    /* The Player status field */
    bool isFold;
    bool isAllIn;

    Client2ServerMsg()
    {
        this->isFold = false;
        this->isAllIn = false;
    }

    void Init()
    {
        this->playerId = -1;
        this->isFold = false;
        this->isAllIn = false;
        this->bet = 0;
    }

    ~Client2ServerMsg()
    {
    }

    Client2ServerMsg(const Client2ServerMsg &msg)
    {
        this->playerId = msg.playerId;
        this->isFold = msg.isFold;
        this->isAllIn = msg.isAllIn;
        this->bet = msg.bet;
    }

    bool operator==(const Client2ServerMsg &msg)
    {
        if (this->playerId == msg.playerId && this->isFold == msg.isFold && this->isAllIn == msg.isAllIn && this->bet == msg.bet)
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

    
    void Analysis(char *str)
    {
        const char *delim = ";";
        char *p = NULL;
        char  itemStr[cRespMsgItemCount][20];
        int     itemValue[cRespMsgItemCount];
        int     itemIndex = 0;
        
        //cout<<"Analysis Reponse:{\n"<<str<<"}"<<endl;

        p = strtok(str, delim);

        while(p) 
        {
            strcpy(itemStr[itemIndex++], p);
            p = strtok(NULL, delim);
        }

        //cout<<"Dispatch each item string, total item count "<<itemIndex<<endl;

        int i=0;
        while(i < itemIndex)
        {
            char * pVal = strchr(itemStr[i], int(':'));
            itemValue[i] = atoi(pVal+1);

            //cout<< itemStr[i] << " , pVal "<<pVal<<", val"<< itemValue[i]<<endl;
            
            i++;
        }
        //cout<<"Dispatch Finish !"<<endl;

        this->type       = itemValue[0];
        this->playerId = itemValue[1];
        this->action     = itemValue[2]; // PlayerAction
        this->bet         = itemValue[3];
        this->totalBet = itemValue[4];

        if (this->action == cPLAYER_FOLD)
        {
            this->isFold = true;
        }
        else if (this->action == cPLAYER_ALLIN)
        {
            this->isAllIn = true;
        }

        return;
    }

    void setBet(int bet)
    {
        this->bet = bet;
        return;
    }

    int getBet()
    {
        return bet;
    }

    int getTotalBet()
    {
        return this->totalBet;
    }

    void setPlayerId(int playerId)
    {
        this->playerId = playerId;
    }

    int getPlayerId()
    {
        return this->playerId;
    }

    private:
    /* The response message field */
    int type;        //Message Type
    int playerId; // Player ID
    int action;    // Player Action  : PlayerAction
    int bet;         // Player Pay Bet
    int totalBet;  // Player remind total bet
    
};

ostream &operator<<(ostream &os, const Client2ServerMsg &ob)
{
    os << "  Client2ServerMsg " << endl;

    return os;
}

#endif
