cmake_minimum_required(VERSION 3.2)
project(dumpthumbnailer)


find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)

set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${KDE4_INCLUDES})



#sources
set(dumpcreator_SRCS dumpcreator.cpp)
kde4_add_plugin(dumpthumbnailer ${dumpcreator_SRCS})


## includes and linking
target_link_libraries(dumpthumbnailer ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS})

## install the plugin
install(TARGETS dumpthumbnailer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES dumpthumbnailer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES dump.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

