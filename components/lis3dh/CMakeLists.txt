set(REQ driver)

idf_component_register(SRCS "lis3dh.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${REQ})