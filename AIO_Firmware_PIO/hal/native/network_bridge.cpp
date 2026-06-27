#undef INPUT
#undef OUTPUT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#define INPUT 0x01
#define OUTPUT 0x02

#include "WiFi.h"
#include "HTTPClient.h"
#include "Stream.h"
#include "Arduino.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

static bool s_wsaInitialized = false;

static void ensureWSA()
{
    if (!s_wsaInitialized)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        s_wsaInitialized = true;
    }
}

static void setSocketTimeout(int sock, int timeout_ms)
{
    DWORD t = (DWORD)timeout_ms;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&t, sizeof(t));
}

static void setNonBlocking(int sock, bool nonblock)
{
    u_long mode = nonblock ? 1 : 0;
    ioctlsocket(sock, FIONBIO, &mode);
}

IPAddress WiFiClass::localIP()
{
    ensureWSA();
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent *h = gethostbyname(hostname);
    if (h && h->h_addr_list[0])
    {
        struct in_addr addr;
        memcpy(&addr, h->h_addr_list[0], sizeof(addr));
        return IPAddress(0, 0, 0, 0);
    }
    return IPAddress(127, 0, 0, 1);
}

// ==================== WiFiClient ====================

void WiFiClient::_init()
{
    _sock = -1;
}

void WiFiClient::_close()
{
    if (_sock >= 0)
    {
        closesocket(_sock);
        _sock = -1;
    }
}

WiFiClient::WiFiClient()
{
    _init();
}

WiFiClient::WiFiClient(int fd) : _sock(fd)
{
}

WiFiClient::~WiFiClient()
{
    _close();
}

WiFiClient::WiFiClient(const WiFiClient &other)
{
    _sock = -1;
    if (other._sock >= 0)
    {
        _sock = other._sock;
    }
}

WiFiClient::WiFiClient(WiFiClient &&other)
{
    _sock = other._sock;
    other._sock = -1;
}

WiFiClient &WiFiClient::operator=(const WiFiClient &other)
{
    if (this != &other)
    {
        _close();
        if (other._sock >= 0)
        {
            _sock = other._sock;
        }
    }
    return *this;
}

WiFiClient &WiFiClient::operator=(WiFiClient &&other)
{
    if (this != &other)
    {
        _close();
        _sock = other._sock;
        other._sock = -1;
    }
    return *this;
}

int WiFiClient::connect(IPAddress ip, uint16_t port)
{
    ensureWSA();
    _close();
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0) return 0;

    setSocketTimeout(_sock, _timeout);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl((uint32_t)ip);

    if (::connect(_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        _close();
        return 0;
    }
    setNonBlocking(_sock, true);
    return 1;
}

int WiFiClient::connect(const char *host, uint16_t port)
{
    return connect(host, port, (int)_timeout);
}

int WiFiClient::connect(const char *host, uint16_t port, int timeout)
{
    Serial.printf("[WIFI_CLIENT] connect(%s:%d, timeout=%d) enter\n", host, port, timeout);
    ensureWSA();
    _close();
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0) {
        Serial.printf("[WIFI_CLIENT] socket() failed\n");
        return 0;
    }

    setSocketTimeout(_sock, timeout > 0 ? timeout : 1000);

    Serial.printf("[WIFI_CLIENT] gethostbyname(%s) starting...\n", host);
    struct hostent *h = gethostbyname(host);
    Serial.printf("[WIFI_CLIENT] gethostbyname(%s) done, h=%p\n", host, (void*)h);
    if (!h)
    {
        Serial.printf("[WIFI_CLIENT] gethostbyname failed\n");
        _close();
        return 0;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, h->h_addr_list[0], h->h_length);

    Serial.printf("[WIFI_CLIENT] ::connect(%s:%d) starting...\n", host, port);
    int conn_ret = ::connect(_sock, (struct sockaddr *)&addr, sizeof(addr));
    Serial.printf("[WIFI_CLIENT] ::connect(%s:%d) done, ret=%d\n", host, port, conn_ret);
    if (conn_ret != 0)
    {
        _close();
        return 0;
    }
    setNonBlocking(_sock, true);
    return 1;
}

size_t WiFiClient::write(uint8_t c)
{
    if (_sock < 0) return 0;
    return send(_sock, (const char *)&c, 1, 0) > 0 ? 1 : 0;
}

size_t WiFiClient::write(const uint8_t *buf, size_t size)
{
    if (_sock < 0) return 0;
    int sent = send(_sock, (const char *)buf, (int)size, 0);
    return sent > 0 ? (size_t)sent : 0;
}

