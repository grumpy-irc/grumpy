PROJECT (grumpycore)
SET(CMAKE_AUTOMOC ON)
SET(QT_USE_QTNETWORK TRUE)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Qml REQUIRED)
set(QT_INCLUDES ${Qt5Network_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS})
include_directories(${QT_INCLUDES})

file (GLOB src "*.cpp" "scripting/*.cpp")
file (GLOB headers "*.h" "scripting/*.h")
file (GLOB resources "*.qrc")

QT5_WRAP_CPP(grumpycore_HEADERS_MOC ${grumpycore_HEADERS})
QT5_ADD_RESOURCES(grumpycore_RESOURCES_RCC ${resources})

ADD_DEFINITIONS(${QT_DEFINITIONS})

if (SQLITE)
  set(src ${src} "../sqlite/sqlite3.c")
  ADD_DEFINITIONS(-DGRUMPY_SQLITE)
endif()

ADD_DEFINITIONS( -DLIBCORE_LIBRARY -DQT_USE_QSTRINGBUILDER )

ADD_LIBRARY(grumpycore SHARED ${src} ${grumpycore_RESOURCES_RCC} ${headers})
if (QT5_BUILD)
    TARGET_LINK_LIBRARIES(grumpycore Qt5::Core Qt5::Network Qt5::Qml)
endif()
TARGET_LINK_LIBRARIES(grumpycore gp ircclient ${QT_LIBRARIES})
if (NOT WIN32)
  INSTALL(TARGETS grumpycore LIBRARY DESTINATION lib)
endif()
