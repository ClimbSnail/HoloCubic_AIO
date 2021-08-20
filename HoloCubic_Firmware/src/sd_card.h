#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define DIR_FILE_NUM 10
#define DIR_FILE_NAME_MAX_LEN 20

extern int photo_file_num;
extern char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

struct File_Info
{
    char *file_name;
    File_Info *next_node;
};

void release_file_info(File_Info* info);

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
