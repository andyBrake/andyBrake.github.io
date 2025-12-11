#ifndef DEALER_H
#define DEALER_H

#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "Queue.h"
#include "types.h"
#include "Ruler.h"

class Dealer
{
    public:

    const static int cCardTotalNum = 52;
    const static int cNumPerColor = 13;

    Dealer()
    {
        splited = 0;
        usedBitmap = 0;

        assert(sizeof(usedBitmap) == 8); // must be 8 Byte

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

        pRemainCardQueue->reset();
        pUselessCardQueue->reset();

        srand((unsigned)time(NULL));

        while(count < cCardTotalNum)
        {
            int index = rand()%cCardTotalNum;

            //cout<<"Rand index : "<<index<<endl;

            if(usedFlag[index] == 1) //already used
            {
                do
                {
                    index = (1 + index)%cCardTotalNum;
                }while(usedFlag[index] == 1);

                selectIndex = index;

                //cout<<"\tPostpone the Rand index to "<<index<<endl;
            }
            else
            {
                selectIndex = index;
            }

            usedFlag[selectIndex] = 1;
            count++;

            //cout<<"Select "<<selectIndex<<" info "<< initCard[selectIndex] <<" , wash into Card Heap, now have "<<count<<" Cards."<<endl;

            pRemainCardQueue->enqueue(initCard[selectIndex]);
        }

        //cout<<"Wash Card Finish! Total Card Num "<<count<<endl;
        return;
    }

    void clearUsedBitmap()
    {
        usedBitmap = 0;
    }


    Card getCardbyOffset(int offset)
    {
        int col = offset%4;
        int v = (offset/4) + 2;

        return Card((CardColor)col, (CardValue)v);
    }
    
    void splitCard(int splitCount = 1) // set the split count, must set before each deal operation
    {
        if (splitCount > cCardTotalNum/2)
        {
            splitCount = cCardTotalNum/2;
        }

        splited += splitCount;

        //cout<<"Start to split "<<splitCount<<" Card, total splited "<<splited<<" Cards" << endl;

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

        //cout<<"Delever "<<count<<" Card out."<<endl;
        return;
    }

    // High-level deal functions for better readability
    
    // Deal 2 hole cards to a player
    void dealHoleCards(Card *playerCards)
    {
        dealCard(2, playerCards);
    }

    // Deal 3 flop cards to public board
    void dealFlop(Card *flopCards)
    {
        dealCard(3, flopCards);
    }

    // Deal 1 turn card to public board
    void dealTurn(Card &turnCard)
    {
        dealCard(1, &turnCard);
    }

    // Deal 1 river card to public board
    void dealRiver(Card &riverCard)
    {
        dealCard(1, &riverCard);
    }

    void acquireSpecialCard(Card card)
    {
        int offsett = getCardBitOffset(card);

        usedBitmap |= (1ull << offsett);
    }

    void setPrivateCardSet(Card cardSet00, Card cardSet01, Card cardSet10, Card cardSet11)
    {
        this->set0[0] = cardSet00;
        this->set0[1] = cardSet01;

        this->set1[0] = cardSet10;
        this->set1[1] = cardSet11;
    }

