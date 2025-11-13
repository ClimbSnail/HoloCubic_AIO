#include "game_snake.h"
#include "game_snake_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "freertos/semphr.h"

// Game name
#define GAME_APP_NAME "Snake"

#define SNAKE_SPEED 1000

struct SnakeAppRunData
{
    unsigned int score;
    int gameStatus;
    BaseType_t xReturned_task_run = pdFALSE;
    TaskHandle_t xHandle_task_run = NULL;
};

static SnakeAppRunData *run_data = NULL;

void taskRun(void *parameter)
{
    while (1)
    {
        // LVGL main task function, handles all LVGL tasks including UI rendering and user input processing
        AIO_LVGL_OPERATE_LOCK(lv_task_handler();)
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    Serial.println("Ending lv_task_handler");
    vTaskDelete(NULL);
}

static int game_snake_init(AppController *sys)
{
    // Random seed
    randomSeed(analogRead(A0));
    // Initialize runtime parameters
    game_snake_gui_init();
    // Initialize runtime parameters
    run_data = (SnakeAppRunData *)calloc(1, sizeof(SnakeAppRunData));
    run_data->score = 0;
    run_data->gameStatus = 0;
    run_data->xReturned_task_run = xTaskCreate(
        taskRun,                      /* Task function */
        "taskRun",                    /* Task name */
        8 * 1024,                     /* Stack size in bytes */
        NULL,                         /* Parameter */
        1,                            /* Priority */
        &run_data->xHandle_task_run); /* Task handle */

    return 0;
}

static void game_snake_process(AppController *sys, const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        run_data->gameStatus = -1;
        sys->app_exit(); // Exit APP
        return;
    }

    // Operation trigger
    if (TURN_RIGHT == act_info->active)
    {
        update_driection(DIR_RIGHT);
    }
    else if (TURN_LEFT == act_info->active)
    {
        update_driection(DIR_LEFT);
    }
    else if (UP == act_info->active)
    {
        update_driection(DIR_UP);
    }
    else if (DOWN == act_info->active)
    {
        update_driection(DIR_DOWN);
    }

    if (run_data->gameStatus == 0 && run_data->xReturned_task_run == pdPASS)
    {
        AIO_LVGL_OPERATE_LOCK(display_snake(run_data->gameStatus, LV_SCR_LOAD_ANIM_NONE););
    }

    // Speed control
    delay(SNAKE_SPEED);
}

static void game_snake_background_task(AppController *sys,
                                       const ImuAction *act_info)
{
    // This is a background task, the main controller will call this function every minute
    // This function should only use "permanent data", other variables may have been released due to lifecycle

    // Send request. If it's a WiFi-related message, game_snake_message_handle will be called automatically when the request completes
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // You can also remove your own background task, best placed in APP-controllable location
    // sys->remove_backgroud_task();

    // Add delay if needed
    // delay(300);
}

static int game_snake_exit_callback(void *param)
{
    // Kill task
    if (run_data->xReturned_task_run == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_run);
    }

    // Release lvgl_mutex semaphore
    xSemaphoreGive(lvgl_mutex);

    // Release page resources
    game_snake_gui_del();

    // Release event resources
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void game_snake_message_handle(const char *from, const char *to,
                                      APP_MESSAGE_TYPE type, void *message,
                                      void *ext_info)
{
    // Currently mainly WiFi on/off events (for power control)
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // WiFi heartbeat response, no processing required
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
    }
    break;
    default:
        break;
    }
}

APP_OBJ game_snake_app = {GAME_APP_NAME, &app_game_snake, "",
                          game_snake_init, game_snake_process, game_snake_background_task,
                          game_snake_exit_callback, game_snake_message_handle};
