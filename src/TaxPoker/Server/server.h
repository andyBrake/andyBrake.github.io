#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <iostream>

#include "../types.h"
#include "../PokerTable.h"

class Server
{
public:
    Server()
    {
        pTable = NULL;
    }

    ~Server()
    {
        if (pTable != NULL)
        {
            delete pTable;
            pTable = NULL;
        }
    }

    void init()
    {
        pTable = new PokerTable(2);
    }

    void startGame()
    {
        pTable->startGame();
    }

private:
    PokerTable *pTable;
};

#endif