    void listAllPublicCardCombine()
    {
        int card0, card1, card2, card3, card4;
        int totalSetCnt = 0;
        int winCnt = 0, equalCnt = 0, loseCnt = 0;

        // card 0
        for (card0 = 0; card0 <cCardTotalNum; card0++)
        {
            if (isUsedOffset(card0))
            {
                continue;
            }
            // card 1
            for(card1=card0+1; card1<cCardTotalNum; card1++)
            {
                if (isUsedOffset(card1))
                {
                    continue;
                }
                // card 2
                for (card2=card1+1; card2<cCardTotalNum; card2++)
                {
                    if (isUsedOffset(card2))
                    {
                        continue;
                    }
                    // card 3
                    for (card3=card2+1; card3<cCardTotalNum; card3++)
                    {
                        if (isUsedOffset(card3))
                        {
                            continue;
                        }   
                        // card 4
                        for (card4=card3+1; card4<cCardTotalNum; card4++)
                        {
                            if (isUsedOffset(card4))
                            {
                                continue;
                            }
                            
                            totalSetCnt++;

                            int ret = CompareCardSetPower(card0, card1, card2, card3, card4);
                            if (ret == 1)
                            {
                                winCnt++;
                            }
                            else if (0 == ret)
                            {
                                equalCnt++;
                            }
                            else // -1 == ret
                            {
                                loseCnt++;
                            }             
                        }
                    }
                }
            }
        }

        cout<<"Total Set count is "<<totalSetCnt<<endl;
        cout<<"Total Win count is "<<winCnt<<" ; Equal count is "<<equalCnt<<" ; Lose count is "<<loseCnt<<endl;
        cout<<"Ratio is "<<(double)winCnt/(double)totalSetCnt <<" ; "<<(double)equalCnt/(double)totalSetCnt
            <<" ; "<<(double)loseCnt/(double)totalSetCnt<<endl;

        if (totalSetCnt != (winCnt + equalCnt + loseCnt))
        {
            cout<<"!!!! Unexpected Error!!!"<<endl;
            exit(0);
        }
        return;
    }

    bool isUsedOffset(int offset)
    {
        if (this->usedBitmap & (1ull<<offset))
        {
            return true;
        }
        return false;
    }

    private:
    Card initCard[cCardTotalNum];
    Card set0[ePrivateCardNum];
    Card set1[ePrivateCardNum];
    int  splited;

    unsigned long usedBitmap;

    Queue<Card> *pRemainCardQueue;
    Queue<Card> *pUselessCardQueue;


    void initTotalCard()
    {
        int index = 0;

        for (int i = 2; i < 15; i++)
        {
            for (int j=0;j<4;j++)
            {
                initCard[index].color = (CardColor)j;
                initCard[index].value = (CardValue)i;
                index++;
            }
        }
        usedBitmap = 0;

        //cout<<"Init total "<<index<<" Cards."<<endl;
        return;
    }

    int getCardBitOffset(Card card)
    {
        int v = (int)(card.value - CV_2);

        v = v*4 + (int)card.color;
        //cout<<card<<"  bit offset is "<<v<<endl;
        return v;
    }



    int CompareCardSetPower(int card0, int card1, int card2, int card3, int card4)
    {
        Card cardSet0[7];
        Card cardSet1[7];
        CardPower cardPower0, cardPower1;
        int power0 = 0, power1 = 0;

        cardSet0[0] = initCard[card0];
        cardSet0[1] = initCard[card1];
        cardSet0[2] = initCard[card2];
        cardSet0[3] = initCard[card3];
        cardSet0[4] = initCard[card4];
        cardSet0[5] = this->set0[0];
        cardSet0[6] = this->set0[1];

        cardSet1[0] = initCard[card0];
        cardSet1[1] = initCard[card1];
        cardSet1[2] = initCard[card2];
        cardSet1[3] = initCard[card3];
        cardSet1[4] = initCard[card4];
        cardSet1[5] = this->set1[0];
        cardSet1[6] = this->set1[1];

        Ruler::sortCardSet(cardSet0);
        Ruler::sortCardSet(cardSet1);

        power0 = Ruler::calCardSetPower(cardSet0, eCardSetNum);
        power1 = Ruler::calCardSetPower(cardSet1, eCardSetNum);

        if (power0 > power1)
        {
        #if 0 // disable print to save time
            cout<<"The Public Cards:"<<endl;
            cout<< initCard[card0] << initCard[card1] << initCard[card2] << initCard[card3] << initCard[card4]<<endl;
                            
            cout<<"The set0 : "<<this->set0[0]<<this->set0[1]<<endl;
            cout<<"The set1 : "<<this->set1[0]<<this->set1[1]<<endl;

            cout<<"The set0 Power is "<<power0<<", the info is:"<<endl;
            cardPower0.display();

            cout<<"The set1 Power is "<<power1<<", the info is:"<<endl;
            cardPower1.display();
        #endif

            return 1;
        }
        else if (power0 == power1)
        {
            return 0;
        }

        return -1;
    }
};

#endif
