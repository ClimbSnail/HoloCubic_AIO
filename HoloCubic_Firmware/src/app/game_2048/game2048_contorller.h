#ifndef GAME2048_H
#define GAME2048_H

#include <iostream>
// #include <stdio.h>
using namespace std;

#include <Arduino.h>

#define SCALE_SIZE 4
#define WIN_SCORE 2048

class GAME2048
{
private:
    int board[4][4];
    int previous[4][4];
    string Location[4][4];
    int moveRecord[4][4];

public:
    void init()
    {
        for (int i = 0; i < SCALE_SIZE * SCALE_SIZE; i++)
        {
            this->board[i / 4][i % 4] = 0;
            this->previous[i / 4][i % 4] = 0;
            this->moveRecord[i / 4][i % 4] = 0;
        }
        // addRandom();
        // addRandom();
    };
    int addRandom(void);
    void moveUp(void);
    void moveDown(void);
    void moveLeft(void);
    void moveRight(void);
    int judge(void);
    void recordLocation(int direction);
    void countMoveRecord(int direction);

    void recordBoard()
    {
        for (int i = 0; i < SCALE_SIZE * SCALE_SIZE; i++)
        {
            this->previous[i / 4][i % 4] = this->board[i / 4][i % 4];
        }
    };

    int comparePre()
    {
        //判断移动后是否相同
        int x = 0;
        for (int i = 0; i < SCALE_SIZE * SCALE_SIZE; i++)
        {
            if (this->board[i / 4][i % 4] == this->previous[i / 4][i % 4])
                x++;
        }

        if (x >= 16)
        {
            return 1;
        }
        return 0;
    };
    int *getBoard()
    {
        return &this->board[0][0];
    };
    string *getLocation()
    {
        return &this->Location[0][0];
    };
    int *getMoveRecord()
    {
        return &this->moveRecord[0][0];
    };
};

#endif
