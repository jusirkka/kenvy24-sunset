cmake_minimum_required(VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INSTALL_PREFIX /usr/kde/3.5)

project(KEnvy24)

find_package(KDE3 REQUIRED)
find_package(ALSA REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

include_directories( 
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDES}  
    ${ALSA_INCLUDE_DIR}
)

link_directories(
    ${KDE3_LIB_DIR}
    ${ALSA_LIB_DIR}
)


add_subdirectory(src)

