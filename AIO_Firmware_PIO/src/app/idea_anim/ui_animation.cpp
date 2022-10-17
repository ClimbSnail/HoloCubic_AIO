#define GUILITE_ON   //Do not define this macro once more!!!
#include "GuiLite.h" //GUI库
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Wire.h>
#include "ui_animation.h"

/*********时钟相关的参数**********/
#define CLOCK_POS_X 120 //时钟中心点x/y坐标//像素点为单位
#define CLOCK_POS_Y 120
#define CLOCK_RADIUS 100                    //时钟半径//像素点为单位
#define CLOCK_ANIMATION_MAX_RADIUS 120      //放动画时时钟最大半径
#define CLOCK_SACLE_RGB GL_RGB(0, 175, 235) //时钟外框颜色
#define HOUR_HAND_LEN 40                    //时针长度
#define HOUR_HAND_RGB GL_RGB(255, 255, 255) //时针颜色
#define HOUR_HAND_ANIMATION_ANGLE 2         //放动画时时针起始及终值角度。弧度为单位
#define MINUTE_HAND_LEN 70                  //与时针参数描述相同
#define MINUTE_HAND_RGB GL_RGB(255, 255, 255)
#define MINUTE_HAND_ANIMATION_ANGLE 4
#define SECOND_HAND_LEN 90
#define SECOND_HAND_RGB GL_RGB(237, 125, 124)
#define SECOND_HAND_ANIMATION_ANGLE 6
/*************************/

/*思维点和连线相关的参数*/
#define MOLECULE_RADIUS 8         //思维点半径
#define MOLECULE_TOTAL 10         //运动思维点的总数
#define BOND_TOTAL 45             //BOND_TOTAL = 1 + 2 + ... + (MOLECULE_TOTAL - 1)//思维点连线的总数，最多是等差数列求和
const int VELOCITY = 2;           //思维点运动速度
const int BOND_MAX_DISTANCE = 80; //思维点之间连线的阈值
/***************/

/******3D方块相关的参数****/
#define SHAPE_SIZE 50 //陀螺仪暂时用不了，现在就没用
/****************/

/***星空穿越特效相关参数*/
#define FACTOR_SIZE 2  //星空粒子大小//方形时为方形的边长//圆形时为圆形半径
#define FACTOR_SHAPE 1 //星空粒子形状//0 方形//1 圆形
/*****************/

const int UI_WIDTH = 240;
const int UI_HEIGHT = 240;
const int FRAME_COUNT = 32; //播放时钟动画的帧数

/************这两个不能动*****/
static c_surface *s_surface;
static c_display *s_display;
/*****************/

unsigned short int randomColor();                                           //产生随机颜色函数声明
void screen_fill_circle(int32_t x0, int32_t y0, int32_t r, uint16_t color); //绘制填充圆形函数声明

/*******这部分矩阵旋转变换，与3D方块有关**********/
void multiply(int m, int n, int p, double *a, double *b, double *c); // a[m][n] * b[n][p] = c[m][p]
void rotateX(double angle, double *point, double *output);           // rotate matrix for X
void rotateY(double angle, double *point, double *output);           // rotate matrix for Y
void rotateZ(double angle, double *point, double *output);           // rotate matrix for Z
void projectOnXY(double *point, double *output, double zFactor);
/*****************/

/**星空穿越特效类*/
class c_star
{
public:
    c_star()
    {
        initialize();
    }
    void initialize()
    {
        m_x = m_start_x = rand() % UI_WIDTH;
        m_y = m_start_y = rand() % UI_HEIGHT;
        m_size = FACTOR_SIZE;
        m_x_factor = UI_WIDTH;
        m_y_factor = UI_HEIGHT;
        m_size_factor = 1;
    }

