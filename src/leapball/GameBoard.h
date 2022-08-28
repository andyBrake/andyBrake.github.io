#ifndef GAME_BOARD_H
#define GAME_BOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <assert.h>
#include "Queue.h"

using namespace std;

static const int cRow = 10, cCol = 10;
static const int cTargetMax = 5;

// enum class BoardEle
//{
static const unsigned int BOARD_ELE_STATUS_HOLE = 0;
static const unsigned int BOARD_ELE_STATUS_OCCUPY = 1;
static const unsigned int BOARD_ELE_STATUS_MAX = 2;
//};

class Pos
{
public:
    int x;
    int y;

    static const int cInvalid = -1;

    friend ostream &operator<<(ostream &, const Pos &);

    Pos(int a = 0, int b = 0) : x(a), y(b)
    {
        pLinkQueue = NULL;
    }

    Pos &operator+(Pos point)
    {
        this->x += point.x;
        this->y += point.y;
        return *this;
    }

    Pos(const Pos &a)
    {
        this->x = a.x;
        this->y = a.y;
        this->pLinkQueue = a.pLinkQueue; // without this statement, will memory collaps
    }

    Pos &operator=(const Pos &a)
    {
        if (this == &a)
        {
            return *this;
        }

        this->x = a.x;
        this->y = a.y;
        this->pLinkQueue = a.pLinkQueue;

        return *this;
    }

    bool operator==(Pos point)
    {
        if (this->x == point.x && this->y == point.y)
        {
            return true;
        }
        return false;
    }

    bool operator!=(Pos point)
    {
        if (*this == point)
        {
            return false;
        }
        return true;
    }

    ~Pos()
    {
        if (pLinkQueue != NULL)
        {
            delete pLinkQueue;
            pLinkQueue = NULL;
        }
    }

    int getId()
    {
        return x * cCol + y;
    }

    bool isValid()
    {
        if (this->x < 0 || this->x >= cRow)
        {
            return false;
        }

        if (this->y < 0 || this->y >= cCol)
        {
            return false;
        }
        return true;
    }

    void addLinkPos(Pos &pos)
    {
        if (NULL == pLinkQueue)
        {
            pLinkQueue = new Queue<Pos>(cRow * cCol);
        }

        int ret = this->pLinkQueue->enqueue(pos);
        if (ret != 0)
        {
            cout << "Link Queue Insert Error!\n";
            exit(1);
        }
    }

    bool isLinkPosExist(Pos &linkPos)
    {
        if (NULL == pLinkQueue)
        {
            return false;
        }
        return pLinkQueue->isExist(linkPos);
    }

    void showLinkPos()
    {
        if (NULL != pLinkQueue)
        {
            cout << "       Start to Display P(" << this->x << " ," << this->y << ") the Link Pos Queue : " << endl;
            pLinkQueue->display();
            cout << "       Finish Display P(" << this->x << " ," << this->y << ")the Link Pos Queue" << endl
                 << endl;
        }
        else
        {
            cout << "The Queue is NULL" << endl;
        }
    }

private:
    Queue<Pos> *pLinkQueue;
};

ostream &operator<<(ostream &os, const Pos &ob)
{
    os << " P[" << ob.x << ", " << ob.y << "] ";

    return os;
}

class GameStatus;
class Graph;

class GameBoard
{
    friend class GameStatus;

public:
    GameBoard()
    {
        for (int i = 0; i < cRow; i++)
        {
            for (int j = 0; j < cCol; j++)
            {
                board[i][j] = BOARD_ELE_STATUS_HOLE;
            }
        }
    }

    ~GameBoard() {}

    void setTarget(Pos target[], int len)
    {
        assert(len <= cTargetMax);

        for (int i = 0; i < len; i++)
        {
            this->target[i] = target[i];
        }
        targetNum = len;
    }

    // Set the map in Board
    void updateStatus(Pos curPoint[], int len)
    {
        for (int i = 0; i < len; i++)
        {
            int x = curPoint[i].x;
            int y = curPoint[i].y;

            board[x][y] = BOARD_ELE_STATUS_OCCUPY;
        }
        return;
    }

    void display();

private:
    unsigned int board[cRow][cCol];
    int targetNum;

    Pos target[cTargetMax];
};

class GameStatus
{
public:
    GameStatus()
    {
        this->pBoard = NULL;
        totalSteps = 0;
        ballCount = 0;
    }

