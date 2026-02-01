find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

qt_standard_project_setup()

set(SOURCES
# C++ source files:
    src/main.cpp
    src/Application.cpp
    src/Application.hpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/IpInputEventFilter.cpp
    src/IpInputEventFilter.hpp
    src/Ipv4Widget.hpp
    src/Ipv4Widget.cpp
    src/Ipv6Widget.hpp
    src/Ipv6Widget.cpp
    src/AbstractIpWidget.hpp
    src/AbstractIpWidget.cpp
    src/SaveAsDialog.hpp
    src/SaveAsDialog.cpp

# Qt UI files:
    ui/MainWindow.ui
    ui/AbstractIpWidget.ui
    ui/SaveAsDialog.ui

# Qt QRC files:
    resources/res.qrc
)

set(TRANSLATIONS
    translations/pl.ts
)

if(APPLE)
    set(MACOS_APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns)
    set_source_files_properties(${MACOS_APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
elseif(WIN32)
    enable_language("RC")
    set(WIN32_RESOURCES ${CMAKE_SOURCE_DIR}/app/resources/res.rc)
endif()

qt_add_executable(SubnetCalculatorApp
    ${SOURCES}
    ${MACOS_APP_ICON}
    ${WIN32_RESOURCES}
)

qt_add_translations(SubnetCalculatorApp
    RESOURCE_PREFIX /translations
    TS_FILES ${TRANSLATIONS}
    SOURCES ${SOURCES}
)

set_target_properties(SubnetCalculatorApp PROPERTIES
    OUTPUT_NAME "SubnetCalculator"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_BUNDLE_NAME "Subnet Calculator"
    WIN32_EXECUTABLE TRUE
)

target_include_directories(SubnetCalculatorApp PRIVATE src)
target_include_directories(SubnetCalculatorApp PRIVATE ../lib/include)
target_link_libraries(SubnetCalculatorApp PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    libSubnetCalculator
)

install(TARGETS SubnetCalculatorApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(deploy_tool_options_arg "")
if(APPLE AND CODE_SIGN_CERT_NAME)
    set(deploy_tool_options_arg
        -hardened-runtime
        -appstore-compliant
        "\"-codesign==${CODE_SIGN_CERT_NAME}\""
        -timestamp
        -dmg
    )
endif()

qt_generate_deploy_app_script(
    TARGET SubnetCalculatorApp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
)

install(SCRIPT ${deploy_script})