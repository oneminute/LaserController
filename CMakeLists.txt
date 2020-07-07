cmake_minimum_required(VERSION 3.5)

project(LaserController LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
add_definitions(-DPCL_ONLY_CORE_POINT_TYPES)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(QT_WIN32_PATH "" CACHE PATH "")
set(OPENCV_WIN32_PATH "" CACHE PATH "")
list(APPEND CMAKE_PROGRAM_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH})

message(CMAKE_PROGRAM_PATH=${CMAKE_PROGRAM_PATH})
message(CMAKE_LIBRARY_PATH=${CMAKE_LIBRARY_PATH})
message(CMAKE_SYSTEM_PROGRAM_PATH=${CMAKE_SYSTEM_PROGRAM_PATH})
message(CMAKE_SYSTEM_PREFIX_PATH=${CMAKE_SYSTEM_PREFIX_PATH})

set(NEW_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH} $ENV{PATH})
set(CMAKE_MSVCIDE_RUN_PATH ${NEW_PATH})
foreach(VAR in ${CMAKE_MSVCIDE_RUN_PATH})
    message(${VAR})
endforeach(VAR in ${CMAKE_MSVCIDE_RUN_PATH})

# Find the QtWidgets library
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Eigen3 REQUIRED)
find_package(Boost 1.66 COMPONENTS timer REQUIRED)
find_package(Qt5 COMPONENTS Widgets Charts LinguistTools OpenGL CONFIG REQUIRED)
find_package(OpenCV REQUIRED QUIET)
find_package(OpenMP REQUIRED QUIET)
find_package(ZLib REQUIRED QUIET)

set(TS_FILES LaserController_zh_CN.ts)

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    ./src
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
)

FILE(GLOB BASE_HEADERS src/*.h ./*.hpp)
FILE(GLOB BASE_SOURCES src/*.cpp)

FILE(GLOB COMMON_HEADERS src/common/*.h)
FILE(GLOB COMMON_SOURCES src/common/*.cpp)

FILE(GLOB LASER_HEADERS src/laser/*.h)
FILE(GLOB LASER_SOURCES src/laser/*.cpp)

FILE(GLOB STATE_HEADERS src/state/*.h)
FILE(GLOB STATE_SOURCES src/state/*.cpp)

FILE(GLOB SVG_HEADERS src/svg/*.h src/svg/*.hpp)
FILE(GLOB SVG_SOURCES src/svg/*.cpp)

FILE(GLOB UI_HEADERS src/ui/*.h)
FILE(GLOB UI_SOURCES src/ui/*.cpp)
FILE(GLOB UI_FILES src/ui/*.ui)

FILE(GLOB UTIL_HEADERS src/util/*.h)
FILE(GLOB UTIL_SOURCES src/util/*.cpp)

FILE(GLOB MOCS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/*.cpp)

set(OTHER_FILES
    config.ini
    README.md
    .gitignore
)

source_group("ui" FILES ${UIS})
source_group("others" FILES ${OTHER_FILES})

set(HEADER_FILES
    ${HEADER_FILES}
    ${BASE_HEADERS}
    ${COMMON_HEADERS}
    ${LASER_HEADERS}
    ${STATE_HEADERS}
    ${SVG_HEADERS}
    ${UI_HEADERS}
    ${UTIL_HEADERS}
)

set(SRC_FILES
    ${SRC_FILES}
    ${BASE_SOURCES}
    ${COMMON_SOURCES}
    ${LASER_SOURCES}
    ${STATE_SOURCES}
    ${SVG_SOURCES}
    ${UI_SOURCES}
    ${UI_FILES}
    ${UTIL_SOURCES}
)

IF(MSVC)
    source_group("source\\common" FILES ${COMMON_SOURCES})
    source_group("header\\common" FILES ${COMMON_HEADERS})
    source_group("source\\laser" FILES ${LASER_SOURCES})
    source_group("header\\laser" FILES ${LASER_HEADERS})
    source_group("header\\state" FILES ${STATE_HEADERS})
    source_group("source\\state" FILES ${STATE_SOURCES})
    source_group("source\\svg" FILES ${SVG_SOURCES})
    source_group("header\\svg" FILES ${SVG_HEADERS})
    source_group("source\\ui" FILES ${UI_SOURCES} ${UI_FILES})
    source_group("header\\ui" FILES ${UI_HEADERS})
    source_group("source\\util" FILES ${UTIL_SOURCES})
    source_group("header\\util" FILES ${UTIL_HEADERS})
    source_group("source\\mocs" FILES ${MOCS})
    source_group("source" FILES ${BASE_SOURCES})
ENDIF(MSVC)

include_directories(${INCLUDE_DIRS})

set(LINK_LIBRARIES 
    Qt5::Widgets Qt5::Charts Qt5::OpenGL ${OpenCV_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES}
)

if(ANDROID)
  add_library(LaserController SHARED ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${TS_FILES})
else()
  add_executable(LaserController ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${TS_FILES})
endif()

message(STATUS Boost_LIBRARIES=${Boost_LIBRARIES})
target_link_libraries(LaserController PRIVATE ${LINK_LIBRARIES})

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

enable_testing()
add_subdirectory(test)

