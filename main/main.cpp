// https://embeddedtutorials.com/eps32/esp-idf-cpp-with-cmake-for-esp32/

#include <stdio.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "esphelpers.h"
#include "tileset.h"
#include "display.h"
#include "joystick.h"
#include "map.h"
#include "buffer.h"
#include "level.h"
#include "tilesdata.h"
#include "game.h"

// static const char *TAG = "main";

CGame game;

void drawScreenTask(void *pvParameter)
{
    while (1)
    {
        switch (game.mode())
        {
        case CGame::MODE_INTRO:
            game.drawLevelIntro();
            vTaskDelay(2000 / portTICK_RATE_MS);
            game.setMode(CGame::MODE_LEVEL);
            break;
        case CGame::MODE_LEVEL:
            game.drawScreen();
        }

        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

extern "C" void app_main(void)
{
    uint32_t before_free = esp_get_free_heap_size();
    printf("free bytes: %d\n", before_free);

    game.init();
    game.loadLevel();

    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned = xTaskCreate(&drawScreenTask, "drawScreen", 2048, NULL, 5, &xHandle);
    printf("%d\n", xReturned);

    int ticks = 0;

    while (1)
    {
        vTaskDelay(40 / portTICK_RATE_MS);

        if (game.mode() != CGame::MODE_LEVEL)
        {
            continue;
        }

        if (ticks % 3 == 0)
        {
            game.managePlayer();
        }

        if (ticks % 3 == 0)
        {
            game.animate();
        }

        if (ticks % 4 == 0)
        {
            game.manageMonsters();
        }
        ++ticks;

        if (game.goalCount() == 0)
        {
            game.nextLevel();
        }
    }

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}