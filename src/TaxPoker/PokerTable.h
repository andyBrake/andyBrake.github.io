#ifndef POKER_TABLE_H
#define POKER_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "types.h"
#include "Player.h"
#include "Dealer.h"
#include "Ruler.h"
#include "SimpleChannel.h"

using namespace std;

static const string strOfStatus[] = {"Init", "Pre Flop", "Post Flop", "Turn", "River", "Final"};

class PokerTable
{
public:
    static const int cMaxPlayerCount = 8;
    static const int cBlindBet = 5;

    PokerTable(int playerCount)
    {
        assert(playerCount <= cMaxPlayerCount);

        this->playerCount = playerCount;

        gameCount = 0;
        roundCount = 0;

        hostPos = -1;
        sbPos = -1;

        bounsPool = 0;
        currentLoopBet = 0;

        this->status = GS_Init;

        /* Default, all player are robot */
        for (int i = 0; i < cMaxPlayerCount; i++)
        {
            allPlayer[i] =new Robot(i);
        }

        channel.init(cMaxPlayerCount);
    }

    ~PokerTable()
    {
    }

    /* start a match, return if generate winner */
    void startGame()
    {
        int ret = 0;
        bool isFold = false;
        bool isAllin = false;
        bool isFinish = false;
        int bet = cBlindBet;

        /* set Host Pos */
        this->hostPos = (1 + hostPos) % this->playerCount;
        this->sbPos = (hostPos + 1) % this->playerCount;
        this->currentLoopBet = cBlindBet;
        this->bounsPool = 0;
        this->dealedCardCount = 0;
        this->stayPlayerCount = this->playerCount;

        dealer.washCard();

        cout << "The Host pos is " << hostPos << ", the Blind pos is " << sbPos << endl;

#if 1 // for test
        dealer.acquireSpecialCard(Card(Spade,   CV_K));
        dealer.acquireSpecialCard(Card(Hearts,  CV_K)); 
        dealer.acquireSpecialCard(Card(Club,    CV_A));
        dealer.acquireSpecialCard(Card(Diamond, CV_A));

        dealer.setPrivateCardSet(Card(Spade,    CV_K),
                                 Card(Hearts,   CV_K),
                                 Card(Club,     CV_A),
                                 Card(Diamond,  CV_A)
                                );
#if 0
        // for test, disable some card here
        dealer.acquireSpecialCard(Card(Spade, CV_10));
        dealer.acquireSpecialCard(Card(Spade, CV_J)); 
        dealer.acquireSpecialCard(Card(Spade, CV_Q));
        dealer.acquireSpecialCard(Card(Spade, CV_K));
        dealer.acquireSpecialCard(Card(Hearts, CV_10));
        dealer.acquireSpecialCard(Card(Hearts, CV_J)); 
        dealer.acquireSpecialCard(Card(Hearts, CV_Q));
        dealer.acquireSpecialCard(Card(Hearts, CV_K));
        dealer.acquireSpecialCard(Card(Club, CV_10));
        dealer.acquireSpecialCard(Card(Club, CV_J)); 
        dealer.acquireSpecialCard(Card(Club, CV_Q));
        dealer.acquireSpecialCard(Card(Club, CV_K));
        dealer.acquireSpecialCard(Card(Diamond, CV_10));
        dealer.acquireSpecialCard(Card(Diamond, CV_J)); 
        dealer.acquireSpecialCard(Card(Diamond, CV_Q));
        dealer.acquireSpecialCard(Card(Diamond, CV_K));
#endif

        dealer.listAllCardCombine();

        unsigned long testRet = dealer.getComboCount(48, 5);
        cout<<"The combo count is "<<testRet<<endl;
        exit(0);
#endif
        /* Set Blind bet */
        ret = acquirePlayerBlind(*allPlayer[sbPos], currentLoopBet);

        ret = waitPlayerPayBlind(*allPlayer[sbPos], bet);

        bounsPool += currentLoopBet;

        cout << "The Blind Bet is " << currentLoopBet << " , the bonus pool is " << bounsPool << endl;

        dealPrivateCards(sbPos);

        /* start to Pre Flop */
        this->status = GS_preFlop;
        betLoop(sbPos);

        cout << "Pre Flop finish, alive " << this->stayPlayerCount << " Player\n"
             << endl;
        cout << "The bonus pool is " << bounsPool << endl;

        assert(this->stayPlayerCount > 0);
        if (this->stayPlayerCount == 1)
        {
            moveBouns();
            return;
        }

        /* start to Post Flop */
        for (this->status = GS_postFlop; this->status < GS_Final; ++(this->status))
        {
            cout << endl;
            cout << "===Start to process " << strOfStatus[(int)this->status] << ", " << this->status << " ...." << endl;
            cout << "\tStill have " << this->stayPlayerCount << " Player alive" << endl;

            isFinish = processRound();
            if (isFinish)
            {
                cout << "Already has Winner!!!" << endl;
                return;
            }
        }

        /* start to check winner */
        cout << "The status " << strOfStatus[(int)this->status] << ", " << this->status << endl;
        cout << "Must check private cards to determine Winenr \n";

        displayPublicCard();
        displayPlayerCard();

        int allPower[8];
        for (int i = 0; i < playerCount; i++)
        {
            Card cardSet[7];
            CardPower cardPower;
            int power = 0;

            if (allPlayer[i]->isFold())
            {
                allPower[i] = 0;

                cout << "  Don't check Player " << i << " due to Fold\n";
                continue;
            }

            for (int index = 0; index < (int)ePublicCardNum; index++)
            {
                cardSet[index] = this->publicCards[index];
            }

            this->allPlayer[i]->getCard(cardSet[ePublicCardNum], cardSet[ePublicCardNum + 1]);

            Ruler::sortCardSet(cardSet);

            cout << "Player " << i << " After Sort\n";
            for (int j = 0; j < 7; j++)
            {
                cout << cardSet[j] << endl;
            }
            cout << endl;

            Ruler::confirmPower(cardSet, cardPower);

            power = Ruler::getPower(cardPower);
            allPower[i] = power;
        }

        cout << endl;
        for (int i = 0; i < 8; i++)
        {
            cout << "Player " << i << " power is " << allPower[i] << endl;
        }

        // TODO :
        return;
    }

