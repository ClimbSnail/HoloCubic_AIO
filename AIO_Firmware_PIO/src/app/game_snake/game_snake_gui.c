#include "game_snake_gui.h"
#include "lvgl.h"
#include "esp_system.h"
#include "esp_log.h"

// 定义屏幕尺寸
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// 贪吃蛇的最大长度
#define SNAKE_SIZE 100

// 贪吃蛇的数据结构
typedef struct
{
    int x;
    int y;
    lv_obj_t *body;
} SnakeSegment;

SnakeSegment snake[SNAKE_SIZE];  // 贪吃蛇的身体
int snakeLength = 1;             // 贪吃蛇的长度
bool gameOver = false;           // 游戏是否结束
Direction direction = DIR_RIGHT; // 贪吃蛇的初始移动方向
static int executed = 0;

// 食物的位置
int foodX;
int foodY;

lv_obj_t *game_snake_gui = NULL;
lv_obj_t *game_snake_area = NULL;
lv_obj_t *game_snake_score = NULL;
lv_obj_t *score_label = NULL;
lv_obj_t *snake_head = NULL;
lv_obj_t *food = NULL;
lv_obj_t *game_over_label = NULL;

static lv_style_t default_style;
static lv_style_t score_style;
static lv_style_t head_style;
static lv_style_t body_style;
static lv_style_t food_style;
static lv_style_t over_style;

static lv_anim_t game_over_label_anim;

