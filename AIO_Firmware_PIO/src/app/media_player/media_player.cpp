#include "media_player.h"
#include "media_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "driver/sd_card.h"
#include "decoder.h"
#include "DMADrawer.h"

#define MEDIA_PLAYER_APP_NAME "Media"

#define MOVIE_PATH "/movie"
#define NO_TRIGGER_ENTER_FREQ_160M 90000UL // 无操作规定时间后进入设置160M主频（90s）
#define NO_TRIGGER_ENTER_FREQ_80M 120000UL // 无操作规定时间后进入设置160M主频（120s）

// 调试开关
#define MEDIA_PLAYER_DEBUG 1

// 天气的持久化配置
#define MEDIA_CONFIG_PATH "/media.cfg"
#define MAX_CONFIG_LINE_LENGTH 32
#define MAX_FILENAME_LENGTH 256

struct MP_Config
{
    uint8_t switchFlag; // 是否自动播放下一个（0不切换 1自动切换）
    uint8_t powerFlag;  // 功耗控制（0低发热 1性能优先）
};

// 播放器状态机
typedef enum {
    PLAYER_STATE_IDLE,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_SWITCHING,
    PLAYER_STATE_ERROR
} player_state_t;

struct MediaAppRunData
{
    PlayDecoderBase *player_decoder;
    unsigned long preTriggerKeyMillis; // 最近一回按键触发的时间戳
    unsigned long lastPowerCheckMillis; // 上次功耗检查时间
    unsigned long lastFrameTime;       // 上次帧播放时间
    int movie_pos_increate;
    File_Info *movie_file; // movie文件夹下的文件指针头
    File_Info *pfile;      // 指向当前播放的文件节点
    File file;
    player_state_t state;
    uint16_t frameDelay;   // 帧间延迟，根据视频调整
    uint8_t retryCount;    // 重试计数器
};

static MP_Config cfg_data;
static MediaAppRunData *run_data = NULL;

// ==================== 辅助函数 ====================

// 安全的配置设置函数
static void set_default_config(MP_Config *cfg)
{
    cfg->switchFlag = 1; // 是否自动播放下一个（0不切换 1自动切换）
    cfg->powerFlag = 0;  // 功耗控制（0低发热 1性能优先）
}

// 安全的配置解析
static bool parse_config_safely(const char* info, MP_Config *cfg)
{
    if (!info || strlen(info) == 0) return false;
    
    // 创建可修改的副本
    char info_copy[256];
    strncpy(info_copy, info, sizeof(info_copy) - 1);
    info_copy[sizeof(info_copy) - 1] = '\0';
    
    char *param[2] = {0};
    
    // analyseParam返回bool，表示是否成功解析
    bool success = analyseParam(info_copy, 2, param);
    if (!success) {
        return false;
    }
    
    // 检查参数是否有效
    if (param[0] && param[1]) {
        cfg->switchFlag = atoi(param[0]);
        cfg->powerFlag = atoi(param[1]);
        
        // 参数验证
        if (cfg->switchFlag > 1) cfg->switchFlag = 1;
        if (cfg->powerFlag > 1) cfg->powerFlag = 1;
        
        return true;
    }
    
    return false;
}

// 文件扩展名检查优化
static bool has_extension(const char* filename, const char* ext)
{
    if (!filename || !ext) return false;
    
    size_t name_len = strlen(filename);
    size_t ext_len = strlen(ext);
    
    if (ext_len >= name_len) return false;
    
    return strcasecmp(filename + name_len - ext_len, ext) == 0;
}


// ==================== 配置管理 ====================

static void write_config(MP_Config *cfg)
{
    if (!cfg) return;
    
    char tmp[MAX_CONFIG_LINE_LENGTH];
    String w_data;
    
    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%u\n", cfg->switchFlag);
    w_data += tmp;
    
    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%u\n", cfg->powerFlag);
    w_data += tmp;
    
    // 直接写入，不检查返回值
    g_flashCfg.writeFile(MEDIA_CONFIG_PATH, w_data.c_str());
}

