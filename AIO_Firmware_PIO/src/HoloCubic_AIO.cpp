/***************************************************
  HoloCubic Multi-Functional Firmware Source Code
  (If referencing the source code from this project, please cite the source)

  Integrates multiple APPs, including built-in weather, clock, photo album, special effects animation, video playback, video projection,
  and file modification through browser. (For specific usage of each APP, refer to the manual)

  Github repositories：https://github.com/ClimbSnail/HoloCubic_AIO

  Last review/edit by ClimbSnail: 2023/01/14
 ****************************************************/

#include "driver/lv_port_indev.h" // Include the header file for the LVGL input device driver
#include "driver/lv_port_fs.h" // Include the header file for the LVGL file system driver

#include "common.h" // Include the header file for common functions
#include "sys/app_controller.h" // Include the header file for the application controller

#include "app/app_conf.h" // Include the header file for the application configuration

#include <SPIFFS.h> // Include the header file for the SPIFFS library
#include <esp32-hal.h> // Include the header file for the ESP32 hardware abstraction layer
#include <esp32-hal-timer.h> // Include the header file for the ESP32 hardware timer

bool isCheckAction = false; // Flag to indicate if action needs to be checked

/*** Component objects ***/
ImuAction *act_info; // Pointer to store the data returned by MPU6050
AppController *app_controller; // Pointer to the application controller

TaskHandle_t handleTaskLvgl; // Task handle for the LVGL update task

// Task to update LVGL
void TaskLvglUpdate(void *parameter)
{
    for (;;)
    {
        AIO_LVGL_OPERATE_LOCK(lv_timer_handler();)
        vTaskDelay(5);
    }
}

TimerHandle_t xTimerAction = NULL; // Timer handle for action check timer

// Timer callback function for action check
void actionCheckHandle(TimerHandle_t xTimer)
{
    // Set the flag to indicate that action needs to be checked
    isCheckAction = true;
}

// Function to print a string to the serial monitor
void my_print(const char *buf)
{
    Serial.printf("%s", buf); // Print the string to the serial monitor
    Serial.flush(); // Flush the serial buffer to ensure the data is sent immediately
}

void setup()
{
    // Begin serial communication for debugging
    Serial.begin(115200);

    // Print the version of the All-In-One (AIO) application and the unique identifier (MAC ID) of the chip
    Serial.println(F("\nAIO (All in one) version " AIO_VERSION "\n"));
    Serial.print(F("ChipID(EfuseMac): "));
    Serial.println(ESP.getEfuseMac());

    // Create an instance of AppController, which controls the application's behavior
    app_controller = new AppController();

    // Initialize the SPIFFS (SPI Flash File System), a file system for SPI NOR flash devices on embedded systems
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

#ifdef PEAK
    // Set up some pins for input and output, likely for controlling hardware components
    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT);
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);
    // Enable power
    pinMode(CONFIG_POWER_EN_PIN, OUTPUT);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    digitalWrite(CONFIG_POWER_EN_PIN, HIGH);
#endif

    // Read configuration data for the system, MPU (Motion Processing Unit), and RGB settings
    app_controller->read_config(&app_controller->sys_cfg);
    app_controller->read_config(&app_controller->mpu_cfg);
    app_controller->read_config(&app_controller->rgb_cfg);

    // Initialize the screen, on-board RGB, ambient-light sensor, and micro SD-Card
    screen.init(app_controller->sys_cfg.rotation, app_controller->sys_cfg.backLight);
    rgb.init();
    rgb.setBrightness(0.05).setRGB(0, 64, 64);
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);
    tf.init();

    // Initialize the file system for the micro SD-Card
    lv_fs_fatfs_init();

    // Initialize the app controller
    app_controller->init();

    // Register apps to the app controller based on the project's needs
#if APP_WEATHER_USE
    app_controller->app_install(&weather_app);
