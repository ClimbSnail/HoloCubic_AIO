#ifndef SIM_DATA_PATH_H
#define SIM_DATA_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

const char* sim_data_get_sd_path(void);
const char* sim_data_get_spiffs_path(void);

#ifdef __cplusplus
}
#endif

#endif