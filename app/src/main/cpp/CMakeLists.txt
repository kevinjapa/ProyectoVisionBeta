# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name.
project("proyectovision")

# Importaciones para usar con OPEN CV
set(OpenCV_STATIC ON)
set(OpenCV_DIR /Users/kevinjapa/Documents/OpenCV-android-sdk/sdk/native/jni)
find_package(OpenCV REQUIRED)
include_directories(/Users/kevinjapa/Documents/OpenCV-android-sdk/sdk/native/jni/include/)

# Creates and names a library, sets it as either STATIC or SHARED, and provides the relative paths to its source code.
add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp)

add_library(parte2-lib SHARED
        second-native-lib.cpp)

find_library(
        jnigraphics-lib jnigraphics
)
find_library(
        log-lib
        log
)

# Specifies libraries CMake should link to your target library.
target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        ${OpenCV_LIBS}
        ${jnigraphics-lib}
        ${log-lib}
)

target_link_libraries(parte2-lib
        android
        ${OpenCV_LIBS}
        ${jnigraphics-lib}
        ${log-lib}
)

