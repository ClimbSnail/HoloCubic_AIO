#ifndef NATIVE_SERVER_H
#define NATIVE_SERVER_H

#ifdef INPUT
#undef INPUT
#endif
#ifdef OUTPUT
#undef OUTPUT
#endif
#ifdef HIGH
#undef HIGH
#endif
#ifdef LOW
#undef LOW
#endif

#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOGDI
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#ifndef INPUT
#define INPUT 0x01
#endif
#ifndef OUTPUT
#define OUTPUT 0x02
#endif
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW  0x0
#endif

#include <string>

class NativeServer
{
    static int _refCount;
    static void _initWinsock();
    static void _cleanupWinsock();

protected:
    SOCKET _listenSocket;
    SOCKET _clientSocket;
    int _port;
    bool _running;
    bool _clientConnected;

public:
    NativeServer(int port = 0);
    virtual ~NativeServer();

    bool begin();
    void stop();
    bool acceptClient();
    void disconnectClient();
    int read(char *buf, int maxLen);
    int write(const char *buf, int len);
    bool isClientConnected();
    bool isRunning() const { return _running; }
    SOCKET getClientSocket() const { return _clientSocket; }
    SOCKET getListenSocket() const { return _listenSocket; }
};

#endif