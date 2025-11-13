#include "media_player.h"
#include "media_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "driver/sd_card.h"
#include "docoder.h"
#include "DMADrawer.h"

#define MEDIA_PLAYER_APP_NAME "Media"

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOVIE_PATH "/movie"
#define NO_TRIGGER_ENTER_FREQ_160M 90000UL // No operation timeout to enter 160MHz frequency mode (90s)
#define NO_TRIGGER_ENTER_FREQ_80M 120000UL // No operation timeout to enter 80MHz frequency mode (120s)

// Media player persistent configuration
#define MEDIA_CONFIG_PATH "/media.cfg"
struct MP_Config
{
    uint8_t switchFlag; // Whether to automatically play the next video (0: no switch, 1: auto switch)
    uint8_t powerFlag;  // Power consumption control (0: low heat, 1: performance priority)
};

static void write_config(MP_Config *cfg)
{
    char tmp[16];
    // Save configuration data to file (persist)
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->switchFlag);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->powerFlag);
    w_data += tmp;
    g_flashCfg.writeFile(MEDIA_CONFIG_PATH, w_data.c_str());
}

static void read_config(MP_Config *cfg)
{
    // If there is a need to persist the configuration file, you can call this function to store data in flash
    // It is recommended to name the configuration file with the APP name as the prefix and ".cfg" as the suffix to avoid confusion when multiple APPs read it
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(MEDIA_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // Default values
        cfg->switchFlag = 1; // Whether to automatically play the next video (0: no switch, 1: auto switch)
        cfg->powerFlag = 0;  // Power consumption control (0: low heat, 1: performance priority)
        write_config(cfg);
    }
    else
    {
        // Parse data
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->switchFlag = atol(param[0]);
        cfg->powerFlag = atol(param[1]);
    }
}

struct MediaAppRunData
{
    PlayDocoderBase *player_docoder;
    unsigned long preTriggerKeyMillis; // Timestamp of the most recent key trigger
    int movie_pos_increate;
    File_Info *movie_file; // Pointer to the head of the file list in the movie folder
    File_Info *pfile;      // Pointer to the currently playing file node
    File file;
};

static MP_Config cfg_data;
static MediaAppRunData *run_data = NULL;

static File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    // Get the next file of type FILE_TYPE_FILE from p_cur_file (i.e., the next non-folder file)
    if (NULL == p_cur_file)
    {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file)
    {
        if (FILE_TYPE_FILE == pfile->file_type)
        {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    return pfile;
}

static bool video_start(bool create_new)
{
    if (NULL == run_data->pfile)
    {
        // Video folder is empty, exit
        return false;
    }

    if (true == create_new)
    {
        run_data->pfile = get_next_file(run_data->pfile, run_data->movie_pos_increate);
    }

    char file_name[FILENAME_MAX_LEN] = {0};
    snprintf(file_name, FILENAME_MAX_LEN, "%s/%s", run_data->movie_file->file_name, run_data->pfile->file_name);

    run_data->file = tf.open(file_name);
    if (NULL != strstr(run_data->pfile->file_name, ".mjpeg") || NULL != strstr(run_data->pfile->file_name, ".MJPEG"))
    {
        // Directly decode MJPEG format video
        run_data->player_docoder = new MjpegPlayDocoder(&run_data->file, true);
        Serial.print(F("MJPEG video start --------> "));
    }
    else if (NULL != strstr(run_data->pfile->file_name, ".rgb") || NULL != strstr(run_data->pfile->file_name, ".RGB"))
    {
        // Use RGB format video
        run_data->player_docoder = new RgbPlayDocoder(&run_data->file, true);
        Serial.print(F("RGB565 video start --------> "));
    }

    Serial.println(file_name);
    return true;
}

static void release_player_docoder(void)
{
    // Release the specific player object
    if (NULL != run_data->player_docoder)
    {
        delete run_data->player_docoder;
        run_data->player_docoder = NULL;
    }
}

static int media_player_init(AppController *sys)
{
    // Adjust RGB mode to HSV color mode
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.20, 0.001, 50};
    set_rgb_and_run(&rgb_setting);

    // Get configuration information
    read_config(&cfg_data);
    // Initialize runtime parameters
    // run_data = (MediaAppRunData *)malloc(sizeof(MediaAppRunData));
    // memset(run_data, 0, sizeof(MediaAppRunData));
    run_data = (MediaAppRunData *)calloc(1, sizeof(MediaAppRunData));
    run_data->player_docoder = NULL;
    run_data->movie_pos_increate = 1;
    run_data->movie_file = NULL; // Pointer to the head of the file list in the movie folder
    run_data->pfile = NULL;      // Pointer to the currently playing file node
    run_data->preTriggerKeyMillis = GET_SYS_MILLIS();

    run_data->movie_file = tf.listDir(MOVIE_PATH);
    if (NULL != run_data->movie_file)
    {
        run_data->pfile = get_next_file(run_data->movie_file->next_node, 1);
    }

    // Set CPU frequency
    setCpuFrequencyMhz(240);

    // Create player
    video_start(false);
    return 0;
}

