set(journalktl_SOURCES main.cpp mainwindow.cpp journalmodel.cpp sortfiltermodel.cpp)
set(journalktl_HEADERS mainwindow.h journalmodel.h sortfiltermodel.h)
set(journalktl_FORMS ../ui/mainwindow.ui)

ki18n_wrap_ui(journalktl_SOURCES
              ../ui/mainwindow.ui)

add_executable(journalktl ${journalktl_SOURCES})

target_link_libraries(journalktl -lsystemd
                                 KF5::CoreAddons
                                 KF5::I18n
                                 KF5::XmlGui)

install(TARGETS journalktl DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES journalktlui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/journalktl )

qt5_use_modules(journalktl Widgets Concurrent)