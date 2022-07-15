cmake_minimum_required(VERSION 3.13)

project("SwitchHBTest" VERSION 1.0.0)

add_executable("SwitchHBTest" "source/main.cpp")
target_include_directories("SwitchHBTest" PRIVATE $ENV{DEVKITPRO}/portlibs/switch/include)
target_link_libraries("SwitchHBTest" glad EGL glapi drm_nouveau nx)

nx_create_nro("SwitchHBTest")