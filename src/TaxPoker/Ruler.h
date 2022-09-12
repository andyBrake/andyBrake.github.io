
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

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

typedef bool (*checkFunc)(const Card cardSet[], CardPower &cardPower);

//checkFunc = bool (*)(const Card cardSet[], CardPower &cardPower);


class Ruler
{
 
    private:
    Ruler(){}
    ~Ruler(){}


    public:
    static const int cLevelCount  = 9; // total 9 level
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

    static bool checkStraightFlush(const Card cardSet[], CardPower &cardPower);

    static bool checkFourKind(const Card cardSet[],  CardPower &cardPower);

    static bool checkWholeHouse(const Card cardSet[],  CardPower &cardPower);

    static bool checkFlush(const Card cardSet[],  CardPower &cardPower);

    static bool checkStraight(const Card cardSet[],  CardPower &cardPower);

    static bool checkSet(const Card cardSet[],  CardPower &cardPower);

    static bool checkTwoPairs(const Card cardSet[],  CardPower &cardPower);

    static bool checkOnePairs(const Card cardSet[],  CardPower &cardPower);

    static bool confirmHighCard(const Card cardSet[],  CardPower &cardPower);

    static void showCardPowerInfo(const CardPower &cardPower)
    {
        printf("Card Power, Level : %u\n\tKey 1 : %d, Key 2 : %d, Key 3 : %d, Key 4 : %d, Key 5 : %d\n\n", 
            (unsigned int)cardPower.level,
            cardPower.key1,
            cardPower.key2,
            cardPower.key3,
            cardPower.key4,
            cardPower.key5
            );
    }

    int getPower(CardPower &cardPower)
    {
        return (cardPower.level * cLevelWeight + cardPower.key1 * cKey1Weight + cardPower.key2 * cKey2Weight \
        + cardPower.key3 * cKey3Weight + cardPower.key4 * cKey4Weight + cardPower.key5);
    }

    static void confirmPower(const Card cardSet[],  CardPower &cardPower)
    {
        constexpr static checkFunc funcArray[Ruler::cLevelCount] = 
        {
            checkStraightFlush,
            checkFourKind,
            checkWholeHouse,
            checkFlush,
            checkStraight,
            checkSet,
            checkTwoPairs,
            checkOnePairs,
            confirmHighCard
        };

        static const string levelName[Ruler::cLevelCount] = 
        {
            "HighCard",
            "OnePair",
            "TwoPair",
            "Set",
            "Straight",
            "Flush",
            "WholeHouse",
            "FourKind",
            "StraightFlush"
        };

        bool bRet = false;

        for (int i =0; i<Ruler::cLevelCount; i++)
        {
            cardPower.level = Unkown;
            cardPower.key1 = cardPower.key2 = cardPower.key3 = cardPower.key4 = cardPower.key5 = 0;

            bRet = funcArray[i](cardSet, cardPower);
            if (bRet)
            {
                cout<<"Confirm Level "<<levelName[Ruler::cLevelCount-i]<<endl;
                Ruler::showCardPowerInfo(cardPower);

                return;
            }
        }
    }
};




bool Ruler::checkStraightFlush(const Card cardSet[], CardPower &cardPower)
{
    /* Because the Straight must have 5 Card, the head should start at most in index 2 Card */
    for (int i=0; i<3; i++)
    {
        int head = i;
        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if ((j - i) == (Ruler::cStraightLength + 1))
            {
                cardPower.level = StraightFlushLevel;
                cardPower.key1 = (int)cardSet[head].value;
                return true;
            }

            if ((cardSet[j].color == cardSet[head].color) 
                && ((int)(cardSet[i].value - cardSet[j].value) == (j-i)))
            {
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
                if ((Ruler::cMaxCardNum - i) == Ruler::cStraightLength)
                {
                    cardPower.level = StraightFlushLevel;
                    cardPower.key1 = (int)cardSet[i].value;

                    return true;
                }

                if (cardSet[i].color == cardSet[0].color && ((int)(cardSet[i].value - value1) == (Ruler::cMaxCardNum - i)))
                {
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


bool Ruler::checkFourKind(const Card cardSet[], CardPower &cardPower)
{
    int len = 1;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        len = 1;

        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len++;
                if (4 == len)
                {
                    cardPower.level = FourKindLevel;
                    cardPower.key1 = (int)cardSet[j].value;
                    return true;
                }
            }
            else
            {
                break;
            }
        }
    }

    return false;
}

bool Ruler::checkWholeHouse(const Card cardSet[], CardPower &cardPower)
{
    int len0, len1;
    eCardValue cardVal3 = CV_INV;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        len0 = 1;

        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len0++;
                if (3 == len0)
                {
                    cardVal3 = cardSet[j].value;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (CV_INV != cardVal3)
        {
            break;
        }
    }

    if (CV_INV == cardVal3)
    {
        return false;
    }

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        len1 = 1;
        /* skip set value */
        if (cardVal3 == cardSet[i].value)
        {
            continue;
        }

        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len1++;
                if (2 == len1)
                {
                    cardPower.level = WholeHouseLevel;
                    cardPower.key1 = (int)cardVal3;
                    cardPower.key2 = (int)cardSet[j].value;

                    return true;
                }
            }
            else
            {
                break;
            }
        }
    }
    return false;
}

