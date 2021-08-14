
// 参考代码 https://github.com/G6EJD/ESP32-8266-File-Upload

#include "network.h"
#include "common.h"
#include "web_setting.h"
#include "FS.h"
#include "HardwareSerial.h"

boolean sd_present = true;
String webpage = "";
String webpage_header = "";
String webpage_footer = "";

void Send_HTML(const String &content)
{
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.send(200, "text/html", "");
    server.sendContent(webpage_header);

    server.sendContent(content);
    server.sendContent(webpage_footer);

    server.sendContent("");
    server.client().stop(); // Stop is needed because no content length was sent
}

String file_size(int bytes)
{
    String fsize = "";
    if (bytes < 1024)
        fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024))
        fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024))
        fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else
        fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
    return fsize;
}

#define SETING_CSS ".input {display: block;margin-top: 10px;}"                                          \
                   ".input span {width: 200px;float: left;float: left;height: 36px;line-height: 36px;}" \
                   ".input input {height: 30px;width: 200px;}"                                          \
                   ".btn {width: 120px;height: 35px;background-color: #000000;border: 0px;color: #ffffff;margin-top: 15px;margin-left: auto;}" // margin-left: 100px;

#define SETTING "<form method=\"GET\" action=\"saveConf\">"                                                                            \
                "<label class=\"input\"><span>WiFi SSID</span><input type=\"text\"name=\"ssid\"value=\"%s\"></label>"                  \
                "<label class=\"input\"><span>WiFi Passwd</span><input type=\"text\"name=\"pass\"value=\"%s\"></label>"                \
                "<label class=\"input\"><span>City Name</span><input type=\"text\"name=\"cityname\"value=\"%s\"></label>"              \
                "<label class=\"input\"><span>Bilibili UID</span><input type=\"text\"name=\"UID\"value=\"%s\"></label>"              \
                "<label class=\"input\"><span>City Language(zh-Hans)</span><input type=\"text\"name=\"language\"value=\"%s\"></label>" \
                "<label class=\"input\"><span>Weather Key</span><input type=\"text\"name=\"weatherKey\"value=\"%s\"></label>"          \
                "</label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submie\"></form>"

