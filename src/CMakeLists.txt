include(AddVersion)

# Dbus is here only because platform plugin linked to it???
find_package(Qt5 CONFIG REQUIRED Network Widgets DBus)

qt5_wrap_ui(UI_HEADERS
  LoginDialog.ui
  MainWindow.ui
  RoomView.ui
  ChatWindow.ui
  RedactDialog.ui
  JoinDialog.ui
  EventSourceView.ui
  showImage.ui
)

### Ereader dev library
file(GLOB SRCS_dev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/*.cpp)
file(GLOB SRCS_dev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/*.ui)
file(GLOB SRCS_dev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/*.cpp)
file(GLOB SRCS_dev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/*.ui)

find_package(Qt5 COMPONENTS Core REQUIRED Widgets)

include_directories(
    ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/
    ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/
)

add_library(ereaderdev SHARED ${SRCS_dev})
target_link_libraries(ereaderdev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/build/libereaderdev.so)
target_link_libraries(ereaderdev Qt5::Core Qt5::Widgets)
###


### Resource files
qt5_add_resources(QT_RESOURCES ${PROJECT_SOURCE_DIR}/resources/eink.qrc)

add_subdirectory(matrix)

add_version(version.cpp)

add_executable(nachat WIN32
  main.cpp
  LoginDialog.cpp
  MainWindow.cpp
  ChatWindow.cpp
  RoomView.cpp
  TimelineView.cpp
  EntryBox.cpp
  RoomMenu.cpp
  sort.cpp
  RoomViewList.cpp
  Spinner.cpp
  RedactDialog.cpp
  JoinDialog.cpp
  version.cpp
  version_string.cpp
  MessageBox.cpp
  EventSourceView.cpp
  ContentCache.cpp
  JoinedRoomListModel.cpp
  showImage.cpp
  ${UI_HEADERS}
  ${QT_RESOURCES}
)

target_link_libraries(nachat
  Qt5::Widgets
  Qt5::Network
  Qt5::DBus
  ereaderdev
  matrix
)

### Qt kobo platform plugin
# It's a plugin, so do it that way
include_directories(${PROJECT_SOURCE_DIR}/deps/qt5-kobo-platform-plugin/src/)
target_include_directories(nachat
    PRIVATE ${PROJECT_SOURCE_DIR}/deps/qt5-kobo-platform-plugin/src/
)
###

install(
  TARGETS nachat
  DESTINATION bin
)