#endif
    // ... (other apps are registered in a similar way)

    // Start the default app
    app_controller->app_auto_start();

    // Display the main screen to speed up the perceived boot time
    app_controller->main_process(&mpu.action_info);

    // Initialize the IMU (Inertial Measurement Unit) as an input device
    mpu.init(app_controller->sys_cfg.mpu_order, app_controller->sys_cfg.auto_calibration_mpu, &app_controller->mpu_cfg);

    // Set up the RGB LED with the settings from the configuration
    RgbConfig *rgb_cfg = &app_controller->rgb_cfg;
    RgbParam rgb_setting = {LED_MODE_HSV, rgb_cfg->min_value_0, rgb_cfg->min_value_1, rgb_cfg->min_value_2,
                            rgb_cfg->max_value_0, rgb_cfg->max_value_1, rgb_cfg->max_value_2,
                            rgb_cfg->step_0, rgb_cfg->step_1, rgb_cfg->step_2,
                            rgb_cfg->min_brightness, rgb_cfg->max_brightness,
                            rgb_cfg->brightness_step, rgb_cfg->time};
    set_rgb_and_run(&rgb_setting, RUN_MODE_TASK);

    // Initialize the action data from the MPU6050 to prevent null pointer issues
    act_info = mpu.getAction();

    // Create a timer to check the actions of the MPU6050 every 200 milliseconds
    xTimerAction = xTimerCreate("Action Check", 200 / portTICK_PERIOD_MS, pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);
}
/**
 * @brief The main loop function of the program.
 * 
 * This function is called repeatedly in the program's main execution loop.
 * It performs the following tasks:
 * 1. Calls the `routine()` function of the `screen` object.
 * 2. Checks if the `mpu` object's encoder is pushed.
 *    - If not pushed, it prints "mpu.Encoder_GetIsPush()1" to the serial monitor and waits for 1 second.
 *    - If still not pushed, it prints "mpu.Encoder_GetIsPush()2" to the serial monitor and turns off the power by setting the `CONFIG_POWER_EN_PIN` to LOW.
 * 3. Checks if `isCheckAction` flag is set.
 *    - If set, it retrieves the action information from the `mpu` object and stores it in the `act_info` variable.
 * 4. Calls the `main_process()` function of the `app_controller` object, passing the `act_info` as an argument.
 * 5. (Commented out) Prints the ambient light intensity divided by 50 to the serial monitor.
 * 6. (Commented out) Sets the brightness of the `rgb` object based on the ambient light intensity divided by 500.
 */
void loop()
{
    screen.routine(); // Call the routine() function of the screen object to update the screen

#ifdef PEAK
    if (!mpu.Encoder_GetIsPush()) // Check if the encoder of the mpu object is not pushed
    {
        Serial.println("mpu.Encoder_GetIsPush()1"); // Print "mpu.Encoder_GetIsPush()1" to the serial monitor
        delay(1000); // Wait for 1 second
        if (!mpu.Encoder_GetIsPush()) // Check if the encoder of the mpu object is still not pushed
        {
            Serial.println("mpu.Encoder_GetIsPush()2"); // Print "mpu.Encoder_GetIsPush()2" to the serial monitor
            // Turn off the power by setting the CONFIG_POWER_EN_PIN to LOW
            digitalWrite(CONFIG_POWER_EN_PIN, LOW);
        }
    }
#endif
    if (isCheckAction) // Check if the isCheckAction flag is set
    {
        isCheckAction = false; // Reset the isCheckAction flag
        act_info = mpu.getAction(); // Retrieve the action information from the mpu object and store it in the act_info variable
    }
    app_controller->main_process(act_info); // Call the main_process() function of the app_controller object, passing the act_info as an argument
    // Serial.println(ambLight.getLux() / 50.0); // (Commented out) Print the ambient light intensity divided by 50 to the serial monitor
    // rgb.setBrightness(ambLight.getLux() / 500.0); // (Commented out) Set the brightness of the rgb object based on the ambient light intensity divided by 500
}
