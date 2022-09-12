#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "Queue.h"

class SimpleChannel
{
    static const int cMaxChannelNum = 10;

    enum ChannelQueueDepth
    {
        eSqDepth = 10,
        eCqDepth = 10
    };

    public:
    SimpleChannel()
    {
        isInit = false;
    }

    ~SimpleChannel()
    {
        if (~isInit)
        {
            return;
        }

        for (int i=0; i<this->count; i++)
        {
            delete sq[i];
            delete cq[i];
        }
    }

    void init(int count)
    {
        this->count = count;
        this->isInit = true;

        for (int i=0; i<count; i++)
        {
            sq[i] = new Queue<Server2ClientMsg>((unsigned int)eSqDepth);
            cq[i] = new Queue<Client2ServerMsg>((unsigned int)eCqDepth);
        }
        return;
    }

    int send2Player(int id, Server2ClientMsg sMsg)
    {
        //int ret ;
#if 0
        if (id >= this->count)
        {
            return -1;
        }
        ret = this->sq[id]->enqueue(sMsg);
        if (ret != 0)
        {
            cout<<"Notify Player "<<id<<" Failure!!!"<<endl;
            return -1;
        }
#endif
        return 0;
    }

    int receivePlayer(int id, Client2ServerMsg &rMsg)
    {
        int ret;
#if 0
        if (id >= this->count)
        {
            return -1;
        }

        while(this->cq[id]->isEmpty()); //loop wait for response
        
        ret = this->cq[id]->pop(rMsg);
        if (ret != 0)
        {
            cout<<"Receive Player "<<id<<" Failure!!!"<<endl;
            return -1;
        }
#endif
        return 0;
    }

    private:
    bool isInit;
    int count;

    Queue<Server2ClientMsg>* sq[cMaxChannelNum];
    Queue<Client2ServerMsg>* cq[cMaxChannelNum];
};