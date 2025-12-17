set(APP_NAME "edgedevice")

list(APPEND EDGEDEVICE_UI
        "${MULTIEDGE_EDGEDEVICE}/ui/edgedevice.ui"
)

list(APPEND EDGEDEVICE_SRC
        "${MULTIEDGE_EDGEDEVICE}/edgedevice.cpp"
        "${MULTIEDGE_EDGEDEVICE}/main.cpp"
)

list(APPEND EDGEDEVICE_HDR
        "${MULTIEDGE_EDGEDEVICE}/edgedevice.hpp"
)

list(APPEND EDGEDEVICE_FILES "${MULTIEDGE_TRANS}" "${MULTIEDGE_RES}" "${EDGEDEVICE_UI}" "${EDGEDEVICE_HDR}" "${EDGEDEVICE_SRC}")
qt_add_executable(${APP_NAME} MANUAL_FINALIZATION "${EDGEDEVICE_FILES}")
target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets areg::areg)
# copying log and router init files to 'bin/config'
add_custom_command( TARGET ${APP_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${AREG_RESUORCES}/areg.init" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config/areg.init"
                    VERBATIM
        )



# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER tech.areg.${APP_NAME})
endif()

set_target_properties(  ${APP_NAME} PROPERTIES
                        ${BUNDLE_ID_OPTION}
                        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
                        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
                        MACOSX_BUNDLE TRUE
                        WIN32_EXECUTABLE TRUE
        )

if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(${APP_NAME})
endif()

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

