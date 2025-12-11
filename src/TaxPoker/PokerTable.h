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
    int addRobotPlayer(void)
    {
        int playerId = -1;
        char name[20];

        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] == NULL)
            {
                playerId = i;
                sprintf(name, "Robot %u", i);
                allPlayer[i] = new Robot(i);
                allPlayer[i]->setName(name);

                cout<<"Add Robot Player to ID "<<playerId<<endl;
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
                pRemotePlayer->setExtraLoad((void *)&this->playerDesc);
                pRemotePlayer->notifyPlayerId();
                allPlayer[i] = pRemotePlayer;

                cout<<"Add Remote Player to ID "<<playerId<<", Sock "<<playerSockId<<endl;
                break;
            }
        }

        return playerId;
    }

    /* To active these waiting player */
    void update()
    {
        char lastAction[10] = " NA ";
        this->playerCount = 0;

        /*Collect all player info firstly*/
        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                this->playerDesc.set(i, allPlayer[i]->getName(), allPlayer[i]->getTotalBet(), lastAction);
            }
            else
            {
                strcpy(this->playerDesc.desc[i], " None ");
            }
        }

        for (int i=0; i<cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                if (PS_Waiting == allPlayer[i]->getStatus())
                {
                    allPlayer[i]->active();
                }

                if (PS_Playing == allPlayer[i]->getStatus())
                {
                    this->playerCount++;
                    this->maxActivePlayerId = i;
                }
            }
        }
        cout<<"Active Player Count "<<this->playerCount<<endl;

        if (this->maxActivePlayerId + 1 != this->playerCount)
        {
            cout<<"Active Player Count "<<this->playerCount<<" doesn't match the Max Active ID "<<this->maxActivePlayerId<<endl;
            assert(0);
        }

        return;
    }

    int calculateWinner()
    {
        int allPower[cMaxPlayerCount];
        int winnerId = -1;
        int maxPower = 0;
        
        for (int i = 0; i < cMaxPlayerCount; i++)
        {
            Card cardSet[7];
            CardPower cardPower;
            int power = 0;

            if (allPlayer[i] == NULL || allPlayer[i]->isFold())
            {
                allPower[i] = 0;

                cout << "  Don't check Player " << i <<endl;
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
            
#if 0
            {
                cout<<"--------------For Debug-------------------------"<<endl;
                Card cardSet0[7] = {{Spade, CV_K}, {Diamond, CV_10}, {Hearts, CV_7}, {Hearts, CV_4}, {Spade, CV_4},      {Club, CV_3},    {Diamond, CV_2}};
                Card cardSet1[7] = {{Spade, CV_A}, {Spade, CV_K}, {Diamond, CV_10}, {Hearts, CV_7}, {Diamond, CV_5}, {Hearts, CV_4}, {Diamond, CV_2}};;
                
                int _power0 = Ruler::calCardSetPower(cardSet0, eCardSetNum);

                int _power1 = Ruler::calCardSetPower(cardSet1, eCardSetNum);

                cout<<"Power 0 "<<_power0<<" , Power 1 "<<_power1<<endl;
            }
#endif
        }

        cout << endl;
        for (int i = 0; i < cMaxPlayerCount; i++)
        {
            if (allPower[i] > maxPower)
            {
                maxPower = allPower[i];
                winnerId = i;
            }
            
            cout << "Player " << i << " power is " << allPower[i] << endl;
        }

        return winnerId;
    }

    void initPlayerRoundInfo()
    {
         for (int i = 0; i < cMaxPlayerCount; i++)
        {
            if (allPlayer[i] != NULL)
            {
                allPlayer[i]->setPayBet(0);
            }
        }
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
        assert(0 == ret);
        ret = waitPlayerPayBlind(allPlayer[sbPos], bet);
        assert(0 == ret);

        bounsPool += currentLoopBet;
        /* to deal all player private cards */
        dealPrivateCards(sbPos);
        /* start to Pre Flop */
        this->status = GS_preFlop;
        betLoop(sbPos);

        cout << "Pre Flop finish, alive " << this->stayPlayerCount << " Player, the bonus pool is "<<bounsPool<<"\n\n";

        assert(this->stayPlayerCount > 0);
        if (this->stayPlayerCount == 1)
        {
            moveBouns();
            return;
        }

        /* start to Post Flop */
        for (this->status = GS_postFlop; this->status < GS_Final; ++(this->status))
        {
            cout << "==============================================================="<<endl;
            cout << "==                 Start to process " << strOfStatus[(int)this->status] << ", " 
                 << this->status << " ...." << endl;
            cout << "==                 Still have " << this->stayPlayerCount << " Player alive" << endl;
            cout << "==============================================================="<<endl;

            initPlayerRoundInfo();

            isFinish = processRound();
            if (isFinish)
            {
                moveBouns();
                return;
            }
        }

        /* start to check winner */
        cout << "==============================================================="<<endl;
        cout << "    The status " << strOfStatus[(int)this->status] << endl;
        cout << "    Must check private cards to determine Winenr \n";
        cout << "==============================================================="<<endl;

        displayPublicCard();
        displayPlayerCard();

        int winnerId = calculateWinner();
        moveBouns(winnerId);

        // TODO :
        return;
    }

    void dealPrivateCards(int startPos)
    {
        Card privateCard[ePrivateCardNum];

        dealer.splitCard(10); // TODO : only split random

        cout<<"Deal all playing player private cards...."<<endl;
        
        /* deal each player private 2 card */
        int pos = startPos;
        do
        {
            dealer.dealHoleCards(privateCard);  // Use high-level function
            Player *curPlayer = allPlayer[pos];
            if (NULL == curPlayer)
            {
                continue;
            }

            // TODO, to send the card to player x
            //cout << "Dealer send Player " << pos << " Card: " << privateCard[0] << ", " << privateCard[1] << endl;
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
        
        // Use high-level deal functions for better readability
        if (this->status == GS_postFlop)
        {
            dealer.dealFlop(&this->publicCards[this->dealedCardCount]);
        }
        else if (this->status == GS_Turn)
        {
            dealer.dealTurn(this->publicCards[this->dealedCardCount]);
        }
        else if (this->status == GS_River)
        {
            dealer.dealRiver(this->publicCards[this->dealedCardCount]);
        }

        /* notify all player this public cards info */
        for (int i=0; i<cMaxPlayerCount; i++)
        {
            Player *player = this->allPlayer[i];
            
            if (NULL == player)
            {
                continue;
            }
            player->notifyPublicCards(dealCardNum, &this->publicCards[this->dealedCardCount]);
        }

        #if 0
        for (int i = 0; i < dealCardNum; i++)
        {
            cout << "Dealer deal public Card, index = " << i + this->dealedCardCount << ", "
                 << this->publicCards[this->dealedCardCount + i] << " ." << endl;
        }
        #endif

        this->dealedCardCount += dealCardNum;

        betLoop(this->hostPos);

        assert(this->stayPlayerCount > 0);
        if (this->stayPlayerCount == 1)
        {
            return true;
        }

        return false;
    }

    // return true if the game finish
    int betLoop(int endPos) // the endPos means this round should be end in which pos
    {
        int startPos = nextPlayerPos(endPos);
        int requireBet = 0;
        int currentBounsPool = this->bounsPool;
        int behindPlayerCount = this->stayPlayerCount - 1;
        int ret = 0, bet = 0;

        cout << "In Bet Loop, status " << strOfStatus[(int)this->status] << ", start pos " << startPos << ", end pos " << endPos << endl;
        do
        {
            bool isFold = false;
            bool isAllin = false;
            bool isRaise = false;

            Player *curPlayer = allPlayer[startPos];

            if (curPlayer->isFold() || NULL == curPlayer)
            {
                cout << "  Skip Position " << startPos << endl;

                if (isLoopEnd(startPos, endPos))
                {
                    break;
                }

                startPos = nextPlayerPos(startPos);
                continue;
            }

            cout << "\n\tTo sync with Player " << curPlayer->getId() << endl;

            if (this->currentLoopBet >= curPlayer->getPayBet())
            {
                requireBet = this->currentLoopBet - curPlayer->getPayBet();
            }
            else
            {
                requireBet = 0;
                cout<<"Current Loop Bet "<<this->currentLoopBet<<", Player Pay Bet "<<curPlayer->getPayBet()<<endl;
                assert(0);
            }

            ret = acquirePlayerAction(*curPlayer, requireBet, currentBounsPool, behindPlayerCount);

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
                if (bet > requireBet)
                {
                    isRaise = true;
                    endPos = prevPlayerPos(startPos);
                    behindPlayerCount = this->stayPlayerCount - 1;
                    this->currentLoopBet = bet;

                    cout<<"  Player "<<curPlayer->getId()<<"  Raise to "<<this->currentLoopBet<<endl;
                }
                else if (bet == requireBet)
                {
                    cout<<"  Player "<<curPlayer->getId()<<" Pay "<<requireBet<<"  to Call "<<this->currentLoopBet<<endl;
                }
                else
                {
                    /* impossible if bet < require bet */
                    cout<<"Bet "<<bet<<" , RequireBet "<<requireBet<<" , ERROR!!!!"<<endl;
                    assert(0);
                }

                currentBounsPool += bet;
                behindPlayerCount--;
            }

            if (isLoopEnd(startPos, endPos))
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
        int winnerCnt = 0;
        int winnerID = 0;
        
        cout << "Already has Winner!!!" << endl;
        
        for (int i = 0; i < playerCount; i++)
        {
            if (!allPlayer[i]->isFold())
            {
                winnerCnt++;
                winnerID = i;
            }
        }
        assert(1 == winnerCnt);

        moveBouns(winnerID);

        return;
    }

    void moveBouns(int winnerId)
    {
        Player *winPlayer = allPlayer[winnerId];
        
        cout<<"Move to Winner Player ID : "<<winnerId<<", bouns : "<<this->bounsPool<<endl;

        winPlayer->adjustBet(this->bounsPool);
        return;
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
        if (0 == ret)
        {
            pPlayer->setPayBet(bet);
        }

        return ret;
    }

    int acquirePlayerAction(Player &player, int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {
        cout << "Player " << player.getId() << " to make decision, bet " << currentLoopBet <<" to Call"<< endl;

        player.acquireAction(currentLoopBet, currentBounsPool, behindPlayerCount, this->status);

        return 0;
    }

    // dead loop for waiting for player Id action
    int waitPlayerAction(Player &player, bool &isFold, bool &isAllin, int &bet)
    {
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
    PlayerDesc playerDesc;

    int playerCount; // how many active player in this round game
    int maxActivePlayerId; // the max Player ID in active status
    int gameCount;  // how many games played
    int roundCount; // how many round of game played

    int hostPos;
    int sbPos;
    int bounsPool;
    int currentLoopBet;  // how many Bet should call in current loop, if any Player raise, should increase it
    int stayPlayerCount; // how many Player still alive
    int dealedCardCount; // how many public card dealed

    GameStatus status; // the enum of game status, to indicate which state of the game, for example, before flop.

    int prevPlayerPos(int curPos) const
    {
        return (curPos - 1 + this->playerCount) % this->playerCount;
    }

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



    bool isLoopEnd(int curPos, int endPos) const
    {
        return (curPos == endPos);
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