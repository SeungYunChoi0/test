cmake_minimum_required(VERSION 2.8)
project(tcnnapp)

add_definitions(-DTCC7500)
SET(TCNN_VERSION_MAJOR "2")
SET(TCNN_VERSION_MINOR "0")
SET(TCNN_VERSION_PATCH "4")

# OpenCV
find_package(OpenCV REQUIRED)
find_package(visionprotocol REQUIRED)
find_package(tcndnpu REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${LINUX_KERNEL_DIR}/include/video/telechips)
include_directories(${VCF_INCLUDE_DIRS})
include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/camera
    ${CMAKE_SOURCE_DIR}/common/display
    ${CMAKE_SOURCE_DIR}/common/message
    ${CMAKE_SOURCE_DIR}/common/utils
    ${CMAKE_SOURCE_DIR}/common/video
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
include_directories(json-c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread -ljson-c -ldl")
set(SOURCES
    src/NnAppMain.c
    src/NnDebug.c
    src/NnMemory.c
    src/NnNeuralNetwork.c
    src/NnPerf.c
    src/NnRtpm.c
    src/NnSignalHandler.c
)
set(SOURCES_COMMON
    common/camera/camera_api.c
    common/display/display_api.c
    common/display/scaler_api.c
    common/message/message_api.c
    common/utils/opencv_api.cpp
    common/utils/perf_api.c
    common/utils/time_api.c
)

set(LIBRARIES
    ${OpenCV_LIBS} dl json-c visionprotocol tcndnpu
)

add_executable(tcnnapp ${SOURCES} ${SOURCES_COMMON})
target_link_libraries(tcnnapp ${LIBRARIES})
# target_compile_options(tcnnapp PRIVATE -Wall)
target_compile_options(tcnnapp PRIVATE -Wall -Wextra -Werror)
install(TARGETS tcnnapp DESTINATION bin)