    void move()
    {
        s_surface->fill_rect(m_x, m_y, m_x + m_size - 1, m_y + m_size - 1, 0, Z_ORDER_LEVEL_0); //clear star footprint

        m_x_factor -= 6;
        m_y_factor -= 6;
        m_size += m_size / 20;
        if (m_x_factor < 1 || m_y_factor < 1)
        {
            return initialize();
        }
        if (m_start_x > (UI_WIDTH / 2) && m_start_y > (UI_HEIGHT / 2))
        {
            m_x = (UI_WIDTH / 2) + (UI_WIDTH * (m_start_x - (UI_WIDTH / 2)) / m_x_factor);
            m_y = (UI_HEIGHT / 2) + (UI_HEIGHT * (m_start_y - (UI_HEIGHT / 2)) / m_y_factor);
        }
        else if (m_start_x <= (UI_WIDTH / 2) && m_start_y > (UI_HEIGHT / 2))
        {
            m_x = (UI_WIDTH / 2) - (UI_WIDTH * ((UI_WIDTH / 2) - m_start_x) / m_x_factor);
            m_y = (UI_HEIGHT / 2) + (UI_HEIGHT * (m_start_y - (UI_HEIGHT / 2)) / m_y_factor);
        }
        else if (m_start_x > (UI_WIDTH / 2) && m_start_y <= (UI_HEIGHT / 2))
        {
            m_x = (UI_WIDTH / 2) + (UI_WIDTH * (m_start_x - (UI_WIDTH / 2)) / m_x_factor);
            m_y = (UI_HEIGHT / 2) - (UI_HEIGHT * ((UI_HEIGHT / 2) - m_start_y) / m_y_factor);
        }
        else if (m_start_x <= (UI_WIDTH / 2) && m_start_y <= (UI_HEIGHT / 2))
        {
            m_x = (UI_WIDTH / 2) - (UI_WIDTH * ((UI_WIDTH / 2) - m_start_x) / m_x_factor);
            m_y = (UI_HEIGHT / 2) - (UI_HEIGHT * ((UI_HEIGHT / 2) - m_start_y) / m_y_factor);
        }

        if (m_x < 0 || (m_x + m_size - 1) >= UI_WIDTH ||
            m_y < 0 || (m_y + m_size - 1) >= UI_HEIGHT)
        {
            return initialize();
        }
        if (FACTOR_SHAPE == 0)
        {
            s_surface->fill_rect(m_x, m_y, m_x + m_size - 1, m_y + m_size - 1, randomColor(), Z_ORDER_LEVEL_0); //draw star
        }
        else if (FACTOR_SHAPE == 1)
        {
            screen_fill_circle(m_x, m_y, m_size, randomColor());
        }
    }
    int m_start_x, m_start_y;
    float m_x, m_y, m_x_factor, m_y_factor, m_size_factor, m_size;
};
/*****************/

/****时钟动画的时钟指针类***/
class c_hand
{
public:
    void set(int x, int y, float start_len, float end_len, float start_angle, float end_angle, unsigned int rgb)
    {
        m_pos_x = x;
        m_pos_y = y;
        m_rgb = rgb;
        m_start_len = start_len;
        m_end_len = end_len;
        m_start_angle = start_angle;
        m_end_angle = end_angle;

        m_len_step = ((m_end_len - m_start_len) / FRAME_COUNT);
        m_angle_step = ((m_end_angle - m_start_angle) / FRAME_COUNT);
    }
    bool show()
    {
        if (fabs(m_start_len - m_end_len) < 1.0)
        {
            return true;
        }
        //erase footprint
        int x = (m_start_len)*cos(m_start_angle) + m_pos_x;
        int y = (m_start_len)*sin(m_start_angle) + m_pos_y;
        s_surface->draw_line(m_pos_x, m_pos_y, x, y, 0, Z_ORDER_LEVEL_0);

        m_start_len += m_len_step;
        m_start_angle += m_angle_step;

        x = (m_start_len)*cos(m_start_angle) + m_pos_x;
        y = (m_start_len)*sin(m_start_angle) + m_pos_y;
        s_surface->draw_line(m_pos_x, m_pos_y, x, y, m_rgb, Z_ORDER_LEVEL_0);

        return false;
    }

private:
    int m_pos_x, m_pos_y;
    unsigned int m_rgb;

    float m_start_len;
    float m_end_len;
    float m_len_step;

    float m_start_angle;
    float m_end_angle;
    float m_angle_step;
};
/*****************/

