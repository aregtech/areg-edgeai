set(APP_NAME "aiagent")

list(APPEND AIAGENT_UI
    "${MULTIEDGE_AIAGENT}/ui/aiagent.ui"
)

list(APPEND AIAGENT_SRC
    "${MULTIEDGE_AIAGENT}/agentchathistory.cpp"
    "${MULTIEDGE_AIAGENT}/agentprocessor.cpp"
    "${MULTIEDGE_AIAGENT}/agentprovider.cpp"
    "${MULTIEDGE_AIAGENT}/aiagent.cpp"
    "${MULTIEDGE_AIAGENT}/main.cpp"
)

list(APPEND AIAGENT_HDR
    "${MULTIEDGE_AIAGENT}/agentchathistory.hpp"
    "${MULTIEDGE_AIAGENT}/agentprocessor.hpp"
    "${MULTIEDGE_AIAGENT}/agentprovider.hpp"
    "${MULTIEDGE_AIAGENT}/aiagent.hpp"
)



list(APPEND AIAGENT_FILES "${MULTIEDGE_TRANS}" "${MULTIEDGE_RES}" "${AIAGENT_UI}" "${AIAGENT_HDR}" "${AIAGENT_SRC}")
qt_add_executable(${APP_NAME} MANUAL_FINALIZATION "${AIAGENT_FILES}")
target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets areg::areg llama gen_multiedge)

# add to the list to finalize later.
list(APPEND AREG_EDGEAI_EXES "${APP_NAME}")
