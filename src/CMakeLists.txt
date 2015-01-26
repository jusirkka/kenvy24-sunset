include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

########### next target ###############

SET(kenvy24gui_SRCS
    kenvy24app.cpp
    kenvy24gui.cpp
    kenvy24dockwidget.cpp
    envycard.cpp
    tabviewimpl.cpp
    mastervolumeimpl.cpp
    mixerinputimpl.cpp
    peakmeterimpl.cpp
    patchboximpl.cpp
    dcopiface.cpp
)


set(kenvy24gui_UI
    tabview.ui
    mastervolume.ui
    mixerinput.ui
    peakmeter.ui
    patchbox.ui
)

kde3_add_dcop_stubs(kenvy24gui_SRCS dcopiface.h)
kde3_add_dcop_skels(kenvy24gui_SRCS dcopiface.h)

kde3_automoc(${kenvy24gui_SRCS}})

kde3_add_ui_files(kenvy24gui_SRCS ${kenvy24gui_UI})


kde3_add_executable(kenvy24 main.cpp ${kenvy24gui_SRCS})

target_link_libraries(kenvy24 ${QT_AND_KDECORE_LIBS} ${ALSA_LIBRARY} kdeui kio DCOP)

install(TARGETS kenvy24 DESTINATION bin)


########### install files ###############

install(FILES  kenvy24ui.rc DESTINATION share/apps/kenvy24)
install(FILES  kenvy24gui.png DESTINATION share/apps/kenvy24/pics)
install(FILES  kenvy24.desktop DESTINATION share/applications/kde)

kde3_install_icons(hicolor)

