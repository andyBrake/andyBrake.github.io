#ifndef PLAYER_H
#define PLAYER_H


#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "types.h"

class Player
{
    public:
    Player()
    {}
    ~Player(){}

    void init()
    {

    }



    private:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];

    int Id; // the sequence in table
    bool isStay;
    int totalBet;
    bool isAllIn;
};



#endif