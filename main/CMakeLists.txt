#Add sources from ui directory
file(GLOB_RECURSE SRC_UI ${CMAKE_SOURCE_DIR} "ui/*.c" "ui/*.cpp")
file(GLOB_RECURSE SRC_HW ${CMAKE_SOURCE_DIR} "hardware_setup/*.cpp")
file(GLOB_RECURSE SRC_LCD ${CMAKE_SOURCE_DIR} "lcd_lvgl/*.c")


idf_component_register(SRCS "main.cpp" ${SRC_UI} ${SRC_HW} ${SRC_LCD}
                    INCLUDE_DIRS "." "ui" "hardware_setup" "lcd_lvgl")
