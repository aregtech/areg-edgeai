set(APP_NAME "edgedevice")

list(APPEND EDGEDEVICE_UI
    "${MULTIEDGE_EDGEDEVICE}/ui/edgedevice.ui"
)

list(APPEND EDGEDEVICE_SRC
    "${MULTIEDGE_EDGEDEVICE}/agentchathistory.cpp"
    "${MULTIEDGE_EDGEDEVICE}/agentconsumer.cpp"
    "${MULTIEDGE_EDGEDEVICE}/edgedevice.cpp"
    "${MULTIEDGE_EDGEDEVICE}/main.cpp"
)

list(APPEND EDGEDEVICE_HDR
    "${MULTIEDGE_EDGEDEVICE}/agentchathistory.hpp"
    "${MULTIEDGE_EDGEDEVICE}/agentconsumer.hpp"
    "${MULTIEDGE_EDGEDEVICE}/edgedevice.hpp"
)

list(APPEND EDGEDEVICE_FILES "${MULTIEDGE_TRANS}" "${MULTIEDGE_RES}" "${EDGEDEVICE_UI}" "${EDGEDEVICE_HDR}" "${EDGEDEVICE_SRC}")
qt_add_executable(${APP_NAME} MANUAL_FINALIZATION "${EDGEDEVICE_FILES}")
target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets areg::areg gen_multiedge)


# add to the list to finalize later.
list(APPEND AREG_EDGEAI_EXES "${APP_NAME}")