size_t WiFiClient::write(const char *str)
{
    return Print::write(str);
}

int WiFiClient::available()
{
    if (_sock < 0) return 0;
    u_long avail = 0;
    ioctlsocket(_sock, FIONREAD, &avail);
    return (int)avail;
}

int WiFiClient::read()
{
    if (_sock < 0) return -1;
    uint8_t c;
    int n = recv(_sock, (char *)&c, 1, 0);
    return n > 0 ? (int)c : (n == 0 ? -1 : -1);
}

int WiFiClient::read(uint8_t *buf, size_t size)
{
    if (_sock < 0) return -1;
    int n = recv(_sock, (char *)buf, (int)size, 0);
    if (n > 0) return n;
    if (n == 0) return 0;
    return -1;
}

int WiFiClient::peek()
{
    if (_sock < 0) return -1;
    uint8_t c;
    int n = recv(_sock, (char *)&c, 1, MSG_PEEK);
    return n > 0 ? (int)c : -1;
}

void WiFiClient::flush()
{
}

void WiFiClient::stop()
{
    _close();
}

uint8_t WiFiClient::connected()
{
    if (_sock < 0) return 0;

    char buf;
    int n = recv(_sock, &buf, 1, MSG_PEEK);
    if (n == 0)
    {
        return 0;
    }
    if (n < 0)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return 1;
        return 0;
    }
    return 1;
}

WiFiClient::operator bool()
{
    return _sock >= 0;
}

void WiFiClient::println(const char *s)
{
    Print::println(s ? s : "");
}

void WiFiClient::println(int n)
{
    Print::println(n);
}

void WiFiClient::print(const char *s)
{
    Print::print(s);
}

void WiFiClient::print(int n)
{
    Print::print(n);
}

void WiFiClient::print(const String &s)
{
    Print::print(s.c_str());
}

void WiFiClient::println(const String &s)
{
    Print::println(s.c_str());
}

// ==================== WiFiServer ====================

WiFiServer::WiFiServer(uint16_t port) : _sock(-1), _port(port), _listening(false)
{
}

WiFiServer::~WiFiServer()
{
    close();
}

void WiFiServer::begin()
{
    begin(_port);
}

void WiFiServer::begin(uint16_t port)
{
    ensureWSA();
    _port = port;
    close();

    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0) return;

    int opt = 1;
    setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        closesocket(_sock);
        _sock = -1;
        return;
    }

    if (listen(_sock, 5) != 0)
    {
        closesocket(_sock);
        _sock = -1;
        return;
    }

    setNonBlocking(_sock, true);
    _listening = true;
}

void WiFiServer::end()
{
    close();
}

void WiFiServer::stop()
{
    close();
}

void WiFiServer::close()
{
    if (_sock >= 0)
    {
        closesocket(_sock);
        _sock = -1;
    }
    _listening = false;
}

void WiFiServer::setNoDelay(bool nodelay)
{
    (void)nodelay;
}

WiFiClient WiFiServer::available()
{
    if (_sock < 0) return WiFiClient();

    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    int clientSock = accept(_sock, (struct sockaddr *)&clientAddr, &addrLen);

    if (clientSock < 0)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return WiFiClient();
        return WiFiClient();
    }

    setNonBlocking(clientSock, true);
    return WiFiClient(clientSock);
}

uint8_t WiFiServer::status()
{
    return _listening ? 1 : 0;
}

bool WiFiServer::hasClient()
{
    if (_sock < 0) return false;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_sock, &readfds);

    struct timeval tv = {0, 0};
    int ret = select(0, &readfds, NULL, NULL, &tv);
    return ret > 0 && FD_ISSET(_sock, &readfds);
}

// ==================== HTTPClient ====================

HTTPClient::HTTPClient()
    : _port(80), _https(false), _httpCode(-1), _contentLength(-1),
      _timeout(1000), _connected(false), _client(nullptr)
{
}

HTTPClient::~HTTPClient()
{
    end();
}

