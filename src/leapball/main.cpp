#include <stdio.h>
#include "Queue.h"
#include "GameBoard.h"

int main()
{

    GameBoard gameBoard;
    Pos initPos[] = {Pos(0, 0), Pos(0, 1), Pos(1, 0), Pos(1, 1)};
    int ballCnt = sizeof(initPos) / sizeof(Pos);
    Pos targetArray[] = {Pos(0, 9)};

    GameStatus initStatus(&gameBoard, initPos, ballCnt);

    // gameBoard.display();
    /* set the start Pos and target Pos */
    gameBoard.updateStatus(initPos, 1);
    gameBoard.setTarget(targetArray, 1);

    // gameBoard.display();
#if 0
    cout << "Start to Test Graph:\n\n"
         << endl;
    Graph theGraph;

    // theGraph.display();
#endif
}

#if 0

int main()
{
    Queue<int> myQueue(10);

    for (int i = 0; i < 12; i++)
    {
        int item = 99;

        int ret = myQueue.enqueue(i);
        cout << "Push " << ret << " ,value " << i << endl;

        ret = myQueue.pop(item);
        cout << "Pop ret " << ret << ", value " << item << endl;
    }

    for (int i = 0; i < 10; i++)
    {
        int item = 10, ret;
        ret = myQueue.pop(item);
        cout << "Pop ret " << ret << ", value " << item << endl;
    }

    return 0;
}

#endif