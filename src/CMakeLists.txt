include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

########### next target ###############

SET(kenvy24gui_SRCS
    mainwindow.cpp
    kenvy24.cpp
    envycard.cpp
    mastervolume.cpp
    mixerinput.cpp
    peakmeter.cpp
    patchbox.cpp
    renamedialog.cpp
    dbusiface.cpp
)

set(kenvy24gui_SRCS
    ${kenvy24gui_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/com.github.kenvy24_sunset.xml
)

qt4_generate_dbus_interface(
    dbusiface.h
    com.github.kenvy24_sunset.xml
    OPTIONS -M
)

qt4_add_dbus_adaptor(kenvy24gui_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/com.github.kenvy24_sunset.xml
    dbusiface.h
    DBusIface
)

set(kenvy24gui_UI
    mainwindow.ui
    mastervolume.ui
    mixerinput.ui
    peakmeter.ui
    patchbox.ui
    renamedialog.ui
)

set(CMAKE_AUTOMOC TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

kde4_add_ui_files(kenvy24gui_SRCS ${kenvy24gui_UI})


kde4_add_executable(kenvy24-sunset main.cpp ${kenvy24gui_SRCS})

target_link_libraries(kenvy24-sunset ${KDE4_KDEUI_LIBS} ${ALSA_LIBRARY})

install(TARGETS kenvy24-sunset DESTINATION bin)


########### install files ###############

install(FILES kenvy24-sunset.png DESTINATION share/apps/kenvy24-sunset/pics)
install(FILES kenvy24-sunset.desktop DESTINATION share/applications/kde)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.github.kenvy24_sunset.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

kde4_install_icons(hicolor)

