idf_component_register(
    SRCS            "hello_world_main.c"
                    "quote_fetcher/quote_fetcher.c"
                    "wifi/wifi.c"
                    "board_init/board_init.c"
    INCLUDE_DIRS    "."
                    "quote_fetcher"  
                    "wifi"            
                    "board_init"  
    REQUIRES driver nvs_flash esp_wifi esp_http_client json esp_http_server)

