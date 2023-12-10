#include "joystick.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_idf_version.h"
#include "soc/clk_tree_defs.h"
#include <cstring>

#define DEBUG_JOYSTICK
//#define ENABLED_BUTTONS
#ifdef ENABLED_BUTTONS
#define JOYSTICK_SW GPIO_NUM_26
#define JOYSTICK_A_BUTTON GPIO_NUM_4
#endif

#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_7       // pin 35 (x) BROWN
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_6       // pin 34 (y) WHITE

/*#if CONFIG_IDF_TARGET_ESP32
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_4
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_5
#else
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_2
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_3
#endif
*/

static adc_oneshot_unit_handle_t adc1_handle;

static const char *TAG = "joystick";
bool initJoystick()
{
ESP_LOGI(TAG, "initJoystick(): started");

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    // enable functionality present in IDF v5.3
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
#else
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
#endif

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

#ifdef DEBUG_JOYSTICK
    const adc_channel_t channels[] = {
        ADC_CHANNEL_0,
        ADC_CHANNEL_1,
        ADC_CHANNEL_2,
        ADC_CHANNEL_3,
        ADC_CHANNEL_4,
        ADC_CHANNEL_5,
        ADC_CHANNEL_6,
        ADC_CHANNEL_7,
    };

    for (int i = 0; i < 8; ++i) {
        const adc_channel_t & chan = channels[i];
        int io_num;
        ESP_ERROR_CHECK(adc_continuous_channel_to_io(ADC_UNIT_1, chan, &io_num));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Mapped to Pin: %d", ADC_UNIT_1 + 1, chan, io_num);
    }
#endif

#ifdef ENABLED_BUTTONS
    esp_err_t ret;

    ret = gpio_set_direction(JOYSTICK_SW, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    ret = gpio_set_direction(JOYSTICK_A_BUTTON, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    ret = gpio_set_pull_mode(JOYSTICK_SW, GPIO_PULLUP_ONLY);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_set_pull_mode Failed (%s)", esp_err_to_name(ret));
    }
#endif
    ESP_LOGI(TAG, "initJoystick(): ended");
    return true;
}

uint16_t readJoystick()
{
    int adc_vrx = 0;
    int adc_vry = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_vrx));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_vry));

    if (adc_vry == -1)
        return false;
    if (adc_vrx == -1)
        return false;

    uint16_t joy = JOY_NONE;

    if (adc_vry < 50)
    {
        joy |= JOY_UP;
    }
    else if (adc_vry > 3000)
    {
        joy |= JOY_DOWN;
    }

    if (adc_vrx < 50)
    {
        joy |= JOY_LEFT;
    }
    else if (adc_vrx > 3000)
    {
        joy |= JOY_RIGHT;
    }

#ifdef ENABLED_BUTTONS
    int button = gpio_get_level(JOYSTICK_SW);
    int a_button = gpio_get_level(JOYSTICK_A_BUTTON);
    if (!button)
    {
        joy |= JOY_BUTTON;
    }

    if (a_button)
    {
        joy |= JOY_A_BUTTON;
    }
#endif

#ifdef DEBUG_JOYSTICK
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, adc_vrx);
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN1, adc_vry);
    if (joy)
    {
        printf("Knob at X:[%d] Y:[%d] [%d]\n", adc_vrx, adc_vry, joy);
    }
#endif

    return joy;
}