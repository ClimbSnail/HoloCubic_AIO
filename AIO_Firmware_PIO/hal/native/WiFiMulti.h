#ifndef WIFI_MULTI_STUB_H
#define WIFI_MULTI_STUB_H

class WiFiMulti
{
public:
    void addAP(const char *ssid, const char *passphrase) { (void)ssid; (void)passphrase; }
    uint8_t run(void) { return 3; }
};

#endif /* WIFI_MULTI_STUB_H */