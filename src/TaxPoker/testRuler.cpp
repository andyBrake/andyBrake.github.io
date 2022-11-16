
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "Ruler.h"

using namespace std;

/*
    Spade   = 0,    // 黑桃
    Hearts  = 1,    // 红桃
    Club    = 2,    // 梅花
    Diamond = 3
*/

void test4TwoPair()
{
    bool bRet = false;

    Card cardSet[7] = {Card(Spade, CV_A),
                       Card(Spade, CV_K),
                       Card(Spade, CV_K),
                       Card(Spade, CV_5),
                       Card(Spade, CV_4),
                       Card(Spade, CV_2),
                       Card(Spade, CV_2)};
    CardPower cardPower;

    cardPower.level = Unkown;
    cardPower.key1 = cardPower.key2 = cardPower.key3 = cardPower.key4 = cardPower.key5 = 0;

    cout << "Test the TwoPair case." << endl;

    bRet = Ruler::checkTwoPairs(cardSet, cardPower);
    if (bRet)
    {
        cout << "\tTure !" << endl;
    }
    else
    {
        cout << "\tFalse" << endl;
    }
    Ruler::showCardPowerInfo(cardPower);
    cout << endl;
}

void test4OnePair()
{
    bool bRet = false;

    Card cardSet[7] = {Card(Spade, CV_A),
                       Card(Hearts, CV_Q),
                       Card(Hearts, CV_10),
                       Card(Club, CV_5),
                       Card(Spade, CV_5),
                       Card(Diamond, CV_3),
                       Card(Spade, CV_2)};
    CardPower cardPower;

    cardPower.level = Unkown;
    cardPower.key1 = cardPower.key2 = cardPower.key3 = cardPower.key4 = cardPower.key5 = 0;

    cout << "Test the OnePair case." << endl;

    bRet = Ruler::checkOnePairs(cardSet, cardPower);
    if (bRet)
    {
        cout << "\tTure !" << endl;
    }
    else
    {
        cout << "\tFalse" << endl;
    }
    Ruler::showCardPowerInfo(cardPower);
    cout << endl;
}

int main()
{
    bool bRet = false;
    CardPower cardPower;
    // StraightFlush
    Card cardSet0[7] = {Card(Spade, CV_A),
                        Card(Spade, CV_K),
                        Card(Spade, CV_Q),
                        Card(Spade, CV_5),
                        Card(Spade, CV_4),
                        Card(Spade, CV_3),
                        Card(Spade, CV_2)};
    // Whole House
    Card cardSet1[7] = {Card(Spade, CV_A),
                        Card(Hearts, CV_K),
                        Card(Club, CV_K),
                        Card(Diamond, CV_K),
                        Card(Hearts, CV_4),
                        Card(Spade, CV_2),
                        Card(Diamond, CV_2)};
    // Set
    Card cardSet2[7] = { Card(Club, CV_K),
                                        Card(Diamond, CV_7),
                                        Card(Club, CV_6),
                                        Card(Diamond, CV_5),
                                        Card(Club, CV_5),
                                        Card(Spade, CV_5),
                                        Card(Hearts, CV_3)};
    // Flush
    Card cardSet3[7] = {Card(Hearts, CV_A),
                        Card(Club, CV_K),
                        Card(Diamond, CV_10),
                        Card(Diamond, CV_10),
                        Card(Diamond, CV_9),
                        Card(Hearts, CV_6),
                        Card(Diamond, CV_2)};
#if 0
    bRet = Ruler::checkStraightFlush(cardSet0, cardPower);
    if (bRet)
    {
        cout << "Straight Ture !" << endl;
    }
    else
    {
        cout << "Straight False" << endl;
    }
#endif
    
#if 0
    bRet = Ruler::checkWholeHouse(cardSet1, cardPower);
    if (bRet)
    {
        cout << "WholeHouse Ture !" << endl;
    }
    else
    {
        cout << "WholeHouse False" << endl;
    }

    bRet = Ruler::checkSet(cardSet2, cardPower);
    if (bRet)
    {
        cout << "Set Ture !" << endl;
    }
    else
    {
        cout << "Set False" << endl;
    }

    bRet = Ruler::checkFlush(cardSet3, cardPower);
    if (bRet)
    {
        cout << "Flush Ture !" << endl;
    }
    else
    {
        cout << "Flush False" << endl;
    }

    bRet = Ruler::checkFourKind(cardSet3, cardPower);
    if (bRet)
    {
        cout << "4k Ture !" << endl;
    }
    else
    {
        cout << "4k False" << endl;
    }

    test4TwoPair();

    test4OnePair();
#endif
    cout << "Test End" << endl;
}