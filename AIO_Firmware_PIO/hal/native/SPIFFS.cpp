#include "SPIFFS.h"
#include "sim_data_path.h"

const char *SPIFFSClass::_basePath = "sim_data/spiffs";

void computeBasePath()
{
    SPIFFSClass::_basePath = sim_data_get_spiffs_path();
}

SPIFFSClass SPIFFS;