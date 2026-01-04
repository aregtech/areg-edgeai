list(APPEND MULTIEDGE_TRANS
    "${MULTIEDGE_RESOURCE}/multiedge_en_US.ts"
)

list(APPEND MULTIEDGE_RES
    "${MULTIEDGE_RESOURCE}/multiedge.qrc"
)

list(APPEND EDGEDEVICE_HDR
    "${MULTIEDGE_RESOURCE}/nemultiedgesettings.hpp"
)

if (WIN32)
    list(APPEND EDGEDEVICE_RC "${MULTIEDGE_RESOURCE}/edgedevice.rc")
    list(APPEND AIAGENT_RC    "${MULTIEDGE_RESOURCE}/aiagent.rc")
endif(WIN32)

qt_create_translation(MULTIEDGE_TRANS_FILES ${CMAKE_SOURCE_DIR} ${MULTIEDGE_TRANS})

# add service interface files.
addServiceInterface(gen_multiedge multiedge/resources/MultiEdge.siml)