/**********时钟动画类*******/
class c_clock
{
public:
    c_clock() : m_x(CLOCK_POS_X), m_y(CLOCK_POS_Y), pi(3.1415926535 / 180) {}
    void set_hands(float start_radius, float end_radius, float hour_start_len, float hour_end_len, float hour_start_angle, float hour_end_angle, unsigned int hour_rgb, float minute_start_len, float minute_end_len, float minute_start_angle, float minute_end_angle, unsigned int minute_rgb, float second_start_len, float second_end_len, float second_start_angle, float second_end_angle, unsigned int second_rgb)
    {
        m_hour.set(m_x, m_y, hour_start_len, hour_end_len, hour_start_angle, hour_end_angle, hour_rgb);
        m_minute.set(m_x, m_y, minute_start_len, minute_end_len, minute_start_angle, minute_end_angle, minute_rgb);
        m_second.set(m_x, m_y, second_start_len, second_end_len, second_start_angle, second_end_angle, second_rgb);
        m_start_radius = start_radius;
        m_end_radius = end_radius;
        m_step = ((m_end_radius - m_start_radius) / FRAME_COUNT);
    }
    bool isComplete()
    {
        return complete;
    }
    void setComplete(bool _com)
    {
        complete = _com;
    }
    bool drawSacle()
    {
        if (fabs(m_start_radius - m_end_radius) < 1.0)
        {
            return true;
        }

        drawCircle(m_start_radius, 0);
        m_start_radius += m_step;
        drawCircle(m_start_radius, CLOCK_SACLE_RGB);
        return false;
    }
    void show()
    {
        complete = drawSacle();
        complete &= m_hour.show();
        complete &= m_minute.show();
        complete &= m_second.show();
    }
    void drawCircle(int r, unsigned int rgb)
    {
        int tmpX, tmpY, lastX, lastY;
        lastX = lastY = 0;

        for (float angle = 0.5; angle < 90; angle += 0.5) //Bigger circle, Smaller angle step
        {
            tmpX = r * cos(angle * pi);
            tmpY = r * sin(angle * pi);
            if (tmpX == lastX && tmpY == lastY)
            {
                continue;
            }

            s_surface->draw_pixel(m_x + tmpX, m_y + tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x - tmpX, m_y + tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x + tmpX, m_y - tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x - tmpX, m_y - tmpY, rgb, Z_ORDER_LEVEL_0);
        }
    }

private:
    int m_x;
    int m_y;
    float m_start_radius;
    float m_end_radius;
    float m_step;

    c_hand m_hour;
    c_hand m_minute;
    c_hand m_second;
    const float pi;
    bool complete = false;
};
/*****************/

/*********指示时间需要调用到的时钟指针类********/
class Time_hand
{
public:
    void set(int x, int y, float len, float angle, unsigned int rgb)
    {
        m_pos_x = x;
        m_pos_y = y;
        m_rgb = rgb;
        m_len = len;
        m_angle = angle;
    }
    void show()
    {
        //erase footprint
        int x = m_len * cos(m_angle) + m_pos_x;
        int y = m_len * sin(m_angle) + m_pos_y;
        s_surface->draw_line(m_pos_x, m_pos_y, x, y, 0, Z_ORDER_LEVEL_0);

        //calculate new angle

        //draw new hand
        x = m_len * cos(m_angle) + m_pos_x;
        y = m_len * sin(m_angle) + m_pos_y;
        s_surface->draw_line(m_pos_x, m_pos_y, x, y, m_rgb, Z_ORDER_LEVEL_0);
    }

private:
    int m_pos_x, m_pos_y;
    unsigned int m_rgb;

    float m_len;
    float m_angle;
};
/*****************/
/***********指示时间调用到的时钟类******/ //这个是时钟最终调用到的类，动画部分已经作为成员变量包含进去了，里面共有进入和退出两种动画
class Time_clock
{
public:
    c_clock clock_enter_animation;
    c_clock clock_leave_animation;