static void media_player_process(AppController *sys,
                                 const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // Exit APP
        return;
    }
    else if (UNKNOWN != act_info->active)
    {
        // Record the operation timestamp
        run_data->preTriggerKeyMillis = GET_SYS_MILLIS();
        // Set CPU frequency
        setCpuFrequencyMhz(240);
    }

    if (NULL == run_data->pfile)
    {
        Serial.println(F("Not Found File."));
        sys->app_exit(); // Exit APP
        return;
    }

    if (TURN_RIGHT == act_info->active || TURN_LEFT == act_info->active)
    {
        // Switch direction
        if (TURN_RIGHT == act_info->active)
        {
            run_data->movie_pos_increate = 1;
        }
        else if (TURN_LEFT == act_info->active)
        {
            run_data->movie_pos_increate = -1;
        }
        // End playback
        release_player_docoder();
        run_data->file.close(); // Try to close the file

        // Create player
        video_start(true);
        vTaskDelay(400 / portTICK_PERIOD_MS); // Delay playback to avoid hand shake
    }

    if (NULL == run_data->pfile)
    {
        // No playable files exist
        sys->app_exit(); // Exit APP
        return;
    }

    // Frequency control to reduce heat generation
    if (getCpuFrequencyMhz() > 80 && 0 == cfg_data.powerFlag)
    {
        if (getCpuFrequencyMhz() > 160 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_160M)
        {
            // Set CPU frequency
            setCpuFrequencyMhz(160);
        }
        else if (getCpuFrequencyMhz() > 80 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_80M)
        {
            setCpuFrequencyMhz(80);
        }
    }

    if (!run_data->file)
    {
        Serial.println(F("Failed to open file for reading"));
        return;
    }

    if (run_data->file.available())
    {
        // Play one frame of data
        run_data->player_docoder->video_play_screen();
    }
    else
    {
        // End playback
        release_player_docoder();
        run_data->file.close();
        if (0 == cfg_data.switchFlag)
        {
            // Create player (repeat playback)
            video_start(false);
        }
        else
        {
            // Create player (play next video)
            video_start(true);
        }
    }
}

static void media_player_background_task(AppController *sys,
                                         const ImuAction *act_info)
{
    // This function is a background task, the main controller will call this function every minute
    // This function should only call "resident data", other variables may have been released due to lifecycle
}

static int media_player_exit_callback(void *param)
{
    // End playback
    release_player_docoder();

    run_data->file.close(); // Close file on exit
    // Release the file circular queue
    release_file_info(run_data->movie_file);

    // Release runtime data
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }

    return 0;
}

static void media_player_message_handle(const char *from, const char *to,
                                        APP_MESSAGE_TYPE type, void *message,
                                        void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "switchFlag"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.switchFlag);
        }
        else if (!strcmp(param_key, "powerFlag"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.powerFlag);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "switchFlag"))
        {
            cfg_data.switchFlag = atol(param_val);
        }
        else if (!strcmp(param_key, "powerFlag"))
        {
            cfg_data.powerFlag = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ media_app = {MEDIA_PLAYER_APP_NAME, &app_movie, "",
                     media_player_init, media_player_process, media_player_background_task,
                     media_player_exit_callback, media_player_message_handle};
