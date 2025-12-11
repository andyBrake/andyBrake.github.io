
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

// Main Server Program
int main()
{
    int ret = 0;
    int robotPlayerCnt = 2, remotePlayerCnt = 1;
    int totalPlayerCnt = robotPlayerCnt + remotePlayerCnt;
    Server server;

    // Initialize server with correct player count
    server.init(totalPlayerCnt);

    /* Simulate some Player */
    server.Debug_AddPlayer(robotPlayerCnt);

    /* there is 1 remote player to play game */
    server.waitPlayerJoin(totalPlayerCnt);

    cout<<"\n*******************************************"<<endl;
    cout<<"\n\tStart to game............."<<endl;
    cout<<"\n*******************************************\n"<<endl;

    server.startGame();

    server.stop();
}
