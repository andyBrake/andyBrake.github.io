#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "Dealer.h"

using namespace std;

int main()
{
    Dealer dealer;
    Card holeCards[2];
    Card flopCards[3];
    Card turnCard;
    Card riverCard;

    cout<<"=========================================="<<endl;
    cout<<"Test High-Level Dealer Functions"<<endl;
    cout<<"=========================================="<<endl;

    dealer.washCard();
    dealer.splitCard(5);

    // Test dealHoleCards
    cout<<"\n1. Testing dealHoleCards()..."<<endl;
    dealer.dealHoleCards(holeCards);
    cout<<"   Player hole cards: "<<holeCards[0]<<" "<<holeCards[1]<<endl;

    // Test dealFlop
    cout<<"\n2. Testing dealFlop()..."<<endl;
    dealer.dealFlop(flopCards);
    cout<<"   Flop cards: "<<flopCards[0]<<" "<<flopCards[1]<<" "<<flopCards[2]<<endl;

    // Test dealTurn
    cout<<"\n3. Testing dealTurn()..."<<endl;
    dealer.dealTurn(turnCard);
    cout<<"   Turn card: "<<turnCard<<endl;

    // Test dealRiver
    cout<<"\n4. Testing dealRiver()..."<<endl;
    dealer.dealRiver(riverCard);
    cout<<"   River card: "<<riverCard<<endl;

    cout<<"\n=========================================="<<endl;
    cout<<"All High-Level Functions Tested Successfully!"<<endl;
    cout<<"=========================================="<<endl;

    return 0;
}
