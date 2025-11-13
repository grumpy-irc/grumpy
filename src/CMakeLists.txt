# So that MacOS builds are working on older versions as well
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

project(GrumpyIRC)
cmake_minimum_required (VERSION 2.8.0)
if(WIN32)
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
        add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
    endif()
endif()

if(WIN32)
    if(MINGW)
        if (QT6_BUILD)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    endif()
else()
    if (QT6_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
    endif()
endif()

# Settings
option(LINUX_SNAP "Build using snap" false)
option(QT5_BUILD "Build with Qt5" true)
option(QT6_BUILD "Build with Qt6" false)
option(EXTREME_DEBUG "Enable some resource expensive and slowing debug info" false)
option(UNIT_TESTS "Enable unit tests" false)
option(GRUMPY_PROFILER "Build internal profiler (extreme slowdown)" false)

# Configuration
if (GRUMPY_PROFILER)
    ADD_DEFINITIONS(-DGRUMPY_PROFILER)
endif()
if (UNIT_TESTS)
    SET(QT_USE_QTTEST TRUE)
endif()
if (QT6_BUILD)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Xml Network Qml Sql)
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Xml Qt6::Network Qt6::Qml Qt6::Sql)
elseif (QT5_BUILD)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Xml Network Qml Sql)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Qml Qt5::Sql)
endif()
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Unit test
if (UNIT_TESTS)
    add_subdirectory (tests)
endif()

# Libraries
add_subdirectory (libirc2htmlcode)
add_subdirectory (libirc/libirc/)
add_subdirectory (libirc/libircclient/)
add_subdirectory (libgp)

# GrumpyIRC client itself
add_subdirectory (libcore)
add_subdirectory (grumpyd)
add_subdirectory (GrumpyIRC)


# Packaging code
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Petr Bena")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "GrumpyChat IRC client")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GrumpyChat IRC client")
SET(CPACK_PACKAGE_NAME "GrumpyIRC")
SET(CPACK_PACKAGE_VENDOR "Petr Bena")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5, libqt5widgets5, libqt5xml5, libqt5network5, libqt5core5a, libqt5qml5, libqt5sql5")
INCLUDE(CPack)
