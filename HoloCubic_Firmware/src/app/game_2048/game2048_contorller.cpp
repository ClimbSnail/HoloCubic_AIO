/***************************************************
  2048 APP

  聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、视频投影、
  浏览器文件修改。（各APP具体使用参考说明书）

  Github repositories：https://github.com/AndyXFuture/HoloCubic-2048-anim

  Last review/edit by AndyXFuture: 2021/11/12
 ****************************************************/

#include "game2048_contorller.h"
#include <Arduino.h>

/*
 * 随机刷新一个2或4
 * 返回刷新的位置
 */
int GAME2048::addRandom(void)
{
    int rand;
    while (1)
    {
        rand = random(3200) % 16;
        if (this->board[rand / 4][rand % 4] == 0)
        {
            this->board[rand / 4][rand % 4] = 2;
            break;
        }
    }
    return rand;
}

/*
 *   记录移动原来的位置，有数字则按方向填入ABCD
 *   direction  1.上 2.下 3.左 4.右
 *   为了减少代码量优化过，需要理解逻辑可直接看左方向部分的代码
 *   应该还可以再精简（懒）
 */
void GAME2048::recordLocation(int direction)
{
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            //无数字则为空
            if (board[i][j] == 0)
            {
                Location[i][j] = "";
            }
            else
            {
                //有数字根据方向填入ABCD
                switch (direction)
                {
                case 1: //上
                case 2: //下
                    switch (i)
                    {
                    case 0:
                        Location[i][j] = "A";
                        break;
                    case 1:
                        Location[i][j] = "B";
                        break;
                    case 2:
                        Location[i][j] = "C";
                        break;
                    case 3:
                        Location[i][j] = "D";
                        break;
                    }
                    break;
                case 3: //左
                case 4: //右
                    switch (j)
                    {
                    case 0:
                        Location[i][j] = "A";
                        break;
                    case 1:
                        Location[i][j] = "B";
                        break;
                    case 2:
                        Location[i][j] = "C";
                        break;
                    case 3:
                        Location[i][j] = "D";
                        break;
                    }
                    break;
                }
            }
        }
    }
}

/*
 *   通过解析Location变量的变化，获取并记录移动距离和合并位置
 *   direction  1.上 2.下 3.左 4.右
 *   >4则有合并,-8则是移动的值
 *   <4则直接就是移动的值
 */
void GAME2048::countMoveRecord(int direction)
{

    //清空
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            moveRecord[i][j] = 0;
        }
    }
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            switch (direction)
            {
            case 1:
            case 2:
                //移动检测
                if (Location[i][j].find("A") != -1)
                {
                    moveRecord[0][j] += i;
                }
                if (Location[i][j].find("B") != -1)
                {
                    moveRecord[1][j] += i - 1;
                }
                if (Location[i][j].find("C") != -1)
                {
                    moveRecord[2][j] += i - 2;
                }
                if (Location[i][j].find("D") != -1)
                {
                    moveRecord[3][j] += i - 3;
                }
                break;
            case 3:
            case 4:
                //移动检测
                if (Location[i][j].find("A") != -1)
                {
                    moveRecord[i][0] += j;
                }
                if (Location[i][j].find("B") != -1)
                {
                    moveRecord[i][1] += j - 1;
                }
                if (Location[i][j].find("C") != -1)
                {
                    moveRecord[i][2] += j - 2;
                }
                if (Location[i][j].find("D") != -1)
                {
                    moveRecord[i][3] += j - 3;
                }
                break;
            }
            //合并检测
            if (Location[i][j].length() == 2)
            {
                moveRecord[i][j] += 8;
            }
        }
    }
}

void GAME2048::moveUp(void)
{
    recordLocation(1); //记录位置
    recordBoard();     //记录数值
    //移动两次
    for (int x = 0; x < 2; x++)
    {
        for (int i = 0; i < SCALE_SIZE - 1; i++)
        {
            for (int j = 0; j < SCALE_SIZE; j++)
            {
                if (board[i][j] == 0)
                {
                    board[i][j] = board[i + 1][j];
                    board[i + 1][j] = 0;
                    //动画轨迹记录
                    Location[i][j] = Location[i + 1][j];
                    Location[i + 1][j] = "";
                }
            }
        }
    }

    //相加
    for (int i = 0; i < SCALE_SIZE - 1; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            if (board[i][j] == board[i + 1][j])
            {
                board[i][j] *= 2;
                board[i + 1][j] = 0;
                //动画轨迹记录
                Location[i][j].append(Location[i + 1][j]);
                Location[i + 1][j] = "";
            }
        }
    }

    //移动
    for (int i = 0; i < SCALE_SIZE - 1; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            if (board[i][j] == 0)
            {
                board[i][j] = board[i + 1][j];
                board[i + 1][j] = 0;
                //动画轨迹记录
                Location[i][j] = Location[i + 1][j];
                Location[i + 1][j] = "";
            }
        }
    }
    countMoveRecord(1);
}

