#ifndef TYPE_H
#define TYPE_H

#include <iostream>

using namespace std;

enum Color
{
    Spade   = 0,    // 黑桃    \u2664
    Hearts  = 1,    // 红桃    \u2661
    Club    = 2,    // 梅花    \u2663 
    Diamond = 3,    // 方片    \u2662 

    ColorNum = 4
};

enum eCardValue
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

    CV_J,
    CV_Q,
    CV_K,
    CV_A

};


struct Card
{
    friend ostream &operator<<(ostream &, const Card &);

    Card()
    {
        this->color = Spade;
        this->value = CV_INV;
    }

    Card(Color color, eCardValue value)
    {
        this->color = color;
        this->value = value;
    }

    Card(const Card &card)
    {
        this->color = card.color;
        this->value = card.value;
    }

    bool operator==(const Card &card)
    {
        if (this->color == card.color && this->value == card.value)
        {
            return true;
        }

        return false;
    }

    ~Card(){}

    Color color; // enum of Card color, total 4 color
    eCardValue   value; // from 1 to 14. J means 11, Q means 12, K means 13， A means 14 or 1.
};

ostream &operator<<(ostream &os, const Card &ob)
{
    static string s[] = {"\u2664", "\u2661", "\u2663", "\u2662"};
    static string v[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

    os << "  Card : color " << ob.color << ", value " << ob.value<<". "\
        <<s[(int)ob.color]<<v[int(ob.value) - 2];

    return os;
}

enum
{
    ePublicCardNum  = 5,
    ePrivateCardNum = 2,

    eFlopCardNum    = 3,
    eTurnCardNum    = 1,
    eRiverCardNum   = 1
};

enum eLevel
{
    Unkown = 0,
    HighCardLevel = 1,
    OnePairsLevel = 2,
    TwoPairsLevel = 3,
    SetLevel      = 4,
    StraightLevel = 5,
    FlushLevel    = 6,
    WholeHouseLevel = 7, // 3 + 2
    FourKindLevel   = 8, // Four of a Kind
    StraightFlushLevel = 9, // same color, sort 
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
   //Point& operator--();       // Prefix decrement operator.
   //Point operator--(int);     // Postfix decrement operator.

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
    {}

    ~Server2ClientMsg()
    {}

    Server2ClientMsg(const Server2ClientMsg& msg)
    {
        this->playerId = msg.playerId;
        this->bonusPool = msg.bonusPool;
        this->curBet = msg.curBet;
        this->behindPlayerCount = msg.behindPlayerCount;
        this->isBlind = msg.isBlind;
    }

    bool operator==(const Server2ClientMsg& msg)
    {
        if (this->playerId == msg.playerId
            && this->bonusPool == msg.bonusPool
            && this->curBet == msg.curBet
            && this->behindPlayerCount == msg.behindPlayerCount
            && this->isBlind == msg.isBlind)
        {
            return true;
        }

        return false;
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
    int  bet;

    Client2ServerMsg()
    {}

    ~Client2ServerMsg()
    {}

    Client2ServerMsg(const Client2ServerMsg& msg)
    {
        this->PlayerId = msg.PlayerId;
        this->isFold = msg.isFold;
        this->isAllIn = msg.isAllIn;
        this->bet = msg.bet;
    }

    bool operator==(const Client2ServerMsg& msg)
    {
        if(this->PlayerId == msg.PlayerId
            && this->isFold == msg.isFold
            && this->isAllIn == msg.isAllIn
            && this->bet == msg.bet)
        {
            return true;
        }
        return false;
    }
};

ostream &operator<<(ostream &os, const Client2ServerMsg &ob)
{
    os << "  Client2ServerMsg " << endl;

    return os;
}

#endif