#ifndef WIFI_STUB_H
#define WIFI_STUB_H

#include <stdint.h>
#include "IPAddress.h"
#include "Client.h"

#define WL_CONNECTED 3
#define WL_DISCONNECTED 6
#define WL_IDLE_STATUS 0
#define WL_NO_SSID_AVAIL 1
#define WL_SCAN_COMPLETED 2
#define WL_CONNECT_FAILED 4
#define WL_CONNECTION_LOST 5

#define WIFI_MODE_STA  1
#define WIFI_MODE_AP   2
#define WIFI_MODE_APSTA 3

class WiFiClass
{
public:
    int status() { return WL_CONNECTED; }
    void begin(const char *ssid, const char *password) { (void)ssid; (void)password; }
    void disconnect() {}
    IPAddress localIP();
    IPAddress softAPIP() { return IPAddress(192, 168, 4, 1); }
    int getMode() { return WIFI_MODE_STA; }
};

extern WiFiClass WiFi;

class WiFiClient : public Client
{
public:
    WiFiClient();
    WiFiClient(int fd);
    ~WiFiClient();
    WiFiClient(const WiFiClient &other);
    WiFiClient(WiFiClient &&other);
    WiFiClient &operator=(const WiFiClient &other);
    WiFiClient &operator=(WiFiClient &&other);

    int connect(IPAddress ip, uint16_t port) override;
    int connect(const char *host, uint16_t port) override;
    int connect(const char *host, uint16_t port, int timeout) override;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buf, size_t size) override;
    int available() override;
    int read() override;
    int read(uint8_t *buf, size_t size) override;
    int peek() override;
    void flush() override;
    void stop() override;
    uint8_t connected() override;
    operator bool();
    void println(const char *s = "");
    void println(int n);
    void println(const String &s);
    void print(const char *s);
    void print(int n);
    void print(const String &s);
    size_t write(const char *str);

private:
    int _sock;
    void _init();
    void _close();
};

class WiFiServer
{
public:
    WiFiServer(uint16_t port = 80);
    ~WiFiServer();
    void begin();
    void begin(uint16_t port);
    void end();
    void stop();
    void close();
    void setNoDelay(bool nodelay);
    WiFiClient available();
    bool hasClient();
    uint8_t status();

private:
    int _sock;
    uint16_t _port;
    bool _listening;
};

#endif