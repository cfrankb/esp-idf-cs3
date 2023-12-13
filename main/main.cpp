// https://embeddedtutorials.com/eps32/esp-idf-cpp-with-cmake-for-esp32/

#include <stdio.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
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
#include "engine.h"

// static const char *TAG = "main";
CEngine *engine = nullptr;

void drawScreenTask(void *pvParameter)
{
    CGame &game = engine->game();
    while (1)
    {
        switch (game.mode())
        {
        case CGame::MODE_INTRO:
        case CGame::MODE_RESTART:
        case CGame::MODE_GAMEOVER:
            engine->drawLevelIntro();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            if (game.mode() == CGame::MODE_GAMEOVER)
            {
                game.restartGame();
            }
            else
            {
                game.setMode(CGame::MODE_LEVEL);
            }
            break;
        case CGame::MODE_LEVEL:
            engine->drawScreen();
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    engine = new CEngine();
    CGame &game = engine->game();

    uint32_t before_free = esp_get_free_heap_size();
    printf("free bytes: %ld\n", before_free);

    engine->mutex().lock();
    game.loadLevel(false);
    engine->mutex().unlock();

    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned = xTaskCreate(&drawScreenTask, "drawScreen", 2048, NULL, 5, &xHandle);
    printf("%d\n", xReturned);

    int ticks = 0;

    while (1)
    {
        vTaskDelay(40 / portTICK_PERIOD_MS);

        engine->mainLoop(ticks);
        if (game.isPlayerDead())
        {
            game.killPlayer();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            if (!game.isGameOver())
            {
                engine->mutex().lock();
                game.restartLevel();
                engine->mutex().unlock();
            }
            else
            {
                game.setMode(CGame::MODE_GAMEOVER);
            }
        }

        ++ticks;
    }

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}