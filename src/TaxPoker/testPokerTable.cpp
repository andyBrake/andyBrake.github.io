#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "PokerTable.h"

using namespace std;


int main()
{
    // Create a poker table with 3 robot players for quick testing
    PokerTable table(3);
    
    // Add robot players
    table.addRobotPlayer();
    table.addRobotPlayer();
    table.addRobotPlayer();
    
    // Activate players
    table.update();
    
    // Start one game to test the deal functions
    cout<<"\n========================================"<<endl;
    cout<<"Starting a poker game to test deal functions..."<<endl;
    cout<<"========================================\n"<<endl;
    
    table.startGame();
    
    cout<<"\n========================================"<<endl;
    cout<<"Game finished successfully!"<<endl;
    cout<<"========================================\n"<<endl;
    
    // Optional: Run the intensive card power test (commented out by default)
    // PokerTable::runTest();
    
    // WARNING: This test will take hundreds of years to complete!
    // PokerTable::runAllCaseTest();
    
    // Single player test (also intensive)
    // PokerTable::runSinglePlayerTest();
    
    return 0;
}