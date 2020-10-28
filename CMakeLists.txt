cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0087 OLD)

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

# declare variables
set(QT_WIN32_PATH "" CACHE PATH "")
set(OPENCV_WIN32_PATH "" CACHE PATH "")
list(APPEND CMAKE_PROGRAM_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH})

set(NEW_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH} $ENV{PATH})
set(CMAKE_MSVCIDE_RUN_PATH ${NEW_PATH})

# Find the QtWidgets library
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Eigen3 REQUIRED)
find_package(Boost 1.66 COMPONENTS timer REQUIRED)
find_package(Qt5 COMPONENTS Widgets Charts LinguistTools OpenGL CONFIG REQUIRED)
find_package(OpenCV COMPONENTS core imgcodecs imgproc REQUIRED QUIET)
find_package(OpenMP REQUIRED QUIET)
find_package(ZLIB REQUIRED QUIET)
find_package(glog REQUIRED QUIET)
find_package(gflags REQUIRED QUIET)

set(TS_FILES translations/LaserController_zh_CN.ts)

add_subdirectory(src/widget/LRWidgets)

#add_subdirectory(widgets)
set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    src
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    src/widget/LRWidgets
)

FILE(GLOB BASE_HEADERS src/*.h ./*.hpp)
FILE(GLOB BASE_SOURCES src/*.cpp)

FILE(GLOB COMMON_HEADERS src/common/*.h)
FILE(GLOB COMMON_SOURCES src/common/*.cpp)

FILE(GLOB HALFTONE_HEADERS src/halftone/*.h)
FILE(GLOB HALFTONE_SOURCES src/halftone/*.cpp)

FILE(GLOB IMPORT_HEADERS src/import/*.h)
FILE(GLOB IMPORT_SOURCES src/import/*.cpp)

FILE(GLOB LASER_HEADERS src/laser/*.h)
FILE(GLOB LASER_SOURCES src/laser/*.cpp)

FILE(GLOB RES src/res/*.qrc)

FILE(GLOB SCENE_HEADERS src/scene/*.h)
FILE(GLOB SCENE_SOURCES src/scene/*.cpp)

FILE(GLOB STATE_HEADERS src/state/*.h)
FILE(GLOB STATE_SOURCES src/state/*.cpp)

FILE(GLOB SVG_HEADERS src/svg/*.h src/svg/*.hpp)
FILE(GLOB SVG_SOURCES src/svg/*.cpp)

FILE(GLOB TASK_HEADERS src/task/*.h src/task/*.hpp)
FILE(GLOB TASK_SOURCES src/task/*.cpp)

FILE(GLOB UI_HEADERS src/ui/*.h)
FILE(GLOB UI_SOURCES src/ui/*.cpp)
FILE(GLOB UI_FILES src/ui/*.ui)

FILE(GLOB UTIL_HEADERS src/util/*.h)
FILE(GLOB UTIL_SOURCES src/util/*.cpp)

FILE(GLOB WIDGET_HEADERS src/widget/*.h)
FILE(GLOB WIDGET_SOURCES src/widget/*.cpp)

FILE(GLOB MOCS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/*.cpp)

set(OTHER_FILES
    config.ini
    README.md
    ReleaseNotes.txt
    .gitignore
)

source_group("ui" FILES ${UIS})
source_group("others" FILES ${OTHER_FILES})

set(HEADER_FILES
    ${HEADER_FILES}
    ${BASE_HEADERS}
    ${COMMON_HEADERS}
    ${HALFTONE_HEADERS}
    ${IMPORT_HEADERS}
    ${LASER_HEADERS}
	${SCENE_HEADERS}
    ${STATE_HEADERS}
    ${SVG_HEADERS}
    ${TASK_HEADERS}
    ${UI_HEADERS}
    ${UTIL_HEADERS}
    ${WIDGET_HEADERS}
)

set(SRC_FILES
    ${SRC_FILES}
    ${BASE_SOURCES}
    ${COMMON_SOURCES}
    ${HALFTONE_SOURCES}
    ${IMPORT_SOURCES}
    ${LASER_SOURCES}
    ${RES}
	${SCENE_SOURCES}
    ${STATE_SOURCES}
    ${SVG_SOURCES}
    ${TASK_SOURCES}
    ${UI_SOURCES}
    ${UI_FILES}
    ${UTIL_SOURCES}
    ${WIDGET_SOURCES}
)

IF(MSVC)
    source_group("source\\common" FILES ${COMMON_SOURCES})
    source_group("header\\common" FILES ${COMMON_HEADERS})
    source_group("source\\halftone" FILES ${HALFTONE_SOURCES})
    source_group("header\\halftone" FILES ${HALFTONE_HEADERS})
    source_group("source\\import" FILES ${IMPORT_SOURCES})
    source_group("header\\import" FILES ${IMPORT_HEADERS})
    source_group("source\\laser" FILES ${LASER_SOURCES})
    source_group("header\\laser" FILES ${LASER_HEADERS})
	source_group("source\\scene" FILES ${SCENE_SOURCES})
	source_group("header\\scene" FILES ${SCENE_HEADERS})
    source_group("header\\state" FILES ${STATE_HEADERS})
    source_group("source\\state" FILES ${STATE_SOURCES})
    source_group("source\\svg" FILES ${SVG_SOURCES})
    source_group("header\\svg" FILES ${SVG_HEADERS})
    source_group("source\\task" FILES ${TASK_SOURCES})
    source_group("header\\task" FILES ${TASK_HEADERS})
    source_group("source\\ui" FILES ${UI_SOURCES} ${UI_FILES})
    source_group("header\\ui" FILES ${UI_HEADERS})
    source_group("source\\util" FILES ${UTIL_SOURCES})
    source_group("header\\util" FILES ${UTIL_HEADERS})
    source_group("header\\widget" FILES ${WIDGET_HEADERS})
    source_group("source\\widget" FILES ${WIDGET_SOURCES})
    source_group("source\\mocs" FILES ${MOCS})
    source_group("source" FILES ${BASE_SOURCES})
    source_group("res" FILES ${RES})
ENDIF(MSVC)

include_directories(${INCLUDE_DIRS})
set(LINK_LIBRARIES 
    Qt5::Widgets Qt5::Charts Qt5::OpenGL
    ${OpenCV_LIBRARIES} 
    ${Boost_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    glog::glog
    ${GFLAGS_LIBRARIES}
    LRWidgets
)

if(ANDROID)
  add_library(LaserController SHARED ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${TS_FILES})
else()
  add_executable(LaserController ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${TS_FILES})
endif()

target_link_libraries(LaserController PRIVATE ${LINK_LIBRARIES})

set(QM_FILES
    ${QM_FILES}
    ${SRC_FILES}
    ${HEADER_FILES}
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_subdirectory(test)

INSTALL(TARGETS LaserController
        DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT runtime)

INSTALL(FILES 
            "${CMAKE_SOURCE_DIR}/third/bin/LaserLib32.dll"
            "${CMAKE_SOURCE_DIR}/third/others/AccBuf.txt"
            "${CMAKE_SOURCE_DIR}/ReleaseNotes.txt"
        DESTINATION ${CMAKE_INSTALL_PREFIX})

INSTALL(FILES 
            "${CMAKE_SOURCE_DIR}/translations/LaserController_zh_CN.qm"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/translations)

foreach(_lib IN LISTS OpenCV_LIBS)
    INSTALL(FILES 
                "${OpenCV_DIR}/../bin/${_lib}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll"
            DESTINATION ${CMAKE_INSTALL_PREFIX})
endforeach()

get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
INSTALL(CODE "
        execute_process(
            COMMAND \"${QT_BIN_DIR}/windeployqt.exe\"
                    --dir ${CMAKE_INSTALL_PREFIX}
                    --verbose 1
                    --release
                    --no-translations
                    ${CMAKE_INSTALL_PREFIX}/LaserController.exe
        )
        ")
