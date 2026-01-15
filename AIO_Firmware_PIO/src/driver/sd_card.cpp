#include "sd_card.h"
#include "SD_MMC.h"
#include <string.h>
#include "common.h"

#define TF_VFS_IS_NULL(RET)                           \
    if (NULL == tf_vfs)                               \
    {                                                 \
        Serial.println("[Sys SD Card] Mount Failed"); \
        return RET;                                   \
    }

int photo_file_num = 0;
char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

static fs::FS *tf_vfs = NULL;

// ==================== 文件链表释放函数 ====================
 void release_file_info(File_Info *head)
{

    if (!head) return;
    
    // 检查是否是空链表（只有头节点）
    if (head->next_node == head) {
        if (head->file_name) { 
            free(head->file_name);
        }
        free(head);
        return;
    }
    
    // 断开循环链表，变成单向链表
    // 获取第一个文件节点
    File_Info *first_file = head->next_node;
    // 获取最后一个文件节点（通过第一个节点的front_node）
    File_Info *last_file = first_file->front_node;
    
    // 断开循环：将最后一个节点的next置为NULL
    if (last_file) {
        last_file->next_node = NULL;
    }
    
    // 现在可以安全遍历单向链表
    File_Info *current = first_file;
    while (current) {
        File_Info *next = current->next_node;
        
        if (current->file_name) {
            free(current->file_name);
            current->file_name = NULL;
        }
        
        free(current);
        current = next;
    }
    
    // 最后释放头节点
    if (head->file_name) {
        free(head->file_name);
        head->file_name = NULL;
    }
    free(head);
}


void join_path(char *dst_path, const char *pre_path, const char *rear_path)
{
    while (*pre_path != 0)
    {
        *dst_path = *pre_path;
        ++dst_path;
        ++pre_path;
    }
    if (*(pre_path - 1) != '/')
    {
        *dst_path = '/';
        ++dst_path;
    }

    if (*rear_path == '/')
    {
        ++rear_path;
    }
    while (*rear_path != 0)
    {
        *dst_path = *rear_path;
        ++dst_path;
        ++rear_path;
    }
    *dst_path = 0;
}

/*
 * get file basename
 */
static const char *get_file_basename(const char *path)
{
    // 获取最后一个'/'所在的下标
    const char *ret = path;
    for (const char *cur = path; *cur != 0; ++cur)
    {
        if (*cur == '/')
        {
            ret = cur + 1;
        }
    }
    return ret;
}

void SdCard::init()
{
    SPIClass *sd_spi = new SPIClass(HSPI);          // another SPI
    sd_spi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS); // Replace default HSPI pins
    if (!SD.begin(SD_SS, *sd_spi, 80000000))        // SD-Card SS pin is 15
    {
        Serial.println("Card Mount Failed");
        return;
    }
    tf_vfs = &SD;
    uint8_t cardType = SD.cardType();

    // 目前SD_MMC驱动与硬件引脚存在冲突
    // if(!SD_MMC.begin("/", true)){
    //     Serial.println("Card Mount Failed");
    //     return;
    // }
    // tf_vfs = &SD_MMC;
    // uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void SdCard::listDir(const char *dirname, uint8_t levels)
{
    TF_VFS_IS_NULL()

    Serial.printf("Listing directory: %s\n", dirname);
    photo_file_num = 0;

    File root = tf_vfs->open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    int dir_len = strlen(dirname) + 1;

    File file = root.openNextFile();
    while (file && photo_file_num < DIR_FILE_NUM)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            // 只取文件名 保存到file_name_list中
            strncpy(file_name_list[photo_file_num], file.name() + dir_len, DIR_FILE_NAME_MAX_LEN - 1);
            file_name_list[photo_file_num][strlen(file_name_list[photo_file_num]) - 4] = 0;

            char file_name[FILENAME_MAX_LEN] = {0};
            sprintf(file_name, "%s/%s.bin", dirname, file_name_list[photo_file_num]);
            Serial.print(file_name);
            ++photo_file_num;
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    Serial.println(photo_file_num);
}

