cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0087 OLD)
#cmake_policy(SET CMP0121 NEW)

project(CNELaser LANGUAGES CXX)

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
option(CNELASER_BUILD_EXAMPLES "Build Examples and Tech tests" OFF)
option(CNELASER_BUILD_COMPORT "Build Comport" OFF)
list(APPEND CMAKE_PROGRAM_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS x64)
    set(CNELaser_ARCH "64")
    add_compile_definitions(ARCH_x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(STATUS x32)
    set(CNELaser_ARCH "32")
    add_compile_definitions(ARCH_x32)
endif()


set(NEW_PATH ${QT_WIN32_PATH} ${OPENCV_WIN32_PATH} $ENV{PATH})
set(CMAKE_MSVCIDE_RUN_PATH ${NEW_PATH})


# Find the QtWidgets library
find_package(Eigen3 REQUIRED REQUIRED)
find_package(Qt5 COMPONENTS Widgets Charts LinguistTools OpenGL Concurrent Multimedia MultimediaWidgets SerialPort Test CONFIG REQUIRED)
find_package(OpenCV COMPONENTS core imgcodecs imgproc highgui video videoio flann features2d xfeatures2d calib3d tracking REQUIRED)
find_package(glog REQUIRED QUIET)
find_package(gflags REQUIRED QUIET)
find_package(LZ4 REQUIRED)
find_package(FLANN REQUIRED QUIET)

set(TS_FILES translations/*.ts)
set(VG_CORE_AUTO_TLI "${PROJECT_SOURCE_DIR}/third/cdr/include/VGCoreAuto.tli")
configure_file(${PROJECT_SOURCE_DIR}/third/cdr/include/VGCoreAuto.tlh.in VGCoreAuto.tlh)

add_subdirectory(src/widget/LRWidgets)
add_subdirectory(src/widget/ADS)

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    src
    ${EIGEN3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${FLANN_INCLUDE_DIR}
    src/widget/LRWidgets
	${PROJECT_BINARY_DIR}
)

FILE(GLOB BASE_HEADERS src/*.h ./*.hpp)
FILE(GLOB BASE_SOURCES src/*.cpp)

FILE(GLOB COMMON_HEADERS src/common/*.h)
FILE(GLOB COMMON_SOURCES src/common/*.cpp)

FILE(GLOB COMPORT_HEADERS src/comport/*.h)
FILE(GLOB COMPORT_SOURCES src/comport/*.cpp)

FILE(GLOB CAMERA_HEADERS src/camera/*.h)
FILE(GLOB CAMERA_SOURCES src/camera/*.cpp)

FILE(GLOB DLLTEST_HEADERS src/dlltest/*.h)
FILE(GLOB DLLTEST_SOURCES src/dlltest/*.cpp)

FILE(GLOB EXCEPTION_HEADERS src/exception/*.h)
FILE(GLOB EXCEPTION_SOURCES src/exception/*.cpp)

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

FILE(GLOB ALGORITHM_HEADERS src/algorithm/*.h)
FILE(GLOB ALGORITHM_SOURCES src/algorithm/*.cpp)

FILE(GLOB MOCS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/*.cpp)
FILE(GLOB DOCS docs/*.*)
FILE(GLOB TRANSLATIONS translations/*.ts)
LIST(FILTER DOCS EXCLUDE REGEX "[$]")

set(OTHER_FILES
    README.md
    ReleaseNotes.md
    cmake_install_Debug.cmake
    cmake_install_Release.cmake
    cmake_install_relwithdebinfo.txt
    third/cdr/include/VGCoreAuto.tlh.in
    src/res/icons.qrc
    src/res/laser.rc
    res/constraints.csv
    cmake/compress.cmake
    cmake/GenerateConstraints.cmake
    .gitignore
    ${TRANSLATIONS}
)

source_group("ui" FILES ${UIS})
source_group("others" FILES ${OTHER_FILES})
source_group("docs" FILES ${DOCS})
#source_group("translations" FILES ${TRANSLATIONS})

set(HEADER_FILES
    ${HEADER_FILES}
    ${BASE_HEADERS}
    ${COMMON_HEADERS}
    ${CAMERA_HEADERS}
    ${DLLTEST_HEADERS}
    ${EXCEPTION_HEADERS}
    ${ALGORITHM_HEADERS}
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
    ${CAMERA_SOURCES}
    ${DLLTEST_SOURCES}
    ${EXCEPTION_SOURCES}
    ${ALGORITHM_SOURCES}
    ${WYKOBI_SOURCES}
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
    source_group("source\\comport" FILES ${COMPORT_SOURCES})
    source_group("header\\comport" FILES ${COMPORT_HEADERS})
    source_group("source\\camera" FILES ${CAMERA_SOURCES})
    source_group("header\\camera" FILES ${CAMERA_HEADERS})
    source_group("source\\dlltest" FILES ${DLLTEST_SOURCES})
    source_group("header\\dlltest" FILES ${DLLTEST_HEADERS})
    source_group("source\\algorithm" FILES ${ALGORITHM_SOURCES})
    source_group("header\\algorithm" FILES ${ALGORITHM_HEADERS})
    source_group("source\\exception" FILES ${EXCEPTION_SOURCES})
    source_group("header\\exception" FILES ${EXCEPTION_HEADERS})
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
    source_group("header" FILES ${BASE_HEADERS})
    source_group("source" FILES ${BASE_SOURCES})
    source_group("res" FILES ${RES})
ENDIF(MSVC)

if (${CNELASER_BUILD_COMPORT})
    add_library(CNEComPort SHARED ${COMPORT_HEADERS} ${COMPORT_SOURCES})
    target_link_libraries(CNEComPort PRIVATE
        Qt5::SerialPort
    )
    target_compile_definitions(CNEComPort PRIVATE COMPORT_LIBRARY)

    add_executable(CNEComPortTesting src/testing/comport/ComPortTesting.cpp)
    add_test(NAME CNEComPortTesting COMMAND CNEComPortTesting)
    target_include_directories(CNEComPortTesting PUBLIC src/comport)
    target_link_libraries(CNEComPortTesting PRIVATE
        Qt5::SerialPort
        Qt5::Test
        CNEComPort
    )

    add_library(DllTest SHARED ${DLLTEST_HEADERS} ${DLLTEST_SOURCES})
    target_link_libraries(DllTest PRIVATE
        Qt5::SerialPort
    )
    target_compile_definitions(DllTest PRIVATE DLLTEST_LIBRARY)

    add_executable(DllTestTesting src/testing/dlltest/DllTest.cpp)
    add_test(NAME DllTestTesting COMMAND DllTestTesting)
    target_include_directories(DllTestTesting PUBLIC src/dlltest)
    target_link_libraries(DllTestTesting PRIVATE
        Qt5::SerialPort
        Qt5::Test
        DllTest
    )
endif()

set(LINK_LIBRARIES 
    Eigen3::Eigen
    Qt5::Widgets Qt5::Charts Qt5::OpenGL Qt5::Concurrent Qt5::Test Qt5::Multimedia Qt5::MultimediaWidgets
    LZ4::lz4_static
    FLANN::FLANN
    ${OpenCV_LIBRARIES} 
    ${Boost_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    glog::glog
    ${GFLAGS_LIBRARIES}
    LRWidgets
    qtadvanceddocking
)

add_custom_target(GenerateConstraints ALL
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateConstraints.cmake"
)

if(ANDROID)
  add_library(CNELaser SHARED ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${DOCS} ${TS_FILES})
else()
  add_executable(CNELaser ${SRC_FILES} ${SRC_FILES} ${HEADER_FILES} ${OTHER_FILES} ${DOCS})
endif()

add_dependencies(CNELaser GenerateConstraints)

target_link_libraries(CNELaser PRIVATE ${LINK_LIBRARIES})
target_include_directories(CNELaser PUBLIC ${INCLUDE_DIRS})
set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

#add_custom_command(TARGET CNELaser
#    PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateConstraints.cmake"
#    DEPENDS CNELaser
#)

if(CNELASER_BUILD_EXAMPLES)
    add_executable(TestBoostGeometry test/TestBoostGeometry.cpp)
    target_link_libraries(TestBoostGeometry PRIVATE ${LINK_LIBRARIES})
    add_executable(TestQtClosestPoint test/TestQtClosestPoint.cpp)
    target_link_libraries(TestQtClosestPoint PRIVATE ${LINK_LIBRARIES})
    add_executable(TestOpenCVFlann test/TestOpenCVFlann.cpp)
    target_link_libraries(TestOpenCVFlann PRIVATE ${LINK_LIBRARIES})
    add_executable(TestQPainterPath test/TestQPainterPath.cpp)
    target_link_libraries(TestQPainterPath PRIVATE ${LINK_LIBRARIES})
    add_executable(TestDriver test/TestDriver.cpp)
    target_link_libraries(TestDriver PRIVATE ${LINK_LIBRARIES})
    add_executable(TestException test/TestException.cpp src/exception/LaserException.h src/exception/LaserException.cpp)
    target_link_libraries(TestException PRIVATE ${LINK_LIBRARIES})
    add_executable(TestGeometry test/TestGeometry.cpp)
    target_link_libraries(TestGeometry PRIVATE ${LINK_LIBRARIES})

    FILE(GLOB TestDelphiLibrarySrc test/DelphiLibraryTest/*.*)
    add_executable(TestDelphiLibrary ${TestDelphiLibrarySrc})
    target_link_libraries(TestDelphiLibrary PRIVATE ${LINK_LIBRARIES})
endif()

set(QM_FILES
    ${QM_FILES}
    ${SRC_FILES}
    ${HEADER_FILES}
)

#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})

add_custom_target (GenerateTranslations 
    Qt5::lupdate ${CMAKE_SOURCE_DIR}/src -ts ${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_zh_CN.ts ${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_en_US.ts ${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_ru_RU.ts
)

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
include(cmake_install_${CMAKE_BUILD_TYPE}.cmake)
#include(cmake_install_release.cmake)
