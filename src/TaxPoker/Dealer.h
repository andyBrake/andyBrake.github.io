#ifndef DEALER_H
#define DEALER_H

#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "Queue.h"
#include "types.h"


class Dealer
{
    public:

    const static int cCardTotalNum = 52;
    const static int cNumPerColor = 13;

    Dealer()
    {
        splited = 0;

        pRemainCardQueue  = new Queue<Card>(cCardTotalNum);
        pUselessCardQueue = new Queue<Card>(cCardTotalNum);

        return;
    }

    ~Dealer()
    {
        delete pRemainCardQueue;
        delete pUselessCardQueue;
    }

    void display()
    {
        int i=0;
        while(i < cCardTotalNum)
        {
            cout<<"The index "<<i<<" : "<<initCard[i]<<endl;
            i++;
        }

        return;
    }

    void washCard()
    {
        int count = 0, selectIndex = 0;
        int usedFlag[cCardTotalNum] = {0}; // 0 means avaiable

        splited = 0;

        initTotalCard();
        display();


        pRemainCardQueue->reset();
        pUselessCardQueue->reset();

        srand((unsigned)time(NULL));

        while(count < cCardTotalNum)
        {
            int index = rand()%cCardTotalNum;

            cout<<"Rand index : "<<index<<endl;

            if(usedFlag[index] == 1) //already used
            {
                do
                {
                    index = (1 + index)%cCardTotalNum;
                }while(usedFlag[index] == 1);

                selectIndex = index;

                cout<<"\tPostpone the Rand index to "<<index<<endl;
            }
            else
            {
                selectIndex = index;
            }

            usedFlag[selectIndex] = 1;
            count++;

            cout<<"Select "<<selectIndex<<" info "<< initCard[selectIndex] <<" , wash into Card Heap, now have "<<count<<" Cards."<<endl;

            pRemainCardQueue->enqueue(initCard[selectIndex]);
        }

        cout<<"Wash Card Finish!"<<endl;
        return;
    }

    void splitCard(int splitCount = 1) // set the split count, must set before each deal operation
    {
        if (splitCount > cCardTotalNum/2)
        {
            splitCount = cCardTotalNum/2;
        }

        splited += splitCount;

        cout<<"Start to split "<<splitCount<<" Card, total splited "<<splited<<" Cards" << endl;

        while (splitCount > 0)
        {
            Card card;
            int ret = pRemainCardQueue->pop(card);
            pUselessCardQueue->enqueue(card);

            assert(ret == 0);

            splitCount--;
        }
        
        return;
    }

    // get dealCount Cards from remain card
    void dealCard(const int dealCount, Card *outCards)
    {
        int count = 0;

        while(count < dealCount)
        {
            Card card;

            int ret = pRemainCardQueue->pop(card);
            assert(0 == ret);

            outCards[count] = card;
            count++;
        }

        cout<<"Delever "<<count<<" Card out."<<endl;
        return;
    }

    private:
    Card initCard[cCardTotalNum];
    int  splited;

    Queue<Card> *pRemainCardQueue;
    Queue<Card> *pUselessCardQueue;


    void initTotalCard()
    {
        int index = 0;

        for (int i=2;i<15;i++)
        {
            for (int j=0;j<4;j++)
            {
                initCard[index].color = (Color)j;
                initCard[index].value = (eCardValue)i;
                index++;
            }
        }

        cout<<"Init total "<<index<<" Cards."<<endl;
        return;
    }
};

#endif