void HTTPClient::_parseUrl(const char *url)
{
    _host = "";
    _path = "/";
    _port = 80;
    _https = false;

    if (!url || url[0] == '\0') return;

    const char *p = url;

    if (strncmp(p, "https://", 8) == 0)
    {
        _https = true;
        _port = 443;
        p += 8;
    }
    else if (strncmp(p, "http://", 7) == 0)
    {
        _https = false;
        _port = 80;
        p += 7;
    }

    const char *hostStart = p;
    const char *hostEnd = strchr(hostStart, '/');
    const char *portStart = strchr(hostStart, ':');

    if (portStart && (!hostEnd || portStart < hostEnd))
    {
        _host = String(hostStart, (int)(portStart - hostStart));
        _port = (uint16_t)atoi(portStart + 1);
        if (hostEnd)
            _path = String(hostEnd);
        else
            _path = "/";
    }
    else if (hostEnd)
    {
        _host = String(hostStart, (int)(hostEnd - hostStart));
        _path = String(hostEnd);
    }
    else
    {
        _host = String(hostStart);
        _path = "/";
    }
}

bool HTTPClient::begin(const char *url)
{
    end();
    _parseUrl(url);
    _httpCode = -1;
    _contentLength = -1;
    _responseBody = "";
    _headers = "";
    _connected = true;
    return true;
}

bool HTTPClient::begin(String url)
{
    return begin(url.c_str());
}

void HTTPClient::setTimeout(uint16_t timeout)
{
    _timeout = timeout;
}

void HTTPClient::addHeader(const char *name, const char *value)
{
    _headers += String(name) + ": " + String(value) + "\r\n";
}

int HTTPClient::GET()
{
    fflush(stdout);
    fprintf(stderr, "[HTTP_GET] ENTER\n");
    fflush(stderr);
    printf("[HTTP_GET] ENTER\n");
    fflush(stdout);
    Serial.printf("[HTTP_GET] ENTER\n");
    Serial.flush();
    int ret = _sendRequest("GET", nullptr);
    fprintf(stderr, "[HTTP_GET] _sendRequest returned %d\n", ret);
    fflush(stderr);
    printf("[HTTP_GET] _sendRequest returned %d\n", ret);
    fflush(stdout);
    return ret;
}

int HTTPClient::POST(const char *body)
{
    return _sendRequest("POST", body);
}

int HTTPClient::POST(String body)
{
    return POST(body.c_str());
}

int HTTPClient::sendRequest(const char *method, const char *body)
{
    return _sendRequest(method, body);
}

int HTTPClient::_sendRequestHttps(const char *method, const char *body)
{
    Serial.printf("[HTTP_HTTPS] ENTER: method=%s, host=%s, port=%d, path=%s\n",
                  method, _host.c_str(), _port, _path.c_str());
    Serial.flush();
    printf("[HTTP] >>> HTTPS %s %s:%d%s\n", method, _host.c_str(), _port, _path.c_str());
    fflush(stdout);

    int wlen = MultiByteToWideChar(CP_UTF8, 0, _host.c_str(), -1, NULL, 0);
    wchar_t *whost = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, _host.c_str(), -1, whost, wlen);

    wlen = MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, NULL, 0);
    wchar_t *wpath = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, _path.c_str(), -1, wpath, wlen);

    DWORD dwFlags = WINHTTP_FLAG_SECURE;
    DWORD dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;

    HINTERNET hSession = WinHttpOpen(L"HoloCubic-AIO/1.0",
        dwAccessType, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        printf("[HTTP] *** WinHttpOpen failed: %lu\n", GetLastError());
        delete[] whost; delete[] wpath;
        _httpCode = -1;
        return -1;
    }

    WinHttpSetTimeouts(hSession, _timeout, _timeout, _timeout, _timeout);

    HINTERNET hConnect = WinHttpConnect(hSession, whost, _port, 0);
    if (!hConnect)
    {
        printf("[HTTP] *** WinHttpConnect failed: %lu\n", GetLastError());
        WinHttpCloseHandle(hSession);
        delete[] whost; delete[] wpath;
        _httpCode = -1;
        return -1;
    }

    wchar_t *wmethod = new wchar_t[16];
    MultiByteToWideChar(CP_UTF8, 0, method, -1, wmethod, 16);

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, wmethod, wpath,
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
    if (!hRequest)
    {
        printf("[HTTP] *** WinHttpOpenRequest failed: %lu\n", GetLastError());
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        delete[] wmethod; delete[] whost; delete[] wpath;
        _httpCode = -1;
        return -1;
    }

    String requestHeaders = "Host: " + _host + "\r\n";
    if (_headers.length() > 0)
        requestHeaders += _headers;
    if (body)
        requestHeaders += "Content-Type: application/x-www-form-urlencoded\r\n";

    wlen = MultiByteToWideChar(CP_UTF8, 0, requestHeaders.c_str(), -1, NULL, 0);
    wchar_t *wheaders = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, requestHeaders.c_str(), -1, wheaders, wlen);

    const char *bodyData = body ? body : "";
    DWORD bodyLen = body ? (DWORD)strlen(body) : 0;

    BOOL result = WinHttpSendRequest(hRequest, wheaders,
        (DWORD)wcslen(wheaders), (LPVOID)bodyData, bodyLen, bodyLen, 0);

    delete[] wmethod;
    delete[] whost;
    delete[] wpath;
    delete[] wheaders;

    if (!result)
    {
        printf("[HTTP] *** WinHttpSendRequest failed: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        _httpCode = -1;
        return -1;
    }

    printf("[HTTP] connected OK\n");

    result = WinHttpReceiveResponse(hRequest, NULL);
    if (!result)
    {
        printf("[HTTP] *** WinHttpReceiveResponse failed: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        _httpCode = -1;
        return -1;
    }

    printf("[HTTP] got response, reading...\n");

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    _httpCode = (int)statusCode;

    printf("[HTTP] status: HTTP/1.1 %d\n", _httpCode);

    DWORD contentLength = 0;
    DWORD contentLengthSize = sizeof(contentLength);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &contentLength, &contentLengthSize, WINHTTP_NO_HEADER_INDEX);
    _contentLength = (int)contentLength;

    printf("[HTTP] Content-Length=%d, reading body...\n", _contentLength);

    _responseBody = "";
    DWORD bytesAvailable = 0;
    char buf[4096];
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0)
    {
        DWORD bytesToRead = (bytesAvailable < sizeof(buf) - 1) ? bytesAvailable : (sizeof(buf) - 1);
        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buf, bytesToRead, &bytesRead))
        {
            buf[bytesRead] = '\0';
            _responseBody += buf;
        }
        else
        {
            break;
        }
    }

    printf("[HTTP] body len=%d, first 100 chars: %.100s\n",
           (int)_responseBody.length(),
           _responseBody.length() > 0 ? _responseBody.c_str() : "(empty)");

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return _httpCode;
}

