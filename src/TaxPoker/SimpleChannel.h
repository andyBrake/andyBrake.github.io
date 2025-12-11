#ifndef SIMPLE_CHANNEL_H
#define SIMPLE_CHANNEL_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <memory>  // For smart pointers

#include "Queue.h"
#include "types.h"  // Include types for Server2ClientMsg and Client2ServerMsg

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
        count = 0;
        
        // Initialize all smart pointers to nullptr (optional, but explicit)
        for (int i = 0; i < cMaxChannelNum; i++)
        {
            sq[i] = nullptr;
            cq[i] = nullptr;
        }
    }

    ~SimpleChannel()
    {
        // Smart pointers automatically clean up, no manual delete needed
        // unique_ptr will automatically call delete when going out of scope
        if (!isInit)
        {
            return;
        }
        
        // No need for manual cleanup - unique_ptr handles it automatically
    }

    void init(int count)
    {
        this->count = count;
        this->isInit = true;

        for (int i=0; i<count; i++)
        {
            // Use make_unique to create smart pointers (C++14)
            sq[i] = std::make_unique<Queue<Server2ClientMsg>>((unsigned int)eSqDepth);
            cq[i] = std::make_unique<Queue<Client2ServerMsg>>((unsigned int)eCqDepth);
        }
        return;
    }

    int send2Player(int id, const Server2ClientMsg& sMsg)  // Use const reference
    {
        // Boundary check
        if (id >= this->count || id < 0)
        {
            std::cout << "Invalid player ID: " << id << std::endl;
            return -1;
        }

        int ret = this->sq[id]->enqueue(sMsg);
        if (ret != 0)
        {
            std::cout << "Notify Player " << id << " Failure!!!" << std::endl;
            return -1;
        }
        
        return 0;
    }

    int receivePlayer(int id, Client2ServerMsg &rMsg)
    {
        // Boundary check
        if (id >= this->count || id < 0)
        {
            std::cout << "Invalid player ID: " << id << std::endl;
            return -1;
        }

        // Busy-wait (TODO: replace with condition variable for better performance)
        while(this->cq[id]->isEmpty());
        
        int ret = this->cq[id]->pop(rMsg);
        if (ret != 0)
        {
            std::cout << "Receive Player " << id << " Failure!!!" << std::endl;
            return -1;
        }
        
        return 0;
    }

    private:
    bool isInit;
    int count;

    // Use unique_ptr for automatic memory management
    std::unique_ptr<Queue<Server2ClientMsg>> sq[cMaxChannelNum];  // Server to Client queues
    std::unique_ptr<Queue<Client2ServerMsg>> cq[cMaxChannelNum];  // Client to Server queues
};

#endif // SIMPLE_CHANNEL_H