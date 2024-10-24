SET(SOURCES screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_1_png.c
    images/ui_img_2_png.c
    images/ui_img_190695247.c
    images/ui_img_997471977.c
    images/ui_img_1475871244.c
    images/ui_img_1644947644.c
    images/ui_img_1214273956.c
    images/ui_img_wallpaper_1_png.c)

add_library(ui ${SOURCES})