    Time_clock() : m_x(CLOCK_POS_X), m_y(CLOCK_POS_Y), pi(3.1415926535 / 180)
    {
        clock_enter_animation.set_hands(CLOCK_ANIMATION_MAX_RADIUS, CLOCK_RADIUS, 0.0, HOUR_HAND_LEN, 0.0, 2.0, HOUR_HAND_RGB, 0.0, MINUTE_HAND_LEN, 0.0, 4.0, MINUTE_HAND_RGB, 0.0, SECOND_HAND_LEN, 0.0, 6.0, SECOND_HAND_RGB);
        clock_leave_animation.set_hands(CLOCK_RADIUS, CLOCK_ANIMATION_MAX_RADIUS, HOUR_HAND_LEN, 0.0, 2.0, 0.0, HOUR_HAND_RGB, MINUTE_HAND_LEN, 0.0, 4.0, 0.0, MINUTE_HAND_RGB, SECOND_HAND_LEN, 0.0, 6.0, 0.0, SECOND_HAND_RGB);
    }
    void set_hands(float radius, float hour_len, float hour_angle, unsigned int hour_rgb, float minute_len, float minute_angle, unsigned int minute_rgb, float second_len, float second_angle, unsigned int second_rgb)
    {
        m_hour.set(m_x, m_y, hour_len, hour_angle, hour_rgb);
        m_minute.set(m_x, m_y, minute_len, minute_angle, minute_rgb);
        m_second.set(m_x, m_y, second_len, second_angle, second_rgb);
        m_radius = radius;
    }
    void drawSacle() //绘制表盘
    {
        drawCircle(m_radius, CLOCK_SACLE_RGB);
    }
    //zhe li kan zhe gai
    void show()
    {
        drawSacle();
        m_hour.show();
        m_minute.show();
        m_second.show();
    }
    void drawCircle(int r, unsigned int rgb) //画圆
    {
        int tmpX, tmpY, lastX, lastY;
        lastX = lastY = 0;

        for (float angle = 0.5; angle < 90; angle += 0.5) //Bigger circle, Smaller angle step
        {
            tmpX = r * cos(angle * pi);
            tmpY = r * sin(angle * pi);
            if (tmpX == lastX && tmpY == lastY)
            {
                continue;
            }

            s_surface->draw_pixel(m_x + tmpX, m_y + tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x - tmpX, m_y + tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x + tmpX, m_y - tmpY, rgb, Z_ORDER_LEVEL_0);
            s_surface->draw_pixel(m_x - tmpX, m_y - tmpY, rgb, Z_ORDER_LEVEL_0);
        }
    }

private:
    int m_x;
    int m_y;
    float m_radius;

    Time_hand m_hour;
    Time_hand m_minute;
    Time_hand m_second;
    const float pi;
};
/*****************/

/**********思维点类*******/
class Molecule
{
public:
    Molecule()
    {
        x = rand() % UI_WIDTH;
        y = rand() % UI_HEIGHT;
        vx = VELOCITY * ((0 == rand() % 2) ? -1 : 1);
        vy = VELOCITY * ((0 == rand() % 2) ? -1 : 1);
        //color = GL_RGB(rand() % 5 * 32 + 127, rand() % 5 * 32 + 127, rand() % 5 * 32 + 127);
        color = randomColor();
    }

    void move()
    {
        //draw(0);
        if (x <= 0 || x >= UI_WIDTH)
        {
            vx = (0 - vx);
            color = randomColor();
        }
        if (y < 0 || y >= UI_HEIGHT)
        {
            vy = (0 - vy);
            color = randomColor();
        }
        x += vx;
        y += vy;
        //draw(color);

        screen_fill_circle(x, y, MOLECULE_RADIUS, color);
    }

    void draw(unsigned int color)
    {
        s_surface->draw_pixel(x - 2, y - 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x - 1, y - 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x, y - 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 1, y - 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 2, y - 2, color, Z_ORDER_LEVEL_0);

        s_surface->draw_pixel(x - 3, y - 1, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 3, y - 1, color, Z_ORDER_LEVEL_0);

        s_surface->draw_pixel(x - 3, y, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 3, y, color, Z_ORDER_LEVEL_0);

        s_surface->draw_pixel(x - 3, y + 1, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 3, y + 1, color, Z_ORDER_LEVEL_0);

        s_surface->draw_pixel(x - 2, y + 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x - 1, y + 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x, y + 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 1, y + 2, color, Z_ORDER_LEVEL_0);
        s_surface->draw_pixel(x + 2, y + 2, color, Z_ORDER_LEVEL_0);
    }

