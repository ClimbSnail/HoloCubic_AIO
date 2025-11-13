#include "example.h"
#include "example_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#define EXAMPLE_APP_NAME "Example"

// Dynamic data, needs to be released when APP lifecycle ends
struct ExampleAppRunData
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

// Permanent data, not released or deleted with APP lifecycle
struct ExampleAppForeverData
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

// Stores APP runtime parameters, recommended to release in xxx_exit_callback when closing APP
static ExampleAppRunData *run_data = NULL;

// You can also add a small amount of constant variables in memory (no need to release on exit, can be read on second startup)
// Considering all APPs share memory, minimize forever_data usage
static ExampleAppForeverData forever_data;

static int example_init(AppController *sys)
{
    // Initialize runtime parameters
    example_gui_init();
    // Initialize runtime parameters
    run_data = (ExampleAppRunData *)calloc(1, sizeof(ExampleAppRunData));
    run_data->val1 = 0;
    run_data->val2 = 0;
    run_data->val3 = 0;
    // Use variables from forever_data, any function can use them
    Serial.print(forever_data.val1);

    // If persistent configuration file is needed, call this function to store data in flash
    // Configuration file name should start with APP name and end with ".cfg" to avoid conflicts between multiple APPs
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile("/example.cfg", (uint8_t *)info);
    // Parse data
    // Save configuration data to file (persistence)
    g_flashCfg.writeFile("/example.cfg", "value1=100\nvalue2=200");

    return 0;
}

static void example_process(AppController *sys,
                            const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // Exit APP
        return;
    }
    // Send request. If it's a WiFi-related message, example_message_handle will be called automatically when the request completes
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // Add delay if needed
    // delay(300);
}

static void example_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // This is a background task, the main controller will call this function every minute
    // This function should only use "permanent data", other variables may have been released due to lifecycle

    // Send request. If it's a WiFi-related message, example_message_handle will be called automatically when the request completes
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // You can also remove your own background task, best placed in APP-controllable location
    // sys->remove_backgroud_task();

    // Add delay if needed
    // delay(300);
}

static int example_exit_callback(void *param)
{
    // Release resources
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void example_message_handle(const char *from, const char *to,
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
        char *param_key = (char *)message;
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
    }
    break;
    default:
        break;
    }
}

APP_OBJ example_app = {EXAMPLE_APP_NAME, &app_example, "Author HQ\nVersion 2.0.0\n",
                       example_init, example_process, example_background_task,
                       example_exit_callback, example_message_handle};
