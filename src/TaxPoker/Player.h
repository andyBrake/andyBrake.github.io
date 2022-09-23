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
        this->isBlind = false;
        this->isStay = true;
        this->totalBet = 500;
        this->isAllIn = false;

        cout << "Player " << this->id << " Init." << endl;
        return;
    }

    void setBlind()
    {
        this->isBlind = true;
    }

    void adjustBet(int change)
    {
        this->totalBet += change;
        if (this->totalBet < 0)
        {
            cout << "Invalid Total Bet" << this->totalBet << endl;
            exit(-1);
        }
    }

    bool getBlind()
    {
        return this->isBlind;
    }

    int getId()
    {
        return this->id;
    }

    void fold()
    {
        this->isStay = false;
    }

    bool isFold()
    {
        return !this->isStay;
    }

    void allin()
    {
        this->isAllIn = true;
    }

    bool isAllin()
    {
        return this->isAllIn;
    }

    void setCard(Card &card0, Card &card1)
    {
        this->cards[0] = card0;
        this->cards[1] = card1;

        return;
    }

    void getCard(Card &card0, Card &card1)
    {
        card0 = this->cards[0];
        card1 = this->cards[1];

        return;
    }

    void displayCard()
    {
        cout << "Player " << this->id << " hand-card " << this->cards[0] << ", " << this->cards[1] << endl;
    }

private:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];

    int id; // the sequence in table
    bool isBlind;
    bool isStay;
    int totalBet;
    bool isAllIn;
};

class Robot : public Player
{
public:
    Robot(int id)
    {
        init(id);

        this->response.PlayerId = getId();
        this->response.isFold = isFold();
        this->response.isAllIn = isAllin();
        this->response.bet = 0;
    }

    ~Robot()
    {
    }

    void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {
        // Blind Player just call
        if (1 == getId())
        {
            this->response.isFold = false;
            this->response.isAllIn = false;
            this->response.bet = 0;
        }
        else if (2 != getId())
        {
            this->response.isFold = true;
            this->response.isAllIn = false;
            this->response.bet = 0;
        }
        else
        {
            this->response.isFold = false;
            this->response.isAllIn = false;
            this->response.bet = currentLoopBet;
        }
    }

    void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        isFold = this->response.isFold;
        isAllin = this->response.isAllIn;
        bet = this->response.bet;

        return;
    }

private:
    Client2ServerMsg response;
};

#endif