    float x, y, vx, vy;
    unsigned int color;
};
/*****************/

/*******思维点的连线类*********/
class Bond
{
public:
    Bond()
    {
        m0 = m1 = 0;
        x0 = y0 = x1 = y1 = 0;
    }

    static void createBond(Molecule *m0, Molecule *m1)
    {
        float distance = sqrtf((m0->x - m1->x) * (m0->x - m1->x) + (m0->y - m1->y) * (m0->y - m1->y));
        static unsigned int _color = randomColor();

        int index = -1;
        for (int i = 0; i < BOND_TOTAL; i++)
        {
            if ((bonds[i].m0 == m0 && bonds[i].m1 == m1) || (bonds[i].m0 == m1 && bonds[i].m1 == m0))
            {
                index = i;
                break;
            }
        }

        if (index >= 0)
        { //has been registered
            if (distance > BOND_MAX_DISTANCE)
            { //unregister
                s_surface->draw_line(bonds[index].x0, bonds[index].y0, bonds[index].x1, bonds[index].y1, GL_RGB(0, 0, 0), Z_ORDER_LEVEL_0);
                bonds[index].m0 = bonds[index].m1 = 0;
                _color = randomColor();
                return;
            }
            else
            { //update bond & draw
                s_surface->draw_line(bonds[index].x0, bonds[index].y0, bonds[index].x1, bonds[index].y1, GL_RGB(0, 0, 0), Z_ORDER_LEVEL_0);
                s_surface->draw_line(m0->x, m0->y, m1->x, m1->y, _color, Z_ORDER_LEVEL_0);
                bonds[index].x0 = m0->x;
                bonds[index].y0 = m0->y;
                bonds[index].x1 = m1->x;
                bonds[index].y1 = m1->y;
            }
            return;
        }

        if (distance > BOND_MAX_DISTANCE)
        {
            return;
        }
        //register new bond
        index = -1;
        for (int i = 0; i < BOND_TOTAL; i++)
        {
            if (bonds[i].m0 == 0 && bonds[i].m1 == 0)
            {
                index = i;
                break;
            }
        }
        if (index < 0)
        { //bonds full
            ASSERT(false);
            return;
        }

        //register
        bonds[index].m0 = m0;
        bonds[index].m1 = m1;
        bonds[index].x0 = m0->x;
        bonds[index].y0 = m0->y;
        bonds[index].x1 = m1->x;
        bonds[index].y1 = m1->y;
        s_surface->draw_line(m0->x, m0->y, m1->x, m1->y, color, Z_ORDER_LEVEL_0);
    }

    Molecule *m0, *m1;
    float x0, y0, x1, y1;
    static const unsigned int color = GL_RGB(0, 162, 232);
    static Bond bonds[BOND_TOTAL];
};
/*****************/

/*******这部分与3D方块相关，暂时还没有移植完善，现在已经能够调用显示方块自动旋转。修好陀螺仪了再说**********/
// Shape
class Shape
{
public:
    Shape() { angle = 0.5; }
    virtual void draw(int x, int y, bool isErase) = 0;
    virtual void rotate() = 0;

protected:
    double angle;
};

