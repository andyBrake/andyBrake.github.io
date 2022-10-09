#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

enum PlayerType
{
    RobotPlayerType  = 0,
    RemotePlayerType = 1,

    PlayerTypeCnt
};

enum PlayerStatus
{
    PS_Invalid,
    PS_Waiting,
    PS_Playing
};

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
        this->status = PS_Waiting;

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

    virtual PlayerStatus getStatus()
    {
        return this->status;
    }

    virtual void active()
    {
        this->status = PS_Playing;
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

    virtual int acquirePlayerBlind(int blindBet)
    {
        cout<<"Default Player acquirePlayerBlind"<<endl;
        return 0;
    }

    virtual int waitPlayerPayBlind(int &bet)
    {
        cout<<"Default Player waitPlayerPayBlind"<<endl;
        return 0;
    }

    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {
        cout<<"Default Player acquire action"<<endl;
    }

    virtual void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        cout<<"Default Player get action"<<endl;
    }

protected:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];

    int id; // the sequence in table
    PlayerType type;
    PlayerStatus status;
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
        this->type = RobotPlayerType;
        
        this->response.PlayerId = getId();
        this->response.isFold = isFold();
        this->response.isAllIn = isAllin();
        this->response.bet = 0;
        this->blindBet = 0;
    }

    ~Robot()
    {
    }

    virtual int acquirePlayerBlind(int blindBet)
    {
        this->blindBet = blindBet;
        return 0;
    }

    virtual int waitPlayerPayBlind(int &bet)
    {
        bet = this->blindBet;
        this->totalBet -= bet;

        return 0;
    }


    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {
        // Blind Player just call
        if (this->isBlind)
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

    virtual void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        isFold = this->response.isFold;
        isAllin = this->response.isAllIn;
        bet = this->response.bet;

        return;
    }

private:
    Client2ServerMsg response;
    int blindBet;
};


class RemotePlayer : public Player
{
public:
    RemotePlayer(int id)
    {
        init(id);

        this->type = RemotePlayerType;

        this->response.PlayerId = getId();
        this->response.isFold = isFold();
        this->response.isAllIn = isAllin();
        this->response.bet = 0;
        this->blindBet = 0;
        this->playerSockId = -1;
    }

    ~RemotePlayer()
    {
        if (this->playerSockId != -1)
        {
            close(this->playerSockId);
        }
        return;
    }

    void setSockId(int sockId)
    {
        this->playerSockId = sockId;
    }

    virtual int acquirePlayerBlind(int blindBet)
    {
        this->blindBet = blindBet;

        memset(msg, 0, sizeof(msg));
        request.fillMsg(msg, 0, blindBet);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Player sock id "<<this->playerSockId<<endl;
        cout<<"Send Request:\n"<<msg<<endl;
        
        return 0;
    }

    virtual int waitPlayerPayBlind(int &bet)
    {
        bet = this->blindBet;
        this->totalBet -= bet;

        int loop = 10;

        do
        {
            memset(msg, 0, sizeof(msg));
            recv(this->playerSockId, (char*)msg, sizeof(msg), 0);

            cout<<"Rcv Reponse:{\n"<<msg<<"\n}"<<endl;
            sleep(3);
        }while(loop--);
        
        return 0;
    }


    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount)
    {

    }

    virtual void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        isFold = this->response.isFold;
        isAllin = this->response.isAllIn;
        bet = this->response.bet;

        return;
    }

private:
    char msg[1024];
    Server2ClientMsg request;
    Client2ServerMsg response;
    int playerSockId;
    int blindBet;
};

#endif
