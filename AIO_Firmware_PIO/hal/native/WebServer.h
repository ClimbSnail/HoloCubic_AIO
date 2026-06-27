#ifndef WEB_SERVER_STUB_H
#define WEB_SERVER_STUB_H

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cstdio>
#include "NativeServer.h"
#include "FS.h"
#include "WiFi.h"

#define HTTP_GET  0
#define HTTP_POST 1
#define CONTENT_LENGTH_UNKNOWN ((size_t)-1)

#define UPLOAD_FILE_START  0
#define UPLOAD_FILE_WRITE  1
#define UPLOAD_FILE_END    2

class HTTPUpload
{
public:
    int status;
    String filename;
    const uint8_t *buf;
    size_t currentSize;
    size_t totalSize;
};

class WebServer
{
    struct RouteHandler
    {
        std::string uri;
        int method;
        void (*handler)();
        void (*uploadHandler)();
    };

    NativeServer _server;
    std::vector<RouteHandler> _routes;
    std::string _pendingHeaders;
    bool _headersSent;
    int _responseCode;
    std::string _contentType;
    bool _chunked;

    std::string _currentUri;
    int _currentMethod;
    std::map<std::string, std::string> _queryArgs;
    bool _hasArgs;
    HTTPUpload _upload;

    char _recvBuf[8192];
    int _recvLen;

    void _parseRequest();
    void _parseQueryArgs(const std::string &query);
    void _flushHeaders();
    void _sendRaw(const char *data, int len);
    bool _matchRoute(const std::string &uri, int method, RouteHandler *&handler);

public:
    WebServer() : _server(80) {}
    WebServer(int port) : _server(port) {}

    void on(const char *uri, void (*handler)())
    {
        RouteHandler r;
        r.uri = uri;
        r.method = HTTP_GET;
        r.handler = handler;
        r.uploadHandler = nullptr;
        _routes.push_back(r);
    }

    void on(const char *uri, int method, void (*handler)())
    {
        RouteHandler r;
        r.uri = uri;
        r.method = method;
        r.handler = handler;
        r.uploadHandler = nullptr;
        _routes.push_back(r);
    }

    void on(const char *uri, int method, void (*handler)(), void (*uploadHandler)())
    {
        RouteHandler r;
        r.uri = uri;
        r.method = method;
        r.handler = handler;
        r.uploadHandler = uploadHandler;
        _routes.push_back(r);
    }

    void begin();
    void handleClient();

    void send(int code, const char *type, const char *content);
    void send(int code, const char *type, const String &content);
    void send(int code);

    void sendHeader(const char *name, const char *value, bool first = false);
    void sendHeader(const char *name, const String &value, bool first = false);

    void setContentLength(size_t len);
    void sendContent(const char *content);
    void sendContent(const String &content);

    void stop();
    void close();

    WiFiClient &client();
    String arg(const char *name) const;
    String arg(int i) const;
    int args() const { return (int)_queryArgs.size(); }
    bool hasArg(const char *name) const;
    void streamFile(fs::File &file, const char *contentType);
    HTTPUpload &upload() { return _upload; }

    void _parsePostFormData();
    void _parseMultipartFormData(const std::string &boundary);
};

#endif