static void read_config(MP_Config *cfg)
{
    if (!cfg) return;
    
    char info[256] = {0};
    uint16_t size = g_flashCfg.readFile(MEDIA_CONFIG_PATH, (uint8_t *)info);
    
    if (size == 0 || size >= sizeof(info) - 1) {
        set_default_config(cfg);
        write_config(cfg);
        return;
    }
    
    info[size] = 0;
    
    if (!parse_config_safely(info, cfg)) {
        set_default_config(cfg);
        write_config(cfg);
    }
}

// ==================== 文件管理 ====================

static File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    if (!p_cur_file) return NULL;

    File_Info *pfile = (direction == 1) ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file) {
        if (pfile && pfile->file_type == FILE_TYPE_FILE) {
            return pfile;
        }
        pfile = (direction == 1) ? pfile->next_node : pfile->front_node;
    }
    return NULL;
}

// 辅助函数：计算文件数量（跳过头节点）
static int count_files(File_Info *head)
{
    if (!head || !head->next_node) return 0;
    
    int count = 0;
    File_Info *current = head->next_node;
    File_Info *first_file = current;
    
    do {
        if (current->file_type == FILE_TYPE_FILE) {
            count++;
        }
        current = current->next_node;
    } while (current != first_file);
    
    return count;
}

// 检查文件是否存在（通过尝试打开）
static bool file_exists(const char* path)
{
    File file = tf.open(path);
    if (file) {
        file.close();
        return true;
    }
    return false;
}
// 读取索引文件构建文件列表（保持与listDir相同的结构）
static File_Info *load_from_index(const char* index_path)
{
    File index_file = tf.open(index_path);
    if (!index_file) {
        Serial.println("Index file not found, will scan directory");
        return NULL;
    }
    
    // 创建头节点（表示文件夹）
    File_Info *head_file = (File_Info *)malloc(sizeof(File_Info));
    if (!head_file) {
        Serial.println("Memory allocation failed for head file");
        index_file.close();
        return NULL;
    }
    
    head_file->file_type = FILE_TYPE_FOLDER;
    head_file->file_name = strdup(MOVIE_PATH);
    head_file->front_node = NULL;
    head_file->next_node = NULL;
    
    File_Info *tail_file = head_file;
    char line[MAX_FILENAME_LENGTH];
    int file_count = 0;
    
    while (index_file.available()) {
        int bytesRead = index_file.readBytesUntil('\n', line, sizeof(line)-1);
        if (bytesRead > 0) {
            line[bytesRead] = '\0';
            // 去除换行符
            if (bytesRead > 0 && line[bytesRead-1] == '\r') {
                line[bytesRead-1] = '\0';
                bytesRead--;
            }
            
            // 跳过空行
            if (bytesRead == 0) continue;
            
            // 验证文件实际存在
            // char full_path[MAX_FILENAME_LENGTH];
            // snprintf(full_path, sizeof(full_path), "%s/%s", MOVIE_PATH, line);

            // 不要验证了，播放的时候再验证，不然每一个都验证相当于全盘扫面
            // TODO 播放的时候验证文件是否存在，不存在则从索引里面删除
            // if (!file_exists(full_path)) {
            //     Serial.printf("File in index not found: %s\n", full_path);
            //     continue;
            // }
            
            // 创建文件节点
            File_Info *new_file = (File_Info *)malloc(sizeof(File_Info));
            if (!new_file) {
                Serial.println("Memory allocation failed for file info");
                break;
            }
            
            new_file->file_name = strdup(line);
            new_file->file_type = FILE_TYPE_FILE;
            
            // 添加到链表
            tail_file->next_node = new_file;
            new_file->front_node = tail_file;
            new_file->next_node = NULL;
            tail_file = new_file;
            
            file_count++;
        }
    }
    
    index_file.close();
    
    // 将链表设置为循环（与listDir保持一致）
    if (head_file->next_node) {
        // 将最后一个节点的next指向第一个文件节点
        tail_file->next_node = head_file->next_node;
        // 将第一个文件节点的front指向最后一个节点
        head_file->next_node->front_node = tail_file;
    } else {
        // 如果没有文件，头节点自循环
        head_file->next_node = head_file;
        head_file->front_node = head_file;
    }
    
    Serial.printf("Loaded %d files from index\n", file_count);
    return head_file;
}
// 扫描目录并创建索引文件
static File_Info *scan_and_create_index(const char* path, const char* index_path)
{
    Serial.println("Scanning directory to create index...");
    File_Info *file_list = tf.listDir(path);
    if (!file_list) {
        Serial.println("No files found in directory");
        return NULL;
    }
    
    // 创建索引文件
    File index_file = tf.open(index_path, FILE_WRITE);
    if (!index_file) {
        Serial.println("Failed to create index file");
        return file_list; // 返回扫描结果，但不保存索引
    }
    
    // 遍历文件列表，写入索引文件（跳过头节点）
    int file_count = 0;
    if (file_list && file_list->next_node) {
        File_Info *current = file_list->next_node;
        File_Info *first_file = current;
        
        do {
            if (current->file_type == FILE_TYPE_FILE) {
                index_file.println(current->file_name);
                file_count++;
            }
            current = current->next_node;
        } while (current != first_file);
    }
    
    index_file.close();
    Serial.printf("Created index with %d files\n", file_count);
    return file_list;
}



