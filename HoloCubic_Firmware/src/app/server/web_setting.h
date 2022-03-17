#ifndef WEB_SETTING_H
#define WEB_SETTING_H

#include <WString.h>
#include "sys/app_controller.h"

extern AppController *app_controller; // APP控制器
void init_page_header(void);
void init_page_footer(void);
void HomePage(void);
void sys_setting(void);
void rgb_setting(void);
void weather_setting(void);
void weather_old_setting(void);
void bili_setting(void);
void picture_setting(void);
void media_setting(void);
void screen_setting(void);
void File_Download(void);
void File_Upload(void);
void File_Delete(void);
void delete_result(void);
void handleFileUpload(void);
void saveSysConf(void);
void saveRgbConf(void);
void saveWeatherConf(void);
void saveWeatherOldConf(void);
void saveBiliConf(void);
void savePictureConf(void);
void saveMediaConf(void);
void saveScreenConf(void);

void sd_file_download(const String &filename);
void SelectInput(String heading, String command, String arg_calling_name);
void ReportSDNotPresent(void);
void ReportFileNotPresent(const String &target);
void ReportCouldNotCreateFile(const String &target);

#endif