void GAME2048::moveDown(void)
{
    recordLocation(2); //记录位置
    recordBoard();
    //移动
    for (int x = 0; x < 2; x++)
    {
        for (int i = SCALE_SIZE - 1; i > 0; i--)
        {
            for (int j = SCALE_SIZE - 1; j >= 0; j--)
            {
                if (board[i][j] == 0)
                {
                    board[i][j] = board[i - 1][j];
                    board[i - 1][j] = 0;
                    //动画移动轨迹记录
                    Location[i][j] = Location[i - 1][j];
                    Location[i - 1][j] = "";
                }
            }
        }
    }

    //相加
    for (int i = SCALE_SIZE - 1; i > 0; i--)
    {
        for (int j = SCALE_SIZE - 1; j >= 0; j--)
        {
            if (board[i][j] == board[i - 1][j])
            {
                board[i][j] *= 2;
                board[i - 1][j] = 0;
                //动画合并轨迹记录
                Location[i][j].append(Location[i - 1][j]);
                Location[i - 1][j] = "";
            }
        }
    }

    //移动
    for (int i = SCALE_SIZE - 1; i > 0; i--)
    {
        for (int j = SCALE_SIZE - 1; j >= 0; j--)
        {
            if (board[i][j] == 0)
            {
                board[i][j] = board[i - 1][j];
                board[i - 1][j] = 0;
                //动画移动轨迹记录
                Location[i][j] = Location[i - 1][j];
                Location[i - 1][j] = "";
            }
        }
    }
    countMoveRecord(2);
}

void GAME2048::moveLeft(void)
{
    recordLocation(3); //记录位置
    recordBoard();
    //移动
    for (int x = 0; x < 2; x++)
    {
        for (int i = 0; i < SCALE_SIZE; i++)
        {
            for (int j = 0; j < SCALE_SIZE - 1; j++)
            {
                if (board[i][j] == 0)
                {
                    board[i][j] = board[i][j + 1];
                    board[i][j + 1] = 0;
                    //动画移动轨迹记录
                    Location[i][j] = Location[i][j + 1];
                    Location[i][j + 1] = "";
                }
            }
        }
    }
    //相加
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE - 1; j++)
        {
            if (board[i][j] == board[i][j + 1])
            {
                board[i][j] *= 2;
                board[i][j + 1] = 0;
                //动画合并轨迹记录
                Location[i][j].append(Location[i][j + 1]);
                Location[i][j + 1] = "";
            }
        }
    }
    //移动
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE - 1; j++)
        {
            if (board[i][j] == 0)
            {
                board[i][j] = board[i][j + 1];
                board[i][j + 1] = 0;
                //动画移动轨迹记录
                Location[i][j] = Location[i][j + 1];
                Location[i][j + 1] = "";
            }
        }
    }
    countMoveRecord(3);
}
void GAME2048::moveRight(void)
{
    recordLocation(4); //记录位置
    recordBoard();
    //移动两次
    for (int x = 0; x < 2; x++)
    {
        for (int i = SCALE_SIZE - 1; i >= 0; i--)
        {
            for (int j = SCALE_SIZE - 1; j > 0; j--)
            {
                if (board[i][j] == 0)
                {
                    board[i][j] = board[i][j - 1];
                    board[i][j - 1] = 0;
                    //动画移动轨迹记录
                    Location[i][j] = Location[i][j - 1];
                    Location[i][j - 1] = "";
                }
            }
        }
    }

    //相加
    for (int i = SCALE_SIZE - 1; i >= 0; i--)
    {
        for (int j = SCALE_SIZE - 1; j > 0; j--)
        {
            if (board[i][j] == board[i][j - 1])
            {
                board[i][j] *= 2;
                board[i][j - 1] = 0;
                //动画合并轨迹记录
                Location[i][j].append(Location[i][j - 1]);
                Location[i][j - 1] = "";
            }
        }
    }

    //移动
    for (int i = SCALE_SIZE - 1; i >= 0; i--)
    {
        for (int j = SCALE_SIZE - 1; j > 0; j--)
        {
            if (board[i][j] == 0)
            {
                board[i][j] = board[i][j - 1];
                board[i][j - 1] = 0;
                //动画移动轨迹记录
                Location[i][j] = Location[i][j - 1];
                Location[i][j - 1] = "";
            }
        }
    }
    countMoveRecord(4);
}

/*
 * judge()判断当前游戏状态
 * 返回0：游戏可以继续
 * 返回1：游戏获胜
 * 返回2：游戏无法继续，失败
 */
int GAME2048::judge(void)
{
    //判赢
    for (int i = 0; i <= SCALE_SIZE * SCALE_SIZE; i++)
    {
        if (board[i / 4][i % 4] >= WIN_SCORE)
        {
            return 1; // Win
        }
    }
    //判空
    for (int i = 0; i <= SCALE_SIZE * SCALE_SIZE; i++)
    {
        if (board[i / 4][i % 4] == 0)
        {
            return 0;
        }
    }

    //判相邻相同
    for (int i = 0; i < SCALE_SIZE; i++)
    {
        for (int j = 0; j < SCALE_SIZE; j++)
        {
            if (i < 3)
            {
                if (board[i][j] == board[i + 1][j])
                {
                    return 0;
                }
            }
            if (j < 3)
            {
                if (board[i][j] == board[i][j + 1])
                {
                    return 0;
                }
            }
        }
    }

    return 2; // Defeatd
}
