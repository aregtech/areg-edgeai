list(APPEND MULTIEDGE_TRANS
    ${MULTIEDGE_RESOURCE}/multiedge_en_US.ts
)

list(APPEND MULTIEDGE_RES
    ${MULTIEDGE_RESOURCE}/multiedge.qrc
)

qt_create_translation(MULTIEDGE_TRANS_FILES ${CMAKE_SOURCE_DIR} ${MULTIEDGE_TRANS})

# add service interface files.
addServiceInterface(gen_multiedge multiedge/resources/MultiEdge.siml)