File_Info *SdCard::listDir(const char *dirname)
{
    TF_VFS_IS_NULL(NULL)

    Serial.printf("Listing directory: %s\n", dirname);

    File root = tf_vfs->open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return NULL;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return NULL;
    }

    // 头节点的创建（头节点用来记录此文件夹）
    File_Info *head_file = (File_Info *)malloc(sizeof(File_Info));
    if (!head_file) return NULL;
    
    head_file->file_type = FILE_TYPE_FOLDER;
    head_file->file_name = strdup(dirname); // 使用strdup简化
    if (!head_file->file_name) {
        free(head_file);
        return NULL;
    }
    head_file->front_node = NULL;
    head_file->next_node = NULL;

    File_Info *file_node = head_file;

    File file = root.openNextFile();
    while (file)
    {
        const char *fn = get_file_basename(file.name());
        int filename_len = strlen(fn);
        if (filename_len > FILENAME_MAX_LEN - 10)
        {
            Serial.println("Filename is too long.");
            file = root.openNextFile();
            continue;
        }

        // 创建新节点
        File_Info *new_node = (File_Info *)malloc(sizeof(File_Info));
        if (!new_node) break;
        
        new_node->front_node = file_node;
        new_node->next_node = NULL;
        
        // 使用strdup分配并复制文件名
        new_node->file_name = strdup(fn);
        if (!new_node->file_name) {
            free(new_node);
            break;
        }

        // 设置文件类型
        if (file.isDirectory())
        {
            new_node->file_type = FILE_TYPE_FOLDER;
        }
        else
        {
            new_node->file_type = FILE_TYPE_FILE;
        }

        // 连接到链表
        file_node->next_node = new_node;
        file_node = new_node;

        // 打印信息
        char tmp_file_name[FILENAME_MAX_LEN] = {0};
        join_path(tmp_file_name, dirname, file_node->file_name);
        
        if (file_node->file_type == FILE_TYPE_FOLDER)
        {
            Serial.print("  DIR : ");
            Serial.println(tmp_file_name);
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(tmp_file_name);
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }

        file = root.openNextFile();
    }

    // 处理循环链表连接
    if (head_file->next_node)
    {
        file_node->next_node = head_file->next_node;
        head_file->next_node->front_node = file_node;
    }
    
    return head_file;
}

void SdCard::createDir(const char *path)
{
    TF_VFS_IS_NULL()

    Serial.printf("Creating Dir: %s\n", path);
    if (tf_vfs->mkdir(path))
    {
        Serial.println("Dir created");
    }
    else
    {
        Serial.println("mkdir failed");
    }
}

void SdCard::removeDir(const char *path)
{
    TF_VFS_IS_NULL()

    Serial.printf("Removing Dir: %s\n", path);
    if (tf_vfs->rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

void SdCard::readFile(const char *path)
{
    TF_VFS_IS_NULL()

    Serial.printf("Reading file: %s\n", path);

    File file = tf_vfs->open(path);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

String SdCard::readFileLine(const char *path, int num)
{
    TF_VFS_IS_NULL("")

    Serial.printf("Reading file: %s line: %d\n", path, num);

    File file = tf_vfs->open(path);
    if (!file)
    {
        return ("Failed to open file for reading");
    }

    char *p = buf;
    while (file.available())
    {
        char c = file.read();
        if (c == '\n')
        {
            num--;
            if (num == 0)
            {
                *(p++) = '\0';
                String s(buf);
                s.trim();
                return s;
            }
        }
        else if (num == 1)
        {
            *(p++) = c;
        }
    }
    file.close();

    return String("error parameter!");
}

void SdCard::writeFile(const char *path, const char *info)
{
    TF_VFS_IS_NULL()

    Serial.printf("Writing file: %s\n", path);

    File file = tf_vfs->open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.println(info))
    {
        Serial.println("Write succ");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

File SdCard::open(const String &path, const char *mode)
{
    // TF_VFS_IS_NULL(RET)

    return tf_vfs->open(path, mode);
}

void SdCard::appendFile(const char *path, const char *message)
{
    TF_VFS_IS_NULL()

    Serial.printf("Appending to file: %s\n", path);

    File file = tf_vfs->open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
    file.close();
}

void SdCard::renameFile(const char *path1, const char *path2)
{
    TF_VFS_IS_NULL()

    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (tf_vfs->rename(path1, path2))
    {
        Serial.println("File renamed");
    }
    else
    {
        Serial.println("Rename failed");
    }
}

boolean SdCard::deleteFile(const char *path)
{
    TF_VFS_IS_NULL(false)

    Serial.printf("Deleting file: %s\n", path);
    if (tf_vfs->remove(path))
    {
        Serial.println("File deleted");
        return true;
    }
    else
    {
        Serial.println("Delete failed");
    }
    return false;
}

boolean SdCard::deleteFile(const String &path)
{
    TF_VFS_IS_NULL(false)

    Serial.printf("Deleting file: %s\n", path);
    if (tf_vfs->remove(path))
    {
        Serial.println("File deleted");
        return true;
    }
    else
    {
        Serial.println("Delete failed");
    }
    return false;
}

void SdCard::readBinFromSd(const char *path, uint8_t *buf)
{
    TF_VFS_IS_NULL()

    File file = tf_vfs->open(path);
    size_t len = 0;
    if (file)
    {
        len = file.size();

        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }

        file.close();
    }
    else
    {
        Serial.println("Failed to open file for reading");
    }
}

void SdCard::writeBinToSd(const char *path, uint8_t *buf)
{
    TF_VFS_IS_NULL()

    File file = tf_vfs->open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    for (i = 0; i < 2048; i++)
    {
        file.write(buf, 512);
    }
    file.close();
}

void SdCard::fileIO(const char *path)
{
    TF_VFS_IS_NULL()

    File file = tf_vfs->open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file)
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    }
    else
    {
        Serial.println("Failed to open file for reading");
    }

    file = tf_vfs->open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++)
    {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}
