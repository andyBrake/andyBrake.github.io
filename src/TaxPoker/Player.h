#ifndef PLAYER_H
#define PLAYER_H

#include <unistd.h>       // for sleep, close
#include <sys/socket.h>   // for send, recv
#include <cstring>        // for strlen, strncpy
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

class PlayerDesc
{
    
    public:
        char desc[8][50];

        void set(int id, const char* name, int totalBet, char lastAction[])
        {
            const char *playerInfoFormat = "%s (%u) (%s) "; // name + total_bet + last_action
            sprintf(desc[id], playerInfoFormat, name, totalBet, lastAction);
            return;
        }
};

class Player
{
public:
    const static int cMaxNameLen = 20;
    
    Player()
    {
        strcpy(this->name, "Default Player");
    }

    ~Player()
    {
    }

    virtual bool isRemotePlayer()
    {
        return false;
    }

    void init(int pos)
    {
        this->id = pos;
        this->isBlind = false;
        this->isStay = true;
        this->totalBet = 500;
        this->isAllIn = false;
        this->status = PS_Waiting;
        this->currentPayBet = 0;

        return;
    }

    void setBlind()
    {
        this->isBlind = true;
    }

    void setName(char name[])
    {
        strncpy(this->name, name, cMaxNameLen);
    }

    const char *getName(void) const
    {
        return this->name;
    }

    virtual void notifyPublicCards(const int cardCnt, Card *outCards)
    {
        return;
    }

    virtual void adjustBet(int change)
    {
        this->totalBet += change;
        if (this->totalBet < 0)
        {
            cout << "Invalid Total Bet" << this->totalBet << endl;
            exit(-1);
        }
        cout<<"Player "<<this->id<<" adjust bet "<<change<<" to total "<<this->totalBet<<endl;
        return;
    }

    int getTotalBet()const
    {
        return this->totalBet;
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

    virtual void setPayBet(int currentPayBet)
    {
        this->currentPayBet = currentPayBet;
    }

    virtual int getPayBet()
    {
        return this->currentPayBet;
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

    virtual void setCard(Card &card0, Card &card1)
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

    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount, GameStatus status)
    {
        cout<<"Default Player acquire action"<<endl;
    }

    virtual void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        cout<<"Default Player get action"<<endl;
    }

    virtual void setExtraLoad(void *p)
    {
        return;
    }

    virtual void * getExtraLoad() const
    {
        return NULL;
    }

protected:
    int channelId; // the channel ID to communicate with this player client
    Card cards[2];

    int id; // the sequence in table, namely the Player ID
    char name[cMaxNameLen];
    PlayerType type;
    PlayerStatus status;
    int blindBet;
    int currentPayBet; // already pay bet in this loop
    bool isBlind;
    bool isStay;
    int totalBet;
    bool isAllIn;
};

class Robot : public Player
{
public:
    /*
        0 : always fold
        1 : just call
        2 : Raise to 10
        3 : all-in
    */
    const int cRobotStrategy = 0; 
    
    Robot(int id)
    {
        init(id);
        this->type = RobotPlayerType;
        
        this->response.Init();
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

        cout<<"Robot Player pay the Blind Bet : "<<blindBet<<", Remind Total Bet: "<<this->totalBet<<endl;

        return 0;
    }


    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount, GameStatus status)
    {
        // Blind Player just call
        if (this->isBlind)
        {
            this->response.isFold = false;
            this->response.isAllIn = false;
            
            this->response.setBet(currentLoopBet);
        }
        #if 0
        else if (2 != getId())
        {
            this->response.isFold = true;
            this->response.isAllIn = false;
            this->response.bet = 0;
        }
        #endif
        else
        {
            if (0 == cRobotStrategy)
            {
                this->response.isFold = true;
                this->response.isAllIn = false;
                
                this->response.setBet(0);
                return;
            }
            /* the robot just bet the necessary bet */
            if (1 == cRobotStrategy)
            {
                this->response.isFold = false;
                this->response.isAllIn = false;
                
                this->response.setBet(currentLoopBet);
                return;
            }
            /* Robot try to raise */
            if (2 == cRobotStrategy)
            {
                this->response.isFold = false;
                this->response.isAllIn = false;
                currentLoopBet = (currentLoopBet < 10) ? 10 : currentLoopBet;
                
                this->response.setBet(currentLoopBet);
                return;
            }
            
        }
    }

    virtual void getAction(bool &isFold, bool &isAllin, int &bet)
    {
        isFold = this->response.isFold;
        isAllin = this->response.isAllIn;
        bet = this->response.getBet();

        this->totalBet -= bet;

        this->currentPayBet = bet;

        return;
    }

