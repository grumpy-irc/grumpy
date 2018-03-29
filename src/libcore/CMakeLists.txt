PROJECT (grumpycore)
SET(CMAKE_AUTOMOC ON)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSCRIPT TRUE)

if(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Script REQUIRED)
  set(QT_INCLUDES ${Qt5Network_INCLUDE_DIRS} ${Qt5Script_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
else()
  find_package(Qt4 REQUIRED)
endif()

file (GLOB src "*.cpp")
file (GLOB headers "*.h")
file (GLOB resources "*.qrc")

if(QT5_BUILD)
    QT5_WRAP_CPP(grumpycore_HEADERS_MOC ${grumpycore_HEADERS})
    QT5_ADD_RESOURCES(grumpycore_RESOURCES_RCC ${resources})
else()
    QT4_WRAP_CPP(grumpycore_HEADERS_MOC ${grumpycore_HEADERS})
    QT4_ADD_RESOURCES(grumpycore_RESOURCES_RCC ${resources})
endif()

if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})

if (SQLITE)
  set(src ${src} "../sqlite/sqlite3.c")
  ADD_DEFINITIONS(-DGRUMPY_SQLITE)
endif()

ADD_DEFINITIONS( -DLIBCORE_LIBRARY -DQT_USE_QSTRINGBUILDER )


ADD_LIBRARY(grumpycore SHARED ${src} ${headers})
if (QT5_BUILD)
    qt5_use_modules(grumpycore Core Network Script)
endif()
TARGET_LINK_LIBRARIES(grumpycore gp ircclient ${QT_LIBRARIES})
if (NOT WIN32)
  INSTALL(TARGETS grumpycore LIBRARY DESTINATION lib)
endif()
