/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/
#include <ArduinoJson.h>
#include <Arduino.h>
#include "ArduinoUZlib.h" 
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;

// ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getHeapSize(), ESP.getMaxAllocHeap()
void heap(){
  Serial.print("FREE_HEAP[");
  Serial.print(ESP.getFreeHeap());
  Serial.print("]\n");
}
void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ssid", "password");
}
void log(const char *str) {
  Serial.println(str);
}


static uint8_t buffer[1280]={0};
size_t readBytesSize=0;


void fetchBuffer() {

   if ((WiFiMulti.run() == WL_CONNECTED)) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    Serial.print("[HTTPS] begin...\n");
    HTTPClient https;

    if (https.begin(*client, "https://192.168.2.144:8082/test")) {
      https.addHeader("Accept-Encoding", "gzip");
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {

          // get length of document (is -1 when Server sends no Content-Length header)
          int len = https.getSize();

          // create buffer for read
          static uint8_t buff[128] = { 0 };

          // read all data from server
          int offset=0;
          Serial.println("allocate");
          //  为什么这里分配内存会报错？
          // if(inbuf==NULL) inbuf=(uint8_t*)malloc(sizeof(uint8_t)*128);
          while (https.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = client->available();
            
            if (size) {
              // read up to 128 byte
              int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
              // int c = client->readBytes(buff, size);
              // Serial.println("memcpy");
              memcpy(buffer+offset, buff, sizeof(uint8_t)*c);
              offset+=c;
              if(c>0 && c!=16) {
                log("======rb====");
                Serial.printf("%d,", buff[c-3]);
                Serial.printf("%d,", buff[c-2]);
                Serial.printf("%d,", buff[c-1]);
                log("\n======rb end====");
              }
              // write it to Serial
              // Serial.write(buff, c);
              if (len > 0) {
                len -= c;
              }
              
            }
            delay(1);
          }
          readBytesSize=offset;
          Serial.printf("offset=%d\n", offset);
          Serial.write(buffer, offset);
          Serial.print("[HTTPS] connection closed or file end.\n");

        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("Unable to connect\n");
    }
  }
}

void loop() {
  uint8_t *outbuf1=NULL;
  // wait for WiFi connection
  fetchBuffer();
  Serial.printf("\nAfter fetch, buffer size=%d\n", readBytesSize);
  delay(1000);
  if(readBytesSize) {
    // write it to Serial
    log("===buf===");
    Serial.printf("%d,", readBytesSize-3);
    Serial.printf("%d,", readBytesSize-2);
    Serial.printf("%d,", readBytesSize-1);
    log("\n===buf end===");    
    Serial.write(buffer,readBytesSize);
    uint32_t out_size=0;
    int result=ArduinoUZlib::decompress(buffer, readBytesSize, outbuf1, out_size);
    printf("outsize=%d, result=\n", out_size,result);
    parseJSON((char*)outbuf1, out_size);
    // Serial.write(outbuf,out_size);
  }else {
    Serial.println("no avali size!");
  }
if (outbuf1!=NULL){
  free(outbuf1);
  outbuf1=NULL;
}
  Serial.println("Wait 10s before the next round...");
  delay(5000);
}


void parseJSON(char *input, int inputLength) {
  // char* input;
  // size_t inputLength; (optional)
  DynamicJsonDocument doc(6144);
  DeserializationError error = deserializeJson(doc, input, inputLength);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  const char* code = doc["code"]; // "200"
  const char* updateTime = doc["updateTime"]; // "2022-12-05T15:35+08:00"
  const char* fxLink = doc["fxLink"]; // "http://hfx.link/1u0r1"

  for (JsonObject hourly_item : doc["hourly"].as<JsonArray>()) {

    // const char* hourly_item_fxTime = hourly_item["fxTime"]; // "2022-12-05T17:00+08:00", ...
    const char* hourly_item_fxTime = hourly_item["fxTime"]; // "2022-12-05T17:00+08:00", ...
    const char* hourly_item_temp = hourly_item["temp"]; // "15", "13", "13", "12", "11", "11", "10", "10", ...
    Serial.printf("%s,", hourly_item_temp);
    const char* hourly_item_icon = hourly_item["icon"]; // "100", "150", "150", "150", "150", "150", "150", ...
    const char* hourly_item_text = hourly_item["text"]; // "晴", "晴", "晴", "晴", "晴", "晴", "晴", "多云", "多云", ...
    const char* hourly_item_wind360 = hourly_item["wind360"]; // "22", "24", "30", "33", "33", "31", "30", ...
    const char* hourly_item_windDir = hourly_item["windDir"]; // "东北风", "东北风", "东北风", "东北风", "东北风", "东北风", ...
    const char* hourly_item_windScale = hourly_item["windScale"]; // "3-4", "3-4", "3-4", "3-4", "3-4", ...
    const char* hourly_item_windSpeed = hourly_item["windSpeed"]; // "16", "16", "16", "16", "14", "14", ...
    const char* hourly_item_humidity = hourly_item["humidity"]; // "57", "63", "63", "65", "66", "67", "68", ...
    const char* hourly_item_pop = hourly_item["pop"]; // "1", "3", "6", "6", "6", "6", "6", "6", "7", "7", ...
    const char* hourly_item_precip = hourly_item["precip"]; // "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", ...
    const char* hourly_item_pressure = hourly_item["pressure"]; // "1013", "1013", "1012", "1012", "1012", ...
    const char* hourly_item_cloud = hourly_item["cloud"]; // "5", "5", "4", "4", "7", "9", "11", "33", "54", ...
    const char* hourly_item_dew = hourly_item["dew"]; // "7", "6", "6", "6", "5", "5", "5", "5", "5", "4", ...
  }
  Serial.println();
  JsonArray refer_sources = doc["refer"]["sources"];
  const char* refer_sources_0 = refer_sources[0]; // "QWeather"
  const char* refer_sources_1 = refer_sources[1]; // "NMC"
  const char* refer_sources_2 = refer_sources[2]; // "ECMWF"

  const char* refer_license_0 = doc["refer"]["license"][0]; // "CC BY-SA 4.0"

}