private:
    Client2ServerMsg response;
};


class RemotePlayer : public Player
{
public:
    const int cRemoteTimeOut = 3; // remote player assume 3s time out

    RemotePlayer(int id)
    {
        init(id);

        this->type = RemotePlayerType;

        this->response.Init();

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

    virtual bool isRemotePlayer()
    {
        return true;
    }

    void setSockId(int sockId)
    {
        this->playerSockId = sockId;
    }

    void notifyPlayerId()
    {
        memset(msg, 0, sizeof(msg));
        request.fillAssignMsg(msg, this->name, this->id);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Connect Send Request:\n{\n"<<msg<<"}"<<endl;
        sleep(1);
        
        do
        {
            memset(msg, 0, sizeof(msg));
            recv(this->playerSockId, (char*)msg, sizeof(msg), 0);

            cout<<"Confirm Ready Rcv Reponse:\n{\n"<<msg<<"}"<<endl;
        }while(0);
    }

    virtual void notifyPublicCards(const int cardCnt, Card *outCards)
    {
        Card cards[3];

        assert(cardCnt <= 3);

        for (int i=0; i<cardCnt; i++)
        {
            cards[i] = outCards[i];
        }
        
        memset(msg, 0, sizeof(msg));
        request.fillCardMsg(msg, -1, cardCnt, cards);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Public Cards Send Request:\n{\n"<<msg<<"}"<<endl;
        sleep(1);
        
        do
        {
            memset(msg, 0, sizeof(msg));
            recv(this->playerSockId, (char*)msg, sizeof(msg), 0);

            cout<<"Confirm Cards, Rcv Reponse:\n{\n"<<msg<<"}"<<endl;
        }while(0);
    }

    virtual void active()
    {
        Player::active();

        /* Notify the Remote Player Start Game */
        memset(msg, 0, sizeof(msg));
        request.fillSyncMsg(msg, this->id, ClientStatus::cSTATUS_READY, this->pDesc->desc);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Start Send Request:{\n"<<msg<<"}"<<endl;
        sleep(1);

        /* must wait for the Client confirm start status */
        do
        {
            memset(msg, 0, sizeof(msg));
            recv(this->playerSockId, (char*)msg, sizeof(msg), 0);

            cout<<"Confirm Start, Rcv Reponse:{\n"<<msg<<"}"<<endl;
        }while(0);

        cout<<endl;
        return;
    }

    virtual void setCard(Card &card0, Card &card1)
    {
        Player::setCard(card0, card1);

        Card cards[3] = {card0, card1, Card()};
        
        /* Notify remote player */
        memset(msg, 0, sizeof(msg));
        request.fillCardMsg(msg, this->id, 2, cards);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Start Send Request:{\n"<<msg<<"}"<<endl;
        sleep(1);

        /* Waiting for resp */
        do
        {
            memset(msg, 0, sizeof(msg));
            recv(this->playerSockId, (char*)msg, sizeof(msg), 0);

            cout<<"Confirm Card , Rcv Reponse:{\n"<<msg<<"}"<<endl;
        }while(0);

        cout<<endl;

        return;
    }
    
    virtual void adjustBet(int change)
    {
        Player::adjustBet(change);

        memset(msg, 0, sizeof(msg));

        request.fillAdjustBetMsg(msg, this->id, change);

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Acquire Blind Send Request:{\n"<<msg<<"}"<<endl;
    }

    virtual int acquirePlayerBlind(int blindBet)
    {
        this->blindBet = blindBet;

        memset(msg, 0, sizeof(msg));
        // when acquire blind, the bonus pool must be zero
        request.fillActionMsg(msg, this->id, 
                                    0, // Blind option
                                    blindBet, // bet
                                    0, // behind player count, useless now
                                    0, // bonus
                                    GS_Init // 设置盲注
                                    );

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Acquire Blind Send Request:{\n"<<msg<<"}"<<endl;
        
        return 0;
    }

    virtual int waitPlayerPayBlind(int &bet)
    {
         int loop = cRemoteTimeOut;

        do
        {
            sleep(1);
            
            memset(msg, 0, sizeof(msg));
            int len = recv(this->playerSockId, (char*)msg, sizeof(msg), 0);
            if (len > 0)
            {
                cout<<"Rcv Blind Reponse:{\n"<<msg<<"}"<<endl;
                cout<<endl;
                break;
            }
        }while(--loop > 0);

        if (loop <= 0)
        {
            cout<<"Doesn't rcv Response!!!!"<<endl;
            
            bet = 0;
            return -1;
        }

        this->response.Analysis(msg);

        if (this->blindBet != this->response.getBet())
        {
            cout<<"Blind Resp Error, set blind bet is "<<this->blindBet <<" , rec blind bet is "<<this->response.getBet()<<endl;
        }
        
        bet = this->response.getBet();
        this->totalBet -= bet;
        
        return 0;
    }


    virtual void acquireAction(int currentLoopBet, int currentBounsPool, int behindPlayerCount, GameStatus status)
    {
        memset(msg, 0, sizeof(msg));
        
        request.fillActionMsg(msg, this->id, 
                                    1, // 1 means Bet
                                    currentLoopBet, // bet
                                    behindPlayerCount, // behind player count, useless now
                                    currentBounsPool, // bonus
                                    status
                                    );

        send(this->playerSockId, (char*)&msg[0], strlen(msg), 0);

        cout<<"Acquire Bet Action Send Request:{\n"<<msg<<"}"<<endl;

    }

    virtual void getAction(bool &isFoldPara, bool &isAllinPara, int &bet)
    {
        int loop = cRemoteTimeOut * 10;
        int len = 0;

        do
        {
            sleep(1);
            
            memset(msg, 0, sizeof(msg));
            len = recv(this->playerSockId, (char*)msg, sizeof(msg), 0);
            if (len > 0)
            {
                cout<<"Rcv Action Reponse:{\n"<<msg<<"}"<<endl;
                cout<<endl;
                break;
            }

            cout<<"Rcv Reponse is Empty"<<endl;
        }while(--loop > 0);

        this->response.Analysis(msg);

        isFoldPara = this->response.isFold;
        isAllinPara = this->response.isAllIn;
        bet = this->response.getBet();
        /* Update Player status */
        this->isStay = !isFoldPara;
        this->isAllIn = isAllinPara;
        this->totalBet -= bet;
        if (response.getTotalBet() != this->totalBet)
        {
            cout<<"Total Bet "<<response.getTotalBet() << " != "<<this->totalBet<<endl;
            assert(0);
        }
        this->currentPayBet += bet;

        return;
    }

    virtual void setExtraLoad(void *p)
    {
        this->pDesc = (PlayerDesc *)p;
        return;
    }

    virtual void * getExtraLoad() const
    {
        return (void *)pDesc;
    }
private:
    char msg[1024];
    PlayerDesc *pDesc;
    //char pDesc[PokerTable::cMaxPlayerCount][Player::cMaxNameLen * 2];
    Server2ClientMsg request;
    Client2ServerMsg response;
    int playerSockId;
};

#endif
