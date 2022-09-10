
#include <stdlib.h>
#include <iostream>

#include "types.h"

using namespace std;


struct CardPower
{
    eLevel level;
    int key1;
    int key2;
    int key3;
    int key4;
    int key5;

    CardPower()
    {
        level = Unkown;
        key1 = 0;
        key2 = 0;
        key3 = 0;
        key4 = 0;
        key5 = 0;
    }
    ~CardPower()
    {}
};

class Ruler
{
    private:
    Ruler(){}
    ~Ruler(){}

    public:
    static const int cLevelWeight = 100000;
    static const int cKey1Weight  = 10000;
    static const int cKey2Weight  = 1000;
    static const int cKey3Weight  = 100;
    static const int cKey4Weight  = 10;
    static const int cMaxCardNum  = 7; // at most, there are 7 Card
    static const int cStraightLength = 5;

    

    /* the Ruler function must be staic */
    static int calCardSetPower(const Card cardSet[], const int cardCount) // the card count should be 7
    {
        //struct Ruler::CardPower cardPower;
        int power = 0;

        //: TODO

        
        //power = getPower(cardPower);
        return power;
    }

    static bool checkStraightFlush(const Card cardSet[], CardPower *pCardPower);


#if 0
    bool checkFourKind(const Card cardSet[], struct CardPower &cardPower);

    bool checkWholeHouse(const Card cardSet[], struct CardPower &cardPower);

    bool checkFlush(const Card cardSet[], struct CardPower &cardPower);

    bool checkStraight(const Card cardSet[], struct CardPower &cardPower);

    bool checkTwoPairs(const Card cardSet[], struct CardPower &cardPower);

    bool checkOnePairs(const Card cardSet[], struct CardPower &cardPower);

    void confirmHighCard(const Card cardSet[], struct CardPower &cardPower);

#endif
    int getPower(CardPower &cardPower)
    {
        return (cardPower.level * cLevelWeight + cardPower.key1 * cKey1Weight + cardPower.key2 * cKey2Weight \
        + cardPower.key3 * cKey3Weight + cardPower.key4 * cKey4Weight + cardPower.key5);
    }
};




bool Ruler::checkStraightFlush(const Card cardSet[], CardPower *pCardPower)
{
    /* Because the Straight must have 5 Card, the head should start at most in index 2 Card */
    for (int i=0; i<3; i++)
    {
        int head = i;
        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            cout<<"j = "<<j<<", i = "<<i<<endl;

            if ((j - i) == (Ruler::cStraightLength + 1))
            {
                pCardPower->level = StraightFlushLevel;
                pCardPower->key1 = (int)cardSet[head].value;
                return true;
            }

            if ((cardSet[j].color == cardSet[head].color) 
                && ((int)(cardSet[i].value - cardSet[j].value) == (j-i)))
            {
                cout<<"UU "<<cardSet[j].value<<endl;
                continue;
            }
            else
            {
                break;
            }
        }
        /* the A could be used as 1 */
        if (cardSet[0].value == CV_A)
        {
            eCardValue value1 = CV_1;

            for (int i=Ruler::cMaxCardNum-1; i >= 0; i--)
            {
                cout<<"**** i= "<<i<<", left"<<Ruler::cMaxCardNum - i<<endl;

                if ((Ruler::cMaxCardNum - i) == Ruler::cStraightLength)
                {
                    pCardPower->level = StraightFlushLevel;
                    pCardPower->key1 = (int)cardSet[i].value;
                    cout<<" ---- "<< i<<", max = "<< Ruler::cMaxCardNum<<endl;
                    return true;
                }

                if (cardSet[i].color == cardSet[0].color && ((int)(cardSet[i].value - value1) == (Ruler::cMaxCardNum - i)))
                {
                    cout<<" i= "<<i<<endl;

                    continue;
                }
                else
                {
                    return false;
                }
            }
        }

        return false;
    }

    return false;
}




#if 0
bool checkFourKind(const Card cardSet[], struct CardPower &cardPower);

bool checkWholeHouse(const Card cardSet[], struct CardPower &cardPower);

bool checkFlush(const Card cardSet[], struct CardPower &cardPower);

bool checkStraight(const Card cardSet[], struct CardPower &cardPower);

bool checkTwoPairs(const Card cardSet[], struct CardPower &cardPower);

bool checkOnePairs(const Card cardSet[], struct CardPower &cardPower);

void confirmHighCard(const Card cardSet[], struct CardPower &cardPower);
#endif