    GameStatus(GameBoard *pBoard, Pos curPos[], int len)
    {
        this->pBoard = pBoard;
        totalSteps = 0;
        ballCount = len;

        if (len > cTargetMax)
        {
            cout << "Too many ball" << endl;
            exit(1);
        }

        for (int i = 0; i < len; i++)
        {
            this->curPos[i] = curPos[i];
            cout << this->curPos[i] << endl;
        }

        cout << " 1." << this->curPos[2] << endl;

        initPosMap();

        updatePosLink();

        cout << "Create Game Status : Ball " << ballCount << endl;
        for (int i = 0; i < ballCount; i++)
        {
            this->curPos[i].showLinkPos();
        }

        return;
    }
    // check the pos if occupied
    bool isOccupied(Pos &pos)
    {
        return false;
    }

    Pos getMirrorPos(Pos &startPos, Pos &stonePos)
    {
        Pos pos(Pos::cInvalid, Pos::cInvalid);

        if (startPos.x == stonePos.x)
        {
            pos.y = (stonePos.y - startPos.y) + stonePos.y;
            pos.x = stonePos.x;
        }
        else if (startPos.y == stonePos.y)
        {
            pos.x = (stonePos.x - startPos.x) + stonePos.x;
            pos.y = stonePos.y;
        }
        else
        {
            cout << "Invalid Error: " << startPos.x << ", " << startPos.y;
            cout << "stone :" << stonePos.x << ", " << stonePos.y << endl;
            exit(1);
        }

        return pos;
    }

    int GetAvailablePosInDirection(Pos &startPos, int dir = 0)
    {
        Pos checkPos = startPos;

        // right dir
        while (checkPos.x + 1 < cCol)
        {
            checkPos.x++;
            if (isOccupied(checkPos))
            {
                Pos mirrorPos = getMirrorPos(startPos, checkPos);
                if (mirrorPos.isValid())
                {
                    startPos.addLinkPos(mirrorPos);
                }
            }
        }

        return 0;
    }

    ~GameStatus()
    {
    }

private:
    int trend()
    {
        int total = 0;

        for (int i = 0; i < pBoard->targetNum; i++)
        {
            total += (abs(curPos[i].x - pBoard->target[i].x) + abs(curPos[i].y - pBoard->target[i].y));
        }

        return total;
    }

    void initPosMap()
    {
        for (int i = 0; i < cRow; i++)
        {
            for (int j = 0; j < cCol; j++)
            {
                this->map[i][j] = BOARD_ELE_STATUS_HOLE;
            }
        }

        for (int i = 0; i < ballCount; i++)
        {
            int x = curPos[i].x;
            int y = curPos[i].y;

            map[x][y] = BOARD_ELE_STATUS_OCCUPY;
        }
    }

    Pos getDirPos(const Pos &thePos, int dir, int distance)
    {
        Pos retPos;

        if (dir < 0 || dir > 3)
        {
            cout << "Invalid Dir " << dir << endl;
            exit(1);
        }
        // dir = 0, 1, 2, 3.
        if (0 == dir) // left
        {
            retPos.x = thePos.x - distance;
            retPos.y = thePos.y;
        }

        if (1 == dir) // right
        {
            retPos.x = thePos.x + distance;
            retPos.y = thePos.y;
        }

        if (2 == dir) // top
        {
            retPos.x = thePos.x;
            retPos.y = thePos.y - distance;
        }

        if (3 == dir) // right
        {
            retPos.x = thePos.x;
            retPos.y = thePos.y + distance;
        }

        return retPos;
    }

    // isFirstStep is true means the ball can rool, besides skip
    void updateSinglePosLink(Pos &srcPos, Pos &linkPos)
    {
        // in each direction, distance is 0 ~ max(cRow, cCol)
        int maxDistance = (cRow > cCol) ? cRow : cCol;
        bool isFirstStep = (srcPos == linkPos) ? true : false;
        Pos mirrorPos;

        cout << "\nStart to update Pos " << srcPos << " LinkPos " << linkPos << " isFirstStep " << isFirstStep << endl;

        for (int dir = 0; dir < 4; dir++)
        {
            for (int distance = 1; distance < maxDistance; distance++)
            {
                Pos nextPos = getDirPos(linkPos, dir, distance);

                if (!nextPos.isValid())
                {
                    break;
                }

                cout << "Get the Pos " << linkPos << " next Pos " << nextPos << endl;

                if (map[nextPos.x][nextPos.y] == BOARD_ELE_STATUS_OCCUPY)
                {
                    mirrorPos = getMirrorPos(linkPos, nextPos);

                    cout << "Check Miiror, Stone is " << nextPos << ", Mirror is" << mirrorPos << endl;

                    if (mirrorPos.isValid() && map[mirrorPos.x][mirrorPos.y] == BOARD_ELE_STATUS_HOLE)
                    {
                        cout << "\t\tAdd mirror pos to Link : " << mirrorPos << endl;
                        if (!srcPos.isLinkPosExist(mirrorPos))
                        {
                            srcPos.addLinkPos(mirrorPos);

                            updateSinglePosLink(srcPos, mirrorPos);
                        }
                    }
                }
                else if (map[nextPos.x][nextPos.y] == BOARD_ELE_STATUS_HOLE)
                {
                    if (isFirstStep && distance == 1)
                    {
                        if (!srcPos.isLinkPosExist(nextPos))
                        {
                            cout << "\tAdd pos to Link : " << nextPos << endl;
                            srcPos.addLinkPos(nextPos);
                        }
                    }
                }
                else
                {
                    // Nothing
                }
            }
        }
    }
    // To update the cur Pos link
    void updatePosLink()
    {
        for (int i = 0; i < ballCount; i++)
        {
            Pos &thePos = curPos[i];

            updateSinglePosLink(thePos, thePos);
        }
    }

