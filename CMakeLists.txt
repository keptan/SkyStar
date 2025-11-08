# SDL2pp tutorial
#
# Written in 2015 by Dmitry Marakasiv <amdmi3@amdmi3.ru>
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along with
# this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(SkyStar)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ADD_DEFINITIONS(
        -DIMGUI_IMPL_SDL_DISABLE_X11_FUNCTIONS
        -DSDL_VIDEO_DRIVER_X11=0
        -DSDL_VIDEO_DRIVER_WAYLAND=1
)


# set up SDL2pp library
SET(SDL2PP_WITH_IMAGE YES)
SET(SDL2PP_WITH_TTF YES)
SET(SDL2PP_WITH_MIXER YES)
ADD_SUBDIRECTORY(SDL2pp)

# add compilation flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g")
ADD_DEFINITIONS(-DDATA_PATH="${PROJECT_SOURCE_DIR}/data")

# Add SDL2pp headers globally
INCLUDE_DIRECTORIES(${SDL2PP_INCLUDE_DIRS})

# imgui setup
set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_BACKENDS_PATH ${IMGUI_PATH}/backends)

set(IMGUI_SOURCES
        "${IMGUI_PATH}/imgui.cpp"
        "${IMGUI_PATH}/imgui_demo.cpp"
        "${IMGUI_PATH}/imgui_draw.cpp"
        "${IMGUI_PATH}/imgui_tables.cpp"
        "${IMGUI_PATH}/imgui_widgets.cpp"
)

# Explicitly list backend sources. You need both the platform (SDL2)
# and the renderer (SDL_Renderer) backends.
set(IMGUI_BACKEND_SOURCES
        "${IMGUI_BACKENDS_PATH}/imgui_impl_sdl2.cpp"
        "${IMGUI_BACKENDS_PATH}/imgui_impl_sdlrenderer2.cpp"
)

# Create a static library for imgui
add_library(imgui STATIC
        ${IMGUI_SOURCES}
        ${IMGUI_BACKEND_SOURCES}
)

# Tell the imgui target where to find its own headers.
# By using PUBLIC, any target that links to 'imgui' will
# also automatically get these include directories.
target_include_directories(imgui PUBLIC
        ${IMGUI_PATH}
        ${IMGUI_BACKENDS_PATH}
)

# Link imgui to SDL2pp, since the backends depend on it.
# By using PUBLIC, any target that links to 'imgui' will
# also automatically link to SDL2pp.
target_link_libraries(imgui PUBLIC ${SDL2PP_LIBRARIES})

ADD_EXECUTABLE(test test.cpp
        debugGraphics.h)
TARGET_LINK_LIBRARIES(test PUBLIC imgui)
# TARGET_LINK_LIBRARIES(tree ${SDL2PP_LIBRARIES})
#add_executable(macroEcon MacroEcon/macroEcon.cpp)
