

enum Color
{
    Spade = 0, // 黑桃
    Hearts,    // 红桃 
    Club,      // 梅花
    Diamond,   // 方片

    ColorNum
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
    os << "  Card : color " << ob.color << ", value " << ob.value<< " ";

    return os;
}

enum eLevel
{
    Unkown = 0,
    HighCardLevel = 1,
    OnePairsLevel,
    TwoPairsLevel,
    SetLevel,
    StraightLevel,
    FlushLevel,
    WholeHouseLevel, // 3 + 2
    FourKindLevel, // Four of a Kind
    StraightFlushLevel, // same color, sort 
};


enum GameStatus
{
    GS_Init = 0,

    GS_preFlop,
    GS_postFlop,

    GS_Turn,
    GS_River,
    GS_Final
}