// ==================== 播放器核心 ====================

static void release_player_decoder(void)
{
    if (run_data && run_data->player_decoder) {
        delete run_data->player_decoder;
        run_data->player_decoder = NULL;
    }
}

static bool open_video_file(void)
{
    if (!run_data || !run_data->pfile) return false;
    
    char file_name[MAX_FILENAME_LENGTH] = {0};
    snprintf(file_name, sizeof(file_name), "%s/%s", 
             run_data->movie_file->file_name, run_data->pfile->file_name);

    // 尝试多次打开文件
    for (int attempt = 0; attempt < 3; attempt++) {
        run_data->file = tf.open(file_name);
        if (run_data->file) {
            run_data->retryCount = 0;
            return true;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 重试前延迟
    }
    
    Serial.printf("Failed to open file after 3 attempts: %s\n", file_name);
    if (run_data) {
        run_data->state = PLAYER_STATE_ERROR;
    }
    return false;
}

static bool create_video_decoder(void)
{
    if (!run_data || !run_data->file) return false;
        
    release_player_decoder(); // 先释放之前的解码器
    
    if (has_extension(run_data->pfile->file_name, ".mjpeg") || 
        has_extension(run_data->pfile->file_name, ".MJPEG")) {
        run_data->player_decoder = new MjpegPlayDecoder(&run_data->file, true);
        if (run_data) run_data->frameDelay = 40; // MJPEG 通常25fps
        Serial.print("MJPEG video start --------> ");
    }
    else if (has_extension(run_data->pfile->file_name, ".rgb") || 
             has_extension(run_data->pfile->file_name, ".RGB")) {
        run_data->player_decoder = new RgbPlayDecoder(&run_data->file, true);
        if (run_data) run_data->frameDelay = 33; // RGB 通常30fps
        Serial.print("RGB565 video start --------> ");
    }
    else {
        Serial.printf("Unsupported format: %s\n", run_data->pfile->file_name);
        return false;
    }
    
    Serial.println(run_data->pfile->file_name);
    return (run_data->player_decoder != NULL);
}

static bool video_start(bool create_new)
{
    if (!run_data || !run_data->movie_file) return false;

    if (create_new && run_data->pfile) {
        File_Info *next_file = get_next_file(run_data->pfile, run_data->movie_pos_increate);
        if (!next_file) {
            // 如果没有下一个文件，根据配置决定是否循环
            if (cfg_data.switchFlag) {
                next_file = get_next_file(run_data->movie_file->next_node, run_data->movie_pos_increate);
            }
        }
        run_data->pfile = next_file;
    }

    if (!run_data->pfile) return false;

    run_data->state = PLAYER_STATE_SWITCHING;
    
    if (!open_video_file()) return false;
    if (!create_video_decoder()) return false;
    
    run_data->state = PLAYER_STATE_PLAYING;
    run_data->lastFrameTime = GET_SYS_MILLIS();
    return true;
}

// ==================== 功耗管理 ====================

static void manage_power_consumption(void)
{
    if (!run_data) return;
    
    uint32_t current_time = GET_SYS_MILLIS();
    
    // 降低检查频率，每5秒检查一次
    if (current_time - run_data->lastPowerCheckMillis < 5000) return;
    run_data->lastPowerCheckMillis = current_time;
    
    if (cfg_data.powerFlag == 0) { // 低功耗模式
        uint32_t idle_time = current_time - run_data->preTriggerKeyMillis;
        uint32_t current_freq = getCpuFrequencyMhz();
        
        if (idle_time >= NO_TRIGGER_ENTER_FREQ_80M && current_freq > 80) {
            setCpuFrequencyMhz(80);
        }
        else if (idle_time >= NO_TRIGGER_ENTER_FREQ_160M && current_freq > 160) {
            setCpuFrequencyMhz(160);
        }
    }
}

// ==================== 用户输入处理 ====================

static bool handle_user_input(AppController *sys, const ImuAction *act_info)
{
    if (!act_info || !run_data) return true;
    
    switch (act_info->active) {
    case RETURN:
        sys->app_exit();
        return false;
        
    case TURN_RIGHT:
    case TURN_LEFT:
        run_data->movie_pos_increate = (act_info->active == TURN_RIGHT) ? 1 : -1;
        run_data->state = PLAYER_STATE_SWITCHING;
        release_player_decoder();
        if (run_data->file) {
            run_data->file.close();
        }
        video_start(true);
        vTaskDelay(400 / portTICK_PERIOD_MS);
        break;
        
    case UNKNOWN:
        break;
        
    default:
        run_data->preTriggerKeyMillis = GET_SYS_MILLIS();
        setCpuFrequencyMhz(240);
        break;
    }
    
    return true;
}

// ==================== 播放管理 ====================

static bool manage_playback(AppController *sys)
{
    if (!run_data || !run_data->pfile) {
        Serial.println("No file to play");
        sys->app_exit();
        return false;
    }
    
    if (!run_data->file) {
        Serial.println("File not available");
        if (run_data->retryCount++ < 3) {
            video_start(false); // 尝试重新打开
        } else {
            sys->app_exit();
            return false;
        }
    }
    
    return true;
}

static void prepare_next_frame(void)
{
    if (!run_data) return;
    
    if (!run_data->file || !run_data->file.available()) {
        // 文件播放结束
        release_player_decoder();
        if (run_data->file) {
            run_data->file.close();
        }
        
        if (cfg_data.switchFlag) {
            video_start(true); // 播放下一个
        } else {
            video_start(false); // 重复播放当前
        }
        return;
    }
    
    // 控制帧率
    uint32_t current_time = GET_SYS_MILLIS();
    if (current_time - run_data->lastFrameTime < run_data->frameDelay) {
        return; // 还没到下一帧时间
    }
    
    // 播放一帧数据
    if (run_data->player_decoder) {
        run_data->player_decoder->video_play_screen();
        run_data->lastFrameTime = current_time;
    }
}

// ==================== 主函数 ====================

static int media_player_init(AppController *sys)
{
    // 调整RGB模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            150, 200, 1, 50};
    set_rgb_and_run(&rgb_setting);

    // 读取配置
    read_config(&cfg_data);
    
    // 分配运行数据内存
    run_data = (MediaAppRunData *)calloc(1, sizeof(MediaAppRunData));
    if (!run_data) {
        Serial.println("Failed to allocate memory for media player");
        return -1;
    }
    
    // 初始化运行数据
    run_data->state = PLAYER_STATE_IDLE;
    run_data->movie_pos_increate = 1;
    run_data->preTriggerKeyMillis = GET_SYS_MILLIS();
    run_data->lastPowerCheckMillis = GET_SYS_MILLIS();
    run_data->lastFrameTime = GET_SYS_MILLIS();
    run_data->frameDelay = 40; // 默认25fps
    run_data->retryCount = 0;
    
    // 扫描视频文件
    // run_data->movie_file = tf.listDir(MOVIE_PATH);
    // if (run_data->movie_file) {
    //     run_data->pfile = get_next_file(run_data->movie_file->next_node, 1);
    // }

       // 扫描视频文件 - 使用索引文件加速
    const char* index_path = "/movie/movie.txt";
    
    // 先尝试从索引文件加载
    run_data->movie_file = load_from_index(index_path);
    
    // 如果索引加载失败，扫描目录并创建索引
    if (!run_data->movie_file) {
        run_data->movie_file = scan_and_create_index(MOVIE_PATH, index_path);
    }
    
    if (run_data->movie_file) {
        // 获取第一个文件节点（跳过头节点）
        if (run_data->movie_file->next_node && 
            run_data->movie_file->next_node != run_data->movie_file) {
            run_data->pfile = run_data->movie_file->next_node;
        } else {
            run_data->pfile = NULL;
        }
        
        // 如果没有找到有效文件
        if (!run_data->pfile) {
            Serial.println("No valid video files found");
            // 清理并退出
            if (run_data->movie_file) {
                release_file_info(run_data->movie_file);
                run_data->movie_file = NULL;
            }
            return -1;
        }
        
        Serial.printf("Found %d video files\n", count_files(run_data->movie_file));
    } else {
        Serial.println("No video files found");
        return -1;
    }
    // 设置初始CPU频率
    setCpuFrequencyMhz(240);
    
    // 开始播放
    if (!video_start(false)) {
        Serial.println("Failed to start video playback");
        return -1;
    }
    
    return 0;
}

static void media_player_process(AppController *sys, const ImuAction *act_info)
{
    
    if (!handle_user_input(sys, act_info)) return;
    if (!manage_playback(sys)) return;
    
    manage_power_consumption();
    prepare_next_frame();
}

// static unsigned long last_index_check = 0;
// unsigned long current_time = GET_SYS_MILLIS();
static void media_player_background_task(AppController *sys, const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 可以在这里执行一些低优先级的维护任务

    // // 每5分钟检查一次索引是否需要更新
    // if (current_time - last_index_check > 300000) {
    //     last_index_check = current_time;
        
    //     // 检查movie目录下的文件数量是否与索引匹配
    //     if (run_data && run_data->movie_file) {
    //         int indexed_count = count_files(run_data->movie_file);
            
    //         // 简单检查：如果目录中的文件数量变化较大，重新创建索引
    //         // 这里可以更复杂的检查，比如比较文件修改时间等
    //         File temp_dir = tf.open(MOVIE_PATH);
    //         if (temp_dir) {
    //             int actual_count = 0;
    //             File entry = temp_dir.openNextFile();
    //             while (entry) {
    //                 if (!entry.isDirectory()) {
    //                     actual_count++;
    //                 }
    //                 entry = temp_dir.openNextFile();
    //             }
    //             temp_dir.close();
                
    //             if (abs(actual_count - indexed_count) > 2) { // 允许2个文件的差异
    //                 Serial.println("Directory changed, updating index...");
    //                 if (run_data->movie_file) {
    //                     release_file_info(run_data->movie_file);
    //                 }
    //                 run_data->movie_file = scan_and_create_index(MOVIE_PATH, "/movie/movie.txt");
    //                 if (run_data->movie_file) {
    //                     run_data->pfile = get_next_file(run_data->movie_file->next_node, 1);
    //                 }
    //             }
    //         }
    //     }
    // }

}

static int media_player_exit_callback(void *param)
{
    Serial.println("Media player exiting...");
    
    // 结束播放
    release_player_decoder();
    
    if (run_data) {
        if (run_data->file) {
            run_data->file.close();
        }
        
        // 释放文件循环队列
        if (run_data->movie_file) {
            release_file_info(run_data->movie_file);
        }
        
        free(run_data);
        run_data = NULL;
    }
    
    return 0;
}

static void media_player_message_handle(const char *from, const char *to,
                                        APP_MESSAGE_TYPE type, void *message,
                                        void *ext_info)
{
    if (!message) return;
    
    switch (type) {
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "switchFlag")) {
            snprintf((char *)ext_info, 32, "%u", cfg_data.switchFlag);
        }
        else if (!strcmp(param_key, "powerFlag")) {
            snprintf((char *)ext_info, 32, "%u", cfg_data.powerFlag);
        }
        else {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
        break;
    }
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "switchFlag")) {
            cfg_data.switchFlag = atoi(param_val);
        }
        else if (!strcmp(param_key, "powerFlag")) {
            cfg_data.powerFlag = atoi(param_val);
        }
        break;
    }
    case APP_MESSAGE_READ_CFG:
        read_config(&cfg_data);
        break;
    case APP_MESSAGE_WRITE_CFG:
        write_config(&cfg_data);
        break;
    default:
        break;
    }
}

APP_OBJ media_app = {MEDIA_PLAYER_APP_NAME, &app_movie, "",
                     media_player_init, media_player_process, media_player_background_task,
                     media_player_exit_callback, media_player_message_handle};