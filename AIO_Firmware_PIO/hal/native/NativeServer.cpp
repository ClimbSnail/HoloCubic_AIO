#include "NativeServer.h"
#include <cstdio>

int NativeServer::_refCount = 0;

void NativeServer::_initWinsock()
{
    if (_refCount == 0)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }
    _refCount++;
}

void NativeServer::_cleanupWinsock()
{
    _refCount--;
    if (_refCount <= 0)
    {
        WSACleanup();
        _refCount = 0;
    }
}

NativeServer::NativeServer(int port)
    : _listenSocket(INVALID_SOCKET), _clientSocket(INVALID_SOCKET),
      _port(port), _running(false), _clientConnected(false)
{
    _initWinsock();
}

NativeServer::~NativeServer()
{
    stop();
    _cleanupWinsock();
}

bool NativeServer::begin()
{
    if (_running)
        return true;

    _listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_listenSocket == INVALID_SOCKET)
        return false;

    int opt = 1;
    setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((u_short)_port);

    if (bind(_listenSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(_listenSocket);
        _listenSocket = INVALID_SOCKET;
        return false;
    }

    if (listen(_listenSocket, 1) == SOCKET_ERROR)
    {
        closesocket(_listenSocket);
        _listenSocket = INVALID_SOCKET;
        return false;
    }

    u_long mode = 1;
    ioctlsocket(_listenSocket, FIONBIO, &mode);

    _running = true;
    printf("[NativeServer] Listening on port %d\n", _port);
    return true;
}

void NativeServer::stop()
{
    if (_clientConnected)
        disconnectClient();

    if (_listenSocket != INVALID_SOCKET)
    {
        closesocket(_listenSocket);
        _listenSocket = INVALID_SOCKET;
    }

    _running = false;
}

bool NativeServer::acceptClient()
{
    if (_clientConnected)
        return false;

    if (_listenSocket == INVALID_SOCKET)
        return false;

    _clientSocket = accept(_listenSocket, NULL, NULL);
    if (_clientSocket == INVALID_SOCKET)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
        {
            printf("[NativeServer] accept error: %d\n", err);
        }
        return false;
    }

    u_long mode = 1;
    ioctlsocket(_clientSocket, FIONBIO, &mode);

    _clientConnected = true;
    printf("[NativeServer] Client connected\n");
    return true;
}

void NativeServer::disconnectClient()
{
    if (_clientSocket != INVALID_SOCKET)
    {
        closesocket(_clientSocket);
        _clientSocket = INVALID_SOCKET;
    }
    _clientConnected = false;
}

int NativeServer::read(char *buf, int maxLen)
{
    if (!_clientConnected || _clientSocket == INVALID_SOCKET)
        return -1;

    int n = recv(_clientSocket, buf, maxLen, 0);
    if (n <= 0)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
        {
            disconnectClient();
        }
        return -1;
    }
    return n;
}

int NativeServer::write(const char *buf, int len)
{
    if (!_clientConnected || _clientSocket == INVALID_SOCKET)
        return -1;

    int n = send(_clientSocket, buf, len, 0);
    if (n == SOCKET_ERROR)
    {
        disconnectClient();
        return -1;
    }
    return n;
}

bool NativeServer::isClientConnected()
{
    if (!_clientConnected || _clientSocket == INVALID_SOCKET)
        return false;

    char c;
    int n = recv(_clientSocket, &c, 1, MSG_PEEK);
    if (n == 0)
    {
        disconnectClient();
        return false;
    }
    if (n < 0)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
        {
            disconnectClient();
            return false;
        }
    }
    return true;
}