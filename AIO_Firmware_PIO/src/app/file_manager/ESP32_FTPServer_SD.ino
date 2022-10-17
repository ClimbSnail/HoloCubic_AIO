#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32FtpServer.h"

const char* ssid = "*********************";
const char* password = "*********************";

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP32FtpServer.h to see ftp verbose on serial

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /////FTP Setup, ensure SD is started before ftp;  /////////
  
  if (SD.begin()) {
      Serial.println("SD opened!");
      ftpSrv.begin("esp32","esp32");    //username, password for ftp.  set ports in ESP32FtpServer.h  (default 21, 50009 for PASV)
  }    
}

void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!   
}
