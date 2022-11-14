
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

#include "server.h"

using namespace std;

#if 0

volatile bool isPlayerReady = false;
int  playerSockId;
int  serverSockId;

void* acceptPlayer(void *arg)
{
    int port = Server::Port;
    char msg[1500]; //buffer to send and receive messages with

    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
 
    serverSockId = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSockId < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    // set the socket to reuse
    int on = 1;
    int s = setsockopt(serverSockId, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (s < 0)
    {
        cout<<"Set Reuse error"<<endl;
        exit(0);
    }

    //bind the socket to its local address
    int bindStatus = bind(serverSockId, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address, err " << bindStatus<< endl;
        exit(0);
    }
    cout << "Start to listen " << endl;
    //listen for up to 5 requests at a time
    listen(serverSockId, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    cout << "Waiting for a client to connect..." << endl;
    
    do
    {
        playerSockId = accept(serverSockId, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if(playerSockId < 0)
        {
            cerr << "Error accepting request from client IP " <<newSockAddr.sin_addr.s_addr << endl;
            exit(1);
        }
        cout << "Connected with client IP " <<newSockAddr.sin_addr.s_addr << endl;

        strcpy(msg, "Server accept your connect require!");
        send(playerSockId, (char*)&msg, strlen(msg), 0);

        isPlayerReady = true;
    }while(0); // so far, only process 1 client

    cout<<"Accept thread finish!"<<endl;
    return NULL;
}


//Server side
int main()
{
    int port = Server::Port;
    char msg[1500]; //buffer to send and receive messages with
    pthread_t tidListen;

    isPlayerReady = false;

    int ret = pthread_create(&tidListen, NULL, acceptPlayer, NULL);
    if (ret != 0)
    {
        cout << "pthread_create error: error_code=" << ret << endl;
    }

    // wait client connected
    while (!isPlayerReady);

    int bytesRead = 0;
    while(1)
    {
        memset(&msg, 0, sizeof(msg)); 
        bytesRead += recv(playerSockId, (char*)&msg, sizeof(msg), 0);
        if(!strncmp(msg, "exit", 4))
        {
            cout << "Client has quit the session" << endl;
            break;
        }
        else
        {
            cout<<"Client Msg:"<<msg<<endl;
            cout<<"Total Rcv bytes "<<bytesRead<<endl;
            sleep(3);
        }
    }

    close(playerSockId);
    close(serverSockId);

    cout<<"main flow finish"<<endl;

    pthread_join(tidListen, NULL);
    
    cout<<"All finish"<<endl;

// the main flow should process the Table run process
////////////////////////////////////////////////////////////////////////
    #if 0
    //lets keep track of the session time
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    //also keep track of the amount of data sent as well
    int bytesRead, bytesWritten = 0;
    while(1)
    {
        //receive a message from the client (listen)
        cout << "Awaiting client response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, "exit"))
        {
            cout << "Client has quit the session" << endl;
            break;
        }
        cout << "Client: " << msg << endl;
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); //clear the buffer
        strcpy(msg, data.c_str());
        if(data == "exit")
        {
            //send to the client that server has closed the connection
            send(newSd, (char*)&msg, strlen(msg), 0);
            break;
        }
        //send the message to client
        bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
    }
    //we need to close the socket descriptors after we're all done
    gettimeofday(&end1, NULL);
    close(newSd);
    close(serverSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec) 
        << " secs" << endl;
    cout << "Connection closed..." << endl;
    #endif
    return 0;   
}
#endif


int main()
{
    int ret = 0;
    Server server;

    server.init();

    /* Simulate some Player */
    server.Debug_AddPlayer(1);

    /* At least there is 2 player to play game */
    server.waitPlayerJoin(2);

    cout<<"\n\tStart to game............."<<endl;

    server.startGame();

    server.stop();
}
