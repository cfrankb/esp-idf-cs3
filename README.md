# About

This project is a game port to ESP-32 microcontroller of an MS-DOS Game,

Current status is alpha. 90% of the core mechanic has been implemented.

## Original MS-DOS

![Image](images/Screenshot_2022-10-19_21-01-20.png 'icon')

## ESP-32

![Image](images/20221103_155303-50.jpg 'icon')

# Software required

esp-idf v4.4 or later.

# Installation

This project is based in part on: https://github.com/nopnop2002/esp-idf-st7789

```Shell
git clone https://github.com/cfrankb/esp-idf-cs3.git
cd esp-idf-cs3/
idf.py set-target {esp32/esp32s2/esp32s3/esp32c3}
idf.py menuconfig
idf.py flash
```

# Tested with this hardware

| Driver  | Resolution | Max SPI Freq | Display Inv |
| ------- | ---------- | ------------ | ----------- |
| ST7789  | 240x320    | 20M          | True        |
| ILI9341 | 240x320    | 40M          | False       |

## ST7789

https://www.aliexpress.com/item/1005004343881408.html

| ST7789   | wirecolor | GPIO Pin   |
| -------- | --------- | ---------- |
| SDK/MISO | purple    |            |
| LED      | grey      | 32 (BL???) |
| }SCK     | yellow    | 18         |
| SDI/MOSI | orange    | 23         |
| DC       | green     | 27         |
| RESET    | brown     | 33         |
| CS       | white     | 14 or 25   |
| GRN      | black     |            |
| VCC      | red       | 3.3v or 5v |

## Joystick

https://www.aliexpress.com/item/32345829014.html

| JOYSTICK | wirecolor | GPIO Pin |
| -------- | --------- | -------- |
| VRX      | BROWN     | 35       |
| VRY      | WHITE     | 34       |
| GRN      | black     |          |
| VCC      | red       | 3.3v     |

https://github.com/DiegoPaezA/ESP32-freeRTOS
