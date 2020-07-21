enable_testing()

#find_package(GTest REQUIRED)

find_package(Qt5Test REQUIRED)
list(APPEND LINK_LIBRARIES Qt5::Test)

include_directories(
    ${INCLUDE_DIRS}
)


list(REMOVE_ITEM SRC_FILES ${BASE_SOURCES})

set(TESTING_FILES
    TestLaserDriver.h
    TestLaserDriver.cpp
    TestStateController.h
    TestStateController.cpp
)

source_group("target_sources" FILES ${SRC_FILES})
source_group("testing_sources" FILES ${TESTING_FILES})

add_executable(LaserControllerTest 
    main.cpp
    ${TESTING_FILES}
    ${SRC_FILES}
)
add_test(NAME LaserControllerTest COMMAND LaserControllerTest)

target_link_libraries(LaserControllerTest ${LINK_LIBRARIES} Qt5::Test)

#gtest_discover_tests(LaserControllerTest)
