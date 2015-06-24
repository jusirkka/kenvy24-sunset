include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

########### next target ###############

SET(kenvy24gui_SRCS
    mainwindow.cpp
    kenvy24.cpp
    dockwidget.cpp
    envycard.cpp
    mastervolume.cpp
    mixerinput.cpp
    peakmeter.cpp
    patchbox.cpp
    dbusiface.cpp
)

set(kenvy24gui_SRCS
    ${kenvy24gui_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/org.kvanttiapina.kenvy24.xml
)

qt4_generate_dbus_interface(
    dbusiface.h
    org.kvanttiapina.kenvy24.xml
    OPTIONS -M
)

qt4_add_dbus_adaptor(kenvy24gui_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kvanttiapina.kenvy24.xml
    dbusiface.h
    DBusIface
)

set(kenvy24gui_UI
    mainwindow.ui
    mastervolume.ui
    mixerinput.ui
    peakmeter.ui
    patchbox.ui
)

set(CMAKE_AUTOMOC TRUE)


#set_target_properties(${kenvy24gui_SRCS} PROPERTIES AUTOMOC TRUE)

kde4_add_ui_files(kenvy24gui_SRCS ${kenvy24gui_UI})


kde4_add_executable(kenvy24 main.cpp ${kenvy24gui_SRCS})

target_link_libraries(kenvy24 ${KDE4_KDEUI_LIBS} ${ALSA_LIBRARY})

install(TARGETS kenvy24 DESTINATION bin)


########### install files ###############

install(FILES kenvy24ui.rc DESTINATION share/apps/kenvy24)
install(FILES kenvy24gui.png DESTINATION share/apps/kenvy24/pics)
install(FILES kenvy24.desktop DESTINATION share/applications/kde)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kvanttiapina.kenvy24.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

kde4_install_icons(hicolor)

