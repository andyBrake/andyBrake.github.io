
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "Ruler.h"

using namespace std;

int main()
{
    bool bRet = false;
    CardPower cardPower;
    Card cardSet[7] = { Card(Spade, CV_A), 
                        Card(Spade, CV_K),
                        Card(Spade, CV_Q),
                        Card(Spade, CV_5),
                        Card(Spade, CV_4),
                        Card(Spade, CV_3),
                        Card(Spade, CV_2)};

    bRet = Ruler::checkStraightFlush(cardSet, &cardPower);
    if (bRet)
    {
        cout<<"Straight Ture !"<<endl;
    }
    else
    {
        cout<<"Straight False"<<endl;
    }
    cout<<"Test End"<<endl;

}