set(REQ driver)

idf_component_register(SRCS "ssd1680_epaper.c" "qy_ssd1680_epaper.c"
                    INCLUDE_DIRS "." "../epaper_driver"
                    REQUIRES ${REQ})