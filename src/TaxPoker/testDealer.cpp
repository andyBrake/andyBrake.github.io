#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "Dealer.h"

using namespace std;

int main()
{
    Dealer dealer;
    int requireCount = 3;
    Card requireCards[10];

    cout<<"Start to test the Dealer!\n";

    dealer.washCard();

    dealer.splitCard();

    dealer.dealCard(requireCount, requireCards);
    for (int i=0;i<requireCount;i++)
    {
        cout<<""<<requireCards[i]<<endl;
    }

    return 0;
}