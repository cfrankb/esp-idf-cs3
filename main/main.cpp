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

CGame game;

void drawScreenTask(void *pvParameter)
{
    while (1)
    {
        //CEngine *engine = game.getEngine();

        switch (game.mode())
        {
        case CGame::MODE_INTRO:
        case CGame::MODE_RESTART:
        case CGame::MODE_GAMEOVER:
            game.drawLevelIntro();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            if (game.mode()== CGame::MODE_GAMEOVER) {
                game.restartGane();
            } else {
                game.setMode(CGame::MODE_LEVEL);
            }
            break;
        case CGame::MODE_LEVEL:
            game.drawScreen();
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    uint32_t before_free = esp_get_free_heap_size();
    printf("free bytes: %ld\n", before_free);

    game.init();
    game.loadLevel(false);

    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned = xTaskCreate(&drawScreenTask, "drawScreen", 2048, NULL, 5, &xHandle);
    printf("%d\n", xReturned);

    int ticks = 0;

    while (1)
    {
        vTaskDelay(40 / portTICK_PERIOD_MS);

        if (game.mode() != CGame::MODE_LEVEL)
        {
            continue;
        }

        if (ticks % 3 == 0 && !game.isPlayerDead())
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

        if (game.isPlayerDead()){
            game.killPlayer();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            if(!game.isGameOver()) {
                game.restartLevel();
            } else {
                game.setMode(CGame::MODE_GAMEOVER);
            }
        }

        ++ticks;

        uint16_t joy = readJoystick();
        if (!game.isGameOver()) {
            if (game.goalCount() == 0 || joy & JOY_A_BUTTON)
            {
                game.nextLevel();
            }
        }
    }

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}