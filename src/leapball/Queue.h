#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>

using namespace std;

template <typename T>
class Queue
{
public:
    Queue(unsigned int depth)
    {
        this->depth = depth + 1; // need a extra slot for full condition check
        this->head = 0;
        this->tail = 0;

        pBuffer = (T *)malloc(sizeof(T) * this->depth);
    }

    ~Queue()
    {
        free(pBuffer);
    }

    bool isFull()
    {
        if ((tail + 1) % depth == head)
        {
            return true;
        }
        return false;
    }

    bool isEmpty()
    {
        if (head == tail)
        {
            return true;
        }
        return false;
    }

    int enqueue(T &item)
    {
        if (isFull())
        {
            return -1;
        }

        if (isExist(item))
        {
            cout << "Duplicated Item " << item << endl;
            return 0;
        }

        pBuffer[tail] = item;
        tail = (1 + tail) % this->depth;

        // cout << "tail is " << tail << ", Head is " << head << endl;
        return 0;
    }

    int pop(T &item)
    {
        if (isEmpty())
        {
            return -1;
        }
        item = pBuffer[this->head];
        head = (head + 1) % this->depth;

        return 0;
    }

    bool isExist(T &obj)
    {
        for (int i = head; i != tail; i = (i + 1) % this->depth)
        {
            if (pBuffer[i] == obj)
            {
                return true;
            }
        }

        return false;
    }

    int count()
    {
        return (tail + this->depth - head) % this->depth;
    }

    void display()
    {
        for (int i = head; i != tail; i = (i + 1) % this->depth)
        {
            cout << "\t" << pBuffer[i] << endl;
        }
    }

private:
    unsigned int depth;
    int head;
    int tail;

    T *pBuffer;
};

#endif