bool Ruler::checkFlush(const Card cardSet[], CardPower &cardPower)
{
    int len4Color[] = {0, 0, 0, 0};
    eCardValue key4Color[] = {CV_INV, CV_INV, CV_INV, CV_INV};

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        int index = (int)(cardSet[i].color);
        len4Color[index]++;

        if (key4Color[index] == CV_INV)
        {
            key4Color[index] = cardSet[i].value;
        }
    }

    for (int i=0;i<4;i++)
    {
        if (len4Color[i] >= 5)
        {
            cardPower.level = FlushLevel;
            cardPower.key1 = (int)key4Color[i];
            return true;
        }
        //cout<<"The color "<<i<<" card count is "<<len4Color[i]<<" Key is "<<key4Color[i]<<endl;
    }

    return false;
}

bool Ruler::checkStraight(const Card cardSet[], CardPower &cardPower)
{
    for (int i=0; i<3; i++)
    {
        int head = i;

        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if ((j - i) == (Ruler::cStraightLength + 1))
            {
                cardPower.level = FlushLevel;
                cardPower.key1 = (int)cardSet[head].value;
                return true;
            }

            if ((int)(cardSet[i].value - cardSet[j].value) == (j-i))
            {
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
                if ((Ruler::cMaxCardNum - i) == Ruler::cStraightLength)
                {
                    cardPower.level = FlushLevel;
                    cardPower.key1 = (int)cardSet[i].value;

                    return true;
                }

                if ((int)(cardSet[i].value - value1) == (Ruler::cMaxCardNum - i))
                {
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

bool Ruler::checkSet(const Card cardSet[], CardPower &cardPower)
{
    int len = 1;

    cardPower.key1 = 0;
    cardPower.key2 = 0;
    cardPower.key3 = 0;
    cardPower.key4 = 0;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len++;
                if (3 == len)
                {
                    cardPower.level = SetLevel;
                    cardPower.key1 = (int)cardSet[j].value;
                    for (int k=0; k<Ruler::cMaxCardNum; k++)
                    {
                        if ((int)cardSet[k].value != cardPower.key1)
                        {
                            if (0 == cardPower.key2)
                            {
                                cardPower.key2 = (int)cardSet[k].value;
                            }
                            else if (0 == cardPower.key3)
                            {
                                cardPower.key3 = (int)cardSet[k].value;
                                return true;
                            }
                        }
                    }
                    
                    return true;
                }
            }
            {
                break;
            }
        }
    }

    return false;
}

bool Ruler::checkTwoPairs(const Card cardSet[], CardPower &cardPower)
{
    eCardValue key4Pair[2] = {CV_INV, CV_INV};
    int pairIndex = 0;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                key4Pair[pairIndex] = cardSet[i].value;
                pairIndex++;

                if (2 == pairIndex)
                {
                    cardPower.level = TwoPairsLevel;
                    cardPower.key1 = (int)key4Pair[0]; 
                    cardPower.key2 = (int)key4Pair[1];
                    for (int k=0; k<Ruler::cMaxCardNum; k++)
                    {
                        if (cardSet[k].value != key4Pair[0] && cardSet[k].value != key4Pair[1])
                        {
                            cardPower.key3 = (int)cardSet[k].value;
                            break;
                        }
                    }
                    
                    return true;
                }
            }
        }
    }

    return false;
}

bool Ruler::checkOnePairs(const Card cardSet[], CardPower &cardPower)
{
    cardPower.key1 = 0;
    cardPower.key2 = 0;
    cardPower.key3 = 0;
    cardPower.key4 = 0;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        for (int j=i; j<Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                cardPower.level = OnePairsLevel;
                cardPower.key1 = (int)cardSet[i].value;
                for (int k=0; k<Ruler::cMaxCardNum; k++)
                {
                    if ((int)cardSet[k].value != cardPower.key1) 
                    {
                        if (cardPower.key2 == 0)
                        {
                            cardPower.key2 = (int)cardSet[k].value;
                        }
                        else if (cardPower.key3 == 0)
                        {
                            cardPower.key3 = (int)cardSet[k].value;
                        }
                        else if (cardPower.key4 == 0)
                        {
                            cardPower.key4 = (int)cardSet[k].value;
                            break;
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool Ruler::confirmHighCard(const Card cardSet[], CardPower &cardPower)
{
    cardPower.level = HighCardLevel;
    cardPower.key1 = 0;
    cardPower.key2 = 0;
    cardPower.key3 = 0;
    cardPower.key4 = 0;
    cardPower.key5 = 0;

    for (int i=0; i<Ruler::cMaxCardNum; i++)
    {
        if (cardPower.key1 == 0)
        {
            cardPower.key1 = (int)cardSet[i].value;
        }
        else if (cardPower.key2 == 0)
        {
            cardPower.key2 = (int)cardSet[i].value;
        }
        else if (cardPower.key3 == 0)
        {
            cardPower.key3 = (int)cardSet[i].value;
        }
        else if (cardPower.key4 == 0)
        {
            cardPower.key4 = (int)cardSet[i].value;
            break;
        }
    }

    return true;
}


