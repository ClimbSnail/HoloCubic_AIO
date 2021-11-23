#include "../../network.h"

class Fans
{
public:
    Fans();
    void init();
    void send();

    int httpCode;
    String httpResponse;

private:
    int fansNum = 0;
    int followNum = 0;
    const int timeout = 1000;
    String url = "http://www.dtmb.top/api/fans/index";
    HTTPClient *http;
};