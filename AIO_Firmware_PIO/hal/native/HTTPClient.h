#ifndef HTTPCLIENT_STUB_H
#define HTTPCLIENT_STUB_H

#include "Arduino.h"
#include "IPAddress.h"

#define HTTP_CODE_OK 200
#define HTTP_CODE_MOVED_PERMANENTLY 301

class HTTPClient
{
public:
    HTTPClient();
    ~HTTPClient();
    bool begin(const char *url);
    bool begin(String url);
    int GET();
    int POST(const char *body);
    int POST(String body);
    int sendRequest(const char *method, const char *body);
    int getSize();
    String getString();
    String errorToString(int error);
    void setTimeout(uint16_t timeout);
    void addHeader(const char *name, const char *value);
    void end();
    bool connected();

private:
    void _parseUrl(const char *url);
    int _sendRequest(const char *method, const char *body);
    int _sendRequestHttps(const char *method, const char *body);

    String _host;
    String _path;
    uint16_t _port;
    bool _https;
    int _httpCode;
    int _contentLength;
    String _responseBody;
    uint16_t _timeout;
    String _headers;
    bool _connected;
    void *_client;
};

#endif