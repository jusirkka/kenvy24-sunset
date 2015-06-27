cmake_minimum_required(VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INSTALL_PREFIX /usr)

project(KEnvy24Sunset)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(ALSA REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -D_DEFAULT_SOURCE)

include_directories( 
    ${KDE4_INCLUDES}
    ${ALSA_INCLUDE_DIR}
)

link_directories(
    ${KDE4_LIB_DIR}
    ${ALSA_LIB_DIR}
)


add_subdirectory(src)

