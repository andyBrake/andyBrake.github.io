#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

class Player
{
    public:
    Player()
    {

    }

    ~Player()
    {

    }

    void init(int pos)
    {
        this->id = pos;
        cout<<"Player "<<this->id<<" Init."<<endl;
        return;
    }

    int getId()
    {
        return this->id;
    }

    void fold()
    {
        this->isStay = false;
    }

    void getCard(Card &card0, Card &card1)
    {
        this->cards[0] = card0;
        this->cards[1] = card1;
        return;
    }

    void displayCard()
    {
        cout<<"Player "<<this->id<<" hand-card "<<this->cards[0]<<", "<<this->cards[1]<<endl;
    }

    private:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];

    int id; // the sequence in table
    bool isStay;
    int  totalBet;
    bool isAllIn;
};

#endif