class Cube : public Shape
{
public:
    Cube()
    {
        memset(points2d, 0, sizeof(points2d));
    }
    virtual void draw(int x, int y, bool isErase)
    {
        for (int i = 0; i < 4; i++)
        {
            /*
      s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 1) % 4][0] + x, points2d[(i + 1) % 4][1] + y, (isErase) ? 0 : 0xffff0000, Z_ORDER_LEVEL_0);
      s_surface->draw_line(points2d[i + 4][0] + x, points2d[i + 4][1] + y, points2d[((i + 1) % 4) + 4][0] + x, points2d[((i + 1) % 4) + 4][1] + y, (isErase) ? 0 : 0xff00ff00, Z_ORDER_LEVEL_0);
      s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 4)][0] + x, points2d[(i + 4)][1] + y, (isErase) ? 0 : 0xffffff00, Z_ORDER_LEVEL_0);
      */
            s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 1) % 4][0] + x, points2d[(i + 1) % 4][1] + y, (isErase) ? 0 : randomColor(), Z_ORDER_LEVEL_0);
            s_surface->draw_line(points2d[i + 4][0] + x, points2d[i + 4][1] + y, points2d[((i + 1) % 4) + 4][0] + x, points2d[((i + 1) % 4) + 4][1] + y, (isErase) ? 0 : randomColor(), Z_ORDER_LEVEL_0);
            s_surface->draw_line(points2d[i][0] + x, points2d[i][1] + y, points2d[(i + 4)][0] + x, points2d[(i + 4)][1] + y, (isErase) ? 0 : randomColor(), Z_ORDER_LEVEL_0);
        }
    }
    virtual void rotate()
    {
        double rotateOut1[3][1], rotateOut2[3][1], rotateOut3[3][1];
        for (int i = 0; i < 8; i++)
        {
            rotateX(angle, points[i], (double *)rotateOut1);
            rotateY(angle, (double *)rotateOut1, (double *)rotateOut2);
            rotateZ(angle, (double *)rotateOut2, (double *)rotateOut3);
            projectOnXY((double *)rotateOut3, (double *)points2d[i], 1);
        }
        angle += 0.1;
    }

private:
    static double points[8][3];
    double points2d[8][2];
};
/*****************/

double Cube::points[8][3] = //方块大小设置
    {
        {-SHAPE_SIZE, -SHAPE_SIZE, -SHAPE_SIZE}, // x, y, z
        {SHAPE_SIZE, -SHAPE_SIZE, -SHAPE_SIZE},
        {SHAPE_SIZE, SHAPE_SIZE, -SHAPE_SIZE},
        {-SHAPE_SIZE, SHAPE_SIZE, -SHAPE_SIZE},
        {-SHAPE_SIZE, -SHAPE_SIZE, SHAPE_SIZE},
        {SHAPE_SIZE, -SHAPE_SIZE, SHAPE_SIZE},
        {SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE},
        {-SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE}};

Time_clock time_clock;              //时钟
c_star stars[100];                  //星空穿越特效
Molecule molecules[MOLECULE_TOTAL]; //思维点
Bond Bond::bonds[BOND_TOTAL];       //思维点的连线
Cube theCube;                       //方块

void create_ui(void *phy_fb, int screen_width, int screen_height, int color_bytes, struct EXTERNAL_GFX_OP *gfx_op) //ui的初始化函数
{
    /**********这部分使用时几乎不需要修改*******/
    if (phy_fb)
    {
        static c_surface surface(UI_WIDTH, UI_HEIGHT, color_bytes, Z_ORDER_LEVEL_0);
        static c_display display(phy_fb, screen_width, screen_height, &surface);
        s_surface = &surface;
        s_display = &display;
    }
    else
    { //for MCU without framebuffer
        static c_surface_no_fb surface_no_fb(UI_WIDTH, UI_HEIGHT, color_bytes, gfx_op, Z_ORDER_LEVEL_0);
        static c_display display(phy_fb, screen_width, screen_height, &surface_no_fb);
        s_surface = &surface_no_fb;
        s_display = &display;
    }

    //background
    s_surface->fill_rect(0, 0, UI_WIDTH, UI_HEIGHT, 0, Z_ORDER_LEVEL_0);
    /*****************/

    /***********这里是自定义需要的初始化******/
    time_clock.set_hands(CLOCK_RADIUS, HOUR_HAND_LEN, 2, HOUR_HAND_RGB, MINUTE_HAND_LEN, 4, MINUTE_HAND_RGB, SECOND_HAND_LEN, 6, SECOND_HAND_RGB);
    /*****************/
}

void ui_update(int choose)
{
    switch (choose)
    {
    case 0:
    {
        /* 时钟显示使用范例
        */
        if (!time_clock.clock_enter_animation.isComplete())
        {
            time_clock.clock_enter_animation.show();
        }
        if (time_clock.clock_enter_animation.isComplete())
        {
            time_clock.show();
        }
    }
    break;
    case 1:
    {
        /*星空背景特效使用范例
        */
        for (int i = 0; i < sizeof(stars) / sizeof(c_star); i++)
        {
            stars[i].move();
        }
    }
    break;
    case 2:
    {
        /*思维点及连线使用范例
        */
        for (int i = 0; i < MOLECULE_TOTAL; i++)
        {
            molecules[i].move();
        }

        for (int i = 0; i < MOLECULE_TOTAL; i++)
        {
            for (int sub_i = i + 1; sub_i < MOLECULE_TOTAL; sub_i++)
            {
                Bond::createBond(&molecules[i], &molecules[sub_i]);
            }
        }
    }
    break;
    case 3:
    {
        /*3D方块使用范例
        */
        //theCube.draw(120, 100, true);//erase footprint
        theCube.rotate();
        theCube.draw(120, 120, false); //refresh cube
    }
    break;
    default:
        break;
    }
}

