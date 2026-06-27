#ifndef PubSubClient_h
#define PubSubClient_h

#include <Arduino.h>
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 30
#endif
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4
#define MQTTCONNACK     2 << 4
#define MQTTPUBLISH     3 << 4
#define MQTTPUBACK      4 << 4
#define MQTTPUBREC      5 << 4
#define MQTTPUBREL      6 << 4
#define MQTTPUBCOMP     7 << 4
#define MQTTSUBSCRIBE   8 << 4
#define MQTTSUBACK      9 << 4
#define MQTTUNSUBSCRIBE 10 << 4
#define MQTTUNSUBACK    11 << 4
#define MQTTPINGREQ     12 << 4
#define MQTTPINGRESP    13 << 4
#define MQTTDISCONNECT  14 << 4
#define MQTTReserved    15 << 4

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

#define MQTT_MAX_HEADER_SIZE 5

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
#endif

#define CHECK_STRING_LENGTH(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) {_client->stop();return false;}

class PubSubClient : public Print {
private:
    Client* _client;
    uint8_t* buffer;
    uint16_t bufferSize;
    uint16_t keepAlive;
    uint16_t socketTimeout;
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;
    MQTT_CALLBACK_SIGNATURE;
    IPAddress ip;
    const char* domain;
    uint16_t port;
    Stream* stream;
    int _state;
public:
    PubSubClient() : _client(nullptr), buffer(nullptr), bufferSize(0), keepAlive(0),
        socketTimeout(0), nextMsgId(0), lastOutActivity(0), lastInActivity(0),
        pingOutstanding(false), callback(nullptr), domain(nullptr), port(0),
        stream(nullptr), _state(MQTT_DISCONNECTED) {}
    PubSubClient(Client& client) : PubSubClient() { _client = &client; }
    PubSubClient(IPAddress, uint16_t, Client& client) : PubSubClient(client) {}
    PubSubClient(IPAddress, uint16_t, Client& client, Stream&) : PubSubClient(client) {}
    PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient(client) {}
    PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&) : PubSubClient(client) {}
    PubSubClient(uint8_t*, uint16_t, Client& client) : PubSubClient(client) {}
    PubSubClient(uint8_t*, uint16_t, Client& client, Stream&) : PubSubClient(client) {}
    PubSubClient(uint8_t*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient(client) {}
    PubSubClient(uint8_t*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&) : PubSubClient(client) {}
    PubSubClient(const char*, uint16_t, Client& client) : PubSubClient(client) {}
    PubSubClient(const char*, uint16_t, Client& client, Stream&) : PubSubClient(client) {}
    PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient(client) {}
    PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&) : PubSubClient(client) {}

    ~PubSubClient() {}

    PubSubClient& setServer(IPAddress, uint16_t) { return *this; }
    PubSubClient& setServer(uint8_t*, uint16_t) { return *this; }
    PubSubClient& setServer(const char*, uint16_t) { return *this; }
    PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE) { return *this; }
    PubSubClient& setClient(Client&) { return *this; }
    PubSubClient& setStream(Stream&) { return *this; }
    PubSubClient& setKeepAlive(uint16_t) { return *this; }
    PubSubClient& setSocketTimeout(uint16_t) { return *this; }

    boolean setBufferSize(uint16_t) { return true; }
    uint16_t getBufferSize() { return 0; }

    boolean connect(const char*) { fprintf(stderr, "[PubSubClient] connect(1arg)\n"); fflush(stderr); return false; }
    boolean connect(const char*, const char*, const char*) { fprintf(stderr, "[PubSubClient] connect(3arg)\n"); fflush(stderr); return false; }
    boolean connect(const char*, const char*, uint8_t, boolean, const char*) { return false; }
    boolean connect(const char*, const char*, const char*, const char*, uint8_t, boolean, const char*) { return false; }
    boolean connect(const char*, const char*, const char*, const char*, uint8_t, boolean, const char*, boolean) { return false; }
    void disconnect() {}
    boolean publish(const char*, const char*) { return false; }
    boolean publish(const char*, const char*, boolean) { return false; }
    boolean publish(const char*, const uint8_t*, unsigned int) { return false; }
    boolean publish(const char*, const uint8_t*, unsigned int, boolean) { return false; }
    boolean publish_P(const char*, const char*, boolean) { return false; }
    boolean publish_P(const char*, const uint8_t*, unsigned int, boolean) { return false; }
    boolean beginPublish(const char*, unsigned int, boolean) { return false; }
    int endPublish() { return false; }
    virtual size_t write(uint8_t) { return 0; }
    virtual size_t write(const uint8_t*, size_t) { return 0; }
    boolean subscribe(const char*) { return false; }
    boolean subscribe(const char*, uint8_t) { return false; }
    boolean unsubscribe(const char*) { return false; }
    boolean loop() { fprintf(stderr, "[PubSubClient] loop()\n"); fflush(stderr); return false; }
    boolean connected() { fprintf(stderr, "[PubSubClient] connected()\n"); fflush(stderr); return false; }
    int state() { return _state; }
};

#endif