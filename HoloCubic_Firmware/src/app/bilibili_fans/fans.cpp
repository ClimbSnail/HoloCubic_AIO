#include "fans.h"

Fans::Fans()
{
}

void Fans::init()
{
    http = new HTTPClient();
    http->setTimeout(timeout);
}

void Fans::send()
{
    http->begin(this->url);
    this->httpCode = http->GET();
    this->httpResponse = http->getString();
    http->end();
}