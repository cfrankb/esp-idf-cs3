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
const int cols = CONFIG_WIDTH / 16;
const int rows = CONFIG_HEIGHT / 16;

CGame game;

void randomMap(CMap &map, CTileSet &tiles)
{
    for (int y = 0; y < map.hei(); ++y)
    {
        for (int x = 0; x < map.len(); ++x)
        {
            uint8_t i = esp_random() % tiles.size();
            map.set(x, y, i);
        }
    }
}

void testTiles(CDisplay &display, CTileSet &tiles)
{
    CMap map(cols, rows);
    randomMap(map, tiles);
    int64_t t0 = esp_timer_get_time();
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            int i = map.at(x, y);
            display.drawTile(x * 16, y * 16, tiles[i], 1);
        }
    }

    int64_t t1 = esp_timer_get_time();

    printf("time: %ld\n", (long)(t1 - t0) / 1000);
}

void testTiles(CDisplay &display, CTileSet &tiles, CMap &map, CBuffer &buffer, int mx, int my)
{
    int64_t t0 = esp_timer_get_time();

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            int i = map.at(x + mx, y + my);
            buffer.drawTile(x * 16, 0, tiles[i]);
        }

        display.drawBuffer(0, y * 16, buffer, 1);
    }

    int64_t t1 = esp_timer_get_time();

    printf("time: %ld\n", (long)(t1 - t0) / 1000);
}

void drawScreenTask(void *pvParameter)
{
    while (1)
    {
        game.drawScreen();
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

        if (ticks % 2 == 0)
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