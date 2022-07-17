cmake_minimum_required(VERSION 3.13)

project("SwitchHBTest" VERSION 1.0.0)

add_executable("SwitchHBTest" "source/main.cpp" "source/ttt/board.cpp" "source/ttt/solver.cpp" "source/gl/tile_renderer.cpp" "source/gl/gl_texture.cpp")
target_include_directories("SwitchHBTest" PRIVATE $ENV{DEVKITPRO}/portlibs/switch/include)
target_compile_options("SwitchHBTest" PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>" "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>")
target_link_libraries("SwitchHBTest" png z glad EGL glapi drm_nouveau nx m)

enable_language("ASM")
dkp_add_embedded_binary_library("SwitchHBTest_assets" ${CMAKE_CURRENT_LIST_DIR}/raw/tile.vs ${CMAKE_CURRENT_LIST_DIR}/raw/tile.fs 
    ${CMAKE_CURRENT_LIST_DIR}/raw/Base.png ${CMAKE_CURRENT_LIST_DIR}/raw/Circle.png ${CMAKE_CURRENT_LIST_DIR}/raw/Cross.png)
dkp_target_use_embedded_binary_libraries("SwitchHBTest" "SwitchHBTest_assets")
nx_create_nro("SwitchHBTest")