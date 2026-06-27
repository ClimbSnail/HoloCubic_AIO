#ifndef DNSSERVER_STUB_H
#define DNSSERVER_STUB_H

#include <stdint.h>

class DNSServer
{
public:
    DNSServer() {}
    void start(uint16_t port, const char *domainName, const char *resolvedIP) { (void)port; (void)domainName; (void)resolvedIP; }
    void processNextRequest() {}
    void stop() {}
};

#endif