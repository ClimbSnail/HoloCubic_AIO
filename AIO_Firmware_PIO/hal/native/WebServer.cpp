#include "WebServer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

static WebServer *g_currentServer = nullptr;

static std::string urlDecode(const std::string &src)
{
    std::string result;
    for (size_t i = 0; i < src.size(); i++)
    {
        if (src[i] == '%' && i + 2 < src.size())
        {
            int c;
            sscanf(src.substr(i + 1, 2).c_str(), "%x", &c);
            result += (char)c;
            i += 2;
        }
        else if (src[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += src[i];
        }
    }
    return result;
}

void WebServer::begin()
{
    _server.begin();
}

void WebServer::handleClient()
{
    if (!_server.isRunning())
        return;

    if (!_server.isClientConnected())
    {
        _server.acceptClient();
        if (!_server.isClientConnected())
            return;

        _recvLen = 0;
        _headersSent = false;
        _pendingHeaders.clear();
        _chunked = false;
        _queryArgs.clear();
        _hasArgs = false;
        memset(&_upload, 0, sizeof(_upload));
    }

    int n = _server.read(_recvBuf + _recvLen, (int)(sizeof(_recvBuf) - _recvLen - 1));
    if (n <= 0)
    {
        if (!_server.isClientConnected())
            _recvLen = 0;
        return;
    }
    _recvLen += n;
    _recvBuf[_recvLen] = '\0';

    const char *headerEnd = strstr(_recvBuf, "\r\n\r\n");
    if (!headerEnd)
    {
        if (_recvLen >= (int)sizeof(_recvBuf) - 1)
        {
            _server.disconnectClient();
            _recvLen = 0;
        }
        return;
    }

    printf("[WebServer] Raw request (first line): %.100s\n", _recvBuf);

    _parseRequest();

    printf("[WebServer] Request: %s method=%d routes=%d\n", _currentUri.c_str(), _currentMethod, (int)_routes.size());

    g_currentServer = this;
    RouteHandler *handler = nullptr;
    if (_matchRoute(_currentUri, _currentMethod, handler))
    {
        printf("[WebServer] Route matched, calling handler\n");
        if (handler->handler)
            handler->handler();
    }
    else
    {
        printf("[WebServer] No route matched, sending 404\n");
        _sendRaw("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n404 Not Found\r\n", 80);
    }
    g_currentServer = nullptr;

    _server.disconnectClient();
    _recvLen = 0;
}

void WebServer::_parseRequest()
{
    _currentMethod = HTTP_GET;
    _currentUri = "/";
    _queryArgs.clear();
    _hasArgs = false;

    const char *lineEnd = strstr(_recvBuf, "\r\n");
    if (!lineEnd)
        return;

    std::string requestLine(_recvBuf, lineEnd - _recvBuf);

    if (requestLine.compare(0, 4, "GET ") == 0)
    {
        _currentMethod = HTTP_GET;
    }
    else if (requestLine.compare(0, 5, "POST ") == 0)
    {
        _currentMethod = HTTP_POST;
    }
    else
    {
        return;
    }

    size_t uriStart = (_currentMethod == HTTP_GET) ? 4 : 5;
    size_t uriEnd = requestLine.find(' ', uriStart);
    if (uriEnd == std::string::npos)
        return;

    std::string fullUri = requestLine.substr(uriStart, uriEnd - uriStart);

    size_t qPos = fullUri.find('?');
    if (qPos != std::string::npos)
    {
        _currentUri = fullUri.substr(0, qPos);
        _parseQueryArgs(fullUri.substr(qPos + 1));
    }
    else
    {
        _currentUri = fullUri;
    }

    if (_currentMethod == HTTP_POST)
    {
        const char *bodyStart = strstr(_recvBuf, "\r\n\r\n");
        if (bodyStart)
        {
            bodyStart += 4;
            const char *contentType = strstr(_recvBuf, "Content-Type: ");
            if (contentType)
            {
                contentType += 14;
                const char *ctEnd = strstr(contentType, "\r\n");
                if (ctEnd)
                {
                    std::string ct(contentType, ctEnd - contentType);
                    if (ct.find("multipart/form-data") != std::string::npos)
                    {
                        const char *bd = strstr(ct.c_str(), "boundary=");
                        if (bd)
                        {
                            std::string boundary = bd + 9;
                            _parseMultipartFormData(boundary);
                        }
                    }
                    else
                    {
                        _parsePostFormData();
                    }
                }
            }
            else
            {
                _parsePostFormData();
            }
        }
    }
}

void WebServer::_parseQueryArgs(const std::string &query)
{
    _hasArgs = true;
    size_t pos = 0;
    while (pos < query.size())
    {
        size_t eq = query.find('=', pos);
        size_t amp = query.find('&', pos);
        if (amp == std::string::npos)
            amp = query.size();

        if (eq != std::string::npos && eq < amp)
        {
            std::string key = urlDecode(query.substr(pos, eq - pos));
            std::string val = urlDecode(query.substr(eq + 1, amp - eq - 1));
            _queryArgs[key] = val;
        }
        pos = amp + 1;
    }
}

void WebServer::_parsePostFormData()
{
    const char *bodyStart = strstr(_recvBuf, "\r\n\r\n");
    if (!bodyStart)
        return;
    bodyStart += 4;

    std::string body(bodyStart, _recvLen - (bodyStart - _recvBuf));
    _parseQueryArgs(body);
}

void WebServer::_parseMultipartFormData(const std::string &boundary)
{
    const char *bodyStart = strstr(_recvBuf, "\r\n\r\n");
    if (!bodyStart)
        return;
    bodyStart += 4;

    std::string body(bodyStart, _recvLen - (bodyStart - _recvBuf));
    std::string delim = "--" + boundary;

    size_t pos = body.find(delim);
    while (pos != std::string::npos)
    {
        pos += delim.size();
        if (body.compare(pos, 2, "--") == 0)
            break;

        if (body[pos] == '\r')
            pos += 2;

        size_t hdrEnd = body.find("\r\n\r\n", pos);
        if (hdrEnd == std::string::npos)
            break;

        std::string partHeaders = body.substr(pos, hdrEnd - pos);
        size_t dataStart = hdrEnd + 4;
        size_t nextPart = body.find(delim, dataStart);
        if (nextPart == std::string::npos)
            break;

        size_t dataEnd = nextPart;
        while (dataEnd > dataStart && body[dataEnd - 1] == '\n')
            dataEnd--;
        if (dataEnd > dataStart && body[dataEnd - 1] == '\r')
            dataEnd--;

        std::string partData = body.substr(dataStart, dataEnd - dataStart);

        const char *nameStart = strstr(partHeaders.c_str(), "name=\"");
        if (nameStart)
        {
            nameStart += 6;
            const char *nameEnd = strchr(nameStart, '"');
            if (nameEnd)
            {
                std::string fieldName(nameStart, nameEnd - nameStart);

                const char *fnStart = strstr(partHeaders.c_str(), "filename=\"");
                if (fnStart)
                {
                    fnStart += 10;
                    const char *fnEnd = strchr(fnStart, '"');
                    if (fnEnd)
                    {
                        _upload.filename = String(std::string(fnStart, fnEnd - fnStart).c_str());
                    }
                    _upload.status = UPLOAD_FILE_START;
                    _upload.buf = (const uint8_t *)partData.c_str();
                    _upload.currentSize = partData.size();
                    _upload.totalSize = partData.size();

                    g_currentServer = this;
                    RouteHandler *handler = nullptr;
                    if (_matchRoute(_currentUri, _currentMethod, handler) && handler->uploadHandler)
                    {
                        handler->uploadHandler();
                    }

                    _upload.status = UPLOAD_FILE_END;
                    if (handler && handler->uploadHandler)
                    {
                        handler->uploadHandler();
                    }
                    g_currentServer = nullptr;
                }
                else
                {
                    _queryArgs[fieldName] = urlDecode(partData);
                }
            }
        }

        pos = nextPart;
    }
    _hasArgs = true;
}

bool WebServer::_matchRoute(const std::string &uri, int method, RouteHandler *&handler)
{
    for (size_t i = 0; i < _routes.size(); i++)
    {
        if (_routes[i].uri == uri && _routes[i].method == method)
        {
            handler = &_routes[i];
            return true;
        }
    }
    return false;
}

void WebServer::_flushHeaders()
{
    if (_headersSent)
        return;
    _headersSent = true;

    char buf[256];
    const char *reason = "OK";
    if (_responseCode == 404)
        reason = "Not Found";
    else if (_responseCode == 500)
        reason = "Internal Server Error";

    snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", _responseCode, reason);
    printf("[WebServer] Response: %s", buf);
    _server.write(buf, (int)strlen(buf));

    if (!_chunked)
    {
        snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", _contentType.c_str());
        _server.write(buf, (int)strlen(buf));
    }

    if (!_pendingHeaders.empty())
    {
        printf("[WebServer] Headers: %s", _pendingHeaders.c_str());
        _server.write(_pendingHeaders.c_str(), (int)_pendingHeaders.size());
        _pendingHeaders.clear();
    }

    if (!_chunked)
    {
        _server.write("Connection: close\r\n", 19);
    }

    _server.write("\r\n", 2);
    printf("[WebServer] chunked=%d headersSent=%d\n", (int)_chunked, (int)_headersSent);
}

void WebServer::_sendRaw(const char *data, int len)
{
    if (len > 0)
        _server.write(data, len);
}

void WebServer::send(int code, const char *type, const char *content)
{
    _responseCode = code;
    _contentType = type ? type : "text/plain";
    _flushHeaders();

    if (content && content[0])
    {
        _server.write(content, (int)strlen(content));
    }

    if (!_chunked)
    {
        _server.write("\r\n", 2);
    }
}

void WebServer::send(int code, const char *type, const String &content)
{
    send(code, type, content.c_str());
}

void WebServer::send(int code)
{
    _responseCode = code;
    _contentType = "text/plain";
    _flushHeaders();
    _server.write("\r\n", 2);
}

void WebServer::sendHeader(const char *name, const char *value, bool first)
{
    (void)first;
    _pendingHeaders += name;
    _pendingHeaders += ": ";
    _pendingHeaders += value;
    _pendingHeaders += "\r\n";
}

void WebServer::sendHeader(const char *name, const String &value, bool first)
{
    sendHeader(name, value.c_str(), first);
}

void WebServer::setContentLength(size_t len)
{
    if (len == CONTENT_LENGTH_UNKNOWN)
    {
        _chunked = true;
        _pendingHeaders += "Transfer-Encoding: chunked\r\n";
    }
    else
    {
        _chunked = false;
        char buf[32];
        snprintf(buf, sizeof(buf), "%zu", len);
        _pendingHeaders += "Content-Length: ";
        _pendingHeaders += buf;
        _pendingHeaders += "\r\n";
    }
    _headersSent = false;
}

void WebServer::sendContent(const char *content)
{
    if (!_headersSent)
    {
        _flushHeaders();
    }

    if (!content || !content[0])
    {
        if (_chunked)
        {
            _server.write("0\r\n\r\n", 5);
        }
        printf("[WebServer] sendContent: empty (chunked terminator)\n");
        return;
    }

    int len = (int)strlen(content);
    printf("[WebServer] sendContent: %d bytes, chunked=%d, first=%.60s\n", len, (int)_chunked, content);

    if (_chunked)
    {
        char lenBuf[16];
        snprintf(lenBuf, sizeof(lenBuf), "%x\r\n", (unsigned int)len);
        _server.write(lenBuf, (int)strlen(lenBuf));
    }

    _server.write(content, len);

    if (_chunked)
    {
        _server.write("\r\n", 2);
    }
}

void WebServer::sendContent(const String &content)
{
    sendContent(content.c_str());
}

void WebServer::stop()
{
    _server.stop();
}

void WebServer::close()
{
    _server.stop();
}

WiFiClient &WebServer::client()
{
    static WiFiClient c;
    return c;
}

String WebServer::arg(const char *name) const
{
    if (g_currentServer)
    {
        auto it = g_currentServer->_queryArgs.find(name);
        if (it != g_currentServer->_queryArgs.end())
            return String(it->second.c_str());
    }
    return String("");
}

String WebServer::arg(int i) const
{
    int idx = 0;
    for (auto &kv : _queryArgs)
    {
        if (idx == i)
            return String(kv.second.c_str());
        idx++;
    }
    return "";
}

bool WebServer::hasArg(const char *name) const
{
    return _queryArgs.find(name) != _queryArgs.end();
}

void WebServer::streamFile(fs::File &file, const char *contentType)
{
    if (!file)
    {
        send(404, "text/plain", "File not found");
        return;
    }

    _responseCode = 200;
    _contentType = contentType ? contentType : "application/octet-stream";
    _chunked = false;

    int size = file.size();
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", size);
    _pendingHeaders += "Content-Length: ";
    _pendingHeaders += buf;
    _pendingHeaders += "\r\n";
    _headersSent = false;

    _flushHeaders();

    file.seek(0);
    char fileBuf[4096];
    while (file.available())
    {
        int n = file.read((uint8_t *)fileBuf, sizeof(fileBuf));
        if (n > 0)
            _server.write(fileBuf, n);
    }
    file.close();
}