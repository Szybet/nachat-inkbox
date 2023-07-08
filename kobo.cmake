# https://github.com/Szybet/kobo-nia-audio/blob/main/apps-on-kobo/kobo.cmake

# Example toolchain file for kobo
# Use with "-DCMAKE_TOOLCHAIN_FILE=../kobo.cmake" for anything that uses cmake, it should work
# https://stackoverflow.com/questions/5098360/cmake-specifying-build-toolchain
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   arm-kobo-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-kobo-linux-gnueabihf-g++)

set( CMAKE_CXX_FLAGS " -pthread -fPIC " )
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")


set(MYSYSROOT /home/build/inkbox/compiled-binaries/arm-kobo-linux-gnueabihf/arm-kobo-linux-gnueabihf/sysroot)
# compiler/linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
# cmake built-in settings to use find_xxx() functions
set(CMAKE_FIND_ROOT_PATH "${MYSYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(Qt5Core_DIR "/home/build/inkbox/compiled-binaries/qt-bin/")
set(Qt5_DIR "/home/build/inkbox/compiled-binaries/qt-bin/")
set(QT_QMAKE_EXECUTABLE "/home/build/inkbox/compiled-binaries/qt-bin/bin/qmake")
set(QUERY_EXECUTABLE "/home/build/inkbox/compiled-binaries/qt-bin/bin/qmake")
set(QT_INSTALL_PREFIX "/home/build/inkbox/compiled-binaries/qt-bin/")
set(QT_MAJOR_VERSION "5")

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)