unsigned short int randomColor()
{
    unsigned char red, green, blue;
    red = rand();
    //__ASM("NOP");
    green = rand();
    //__ASM("NOP");
    blue = rand();
    return (red << 11 | green << 5 | blue);
}

/*********填充圆用到的工具函数********/
void screen_draw_fastVLine(int32_t x, int32_t y, int32_t length, uint16_t color)
{
    // Bounds check
    int32_t y0 = y;
    do
    {
        s_surface->draw_pixel(x, y, color, Z_ORDER_LEVEL_0); // 逐点显示，描出垂直线
        y++;
    } while (y0 + length >= y);
}

void screen_fillCircle_helper(int32_t x0, int32_t y0, int32_t r, uint8_t corner, int32_t delta, uint16_t color)
{

    int32_t f = 1 - r;
    int32_t ddF_x = 1;
    int32_t ddF_y = -2 * r;
    int32_t x = 0;
    int32_t y = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (corner & 0x1)
        {
            screen_draw_fastVLine(x0 + x, y0 - y, 2 * y + delta, color);
            screen_draw_fastVLine(x0 + y, y0 - x, 2 * x + delta, color);
        }
        if (corner & 0x2)
        {
            screen_draw_fastVLine(x0 - x, y0 - y, 2 * y + delta, color);
            screen_draw_fastVLine(x0 - y, y0 - x, 2 * x + delta, color);
        }
    }
}
/*****************/

void screen_fill_circle(int32_t x0, int32_t y0, int32_t r, uint16_t color)
{
    screen_draw_fastVLine(x0, y0 - r, 2 * r, color);
    screen_fillCircle_helper(x0, y0, r, 3, 0, color);
}

// 3D engine
void multiply(int m, int n, int p, double *a, double *b, double *c) // a[m][n] * b[n][p] = c[m][p]
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < p; j++)
        {
            c[i * p + j] = 0;
            for (int k = 0; k < n; k++)
            {
                c[i * p + j] += a[i * n + k] * b[k * p + j];
            }
        }
    }
}

void rotateX(double angle, double *point, double *output) // rotate matrix for X
{
    static double rotation[3][3];
    rotation[0][0] = 1;
    rotation[1][1] = cos(angle);
    rotation[1][2] = 0 - sin(angle);
    rotation[2][1] = sin(angle);
    rotation[2][2] = cos(angle);
    multiply(3, 3, 1, (double *)rotation, point, output);
}

void rotateY(double angle, double *point, double *output) // rotate matrix for Y
{
    static double rotation[3][3];
    rotation[0][0] = cos(angle);
    rotation[0][2] = sin(angle);
    rotation[1][1] = 1;
    rotation[2][0] = 0 - sin(angle);
    rotation[2][2] = cos(angle);
    multiply(3, 3, 1, (double *)rotation, point, output);
}

void rotateZ(double angle, double *point, double *output) // rotate matrix for Z
{
    static double rotation[3][3];
    rotation[0][0] = cos(angle);
    rotation[0][1] = 0 - sin(angle);
    rotation[1][0] = sin(angle);
    rotation[1][1] = cos(angle);
    rotation[2][2] = 1;
    multiply(3, 3, 1, (double *)rotation, point, output);
}

void projectOnXY(double *point, double *output, double zFactor)
{
    static double projection[2][3]; //project on X/Y face
    projection[0][0] = zFactor;     //the raio of point.z and camera.z
    projection[1][1] = zFactor;     //the raio of point.z and camera.z
    multiply(2, 3, 1, (double *)projection, point, output);
}
