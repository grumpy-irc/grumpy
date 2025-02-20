PROJECT (irc2htmlcode)
SET(CMAKE_AUTOMOC ON)

option(QT6_BUILD "Build with Qt6" false)

if(QT6_BUILD)
  find_package(Qt6Core REQUIRED)
  set(QT_INCLUDES ${Qt6Core_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
elseif(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  set(QT_INCLUDES ${Qt5Core_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
else()
  find_package(Qt4 REQUIRED)
endif()

file (GLOB src "*.cpp")
file (GLOB headers "*.h")
file (GLOB resources "*.qrc")

if(QT6_BUILD)
    QT6_WRAP_CPP(irc2htmlcode_HEADERS_MOC ${irc2htmlcode_HEADERS})
    QT6_ADD_RESOURCES(irc2htmlcode_RESOURCES_RCC ${resources})
elseif(QT5_BUILD)
    QT5_WRAP_CPP(irc2htmlcode_HEADERS_MOC ${irc2htmlcode_HEADERS})
    QT5_ADD_RESOURCES(irc2htmlcode_RESOURCES_RCC ${resources})
else()
    QT4_WRAP_CPP(irc2htmlcode_HEADERS_MOC ${irc2htmlcode_HEADERS})
    QT4_ADD_RESOURCES(irc2htmlcode_RESOURCES_RCC ${resources})
endif()

if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS( -DLIBIRC2HTMLCODE_LIBRARY -DQT_USE_QSTRINGBUILDER )

ADD_LIBRARY(irc2htmlcode SHARED ${src} ${headers})

if (QT6_BUILD)
  TARGET_LINK_LIBRARIES(irc2htmlcode Qt6::Core)
elseif (QT5_BUILD)
  TARGET_LINK_LIBRARIES(irc2htmlcode Qt5::Core)
endif()

TARGET_LINK_LIBRARIES(irc2htmlcode ${QT_LIBRARIES})

if (NOT WIN32)
  INSTALL(TARGETS irc2htmlcode LIBRARY DESTINATION lib)
endif()