void init_page_header()
{
    webpage_header = F("<!DOCTYPE html><html>");
    webpage_header += F("<head>");
    webpage_header += F("<title>HoloCubic WebServer</title>"); // NOTE: 1em = 16px
    webpage_header += F("<meta http-equiv='Content-Type' name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width; text/html; charset=utf-8' />");
    webpage_header += F("<style>");
    webpage_header += F(SETING_CSS);
    webpage_header += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#F7F2Fd;}");
    webpage_header += F("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#dcade6;font-size:1em;}");
    webpage_header += F("li{float:left;border-radius:0.375em;border-right:0.06em solid #bbb;}last-child {border-right:none;font-size:85%}");
    webpage_header += F("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%}");
    webpage_header += F("li a:hover{background-color:#EAE3EA;border-radius:0.375em;font-size:85%}");
    webpage_header += F("section {font-size:0.88em;}");
    webpage_header += F("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#558ED5;}");
    webpage_header += F("h2{color:orange;font-size:1.0em;}");
    webpage_header += F("h3{font-size:0.8em;}");
    webpage_header += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}");
    webpage_header += F("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}");
    webpage_header += F("tr:nth-child(odd) {background-color:#eeeeee;}");
    webpage_header += F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%; color:white;font-size:75%;}");
    webpage_header += F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%; color:white;font-size:75%;}");
    webpage_header += F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%; color:white;font-size:75%;}");
    webpage_header += F(".column{float:left;width:50%; height:45%;}");
    webpage_header += F(".row:after{content:'';display:table;clear:both;}");
    webpage_header += F("*{box-sizing:border-box;}");
    webpage_header += F("footer{background-color:#eedfff; text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
    webpage_header += F("button{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%; color:white;font-size:130%;}");
    webpage_header += F(".buttons {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%; color:white;font-size:80%;}");
    webpage_header += F(".buttonsm{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
    webpage_header += F(".buttonm {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%; color:white;font-size:70%;}");
    webpage_header += F(".buttonw {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:40%; color:white;font-size:70%;}");
    webpage_header += F("a{font-size:75%;}");
    webpage_header += F("p{font-size:75%;}");

    webpage_header += F("</style></head><body>");

    webpage_header += F("<h1>HoloCubic_AIO ");
    webpage_header += String(SERVER_VERSION) + "</h1>";
    webpage_header += F("<ul>");
    webpage_header += F("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
    webpage_header += F("<li><a href='/download'>Download</a></li>");
    webpage_header += F("<li><a href='/upload'>Upload</a></li>");
    webpage_header += F("<li><a href='/delete'>Delete</a></li>");
    webpage_header += F("<li><a href='/setting'>Setting</a></li>");
    webpage_header += F("</ul>");
}

void init_page_footer()
{ // Saves repeating many lines of code for HTML page footers
    //   webpage_footer += F("<ul>");
    //   webpage_footer += F("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
    //   webpage_footer += F("<li><a href='/download'>Download</a></li>");
    //   webpage_footer += F("<li><a href='/upload'>Upload</a></li>");
    //   webpage_footer += F("<li><a href='/setting'>Setting</a></li>");
    //   webpage_footer += F("</ul>");
    // webpage_footer += "<footer>&copy;" + String(char(byte(0x40 >> 1))) + String(char(byte(0x88 >> 1))) + String(char(byte(0x5c >> 1))) + String(char(byte(0x98 >> 1))) + String(char(byte(0x5c >> 1)));
    // webpage_footer += String(char((0x84 >> 1))) + String(char(byte(0xd2 >> 1))) + String(char(0xe4 >> 1)) + String(char(0xc8 >> 1)) + String(char(byte(0x40 >> 1)));
    // webpage_footer += String(char(byte(0x64 / 2))) + String(char(byte(0x60 >> 1))) + String(char(byte(0x62 >> 1))) + String(char(0x70 >> 1)) + "</footer>";
    webpage_footer = "<footer>&copy;ClimbSnail 2021</footer>";
    webpage_footer += F("</body></html>");
}

// All supporting functions from here...
void HomePage()
{
    // 指定 target='_blank' 设置新建页面
    webpage = F("<a href='https://github.com/ClimbSnail/HoloCubic' target='_blank'><button>Github</button></a>");
    webpage += F("<a href='https://space.bilibili.com/344470052?spm_id_from=333.788.b_765f7570696e666f.1' target='_blank'><button>BiliBili教程</button></a>");
    Send_HTML(webpage);
}

void Setting()
{
    // 实时读取配置文件
    // config_read("/wifi.txt", &g_cfg);
    char buf[1024];
    sprintf(buf, SETTING, g_cfg.ssid.c_str(), g_cfg.password.c_str(),
            g_cfg.cityname.c_str(), g_cfg.UID.c_str(), g_cfg.language.c_str(),
            g_cfg.weather_key.c_str());
    webpage = buf;
    Send_HTML(webpage);
}

void save_config(void)
{
    webpage = "<h1>Successd! You can continue to operate.</h1>";
    Send_HTML(webpage);

    //获取输入的WIFI账户和密码
    g_cfg.ssid = server.arg("ssid");
    g_cfg.password = server.arg("pass");
    g_cfg.cityname = server.arg("cityname");
    g_cfg.UID = server.arg("UID");
    g_cfg.language = server.arg("language");
    g_cfg.weather_key = server.arg("weatherKey");
    config_save("/wifi.txt", &g_cfg); // 更新配置文件
}

void File_Delete()
{
    webpage = "<h3>Enter filename to delete</h3>"
              "<form action='/delete_result' method='post'>"
              "<input type='text' name='delete_filepath' placeHolder='绝对路径 /image/...'><br>"
              "</label><input class=\"btn\" type=\"submit\" name=\"Submie\" value=\"确认删除\"></form>"
              "<a href='/'>[Back]</a>";
    Send_HTML(webpage);
}

void delete_result(void)
{
    String del_file = server.arg("delete_filepath");
    boolean ret = tf.deleteFile(del_file);
    if (ret)
    {
        webpage = "<h3>Delete succ!</h3><a href='/delete'>[Back]</a>";
        tf.listDir("/image", 250);
    }
    else
    {
        webpage = "<h3>Delete fail! Please check up file path.</h3><a href='/delete'>[Back]</a>";
    }
    tf.listDir("/image", 250);
    Send_HTML(webpage);
}

void File_Download()
{ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
    if (server.args() > 0)
    { // Arguments were received
        if (server.hasArg("download"))
            sd_file_download(server.arg(0));
    }
    else
        SelectInput("Enter filename to download", "download", "download");
}

void sd_file_download(const String &filename)
{
    if (sd_present)
    {
        File download = tf.open("/" + filename);
        if (download)
        {
            server.sendHeader("Content-Type", "text/text");
            server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
            server.sendHeader("Connection", "close");
            server.streamFile(download, "application/octet-stream");
            download.close();
        }
        else
            ReportFileNotPresent(String("download"));
    }
    else
        ReportSDNotPresent();
}

void File_Upload()
{
    tf.listDir("/image", 250);

    webpage = webpage_header;
    webpage += "<h3>Select File to Upload</h3>"
               "<FORM action='/fupload' method='post' enctype='multipart/form-data'>"
               "<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>"
               "<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>"
               "<a href='/'>[Back]</a><br><br>";
    webpage += webpage_footer;
    server.send(200, "text/html", webpage);
}

File UploadFile;
void handleFileUpload()
{                                                   // upload a new file to the Filing system
    HTTPUpload &uploadFileStream = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                                    // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
    String filename = uploadFileStream.filename;
    if (uploadFileStream.status == UPLOAD_FILE_START)
    {
        // String filename = uploadFileStream.filename;
        // if (!filename.startsWith("/image"))
        filename = "/image/" + filename;
        Serial.print("Upload File Name: ");
        Serial.println(filename);
        tf.deleteFile(filename);                    // Remove a previous version, otherwise data is appended the file again
        UploadFile = tf.open(filename, FILE_WRITE); // Open the file for writing in SPIFFS (create it, if doesn't exist)
    }
    else if (uploadFileStream.status == UPLOAD_FILE_WRITE)
    {
        if (UploadFile)
            UploadFile.write(uploadFileStream.buf, uploadFileStream.currentSize); // Write the received bytes to the file
    }
    else if (uploadFileStream.status == UPLOAD_FILE_END)
    {
        if (UploadFile) // If the file was successfully created
        {
            UploadFile.close(); // Close the file again
            Serial.print("Upload Size: ");
            Serial.println(uploadFileStream.totalSize);
            webpage = webpage_header;
            webpage += F("<h3>File was successfully uploaded</h3>");
            webpage += F("<h2>Uploaded File Name: ");
            webpage += filename + "</h2>";
            webpage += F("<h2>File Size: ");
            webpage += file_size(uploadFileStream.totalSize) + "</h2><br>";
            webpage += webpage_footer;
            server.send(200, "text/html", webpage);
            tf.listDir("/image", 250);
        }
        else
        {
            ReportCouldNotCreateFile(String("upload"));
        }
    }
}

void SelectInput(String heading, String command, String arg_calling_name)
{
    webpage = F("<h3>");
    webpage += heading + "</h3>";
    webpage += F("<FORM action='/");
    webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
    webpage += F("<input type='text' name='");
    webpage += arg_calling_name;
    webpage += F("' value=''><br>");
    webpage += F("<type='submit' name='");
    webpage += arg_calling_name;
    webpage += F("' value=''><br>");
    webpage += F("<a href='/'>[Back]</a>");
    Send_HTML(webpage);
}

void ReportSDNotPresent()
{
    webpage = F("<h3>No SD Card present</h3>");
    webpage += F("<a href='/'>[Back]</a><br><br>");
    Send_HTML(webpage);
}

void ReportFileNotPresent(const String &target)
{
    webpage = F("<h3>File does not exist</h3>");
    webpage += F("<a href='/");
    webpage += target + "'>[Back]</a><br><br>";
    Send_HTML(webpage);
}

void ReportCouldNotCreateFile(const String &target)
{
    webpage = F("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
    webpage += F("<a href='/");
    webpage += target + "'>[Back]</a><br><br>";
    Send_HTML(webpage);
}