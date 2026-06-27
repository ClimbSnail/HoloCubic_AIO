#ifndef ESPMDNS_STUB_H
#define ESPMDNS_STUB_H

class MDNSResponder
{
public:
    bool begin(const char *hostname) { (void)hostname; return true; }
    void end(void) {}
};

extern MDNSResponder MDNS;

#endif /* ESPMDNS_STUB_H */