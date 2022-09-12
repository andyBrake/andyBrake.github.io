#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "PokerTable.h"

using namespace std;

int main()
{
    PokerTable table(8);

    table.startGame();
    
    return 0;
}