set(SOURCES 
    main.cpp
    esphelpers.cpp
    tileset.cpp
    joystick.cpp
    display.cpp
    map.cpp
    buffer.cpp
    level.cpp
    tilesdata.cpp
    game.cpp
    actor.cpp
    font.cpp
    colors.cpp
    tft_driver.cpp
    engine.cpp
    maparch.cpp
    animator.cpp
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    INCLUDE_DIRS "./include"
)

#set(CMAKE_CXX_FLAGS "-O3 -mlongcalls -Wno-frame-address  -g -ffunction-sections -fdata-sections -Wall -Werror=all -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wextra -Wno-unused-parameter -Wno-sign-compare -fstrict-volatile-bitfields -Wno-error=unused-but-set-variable -fno-jump-tables -fno-tree-switch-conversion -std=gnu99 -Wno-old-style-declaration -D_GNU_SOURCE")

# Create a SPIFFS image from the contents of the 'font' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash
#spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)

# Create a SPIFFS image from the contents of the 'font' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash
#spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)