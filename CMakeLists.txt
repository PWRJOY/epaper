# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# set(EXTRA_COMPONENT_DIRS ./components)
# set(EXTRA_COMPONENT_DIRS ./components/epaper_driver)



include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 清除原有-Werror选项并添加新选项
#idf_build_set_property(COMPILE_OPTIONS "-Wno-error" REPLACE)
#idf_build_set_property(COMPILE_OPTIONS "-Wall" APPEND)

project(epaper)


