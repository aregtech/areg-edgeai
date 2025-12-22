set(APP_NAME "aiagent")

list(APPEND AIAGENT_UI
    "${MULTIEDGE_AIAGENT}/ui/aiagent.ui"
)

list(APPEND AIAGENT_SRC
    "${MULTIEDGE_AIAGENT}/agentprovider.cpp"
    "${MULTIEDGE_AIAGENT}/aiagent.cpp"
    "${MULTIEDGE_AIAGENT}/main.cpp"
)

list(APPEND AIAGENT_HDR
    "${MULTIEDGE_AIAGENT}/agentprovider.hpp"
    "${MULTIEDGE_AIAGENT}/aiagent.hpp"
)



list(APPEND AIAGENT_FILES "${MULTIEDGE_TRANS}" "${MULTIEDGE_RES}" "${AIAGENT_UI}" "${AIAGENT_HDR}" "${AIAGENT_SRC}")
qt_add_executable(${APP_NAME} MANUAL_FINALIZATION "${AIAGENT_FILES}")
target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets areg::areg gen_multiedge)
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

get_target_property(_qt_bin_dir Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qt_bin_dir}" DIRECTORY)
if (WIN32)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND "${_qt_bin_dir}/windeployqt.exe"
                --no-opengl-sw
                --no-compiler-runtime
                --no-translations
                --no-system-dxc-compiler
                --no-quick-import
                --no-ffmpeg
                "$<TARGET_FILE:${APP_NAME}>"
        COMMENT "Deploying Qt runtime libraries for Win32"
    )
elseif (APPLE)
    add_custom_command(TARGET lusan POST_BUILD
        COMMAND "${_qt_bin_dir}/macdeployqt .exe"
                --no-opengl-sw
                --no-compiler-runtime
                --no-translations
                --no-system-dxc-compiler
                --no-quick-import
                --no-ffmpeg
                "$<TARGET_FILE:${APP_NAME}>"
        COMMENT "Deploying Qt runtime libraries for macOS"
    )
else()
    message(STATUS "Qt deployment skipped on Linux (handled by system packaging)")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(${APP_NAME})
endif()

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

