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

PROJECT(SkyStar)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# set up SDL2pp library
SET(SDL2PP_WITH_IMAGE YES)
SET(SDL2PP_WITH_TTF YES)
SET(SDL2PP_WITH_MIXER YES)

set(CMAKE_CXX_STANDARD 20)
ADD_SUBDIRECTORY(SDL2pp)

# add compilation flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Og -g")
ADD_DEFINITIONS(-DDATA_PATH="${PROJECT_SOURCE_DIR}/data")
INCLUDE_DIRECTORIES(${SDL2PP_INCLUDE_DIRS})


ADD_EXECUTABLE(test test.cpp)
# ADD_EXECUTABLE(tree rtree.cpp)
TARGET_LINK_LIBRARIES(test ${SDL2PP_LIBRARIES})
# TARGET_LINK_LIBRARIES(tree ${SDL2PP_LIBRARIES})
