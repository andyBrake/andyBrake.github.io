#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#include <pthread.h>

#include "../types.h"
#include "../PokerTable.h"

class Server
{
public:
    const static int Port = 10002;
    const static int MsgBufferLen = 1024;

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
        
        isPlayerReady = false;

        int ret = pthread_create(&tidListen, NULL, processAcceptPlayer, this);
        if (ret != 0)
        {
            cout << "pthread_create error: error_code=" << ret << endl;
        }
        cout<<"Server Init Finish!"<<endl;
        return;
    }

    void waitPlayerJoin(int leastPlayerCnt = 2)
    {
        int playerCnt = 0;

        assert(NULL != pTable);
        
        while(playerCnt < leastPlayerCnt)
        {
            playerCnt = pTable->getPlayerCount();
            sleep(3);
        }
        cout<<"Total Player Count "<<playerCnt<<endl;

        return;
    }

    void Debug_AddPlayer(int needPlayerCnt = 1)
    {
        int playerId;

        for (int i=0; i<needPlayerCnt; i++)
        {
            playerId = this->pTable->addPlayer();
            cout<<"Add a Robot as Player "<<playerId<<endl;
            sleep(2);
        }
        
        return;
    }

    /* The thread func to accept Player connect, adding into table */
    void acceptPlayer(void)
    {
        int port = Server::Port;
        char msg[MsgBufferLen];
        char name[30] = "Fa"; 

        sockaddr_in servAddr;
        bzero((char*)&servAddr, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(port);

        this->serverSockId = socket(AF_INET, SOCK_STREAM, 0);
        if(this->serverSockId < 0)
        {
            cerr << "Error establishing the server socket" << endl;
            exit(0);
        }

        // set the socket to reuse
        int on = 1;
        int s = setsockopt(this->serverSockId, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (s < 0)
        {
            cout<<"Set Reuse error"<<endl;
            exit(0);
        }

        //bind the socket to its local address
        int bindStatus = bind(this->serverSockId, (struct sockaddr*) &servAddr, sizeof(servAddr));
        if(bindStatus < 0)
        {
            cerr << "Error binding socket to local address, err " << bindStatus<< endl;
            exit(0);
        }
        //listen for up to 5 requests at a time
        listen(this->serverSockId, 5);
        //receive a request from client using accept
        //we need a new address to connect with the client
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);

        cout << "Waiting for a client to connect..." << endl;
        
        do
        {
            int playerSockId = accept(this->serverSockId, (sockaddr *)&newSockAddr, &newSockAddrSize);
            if(playerSockId < 0)
            {
                cerr << "Error accepting request from client IP " <<newSockAddr.sin_addr.s_addr << endl;
                exit(1);
            }
            cout << "\t\tConnected with client IP " <<inet_ntoa(newSockAddr.sin_addr)<< endl;
            /* get the client player name, and assign a player id right now */
            memset(&msg, 0, sizeof(msg));
            recv(playerSockId, (char*)&msg, sizeof(msg), 0);
            cout<<"Rcv Client Msg:{\n"<<msg<<"\n}"<<endl;

            int playerId = pTable->addPlayer(playerSockId, name);

            isPlayerReady = true;

        }while(0); // so far, only process 1 client

        cout<<"Accept thread finish!"<<endl;
        return;
    }

    void startGame()
    {
        assert(NULL != pTable);

        pTable->update();

        pTable->startGame();
    }

    void stop()
    {
        close(this->serverSockId);
        close(serverSockId);

        pthread_join(tidListen, NULL);
    
        cout<<"Stop the Server!!!"<<endl;
    }

private:
    PokerTable *pTable;
    volatile bool isPlayerReady;
    pthread_t tidListen;
    int  serverSockId;

    static void* processAcceptPlayer(void *arg)
    {
        Server *pServerObj = (Server *)arg;

        cout<<"Process the accept thread"<<endl;

        pServerObj->acceptPlayer();

        return NULL;
    }

    
};

#endif
