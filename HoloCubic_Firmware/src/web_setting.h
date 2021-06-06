#ifndef WEB_SETTING_H
#define WEB_SETTING_H

#define SERVER_VERSION  "1.0"

#include <WString.h>

void init_page_header(void);
void init_page_footer(void);
void HomePage(void);
void Setting(void);
void File_Download(void);
void File_Upload(void);
void File_Delete(void);
void delete_result(void);
void handleFileUpload(void);
void save_config(void);



void sd_file_download(const String &filename);
void SelectInput(String heading, String command, String arg_calling_name);
void ReportSDNotPresent(void);
void ReportFileNotPresent(const String &target);
void ReportCouldNotCreateFile(const String &target);

#endif