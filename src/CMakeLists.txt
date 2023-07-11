include(AddVersion)

find_package(Qt5Network 5 REQUIRED)
find_package(Qt5Widgets 5 REQUIRED)

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
file(GLOB SRCS ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/*.cpp)
file(GLOB SRCS ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/*.ui)
file(GLOB SRCS ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/*.cpp)
file(GLOB SRCS ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/*.ui)

find_package(Qt5 COMPONENTS Core REQUIRED Widgets)

include_directories(
    ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/
    ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/gui/
)

add_library(ereaderdev SHARED ${SRCS})
target_link_libraries(ereaderdev ${PROJECT_SOURCE_DIR}/deps/ereaderdev-lib/ereaderdev/build/libereaderdev.so)
target_link_libraries(ereaderdev Qt5::Core Qt5::Widgets)
###

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
)

target_link_libraries(nachat
  matrix
  Qt5::Widgets
  Qt5::Network
  ereaderdev
)

install(
  TARGETS nachat
  DESTINATION bin
)
