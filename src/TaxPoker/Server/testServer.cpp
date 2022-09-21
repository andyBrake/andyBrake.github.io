#include "server.h"

int main()
{
    int ret = 0;
    Server server;

    server.init();

    server.startGame();
}