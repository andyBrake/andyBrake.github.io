#ifndef RULER_H
#define RULER_H


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "types.h"

using namespace std;

#define STATIC   static

/***************************************************************
The structure to describe the Power of a group card - we call it card set.
Each card set (5 public card + 2 player private card) should have a power value
to indicate how powerful it is. It's easy to compare two card set through the power
***************************************************************/
struct CardPower
{
    CardLevel level;
    
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
    {
    }

    void display()
    {
        cout<<"The Card Level is "<<level<<", then the key :"<<endl;
        cout<<"\tKey1 : "<<key1<<", Key2 : "<<key2<<", Key3 : "<<key3<<", Key4 : "<<key4<<", Key5 : "<<key5<<endl;
    }
};

//typedef bool (*checkFunc)(const Card cardSet[], CardPower &cardPower);
using checkFunc = bool (*)(const Card cardSet[], CardPower &cardPower);

/***************************************************************
The class to arbit which card set is stronger based on their power value.
As well, it provide these method to judge the possibility of card set win.

Notes: The Ruler function must be staic in current design
***************************************************************/
class Ruler
{

public:
    Ruler() {}
    ~Ruler() {}

    /* These check function group to determinate the Card Set Level */
    STATIC bool checkStraightFlush(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkFourKind(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkWholeHouse(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkFlush(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkStraight(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkSet(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkTwoPairs(const Card cardSet[], CardPower &cardPower);
    STATIC bool checkOnePairs(const Card cardSet[], CardPower &cardPower);
    STATIC bool confirmHighCard(const Card cardSet[], CardPower &cardPower);


    /* To calculate the card set power value through the CardPower */    
    static int getPower(CardPower &cardPower)
    {
        return (cardPower.level * cLevelWeight + cardPower.key1 * cKey1Weight + cardPower.key2 * cKey2Weight + cardPower.key3 * cKey3Weight + cardPower.key4 * cKey4Weight + cardPower.key5);
    }

    /* The key method of Ruler, to confirm the card set level, and the related Key value */
    static void confirmPower(const Card cardSet[], CardPower &cardPower)
    {
        static checkFunc funcArray[Ruler::cLevelCount] =
            {
                checkStraightFlush,
                checkFourKind,
                checkWholeHouse,
                checkFlush,
                checkStraight,
                checkSet,
                checkTwoPairs,
                checkOnePairs,
                confirmHighCard};

        static const string levelName[Ruler::cLevelCount] =
            {
                "StraightFlush",
                "FourKind",
                "WholeHouse",
                "Flush",
                "Straight",
                "Set",
                "TwoPair",
                "OnePair",
                "HighCard"};

        bool bRet = false;

        for (int i = 0; i < Ruler::cLevelCount; i++)
        {
            cardPower.level = Unkown;
            cardPower.key1 = cardPower.key2 = cardPower.key3 = cardPower.key4 = cardPower.key5 = 0;

            bRet = funcArray[i](cardSet, cardPower);
            if (bRet)
            {
                cout << "Confirm Level " << levelName[i] << endl;
                Ruler::showCardPowerInfo(cardPower);

                return;
            }
        }
    }
    
public:
    static const int cLevelCount = TotalLevelCount;
    static const int cLevelWeight = 1000000;  // Level Weight must be much bigger
    static const int cKey1Weight = 20000;
    static const int cKey2Weight = 2000;
    static const int cKey3Weight = 200;
    static const int cKey4Weight = 20;
    static const int cMaxCardNum = (int)eCardSetNum;
    static const int cStraightLength = 5;

    static int calCardSetPower(const Card cardSet[], const int cardCount)
    {
        CardPower cardPower;
        int power = 0;

        confirmPower( cardSet, cardPower);

        power = getPower(cardPower);

        return power;
    }

    // Sort in card value decrement order
    static void sortCardSet(Card cardSet[], CardNumber cardCnt = eCardSetNum)
    {
        for (int i = 0; i < (int)cardCnt; i++)
        {
            for (int j = i; j < (int)cardCnt; j++)
            {
                if (cardSet[i].value < cardSet[j].value)
                {
                    CardValue v = cardSet[i].value;
                    CardColor col = cardSet[i].color;

                    cardSet[i].value = cardSet[j].value;
                    cardSet[i].color = cardSet[j].color;

                    cardSet[j].value = v;
                    cardSet[j].color = col;
                }
            }
        }
    }

    static void showCardPowerInfo(const CardPower &cardPower)
    {
        printf("Card Power, Level : %u\n\tKey 1 : %d, Key 2 : %d, Key 3 : %d, Key 4 : %d, Key 5 : %d\n\n",
               (unsigned int)cardPower.level,
               cardPower.key1,
               cardPower.key2,
               cardPower.key3,
               cardPower.key4,
               cardPower.key5);
    }

};

/********************************************************************************************
****************** The implement of the method
********************************************************************************************/

bool Ruler::checkStraightFlush(const Card cardSet[], CardPower &cardPower)
{
    /* Because the Straight must have 5 Card, the head should start at most in index 2 Card */
    for (int i = 0; i < 3; i++)
    {
        int head = i;
        for (int j = i; j < Ruler::cMaxCardNum; j++)
        {
            if ((j - i) == (Ruler::cStraightLength + 1))
            {
                cardPower.level = StraightFlushLevel;
                cardPower.key1 = (int)cardSet[head].value;
                return true;
            }

            if ((cardSet[j].color == cardSet[head].color) && ((int)(cardSet[i].value - cardSet[j].value) == (j - i)))
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
            CardValue value1 = CV_1;

            for (int i = Ruler::cMaxCardNum - 1; i >= 0; i--)
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

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        len = 1;

        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len++;
                if (4 == len)
                {
                    //cout << "Confirm Four Kind " << cardSet[j].value << endl;
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
    CardValue cardVal3 = CV_INV;

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        len0 = 1;

        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
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

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        len1 = 1;
        /* skip set value */
        if (cardVal3 == cardSet[i].value)
        {
            continue;
        }

        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                len1++;
                if (2 == len1)
                {
                    cardPower.level = WholeHouseLevel;
                    cardPower.key1 = (int)cardVal3;
                    cardPower.key2 = (int)cardSet[j].value;

                    //cout << "Whole House : " << cardVal3 << ", " << cardSet[j].value;

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
    CardValue key4Color[] = {CV_INV, CV_INV, CV_INV, CV_INV};

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        int index = (int)(cardSet[i].color);
        len4Color[index]++;

        if (key4Color[index] == CV_INV)
        {
            key4Color[index] = cardSet[i].value;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (len4Color[i] >= 5)
        {
            cardPower.level = FlushLevel;
            cardPower.key1 = (int)key4Color[i];
            return true;
        }
        // cout<<"The color "<<i<<" card count is "<<len4Color[i]<<" Key is "<<key4Color[i]<<endl;
    }

    return false;
}

bool Ruler::checkStraight(const Card cardSet[], CardPower &cardPower)
{
    for (int i = 0; i < 3; i++)
    {
        int head = i;

        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
        {
            if ((j - i) == (Ruler::cStraightLength + 1))
            {
                cardPower.level = FlushLevel;
                cardPower.key1 = (int)cardSet[head].value;
                return true;
            }

            if ((int)(cardSet[i].value - cardSet[j].value) == (j - i))
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
            CardValue value1 = CV_1;

            for (int i = Ruler::cMaxCardNum - 1; i >= 0; i--)
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

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        len = 1;

        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
        {
            if (cardSet[j].value != cardSet[i].value)
            {
                break;
            }

            len++;
            if (3 == len)
            {
                cardPower.level = SetLevel;
                cardPower.key1 = (int)cardSet[j].value;

                for (int k = 0; k < Ruler::cMaxCardNum; k++)
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
    }

    return false;
}

bool Ruler::checkTwoPairs(const Card cardSet[], CardPower &cardPower)
{
    CardValue key4Pair[2] = {CV_INV, CV_INV};
    int pairIndex = 0;

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
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
                    for (int k = 0; k < Ruler::cMaxCardNum; k++)
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

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
    {
        for (int j = i + 1; j < Ruler::cMaxCardNum; j++)
        {
            if (cardSet[i].value == cardSet[j].value)
            {
                cardPower.level = OnePairsLevel;
                cardPower.key1 = (int)cardSet[i].value;
                for (int k = 0; k < Ruler::cMaxCardNum; k++)
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

    for (int i = 0; i < Ruler::cMaxCardNum; i++)
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

#endif