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
SET(BOX2D_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/box2d)
set(CMAKE_CXX_STANDARD 20)
ADD_SUBDIRECTORY(SDL2pp)
ADD_SUBDIRECTORY(${BOX2D_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/box2d)

# add compilation flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3 -g")
ADD_DEFINITIONS(-DDATA_PATH="${PROJECT_SOURCE_DIR}/data")
INCLUDE_DIRECTORIES(${SDL2PP_INCLUDE_DIRS} ${BOX2D_SOURCE_DIR}/include/box2d)


ADD_EXECUTABLE(test test.cpp)
find_package(box2d REQUIRED)
TARGET_LINK_LIBRARIES(test ${SDL2PP_LIBRARIES} box2d::box2d)
