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
set(EDGEAI_MODELS       "${EDGEAI_ROOT}/models")
set(PROJECT_SOURCE_DIR  "${EDGEAI_BASE}")

set(EDGEAI_SRC)
set(EDGEAI_HDR)
set(EDGEAI_RES)
set(EDGEAI_UI)
set(EDGEAI_TRANS)

set(AREG_EDGEAI_EXES)

include("${EDGEAI_CMAKE_CONFIG}/setup.cmake")

if(NOT ${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    message(FATAL_ERROR "AREG-EDGEAI: >>> You should use QT version 6 or newer to compile this project")
endif()

include_directories(${EDGEAI_BASE})
include_directories(${EDGEAI_THIRDPARTY})

include(GNUInstallDirs)
include("${EDGEAI_BASE}/CMakeLists.txt")

# ---------------------------------------------------------
# Copy `areg.init` config file
# ---------------------------------------------------------
add_custom_target(copy-areg-config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AREG_RESUORCES}/areg.init"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config/areg.init"
    COMMENT "Copying Areg configuration file into the build directory"
)

# ---------------------------------------------------------
# Copy models
# ---------------------------------------------------------

# Copy all models recursively to runtime output
add_custom_target(copy-edgeai-models ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Copying EdgeAI models..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/models"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${EDGEAI_MODELS}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/models"
    COMMENT "Copying EdgeAI models to build directory"
)

# ---------------------------------------------------------
# finalize apps
# ---------------------------------------------------------
foreach(APP ${AREG_EDGEAI_EXES})
    areg_edgeai_finalize_apps(${APP})
endforeach()
