find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Application.hpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/Ipv4Widget.hpp
    src/Ipv4Widget.cpp
    src/Ipv6Widget.hpp
    src/Ipv6Widget.cpp

    ui/MainWindow.ui
    ui/Ipv4Widget.ui
    ui/Ipv6Widget.ui
)

qt_add_executable(SubnetCalculatorApp MACOSX_BUNDLE ${SOURCES})

set_target_properties(SubnetCalculatorApp PROPERTIES
    OUTPUT_NAME "SubnetCalculator"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_BUNDLE_NAME "Subnet Calculator"
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/include)
target_link_libraries(SubnetCalculatorApp PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    libSubnetCalculator
)
