option(CMAKE_AUTOUIC "Handle UIC automatically for Qt targets" ON)
option(CMAKE_AUTOMOC "Handle MOC automatically for Qt targets" ON)
option(CMAKE_AUTORCC "Handle RCC automatically for Qt targets" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(AREG_BINARY         shared)
option(AREG_BUILD_TESTS     "Escape building unit tests"    OFF)
option(AREG_BUILD_EXAMPLES  "Escape building examples"      OFF)
option(AREG_EXTENDED        "Escape building extended"      OFF)
option(AREG_GTEST_PACKAGE   "Escape GTest"                  OFF)
option(AREG_ENABLE_OUTPUTS  "Ignore AREG build structure"   OFF)

include(FetchContent)
# Specify where to fetch third-party sources (including AREG SDK).
if (NOT DEFINED AREG_PACKAGES OR "${AREG_PACKAGES}" STREQUAL "")
    set(AREG_PACKAGES "${CMAKE_BINARY_DIR}/packages")
endif()

set(FETCHCONTENT_BASE_DIR "${AREG_PACKAGES}")

find_package(areg CONFIG)
if (NOT areg_FOUND)
    # ##################################################################
    # AREG SDK not found as a package, fetching from GitHub.
    # ##################################################################

    message(STATUS "AREG-EDGEAI: >>> Fetching AREG SDK from GitHub to ${FETCHCONTENT_BASE_DIR}")
        
    FetchContent_Declare(
        areg
        GIT_REPOSITORY https://github.com/aregtech/areg-sdk.git
        GIT_TAG "master"
    )
    message(STATUS "AREG-EDGEAI: >>> AREG SDK sources are fetched, setting up areg-sdk ...")
    FetchContent_MakeAvailable(areg)

    # Set the root directory of the fetched AREG SDK
    set(AREG_SDK_ROOT "${areg_SOURCE_DIR}")
    set(AREG_CMAKE_CONFIG_DIR "${AREG_SDK_ROOT}/conf/cmake")

endif()

include(${AREG_SDK_ROOT}/areg.cmake)
set(AREG_RESOURCES "${AREG_FRAMEWORK}/areg/resources")

# ---------------------------------------------------------
# llama.cpp build options
# ---------------------------------------------------------
set(LLAMA_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER   OFF CACHE BOOL "" FORCE)

set(GGML_OPENMP     ON  CACHE BOOL "" FORCE) # good default
set(LLAMA_NATIVE    OFF CACHE BOOL "" FORCE) # avoid CPU specific flags
set(GGML_CUDA       OFF CACHE BOOL "" FORCE) # explicitly off for now

# ---------------------------------------------------------
# Fetch llama.cpp
# ---------------------------------------------------------
FetchContent_Declare(
    llama
    GIT_REPOSITORY https://github.com/ggml-org/llama.cpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(llama)
set(LLAMA_ROOT "${llama_SOURCE_DIR}")

# ---------------------------------------------------------
# Sanity check
# ---------------------------------------------------------
if (NOT TARGET llama)
    message(FATAL_ERROR "llama.cpp was fetched but target 'llama' was not created")
endif()

# ---------------------------------------------------------
# Functions and macros
# ---------------------------------------------------------
function(areg_edgeai_finalize_apps TARGET_APP_NAME)
    if(${QT_VERSION} VERSION_LESS 6.1.0)
            set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER tech.areg.${TARGET_APP_NAME})
    endif()
    
    set_target_properties(  ${TARGET_APP_NAME} PROPERTIES
                            ${BUNDLE_ID_OPTION}
                            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
                            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
                            MACOSX_BUNDLE TRUE
                            WIN32_EXECUTABLE TRUE
            )
    
    # ---------------------------------------------------------
    # Qt Deployment for target executable
    # ---------------------------------------------------------
    if (WIN32)
        get_target_property(_qt_bin_dir Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qt_bin_dir}" DIRECTORY)
        add_custom_command(TARGET ${TARGET_APP_NAME} POST_BUILD
            COMMAND "${_qt_bin_dir}/windeployqt.exe"
                    --no-opengl-sw
                    --no-compiler-runtime
                    --no-translations
                    --no-system-dxc-compiler
                    --no-quick-import
                    --no-ffmpeg
                    "$<TARGET_FILE:${TARGET_APP_NAME}>"
            COMMENT "'\${TARGET_APP_NAME}\': Deploying Qt runtime libraries for Win32"
        )
    elseif (APPLE)
        get_target_property(_qt_bin_dir Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qt_bin_dir}" DIRECTORY)
        add_custom_command(TARGET ${TARGET_APP_NAME} POST_BUILD
            COMMAND "${_qt_bin_dir}/macdeployqt"
                    --no-opengl-sw
                    --no-compiler-runtime
                    --no-translations
                    --no-system-dxc-compiler
                    --no-quick-import
                    --no-ffmpeg
                    "$<TARGET_FILE:${TARGET_APP_NAME}>"
            COMMENT "'\${TARGET_APP_NAME}\': Deploying Qt runtime libraries for macOS"
        )
    else()
        message(STATUS "Qt deployment skipped on Linux (handled by system packaging)")
    endif()
        
    if(QT_VERSION_MAJOR EQUAL 6)
            qt_finalize_executable(${TARGET_APP_NAME})
    endif()
    
    install(TARGETS ${TARGET_APP_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endfunction(areg_edgeai_finalize_apps)

message(STATUS "-------------------- CMakeLists Status Report Begin --------------------")
message(STATUS "AREG-EDGEAI: >>> Qt Version = \'${QT_VERSION_MAJOR}\'")
message(STATUS "AREG-EDGEAI: >>> CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "AREG-EDGEAI: >>> AREG ROOT directory = ${AREG_SDK_ROOT}")
message(STATUS "AREG-EDGEAI: >>> LLaMA ROOT directory = ${LLAMA_ROOT}")
message(STATUS "-------------------- CMakeLists Status Report End ----------------------")
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
