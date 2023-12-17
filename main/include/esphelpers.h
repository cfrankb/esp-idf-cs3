#ifndef ESP_HELPERS___
#define ESP_HELPERS___

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

void delayMS(int ms);
bool initSpiffs();

#endif