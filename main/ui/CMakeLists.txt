SET(SOURCES screens/ui_Screen5.c
    screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen6.c
    screens/ui_Screen7.c
    screens/ui_Screen8.c
    ui_theme_manager.c
    ui_themes.c
    ui.cpp
    ui_images.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c )

add_library(ui ${SOURCES})
