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

        this->playerCount = 0;

        gameCount = 0;
        roundCount = 0;

        hostPos = -1;
        sbPos = -1;

        bounsPool = 0;
        currentLoopBet = 0;

        this->status = GS_Init;

        for (int i = 0; i < cMaxPlayerCount; i++)
        {
            allPlayer[i] = NULL;
        }

        channel.init(cMaxPlayerCount);
    }

    ~PokerTable()
    {
        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                delete allPlayer[i];
                allPlayer[i] = NULL;
            }
        }
        return;
    }

    int getPlayerCount(bool isActive = false)
    {
        /* total include waiting and playing player, active only consider playing player */
        int totalCnt = 0, activeCnt = 0;

        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                if (PS_Playing == allPlayer[i]->getStatus())
                {
                    activeCnt++;
                }
                
                totalCnt++;
            }
        }

        return isActive ? activeCnt : totalCnt;
    }

    /* Add a Robot Player */
    int addPlayer(void)
    {
        int playerId = -1;

        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] == NULL)
            {
                playerId = i;
                allPlayer[i] = new Robot(i);

                cout<<"Add Player to ID "<<playerId<<endl;
                break;
            }
        }

        return playerId;
    }

    /* Add a Remote Player */
    int addPlayer(int playerSockId, char name[])
    {
        int playerId = -1;

        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] == NULL)
            {
                playerId = i;
                RemotePlayer *pRemotePlayer = new RemotePlayer(playerId);
                pRemotePlayer->setSockId(playerSockId);
                pRemotePlayer->setName(name);
                pRemotePlayer->notifyPlayerId();
                allPlayer[i] = pRemotePlayer;

                cout<<"Add Remote Player to ID "<<playerId<<", Sock "<<playerSockId<<endl;
                break;
            }
        }

        return playerId;
    }

    void update()
    {
        int cnt = 0;
        
        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                if (PS_Waiting == allPlayer[i]->getStatus())
                {
                    allPlayer[i]->active();
                    this->playerCount++;
                    cnt++;
                }
            }
        }
        cout<<"Active Player Count "<<cnt<<endl;
        return;
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

        assert(NULL != allPlayer[this->sbPos]);
        allPlayer[this->sbPos]->setBlind();

        dealer.washCard();

        cout << "The Host pos is " << hostPos << ", the Blind pos is " << sbPos <<", Player cnt "<<this->playerCount<< endl;

        /* Set Blind bet */
        ret = acquirePlayerBlind(allPlayer[sbPos], currentLoopBet);

        ret = waitPlayerPayBlind(allPlayer[sbPos], bet);

        bounsPool += currentLoopBet;

        cout << "The Blind Bet is " << currentLoopBet << " , the bonus pool is " << bounsPool << endl;

        dealPrivateCards(sbPos);

        /* start to Pre Flop */
        this->status = GS_preFlop;
        betLoop(sbPos);

        cout << "Pre Flop finish, alive " << this->stayPlayerCount << " Player\n\n";
        cout << "The bonus pool is " << bounsPool << endl;

        assert(this->stayPlayerCount > 0);
        if (this->stayPlayerCount == 1)
        {
            cout<<"This round finish!\n";
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

            power = Ruler::calCardSetPower(cardSet, eCardSetNum);

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

            if (curPlayer->isFold() || NULL == curPlayer)
            {
                cout << "  Skip Position " << startPos << endl;

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

    int acquirePlayerBlind(Player *pPlayer, int blindBet)
    {
        int ret = 0;

        ret = pPlayer->acquirePlayerBlind(blindBet);

        return ret;
    }

    int waitPlayerPayBlind(Player *pPlayer, int &bet)
    {
        int ret = 0;
        
        ret = pPlayer->waitPlayerPayBlind(bet);

        return ret;
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

    
    // calculate the C(n, m) 
    static unsigned long getComboCount(int n, int m)
    {
        int i = 1;
        int x = 1, y = 1;
        // make sure the n >= m, otherwise swap them
        if (n < m)
        {
            int tmp = n;
            n = m;
            m = tmp;
        }

        while(i <= m)
        {
            y = y * i;
            i++;
        }

        for (i=n; i> (n-m); i--)
        {
            x = x * i;
        }
        cout<<"\t m! = "<<y<<",\t n!/(n-m)! = "<<x<<endl;

        return x/y;
    }

    static void runTest()
    {
        Dealer *pDealer = new Dealer();
        Card player0Card0 = Card(Spade,   CV_2);
        Card player0Card1 = Card(Hearts,  CV_2);
        
        Card player1Card0 = Card(Club,        CV_2);
        Card player1Card1 = Card(Diamond, CV_2);

        cout<<"The Test for : "<<player0Card0<<player0Card1<<" VS "<<player1Card0<<player1Card1<<endl;
        
        pDealer->washCard();
        pDealer->acquireSpecialCard(player0Card0);
        pDealer->acquireSpecialCard(player0Card1); 
        pDealer->acquireSpecialCard(player1Card0);
        pDealer->acquireSpecialCard(player1Card1);

        pDealer->setPrivateCardSet(player0Card0, player0Card1, player1Card0, player1Card1 );

        pDealer->listAllPublicCardCombine();

        delete pDealer;
        pDealer = NULL;

        unsigned long testRet = getComboCount(48, 5);
        cout<<"The combo count is "<<testRet<<endl;

        return;
    }

    /****************************************************************************************************************
    Generate all versus case, and calculate their win possibility, and this case will cost hunderds of years, 
    so don't try to RUN this case directly
    *****************************************************************************************************************/
    static void runAllCaseTest()
    {
        Card cards[4];
        Dealer *pDealer = new Dealer();

        unsigned long ret = getComboCount(52, 4);
        cout<<"C(50, 4) = "<<ret<<endl;

        /* Select 4 card as a group to analysis, there are C(52, 4) combination */
        for (int i0 = 0; i0 < Dealer::cCardTotalNum; i0++)
        {
            for (int i1 = i0+1; i1< Dealer::cCardTotalNum ; i1++)
            {
                for (int i2 = i1+1; i2< Dealer::cCardTotalNum ; i2++)
                {
                    for (int i3 = i2+1; i3< Dealer::cCardTotalNum ; i3++)
                    {
                        cards[0] = pDealer->getCardbyOffset(i0);
                        cards[1] = pDealer->getCardbyOffset(i1);
                        cards[2] = pDealer->getCardbyOffset(i2);
                        cards[3] = pDealer->getCardbyOffset(i3);

                        /* To analysis the group's combination */
                        runSpecTest(pDealer, cards);
                    }
                }
            }
        
        }
        
        delete pDealer;
        return;
    }

    static void runSinglePlayerTest()
    {
        int card0, card1;
        Dealer *pDealer = new Dealer();

        pDealer->washCard();
        pDealer->acquireSpecialCard(Card(Spade,  CV_2));
        pDealer->acquireSpecialCard(Card(Hearts,  CV_2));

        cout<<Card(Spade,  CV_2)<<"  "<<Card(Hearts,  CV_2)<<endl;

         for (card0 = 0; card0 <Dealer::cCardTotalNum; card0++)
        {
            if (pDealer->isUsedOffset(card0))
            {
                continue;
            }
            // card 1
            for(card1=card0+1; card1<Dealer::cCardTotalNum; card1++)
            {
                if (pDealer->isUsedOffset(card1))
                {
                    continue;
                }
                Card c0 = pDealer->getCardbyOffset(card0);
                Card c1 = pDealer->getCardbyOffset(card1);

                pDealer->acquireSpecialCard(c0);
                pDealer->acquireSpecialCard(c1);

                 pDealer->setPrivateCardSet(Card(Spade,  CV_2), Card(Hearts,  CV_2), c0, c1);
                 cout<<" Versus  "<<c0<<" "<<c1<<endl;

                 pDealer->listAllPublicCardCombine();

                 //clear bitmap
                pDealer->clearUsedBitmap();
                pDealer->acquireSpecialCard(Card(Spade,  CV_2));
                pDealer->acquireSpecialCard(Card(Hearts,  CV_2));
            }
        }
       
        delete pDealer;
    }

private:
    Player *allPlayer[cMaxPlayerCount];
    Dealer dealer;
    Card publicCards[ePublicCardNum];
    SimpleChannel channel;

    int playerCount; // how many active player in this round game
    int gameCount;  // how many games played
    int roundCount; // how many round of game played

    int hostPos;
    int sbPos;
    int bounsPool;
    int currentLoopBet;  // how many Bet should call in current loop, if any Player raise, should increase it
    int stayPlayerCount; // how many Player still alive
    int dealedCardCount; // how many public card dealed

    GameStatus status; // the enum of game status, to indicate which state of the game, for example, before flop.

    int nextPlayerPos(int curPos) const
    {
        return (curPos + 1) % this->playerCount;
    }

    /* check if this loop can finish */
    bool isLoopEnd(int curPos) const
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
    void displayPlayerCard() const
    {
        cout << "The Player card :\n";
        for (int i = 0; i < this->playerCount; i++)
        {
            this->allPlayer[i]->displayCard();
        }
        cout << endl;
    }

    /* Assume 4 card as a group, then the 4 card can generate 6 types combination for versus */
    static void runSpecTest(Dealer *pDealer, Card  cards[4])
    {
        pDealer->clearUsedBitmap();

        for (int i=0; i<4; i++)
        {
            pDealer->acquireSpecialCard(cards[i]);
        }
        /* Value same, only the color is different, don't need to judge, because always equal power */
        if ((cards[0].value == cards[1].value) && (cards[0].value == cards[2].value) && (cards[0].value == cards[3].value))
        {
            cout<<"\n\ncombination : "<<cards[0]<<cards[1] << " VS    "<< cards[2] << cards[3] <<endl;
            cout<<"Ratio is "<<0<<" ; "<<1 <<" ; "<<0<<endl;

            return;
        }
    
        for (int i=0; i<4; i++)
        {
            for (int j= i+1; j<4; j++)
            {
                for (int m =0; m<4; m++)
                {
                    int n = 0;
                    
                    if ((m == i) ||( m == j) )
                    {
                        continue;
                    }
                    
                    while((n< 4) & ((n == i) || (n == j) || (n>=m))) n++;
                    if (n >= 4)
                    {
                        continue;
                    }

                    pDealer->setPrivateCardSet(cards[i], cards[j], cards[m], cards[n]);
                    cout<<"\n\ncombination : "<<cards[i]<<cards[j] << " VS    "<< cards[m] << cards[n] <<endl;

                    /* For the 6 types player card combination, each type has C(48, 5) situation */
                    //pDealer->listAllPublicCardCombine();
                }
            }
        }

        return;
    }
};

#endif