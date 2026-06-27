#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#include "ESP32FtpServer.h"
#include "SD.h"
#include "serial_utils.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <utility>

// Global FTP server objects (same as real implementation)
WiFiServer ftpServer(FTP_CTRL_PORT);
WiFiServer dataServer(FTP_DATA_PORT_PASV);

static bool _authenticated = false;

void FtpServer::begin(String uname, String pword)
{
    _FTP_USER = uname;
    _FTP_PASS = pword;
    cwdName[0] = '/';
    cwdName[1] = '\0';

    ftpServer.begin();
    CreateDirectoryA(sd_get_base_path(), NULL);
    printf("[FtpServer] Started on port %d\n", FTP_CTRL_PORT);
}

void FtpServer::handleFTP()
{
    // 先检查是否有新客户端等待连接（即使当前客户端仍连接）
    if (ftpServer.hasClient())
    {
        client.stop();
        WiFiClient newClient = ftpServer.available();
        client = std::move(newClient);
        iCL = 0;
        _authenticated = false;
        cmdLine[0] = '\0';
        dataPassiveConn = false;
        rnfrCmd = false;
        client.println("220 HoloCubic FTP Server Ready");
        printf("[FtpServer] Client connected (hasClient)\n");
        return;
    }

    // Accept new clients
    if (!client || !client.connected())
    {
        WiFiClient newClient = ftpServer.available();
        if (newClient)
        {
            client = std::move(newClient);
            iCL = 0;
            _authenticated = false;
            cmdLine[0] = '\0';
            dataPassiveConn = false;
            rnfrCmd = false;
            client.println("220 HoloCubic FTP Server Ready");
            printf("[FtpServer] Client connected\n");
        }
        else
        {
            return;
        }
    }

    // Read commands from client
    while (client.available())
    {
        char c = (char)client.read();
        if (c == '\n')
        {
            if (iCL > 0 && cmdLine[iCL - 1] == '\r')
                cmdLine[iCL - 1] = '\0';
            else
                cmdLine[iCL] = '\0';

            printf("[FtpServer] -> %s\n", cmdLine);

            char *cmd = cmdLine;
            char *arg = strchr(cmd, ' ');
            if (arg)
            {
                *arg = '\0';
                arg++;
                while (*arg == ' ') arg++;
            }
            parameters = arg;

            // === Command processing ===
            if (strcmp(cmd, "USER") == 0)
            {
                if (arg && strcmp(arg, _FTP_USER.c_str()) == 0)
                    client.println("331 Password required");
                else
                    client.println("530 Invalid username");
            }
            else if (strcmp(cmd, "PASS") == 0)
            {
                if (arg && strcmp(arg, _FTP_PASS.c_str()) == 0)
                {
                    _authenticated = true;
                    client.println("230 User logged in");
                }
                else
                    client.println("530 Login incorrect");
            }
            else if (!_authenticated)
            {
                client.println("530 Not logged in");
            }
            else if (strcmp(cmd, "SYST") == 0)
            {
                client.println("215 UNIX Type: L8");
            }
            else if (strcmp(cmd, "FEAT") == 0)
            {
                client.println("211-Features:");
                client.println(" SIZE");
                client.println(" MDTM");
                client.println("211 End");
            }
            else if (strcmp(cmd, "TYPE") == 0)
            {
                client.println("200 Type set to I");
            }
            else if (strcmp(cmd, "PWD") == 0)
            {
                char msg[512];
                snprintf(msg, sizeof(msg), "257 \"%s\" is current directory", cwdName);
                client.println(msg);
            }
            else if (strcmp(cmd, "CWD") == 0)
            {
                if (!arg || arg[0] == '\0')
                {
                    client.println("550 No directory specified");
                }
                else
                {
                    std::string newPath;
                    if (arg[0] == '/')
                        newPath = arg;
                    else
                    {
                        newPath = cwdName;
                        if (newPath != "/") newPath += "/";
                        newPath += arg;
                    }

                    std::string sdPath = sd_get_base_path() + newPath;
                    if (sdPath.back() != '/') sdPath += "/";

                    WIN32_FIND_DATAA fd;
                    HANDLE h = FindFirstFileA((sdPath + "*").c_str(), &fd);
                    if (h == INVALID_HANDLE_VALUE)
                    {
                        client.println("550 Directory not found");
                    }
                    else
                    {
                        FindClose(h);
                        strncpy(cwdName, newPath.c_str(), FTP_CWD_SIZE - 1);
                        client.println("250 Directory changed");
                    }
                }
            }
            else if (strcmp(cmd, "CDUP") == 0)
            {
                if (strcmp(cwdName, "/") == 0)
                {
                    client.println("250 Already at root");
                }
                else
                {
                    char *lastSlash = strrchr(cwdName, '/');
                    if (lastSlash == cwdName)
                        cwdName[1] = '\0';
                    else if (lastSlash)
                        *lastSlash = '\0';
                    client.println("250 Directory changed");
                }
            }
            else if (strcmp(cmd, "MKD") == 0)
            {
                if (!arg)
                {
                    client.println("550 No directory name");
                }
                else
                {
                    std::string sdPath = sd_get_base_path() + std::string(cwdName);
                    if (sdPath.back() != '/') sdPath += "/";
                    sdPath += arg;
                    if (CreateDirectoryA(sdPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
                    {
                        char msg[512];
                        snprintf(msg, sizeof(msg), "257 \"%s\" created", arg);
                        client.println(msg);
                    }
                    else
                        client.println("550 Create directory failed");
                }
            }
            else if (strcmp(cmd, "RMD") == 0)
            {
                if (!arg)
                {
                    client.println("550 No directory name");
                }
                else
                {
                    std::string sdPath = sd_get_base_path() + std::string(cwdName);
                    if (sdPath.back() != '/') sdPath += "/";
                    sdPath += arg;
                    if (RemoveDirectoryA(sdPath.c_str()))
                        client.println("250 Directory removed");
                    else
                        client.println("550 Remove directory failed");
                }
            }
            else if (strcmp(cmd, "DELE") == 0)
            {
                if (!arg)
                {
                    client.println("550 No file name");
                }
                else
                {
                    std::string sdPath = sd_get_base_path() + std::string(cwdName);
                    if (sdPath.back() != '/') sdPath += "/";
                    sdPath += arg;
                    if (DeleteFileA(sdPath.c_str()))
                        client.println("250 File deleted");
                    else
                        client.println("550 Delete failed");
                }
            }
            else if (strcmp(cmd, "SIZE") == 0)
            {
                if (!arg)
                {
                    client.println("550 No file name");
                }
                else
                {
                    std::string sdPath = sd_get_base_path() + std::string(cwdName);
                    if (sdPath.back() != '/') sdPath += "/";
                    sdPath += arg;

                    WIN32_FILE_ATTRIBUTE_DATA attr;
                    if (GetFileAttributesExA(sdPath.c_str(), GetFileExInfoStandard, &attr))
                    {
                        ULONGLONG fileSize = ((ULONGLONG)attr.nFileSizeHigh << 32) | attr.nFileSizeLow;
                        char msg[64];
                        snprintf(msg, sizeof(msg), "213 %llu", fileSize);
                        client.println(msg);
                    }
                    else
                        client.println("550 File not found");
                }
            }
            else if (strcmp(cmd, "MDTM") == 0)
            {
                if (!arg)
                {
                    client.println("550 No file name");
                }
                else
                {
                    std::string sdPath = sd_get_base_path() + std::string(cwdName);
                    if (sdPath.back() != '/') sdPath += "/";
                    sdPath += arg;

                    WIN32_FILE_ATTRIBUTE_DATA attr;
                    if (GetFileAttributesExA(sdPath.c_str(), GetFileExInfoStandard, &attr))
                    {
                        SYSTEMTIME st;
                        FileTimeToSystemTime(&attr.ftLastWriteTime, &st);
                        char msg[64];
                        snprintf(msg, sizeof(msg), "213 %04d%02d%02d%02d%02d%02d",
                                 st.wYear, st.wMonth, st.wDay,
                                 st.wHour, st.wMinute, st.wSecond);
                        client.println(msg);
                    }
                    else
                        client.println("550 File not found");
                }
            }
            else if (strcmp(cmd, "PASV") == 0)
            {
                // Close previous data connection
                if (data) data.stop();
                dataPassiveConn = false;

                dataServer.stop();
                dataServer.begin(FTP_DATA_PORT_PASV);

                // Get the actual port assigned
                sockaddr_in addr;
                int addrLen = sizeof(addr);
                WiFiServer *srv = &dataServer;
                // We need to get the listening socket from dataServer
                // Since WiFiServer doesn't expose the socket directly,
                // we use a known port
                dataPort = FTP_DATA_PORT_PASV;

                char msg[128];
                snprintf(msg, sizeof(msg), "227 Entering Passive Mode (127,0,0,1,%d,%d)",
                         dataPort >> 8, dataPort & 0xFF);
                client.println(msg);
                dataPassiveConn = true;
            }
            else if (strcmp(cmd, "LIST") == 0 || strcmp(cmd, "NLST") == 0)
            {
                if (!dataPassiveConn)
                {
                    client.println("425 Use PASV first");
                }
                else
                {
                    // Accept data connection
                    data = std::move(dataServer.available());
                    if (!data)
                    {
                        client.println("425 Cannot open data connection");
                    }
                    else
                    {
                        client.println("150 Opening data connection");

                        std::string sdPath = sd_get_base_path();
                        std::string searchPath = sdPath + std::string(cwdName);
                        if (searchPath.back() != '/') searchPath += "/";
                        searchPath += "*";

                        WIN32_FIND_DATAA fd;
                        HANDLE h = FindFirstFileA(searchPath.c_str(), &fd);
                        if (h != INVALID_HANDLE_VALUE)
                        {
                            do
                            {
                                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
                                    continue;

                                SYSTEMTIME st;
                                FileTimeToSystemTime(&fd.ftLastWriteTime, &st);

                                const char *perms = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "drwxr-xr-x" : "-rw-r--r--";
                                ULONGLONG fileSize = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

                                const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                                char line[512];
                                snprintf(line, sizeof(line), "%s 1 owner group %12llu %s %2d %02d:%02d %s\r\n",
                                         perms, fileSize,
                                         months[st.wMonth - 1], st.wDay,
                                         st.wHour, st.wMinute,
                                         fd.cFileName);
                                data.print(line);
                            } while (FindNextFileA(h, &fd));
                            FindClose(h);
                        }

                        data.stop();
                        dataPassiveConn = false;
                        client.println("226 Transfer complete");
                    }
                }
            }
            else if (strcmp(cmd, "RETR") == 0)
            {
                if (!dataPassiveConn || !arg)
                {
                    client.println(dataPassiveConn ? "550 No file name" : "425 Use PASV first");
                }
                else
                {
                    data = std::move(dataServer.available());
                    if (!data)
                    {
                        client.println("425 Cannot open data connection");
                    }
                    else
                    {
                        std::string sdPath = sd_get_base_path() + std::string(cwdName);
                        if (sdPath.back() != '/') sdPath += "/";
                        sdPath += arg;

                        FILE *f = fopen(sdPath.c_str(), "rb");
                        if (!f)
                        {
                            client.println("550 File not found");
                        }
                        else
                        {
                            client.println("150 Opening data connection");

                            char buf[4096];
                            int n;
                            while ((n = (int)fread(buf, 1, sizeof(buf), f)) > 0)
                                data.write((uint8_t *)buf, n);
                            fclose(f);

                            data.stop();
                            dataPassiveConn = false;
                            client.println("226 Transfer complete");
                        }
                    }
                }
            }
            else if (strcmp(cmd, "STOR") == 0)
            {
                if (!dataPassiveConn || !arg)
                {
                    client.println(dataPassiveConn ? "550 No file name" : "425 Use PASV first");
                }
                else
                {
                    data = std::move(dataServer.available());
                    if (!data)
                    {
                        client.println("425 Cannot open data connection");
                    }
                    else
                    {
                        std::string sdPath = sd_get_base_path() + std::string(cwdName);
                        if (sdPath.back() != '/') sdPath += "/";
                        sdPath += arg;

                        FILE *f = fopen(sdPath.c_str(), "wb");
                        if (!f)
                        {
                            client.println("550 Cannot create file");
                        }
                        else
                        {
                            client.println("150 Opening data connection");

                            char buf[4096];
                            int timeout = 0;
                            while (timeout < 500)
                            {
                                int n = data.read((uint8_t *)buf, sizeof(buf));
                                if (n > 0)
                                {
                                    fwrite(buf, 1, n, f);
                                    timeout = 0;
                                }
                                else if (n == 0)
                                    break;
                                else
                                {
                                    Sleep(10);
                                    timeout++;
                                }
                            }
                            fclose(f);

                            data.stop();
                            dataPassiveConn = false;
                            client.println("226 Transfer complete");
                        }
                    }
                }
            }
            else if (strcmp(cmd, "RNFR") == 0)
            {
                if (!arg)
                {
                    client.println("550 No file name");
                }
                else
                {
                    rnfrCmd = true;
                    // Store the RNFR path in a temporary buffer
                    // We use buf for this since it's available
                    strncpy(buf, arg, FTP_BUF_SIZE - 1);
                    client.println("350 Ready for RNTO");
                }
            }
            else if (strcmp(cmd, "RNTO") == 0)
            {
                if (!rnfrCmd || !arg)
                {
                    client.println("503 RNFR required first");
                }
                else
                {
                    std::string sdFrom = sd_get_base_path() + std::string(cwdName);
                    if (sdFrom.back() != '/') sdFrom += "/";
                    sdFrom += buf;

                    std::string sdTo = sd_get_base_path() + std::string(cwdName);
                    if (sdTo.back() != '/') sdTo += "/";
                    sdTo += arg;

                    if (MoveFileA(sdFrom.c_str(), sdTo.c_str()))
                        client.println("250 File renamed");
                    else
                        client.println("550 Rename failed");
                    rnfrCmd = false;
                }
            }
            else if (strcmp(cmd, "QUIT") == 0)
            {
                client.println("221 Goodbye");
                client.stop();
                printf("[FtpServer] Client disconnected\n");
                return;
            }
            else if (strcmp(cmd, "NOOP") == 0)
            {
                client.println("200 NOOP ok");
            }
            else
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "502 '%s': command not understood", cmd);
                client.println(msg);
            }

            iCL = 0;
            cmdLine[0] = '\0';
        }
        else if (iCL < FTP_CMD_SIZE - 1)
        {
            cmdLine[iCL++] = c;
        }
    }
}