    void dealPrivateCards(int startPos)
    {
        Card privateCard[ePrivateCardNum];

        dealer.splitCard(10); // TODO : only split random
        /* deal each player private 2 card */
        int pos = startPos;
        do
        {
            dealer.dealCard(ePrivateCardNum, privateCard);
            Player *curPlayer = allPlayer[pos];

            // TODO, to send the card to player x
            cout << "Dealer send Player " << pos << " Card: " << privateCard[0] << ", " << privateCard[1] << endl;
            curPlayer->setCard(privateCard[0], privateCard[1]);

            /* shift to next player */
            pos = nextPlayerPos(pos);
        } while (pos != startPos);

        return;
    }

    void getDealCount(int &dealCardNum, int &splitCardNum)
    {
        switch (this->status)
        {
        case GS_postFlop:
        {
            dealCardNum = eFlopCardNum;
            splitCardNum = 7;
            break;
        }
        case GS_Turn:
        {
            dealCardNum = eTurnCardNum;
            splitCardNum = 1;
            break;
        }
        case GS_River:
        {
            dealCardNum = eRiverCardNum;
            splitCardNum = 1;
            break;
        }

        default:
            cout << "Invalid Status " << this->status << endl;
            assert(0);
        }
        return;
    }

    bool processRound()
    {
        int dealCardNum = 0;
        int splitCardNum = 0;

        getDealCount(dealCardNum, splitCardNum);

        dealer.splitCard(splitCardNum);
        dealer.dealCard(dealCardNum, &this->publicCards[this->dealedCardCount]);
        for (int i = 0; i < dealCardNum; i++)
        {
            cout << "Dealer deal public Card, index = " << i + this->dealedCardCount << ", "
                 << this->publicCards[this->dealedCardCount + i] << " ." << endl;
        }

        this->dealedCardCount += dealCardNum;

        betLoop(this->hostPos);

        assert(this->stayPlayerCount > 0);
        if (this->stayPlayerCount == 1)
        {
            moveBouns();
            return true;
        }

        return false;
    }

