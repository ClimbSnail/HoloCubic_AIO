#include "idea.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include <stdint.h>
#include "ui_animation.h"

#define IDEA_APP_NAME "Idea"

// Due to slow TFT screen refresh, first allocate a screen resolution-sized space as the image to display
// All drawing operations are performed in virtual space, then call TFT's image display function to show the image

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 240

uint8_t *screen_buf = NULL;
int choose = 0;

/* These two functions only operate on the image, not the screen */
static void screen_clear(uint16_t color);                            // Screen clear function declaration
static void screen_draw_pixel(int32_t x, int32_t y, uint16_t color); // Pixel draw function declaration
/**********************************/

static void gfx_draw_pixel(int x, int y, unsigned int rgb) // Specify GUI library's pixel draw function
{
    screen_draw_pixel(x, y, rgb);
}

struct EXTERNAL_GFX_OP
{
    void (*draw_pixel)(int x, int y, unsigned int rgb);
    void (*fill_rect)(int x0, int y0, int x1, int y1, unsigned int rgb);
} my_gfx_op;

void screen_clear(uint16_t color)
{
    int32_t i = 0;
    int32_t j = 0;

    for (i = 0; i < SCREEN_HEIGHT; ++i)
    {
        for (j = 0; j < SCREEN_WIDTH; ++j)
        {
            screen_buf[i * SCREEN_WIDTH + j] = color;
        }
    }
}

void screen_draw_pixel(int32_t x, int32_t y, uint16_t color) // Specify GUI library's pixel draw function
{
    if ((x >= SCREEN_WIDTH) || (y >= SCREEN_HEIGHT))
        return;
    if ((x < 0) || (y < 0))
        return;
    screen_buf[y * SCREEN_WIDTH + x] = color;
}

static int idea_init(AppController *sys)
{
    screen_buf = (uint8_t *)malloc(SCREEN_HEIGHT * SCREEN_WIDTH); // Dynamically allocate screen resolution-sized space
    if (screen_buf == NULL)
        Serial.println("screen_buf: error");
    else
    {
        Serial.println("screen_buf: OK");
    }
    // Link your LCD driver & start UI:
    my_gfx_op.draw_pixel = gfx_draw_pixel;                       // Specify GuiLite library's pixel draw function
    my_gfx_op.fill_rect = NULL;                                  // gfx_fill_rect;
    create_ui(NULL, SCREEN_WIDTH, SCREEN_HEIGHT, 2, &my_gfx_op); // UI initialization
    screen_clear(0x0000);
    return 0;
}

static void idea_process(AppController *sys,
                         const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == action->active)
    {
        choose = (choose + 1) % 4;
        screen_clear(0x0000);
        delay(500);
    }
    else if (TURN_LEFT == action->active)
    {
        choose = (choose + 4 - 1) % 4;
        screen_clear(0x0000);
        delay(500);
    }

    // Clear screen with black background
    screen_clear(0x0000);                                          // Add code to clear old video memory
    ui_update(choose);                                             // UI update - all effects are called here
    tft->pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_buf); // Display image
    delay(20);                                                     // Change this delay function to adjust effect playback speed
}

static void idea_background_task(AppController *sys,
                                 const ImuAction *act_info)
{
    // This is a background task, the main controller will call this function every minute
    // This function should only use "permanent data", other variables may have been released due to lifecycle
}

static int idea_exit_callback(void *param)
{
    if (NULL != screen_buf)
    {
        free(screen_buf);
        screen_buf = NULL;
    }
    return 0;
}

static void idea_message_handle(const char *from, const char *to,
                                APP_MESSAGE_TYPE type, void *message,
                                void *ext_info)
{
}

APP_OBJ idea_app = {IDEA_APP_NAME, &app_idea, "", idea_init,
                    idea_process, idea_background_task, idea_exit_callback,
                    idea_message_handle};