void game_snake_gui_init(void)
{
    // 数据初始化
    gameOver = false;
    generate_food();
    snake[0].x = foodX + 20;
    snake[0].y = foodY;
    snake[0].body = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == game_snake_gui)
        return;
    lv_obj_clean(act_obj); // 清空此前页面

    // 创建屏幕对象
    game_snake_gui = lv_obj_create(NULL);
    lv_obj_add_style(game_snake_gui, &default_style, LV_STATE_DEFAULT);

    // 贪吃蛇游戏区域绘制
    game_snake_area = lv_obj_create(game_snake_gui);
    lv_obj_set_size(game_snake_area, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(game_snake_area, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(game_snake_area, &default_style, LV_STATE_DEFAULT);

    // 得分区域绘制
    game_snake_score = lv_obj_create(game_snake_gui);
    lv_obj_set_size(game_snake_score, SCREEN_WIDTH, 40);
    lv_obj_align(game_snake_score, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(game_snake_score, &default_style, LV_STATE_DEFAULT);

    // 分数绘制
    score_label = lv_label_create(game_snake_score);
    lv_label_set_text(score_label, "Score: 0");
    lv_obj_align(score_label, LV_ALIGN_CENTER, 0, 0);
    lv_style_init(&score_style);
    lv_style_set_text_color(&score_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&score_style, &lv_font_montserrat_24);
    lv_obj_add_style(score_label, &score_style, LV_STATE_DEFAULT);

    // 创建贪吃蛇头部
    snake_head = lv_obj_create(game_snake_area);
    lv_obj_set_pos(snake_head, snake[0].x, snake[0].y);
    lv_obj_set_size(snake_head, 15, 15);
    lv_style_init(&head_style);
    lv_style_set_bg_color(&head_style, lv_color_hex(0x00ff00));
    lv_style_set_border_color(&head_style, lv_color_hex(0xffffff));
    lv_obj_add_style(snake_head, &head_style, LV_STATE_DEFAULT);

    // 创建食物
    food = lv_obj_create(game_snake_area);
    lv_obj_set_pos(food, foodX, foodY);
    lv_obj_set_size(food, 15, 15);
    lv_style_init(&food_style);
    lv_style_set_bg_color(&food_style, lv_color_hex(0xff0000));
    lv_style_set_border_color(&food_style, lv_color_hex(0xff0000));
    lv_obj_add_style(food, &food_style, LV_STATE_DEFAULT);

    // 屏幕重载
    lv_scr_load(game_snake_gui);
}

/*
 * 其他函数请根据需要添加
 */
void display_snake(int gameStatus, lv_scr_load_anim_t anim_type)
{
    if (gameOver)
    {
        if (!executed)
        {
            executed = 1;
            // 游戏结束label动画
            lv_anim_init(&game_over_label_anim);
            lv_anim_set_var(&game_over_label_anim, game_over_label);
            lv_anim_set_exec_cb(&game_over_label_anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&game_over_label_anim, -SCREEN_WIDTH, SCREEN_WIDTH);
            lv_anim_set_time(&game_over_label_anim, 5000);
            lv_anim_set_repeat_count(&game_over_label_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_set_playback_time(&game_over_label_anim, 5000);
            lv_anim_set_playback_delay(&game_over_label_anim, 100);
            lv_anim_start(&game_over_label_anim);
        }

        return;
    }

    // 移动贪吃蛇的身体
    for (int i = snakeLength - 1; i > 0; i--)
    {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
        if (snake[i].body != NULL)
            lv_obj_set_pos(snake[i].body, snake[i].x, snake[i].y);
    }

    // 根据移动方向更新贪吃蛇的头部位置
    switch (direction)
    {
    case DIR_UP:
        snake[0].y -= 10;
        break;
    case DIR_DOWN:
        snake[0].y += 10;
        break;
    case DIR_LEFT:
        snake[0].x -= 10;
        break;
    case DIR_RIGHT:
        snake[0].x += 10;
        break;
    }

    // 更新贪吃蛇头部的位置
    lv_obj_set_pos(snake_head, snake[0].x, snake[0].y);

    // 检测是否吃到食物
    if (snake[0].x == foodX && snake[0].y == foodY)
    {
        // 增加贪吃蛇的长度
        snakeLength++;

        // 更新分数
        char score[20];
        sprintf(score, "Score: %d", snakeLength - 1);
        lv_label_set_text(score_label, score);

        // 重新生成食物的位置
        generate_food();
        lv_obj_set_pos(food, foodX, foodY);

        // 初始化新的身体部分
        snake[snakeLength - 1].x = snake[snakeLength - 2].x;
        snake[snakeLength - 1].y = snake[snakeLength - 2].y;
        snake[snakeLength - 1].body = lv_obj_create(game_snake_area);
        lv_obj_set_pos(snake[snakeLength - 1].body, snake[snakeLength - 1].x, snake[snakeLength - 1].y);
        lv_obj_set_size(snake[snakeLength - 1].body, 10, 10);
        lv_style_init(&body_style);
        lv_style_set_bg_color(&body_style, lv_color_hex(0x00ff00));
        lv_style_set_border_color(&body_style, lv_color_hex(0x00ff00));
        lv_obj_add_style(snake[snakeLength - 1].body, &body_style, LV_STATE_DEFAULT);
    }

    // 检测是否撞墙
    if (snake[0].x < 0 || snake[0].x >= SCREEN_WIDTH - 30 || snake[0].y < 0 || snake[0].y >= SCREEN_HEIGHT - 70)
        gameOver = true;

    // 检测是否撞到自己
    for (int i = 1; i < snakeLength; i++)
    {
        if (i > 3 && snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            gameOver = true;
            break;
        }
    }

    if (gameOver)
    {
        // 游戏结束
        game_over_label = lv_label_create(game_snake_area);
        lv_label_set_text(game_over_label, "Game Over!");
        lv_obj_align(game_over_label, LV_ALIGN_CENTER, 0, 0);
        lv_style_init(&over_style);
        lv_style_set_text_color(&over_style, lv_color_hex(0xff0000));
        lv_style_set_text_font(&over_style, &lv_font_montserrat_24);
        lv_obj_add_style(game_over_label, &over_style, LV_STATE_DEFAULT);
    }
}

void game_snake_gui_del(void)
{

    // 释放贪吃蛇的身体
    for (int i = 0; i < snakeLength; i++)
    {
        if (snake[i].body != NULL)
        {
            lv_obj_del(snake[i].body);
            snake[i].body = NULL;
        }
    }
    snakeLength = 1;
    executed = 0;

    // 释放贪吃蛇头部
    if (NULL != game_snake_gui)
    {
        lv_obj_clean(game_snake_gui);
        game_snake_gui = NULL;
    }

    // 手动清除样式，防止内存泄漏
    lv_style_reset(&food_style);
    lv_style_reset(&body_style);
    lv_style_reset(&head_style);
    lv_style_reset(&score_style);
    lv_style_reset(&over_style);
    lv_style_reset(&default_style);
}

// 生成食物的位置
void generate_food()
{
    // 随机生成食物的位置
    foodX = rand() % ((SCREEN_WIDTH - 30) / 10) * 10;
    foodY = rand() % ((SCREEN_HEIGHT - 70) / 10) * 10;
}

void update_driection(Direction dir)
{
    // 长度大于1时，不允许反向移动
    if (snakeLength > 1)
    {
        switch (direction)
        {
        case DIR_UP:
            if (dir == DIR_DOWN)
                return;
            break;
        case DIR_DOWN:
            if (dir == DIR_UP)
                return;
            break;
        case DIR_LEFT:
            if (dir == DIR_RIGHT)
                return;
            break;
        case DIR_RIGHT:
            if (dir == DIR_LEFT)
                return;
            break;
        }
    }

    // 更新移动方向
    direction = dir;
}