int HTTPClient::_sendRequest(const char *method, const char *body)
{
    Serial.printf("[HTTP_REQ] _sendRequest: method=%s, host=%s, port=%d, https=%d\n",
                  method, _host.c_str(), _port, (int)_https);
    Serial.flush();
    if (_host.length() == 0) return -1;

    if (_https)
    {
        Serial.printf("[HTTP_REQ] routing to HTTPS...\n");
        Serial.flush();
        return _sendRequestHttps(method, body);
    }

    printf("[HTTP] >>> %s %s:%d%s\n", method, _host.c_str(), _port, _path.c_str());

    WiFiClient client;
    client.setTimeout(_timeout);

    if (!client.connect(_host.c_str(), _port, (int)_timeout))
    {
        printf("[HTTP] *** connect failed: %s:%d\n", _host.c_str(), _port);
        _httpCode = -1;
        return -1;
    }

    printf("[HTTP] connected OK\n");

    String request = String(method) + " " + _path + " HTTP/1.1\r\n";
    request += "Host: " + _host + "\r\n";
    request += "User-Agent: HoloCubic-AIO/1.0\r\n";
    request += "Connection: close\r\n";

    if (_headers.length() > 0)
    {
        request += _headers;
    }

    if (body)
    {
        char lenBuf[32];
        snprintf(lenBuf, sizeof(lenBuf), "%d", (int)strlen(body));
        request += "Content-Type: application/x-www-form-urlencoded\r\n";
        request += "Content-Length: " + String(lenBuf) + "\r\n";
        request += "\r\n";
        request += body;
    }
    else
    {
        request += "\r\n";
    }

    client.write((const uint8_t *)request.c_str(), request.length());

    unsigned long startTime = millis();
    while (client.available() == 0)
    {
        if (millis() - startTime > _timeout)
        {
            printf("[HTTP] *** timeout waiting for response\n");
            client.stop();
            _httpCode = -1;
            return -1;
        }
        delay(10);
    }

    printf("[HTTP] got response, reading...\n");

    // Read status line: "HTTP/1.1 200 OK"
    String statusLine;
    while (client.available())
    {
        char c = (char)client.read();
        if (c == '\n') break;
        if (c != '\r') statusLine += c;
    }

    printf("[HTTP] status: %s\n", statusLine.c_str());

    _httpCode = -1;
    int firstSpace = statusLine.indexOf(' ');
    if (firstSpace >= 0)
    {
        int secondSpace = statusLine.indexOf(' ', firstSpace + 1);
        if (secondSpace < 0) secondSpace = (int)statusLine.length();
        _httpCode = statusLine.substring(firstSpace + 1, secondSpace).toInt();
    }

    printf("[HTTP] code=%d\n", _httpCode);

    _contentLength = -1;
    _responseBody = "";

    // Read headers
    while (true)
    {
        String line;
        while (client.available())
        {
            char c = (char)client.read();
            if (c == '\n') break;
            if (c != '\r') line += c;
        }
        if (line.length() == 0) break;

        if (line.startsWith("Content-Length:"))
        {
            _contentLength = line.substring(15).toInt();
        }
    }

    printf("[HTTP] Content-Length=%d, reading body...\n", _contentLength);

    // Read body
    if (_contentLength > 0)
    {
        // Content-Length 已知：精确读取，读完即止
        while ((int)_responseBody.length() < _contentLength)
        {
            unsigned long waitStart = millis();
            while (client.available() == 0)
            {
                if (millis() - waitStart > _timeout || !client.connected())
                    break;
                delay(10);
            }
            if (client.available() == 0 && !client.connected())
                break;

            while (client.available() && (int)_responseBody.length() < _contentLength)
            {
                char c = (char)client.read();
                _responseBody += c;
            }
        }
    }
    else
    {
        // Content-Length 未知：读到 server 关闭连接
        while (true)
        {
            unsigned long waitStart = millis();
            while (client.available() == 0)
            {
                if (millis() - waitStart > _timeout || !client.connected())
                    break;
                delay(10);
            }
            if (client.available() == 0 && !client.connected())
                break;

            while (client.available())
            {
                char c = (char)client.read();
                _responseBody += c;
            }
        }
    }

    printf("[HTTP] body len=%d, first 100 chars: %.100s\n",
           (int)_responseBody.length(),
           _responseBody.length() > 0 ? _responseBody.c_str() : "(empty)");

    client.stop();
    return _httpCode;
}

