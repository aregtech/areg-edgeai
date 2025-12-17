cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME    areg-edgeai)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

project("${PROJECT_NAME}" 
        VERSION "${PROJECT_VERSION}" 
        DESCRIPTION "Edge AI agent service provider and consumer powered by Areg SDK" 
        HOMEPAGE_URL "https://areg.tech" 
        LANGUAGES CXX C
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EDGEAI_ROOT         "${CMAKE_CURRENT_LIST_DIR}")
set(EDGEAI_CMAKE_CONFIG	"${EDGEAI_ROOT}/conf")
set(EDGEAI_BASE         "${EDGEAI_ROOT}/apps")
set(EDGEAI_THIRDPARTY   "${EDGEAI_ROOT}/thirdparty")

set(EDGEAI_SRC)
set(EDGEAI_HDR)
set(EDGEAI_RES)
set(EDGEAI_UI)
set(EDGEAI_TRANS)

include("${EDGEAI_CMAKE_CONFIG}/setup.cmake")

if(NOT ${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    message(FATAL_ERROR "AREG-EDGEAI: >>> You should use QT version 6 or newer to compile this project")
endif()

include_directories(${EDGEAI_BASE})
include_directories(${EDGEAI_THIRDPARTY})
include("${EDGEAI_BASE}/CMakeLists.txt")


list(APPEND PROJECT_SOURCES "${EDGEAI_TRANS}" "${EDGEAI_RES}" "${EDGEAI_UI}" "${EDGEAI_HDR}" "${EDGEAI_SRC}")

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${EDGEAI_TRANS})


set(TS_FILES areg-edgeai_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        edgeconsumer.cpp
        edgeconsumer.hpp
        edgeconsumer.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(areg-edgeai
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET areg-edgeai APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(areg-edgeai SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(areg-edgeai
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(areg-edgeai PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.areg-edgeai)
endif()
set_target_properties(areg-edgeai PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS areg-edgeai
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(areg-edgeai)
endif()
