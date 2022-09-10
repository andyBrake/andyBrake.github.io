#ifndef POKER_TABLE_H
#define POKER_TABLE_H

#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "types.h"
#include "Player.h"


class PokerTable
{
    typedef ProFunc = void (*func)();

    public:
    static const int cMaxPlayerCount = 8;
    static const int cBlindBet = 5;

    PokerTable(int palyerCount)
    {
        assert(playerCount <= cMaxPlayerCount);

        this->playerCount = playerCount;

        gameCount = 0; 
        roundCount = 0;
        
        hostPos = -1;
        sbPos = -1;

        bounsPool = 0;
        currentLoopBet = 0;

        status = GS_Init;

        for (int i=0; i<cMaxPlayerCount;i++)
        {
            allPlayer[i].init();
        }
    }

    ~PokerTable()
    {}

    void startGame()
    {
        int ret = 0;

        /* set Host Pos */
        hostPos = (1 + hostPos)%this->playerCount;
        sbPos = (hostPos + 1)%this->playerCount;
        currentLoopBet = cBlindBet;
        bounsPool = 0;

        cout<<"The Host pos is "<<hostPos<<", the Blind pos is "<<sbPos<<endl;

        status = GS_preFlop;

        /* Set Blind bet */
        ret = acquirePlayerBlind(allPlayer[sbPos], currentLoopBet);

        ret = waitPlayerAction(allPlayer[sbPos]);

        betLoop(sbPos);

        if ()
        {
            
        }

    }

    // return true if the game finish
    int betLoop(int endPos) // the endPos means this round should be end in which pos 
    {
        int startPos = (endPos + 1)%this->playerCount;

        return 0;
    }

    // give bonus to winner
    void moveBouns() 
    {}

    int acquirePlayerBlind(Player &player, int blindBet)
    {}

    int acquirePlayerAction(Player &player, int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {}

    // dead loop for waiting for player Id action
    int waitPlayerAction(Player &player) 
    {}

    private:
    Player allPlayer[cMaxPlayerCount];

    int playerCount;
    int gameCount; // how many games played
    int roundCount; // how many round of game played
    
    int hostPos;
    int sbPos;
    int bounsPool;
    int currentLoopBet; // how many Bet should call in current loop, if any Player raise, should increase it

    GameStatus status; // the enum of game status, to indicate which state of the game, for example, before flop.


    ProFunc processFuncTable[];
};




#endif