int HTTPClient::getSize()
{
    return (int)_responseBody.length();
}

String HTTPClient::getString()
{
    return _responseBody;
}

String HTTPClient::errorToString(int error)
{
    if (error < 0) return String("connection failed");
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP error %d", error);
    return String(buf);
}

void HTTPClient::end()
{
    _connected = false;
    _responseBody = "";
}

bool HTTPClient::connected()
{
    return _connected;
}

// ==================== Stream methods ====================

bool Stream::find(const char *target)
{
    return find(target, strlen(target));
}

bool Stream::find(const char *target, size_t length)
{
    if (length == 0) return true;

    size_t matched = 0;
    unsigned long start = millis();

    while (true)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                return false;
            continue;
        }

        int c = read();
        if (c < 0) return false;

        if ((char)c == target[matched])
        {
            matched++;
            if (matched == length)
                return true;
        }
        else
        {
            matched = 0;
            if ((char)c == target[0])
                matched = 1;
        }
    }
}

bool Stream::find(uint8_t target)
{
    unsigned long start = millis();
    while (true)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                return false;
            continue;
        }
        int c = read();
        if (c < 0) return false;
        if ((uint8_t)c == target) return true;
    }
}

String Stream::readStringUntil(char terminator)
{
    String result;
    unsigned long start = millis();

    while (true)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                break;
            continue;
        }
        int c = read();
        if (c < 0) break;
        if ((char)c == terminator) break;
        result += (char)c;
    }
    return result;
}

String Stream::readString()
{
    String result;
    unsigned long start = millis();

    while (true)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                break;
            continue;
        }
        int c = read();
        if (c < 0) break;
        result += (char)c;
    }
    return result;
}

size_t Stream::readBytes(uint8_t *buffer, size_t length)
{
    size_t count = 0;
    unsigned long start = millis();

    while (count < length)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                break;
            continue;
        }
        int c = read();
        if (c < 0) break;
        buffer[count++] = (uint8_t)c;
    }
    return count;
}

size_t Stream::readBytesUntil(char terminator, uint8_t *buffer, size_t length)
{
    size_t count = 0;
    unsigned long start = millis();

    while (count < length)
    {
        if (available() == 0)
        {
            if (millis() - start > _timeout)
                break;
            continue;
        }
        int c = read();
        if (c < 0) break;
        if ((char)c == terminator) break;
        buffer[count++] = (uint8_t)c;
    }
    return count;
}