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

#define JOYSTICK_SW GPIO_NUM_26

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

    ret = gpio_set_direction(JOYSTICK_SW, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }
    ret = gpio_set_pull_mode(JOYSTICK_SW, GPIO_PULLUP_ONLY);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_set_pull_mode Failed (%s)", esp_err_to_name(ret));
    }
    return true;
}

bool readJoystick(int &joy, int &aim)
{
    uint32_t adc_vrx = 0;
    uint32_t adc_vry = 0;
    int button = gpio_get_level(JOYSTICK_SW);

    adc_vry = adc1_get_raw((adc1_channel_t)channel1);
    adc_vrx = adc1_get_raw((adc1_channel_t)channel2);
    if (adc_vry == -1)
        return false;
    if (adc_vrx == -1)
        return false;

    aim = AIM_NONE;
    joy = JOY_NONE;

    if (adc_vry < 1800)
    {
        aim = AIM_UP;
        joy |= JOY_UP;
    }
    else if (adc_vry > 3000)
    {
        aim = AIM_DOWN;
        joy |= JOY_DOWN;
    }

    if (adc_vrx < 1800)
    {
        aim = AIM_LEFT;
        joy |= JOY_LEFT;
    }
    else if (adc_vrx > 3000)
    {
        aim = AIM_RIGHT;
        joy |= JOY_RIGHT;
    }

    if (!button)
    {
        joy |= JOY_BUTTON;
    }

    /*if (joy)
    {
        printf("Knob at X:[%d] Y:[%d] [%d] [%d]\n", adc_vrx, adc_vry, button, joy);
    }*/

    return true;
}