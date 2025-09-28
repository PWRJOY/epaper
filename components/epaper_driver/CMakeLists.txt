set(REQ driver)

idf_component_register(SRCS "epaper_font.c" "epaper.c" "epaper_gui.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${REQ})