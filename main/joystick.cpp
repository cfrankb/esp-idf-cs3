#include "joystick.h"
#include <freertos/FreeRTOS.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <driver/adc.h>
#include <cstring>
/*

VRX BROWN   35
VRY WHITE   34

*/

static const adc1_channel_t channel1 = ADC1_CHANNEL_6; // GPIO34 if ADC1, GPIO14 if ADC2
static const adc1_channel_t channel2 = ADC1_CHANNEL_7; // GPIO35 if ADC1 ???
static const char *TAG = "joystick";
bool initJoystick()
{
    esp_err_t ret;
    ret = adc1_config_width(ADC_WIDTH_BIT_12);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "adc1_config_width failed (%s)", esp_err_to_name(ret));
        return false;
    }
    ret = adc1_config_channel_atten(channel1, ADC_ATTEN_DB_11);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "adc1_config_channel_atten Failed (%s)", esp_err_to_name(ret));
        return false;
    }
    ret = adc1_config_channel_atten(channel2, ADC_ATTEN_DB_11);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "adc1_config_channel_atten Failed (%s)", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool readJoystick(int &joy, int &aim)
{
    uint32_t adc_vrx = 0;
    uint32_t adc_vry = 0;
    // char tmp[32];
    adc_vry = adc1_get_raw((adc1_channel_t)channel1);
    adc_vrx = adc1_get_raw((adc1_channel_t)channel2);
    if (adc_vry == -1)
        return false;
    if (adc_vrx == -1)
        return false;

    // tmp[0] = 0;
    aim = AIM_NONE;
    joy = JOY_NONE;

    switch (adc_vrx)
    {
    case 0:
        //    strcat(tmp, "LEFT ");
        aim = AIM_LEFT;
        joy |= JOY_LEFT;
        break;
    case 4095:
        //    strcat(tmp, "RIGHT ");
        aim = AIM_RIGHT;
        joy |= JOY_RIGHT;
    }

    switch (adc_vry)
    {
    case 0:
        // strcat(tmp, "UP ");
        aim = AIM_UP;
        joy |= JOY_UP;
        break;
    case 4095:
        // strcat(tmp, "DOWN ");
        aim = AIM_DOWN;
        joy |= JOY_DOWN;
    }

    /*if (*tmp)
    {
        printf("Knob at X:[%d] Y:[%d] [%s] [%d]\n", adc_vrx, adc_vry, tmp, joy);
    }*/
    return true;
}