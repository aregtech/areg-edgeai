set(APP_NAME "aiagent")

list(APPEND AIAGENT_UI
)

list(APPEND AIAGENT_SRC
    "${MULTIEDGE_AIAGENT}/agentprovider.cpp"
)

list(APPEND AIAGENT_HDR
    "${MULTIEDGE_AIAGENT}/agentprovider.hpp"
)