    unsigned int totalSteps;
    int ballCount;
    GameBoard *pBoard;
    Pos curPos[cTargetMax];
    unsigned int map[cRow][cCol];
    // Graph boardGraph;
};

void GameBoard::display()
{
    std::cout << "The Board status:\n ";

    for (int j = 0; j < cCol; j++)
    {
        cout << " " << j << " ";
    }
    cout << endl;

    for (int i = 0; i < cRow; i++)
    {
        cout << i;
        for (int j = 0; j < cCol; j++)
        {
            if (board[i][j] == BOARD_ELE_STATUS_HOLE)
            {
                cout << " . ";
            }
            else
            {
                cout << " * ";
            }
        }
        cout << endl;
    }

    cout << "The Targe pos is:" << endl;
    for (int i = 0; i < targetNum; i++)
    {
        cout << target[i].x << ", " << target[i].y << endl;
    }
}

class Graph
{
public:
    Graph(int posCnt = cRow * cCol)
    {
        if (posCnt > cTotalPosCount)
        {
            cout << "Too many Pos " << posCnt << endl;
            exit(1);
        }

        for (int i = 0; i < cRow; i++)
        {
            for (int j = 0; j < cCol; j++)
            {
                allPos[(i * cCol) + j].x = i;
                allPos[(i * cCol) + j].y = j;
            }
        }

        initLinkPos();
    }

    ~Graph() {}

    void display()
    {
        for (int i = 0; i < cTotalPosCount; i++)
        {
            cout << allPos[i] << endl;
            allPos[i].showLinkPos();
        }
    }

private:
    static const int cTotalPosCount = cRow * cCol;

    Pos allPos[cTotalPosCount];

    void initLinkPos()
    {
        int linkPosId = Pos::cInvalid;

        for (int i = 0; i < cRow; i++)
        {
            for (int j = 0; j < cCol; j++)
            {
                int curPosId = (i * cCol) + j;
                // cout << "Init   Pos " << Pos(i, j) << " Link Pos." << endl;

                // left Pos valid
                if (Pos(i - 1, j).isValid())
                {
                    linkPosId = Pos(i - 1, j).getId();
                    allPos[curPosId].addLinkPos(allPos[linkPosId]);
                    // cout << "Add Link Pos " << allPos[linkPosId] << endl;
                }
                // right Pos valid
                if (Pos(i + 1, j).isValid())
                {
                    linkPosId = Pos(i + 1, j).getId();
                    allPos[curPosId].addLinkPos(allPos[linkPosId]);
                    // cout << "Add Link Pos " << allPos[linkPosId] << endl;
                }

                // Up Pos valid
                if (Pos(i, j - 1).isValid())
                {
                    linkPosId = Pos(i, j - 1).getId();
                    allPos[curPosId].addLinkPos(allPos[linkPosId]);
                    // cout << "Add Link Pos " << allPos[linkPosId] << endl;
                }

                // Below Pos valid
                if (Pos(i, j + 1).isValid())
                {
                    linkPosId = Pos(i, j + 1).getId();
                    allPos[curPosId].addLinkPos(allPos[linkPosId]);
                    // cout << "Add Link Pos " << allPos[linkPosId] << endl;
                }

                // cout << "Finish Pos " << Pos(i, j) << " Link Pos." << endl;

#if 0 // for debug print
                if (i == 9 && j == 9)
                {
                    allPos[0].showLinkPos();
                    allPos[10].showLinkPos();

                    allPos[55].showLinkPos();
                    allPos[99].showLinkPos();

                    return;
                }
#endif
            }
        }

        return;
    }
};

#endif