    // return true if the game finish
    int betLoop(int endPos) // the endPos means this round should be end in which pos
    {
        int startPos = nextPlayerPos(endPos);
        int currentLoopBet = 0;
        int currentBounsPool = this->bounsPool;
        int behindPlayerCount = this->stayPlayerCount - 1;
        int ret = 0, bet = 0;

        cout << "In Bet Loop, status " << this->status << ", start pos " << startPos << ", end pos " << endPos << endl;
        do
        {
            bool isFold = false;
            bool isAllin = false;
            bool isRaise = false;

            Player *curPlayer = allPlayer[startPos];

            if (curPlayer->isFold())
            {
                cout << "  Skip fold Player " << curPlayer->getId() << endl;

                if (isLoopEnd(startPos))
                {
                    break;
                }

                startPos = nextPlayerPos(startPos);
                continue;
            }

            cout << "\n\tTo sync with Player " << curPlayer->getId() << endl;
            /* In pre flop, the blind player don't bet again in default */
            if (this->status == GS_preFlop && curPlayer->getBlind())
            {
                currentLoopBet = 0;
                bet = currentLoopBet;
            }
            else
            {
                currentLoopBet = this->currentLoopBet;
                bet = currentLoopBet;
            }

            ret = acquirePlayerAction(*curPlayer, currentLoopBet, currentBounsPool, behindPlayerCount);

            ret = waitPlayerAction(*curPlayer, isFold, isAllin, bet);
            assert(0 == ret);
            // if player fold, don't need to update bonus pool
            if (isFold)
            {
                curPlayer->fold();
                this->stayPlayerCount--;
                behindPlayerCount--;
            }
            else
            {
                /* If anyone Raise, he must be the new end pos */
                if (bet > currentLoopBet)
                {
                    isRaise = true;
                    endPos = startPos;
                    currentLoopBet = bet;
                    behindPlayerCount = this->stayPlayerCount;
                }

                currentBounsPool += currentLoopBet;
                behindPlayerCount--;
            }

            if (isLoopEnd(startPos))
            {
                break;
            }
            else
            {
                startPos = nextPlayerPos(startPos);
            }
        } while (1);

        this->currentLoopBet = 0;
        this->bounsPool = currentBounsPool;
        cout << "The current bouns pool is " << this->bounsPool << endl;

        return 0;
    }

    // give bonus to winner
    void moveBouns()
    {
    }

    int acquirePlayerBlind(Player &player, int blindBet)
    {
        int ret = 0;

        ret = player.acquirePlayerBlind(blindBet);
#if 0
        Server2ClientMsg msg;

        msg.playerId = player.getId();
        msg.isBlind = true;
        msg.curBet = blindBet;
        msg.behindPlayerCount = this->playerCount;
        msg.bonusPool = this->bounsPool;
        //ret = channel.send2Player(msg.playerId, msg);
#endif
        return ret;
    }

    int waitPlayerPayBlind(Player &player, int &bet)
    {
        player.waitPlayerPayBlind(bet);

        return 0;
    }

    int acquirePlayerAction(Player &player, int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {
        // cout << "Player " << player.getId() << " to make decision, bet " << currentLoopBet << endl;

        return 0;
    }

    // dead loop for waiting for player Id action
    int waitPlayerAction(Player &player, bool &isFold, bool &isAllin, int &bet)
    {
        player.acquireAction(this->currentLoopBet, 0, 0);
        player.getAction(isFold, isAllin, bet);

        cout << "Player " << player.getId() << " did the action, bet " << bet << ", isFold " << isFold << endl;

        return 0;
    }

private:
    Player *allPlayer[cMaxPlayerCount];
    Dealer dealer;
    Card publicCards[ePublicCardNum];
    SimpleChannel channel;

    int playerCount;
    int gameCount;  // how many games played
    int roundCount; // how many round of game played

    int hostPos;
    int sbPos;
    int bounsPool;
    int currentLoopBet;  // how many Bet should call in current loop, if any Player raise, should increase it
    int stayPlayerCount; // how many Player still alive
    int dealedCardCount; // how many public card dealed

    GameStatus status; // the enum of game status, to indicate which state of the game, for example, before flop.

    int nextPlayerPos(int curPos)
    {
        return (curPos + 1) % this->playerCount;
    }

    /* check if this loop can finish */
    bool isLoopEnd(int curPos)
    {
        /* in pre flop,the blind pos is the last one */
        if (this->status == GS_preFlop)
        {
            if (curPos == nextPlayerPos(this->hostPos))
            {
                return true;
            }
        }
        else // otherwise, the host pos is the last one
        {
            if (curPos == this->hostPos)
            {
                return true;
            }
        }
        return false;
    }

    void displayPublicCard()
    {
        cout << "The public card :\n";
        for (int i = 0; i < this->dealedCardCount; i++)
        {
            cout << "\t";
            cout << publicCards[i] << " " << endl;
        }
        cout << endl;
    }
    void displayPlayerCard()
    {
        cout << "The Player card :\n";
        for (int i = 0; i < this->playerCount; i++)
        {
            this->allPlayer[i]->displayCard();
        }
        cout << endl;
    }
};

#endif