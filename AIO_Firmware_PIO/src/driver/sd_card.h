#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define DIR_FILE_NUM 10
#define DIR_FILE_NAME_MAX_LEN 20
#define FILENAME_MAX_LEN 100

extern int photo_file_num;
extern char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

enum FILE_TYPE : unsigned char
{
    FILE_TYPE_UNKNOW = 0,
    FILE_TYPE_FILE,
    FILE_TYPE_FOLDER
};

struct File_Info
{
    char *file_name;
    FILE_TYPE file_type;
    File_Info *front_node; // 上一个节点
    File_Info *next_node;  // 下一个节点
};

void release_file_info(File_Info *info);

void join_path(char *dst_path, const char *pre_path, const char *rear_path);

// static const char *get_file_basename(const char *path);

class SdCard
{
private:
    char buf[128];

public:
    void init();

    void listDir(const char *dirname, uint8_t levels);

    File_Info *listDir(const char *dirname);

    void createDir(const char *path);

    void removeDir(const char *path);

    void readFile(const char *path);

    String readFileLine(const char *path, int num);

    void writeFile(const char *path, const char *message1);

    File open(const String &path, const char *mode = FILE_READ);

    void appendFile(const char *path, const char *message);

    void renameFile(const char *path1, const char *path2);

    boolean deleteFile(const char *path);

    boolean deleteFile(const String &path);

    void readBinFromSd(const char *path, uint8_t *buf);

    void writeBinToSd(const char *path, uint8_t *buf);

    void fileIO(const char *path);
};

#endif
