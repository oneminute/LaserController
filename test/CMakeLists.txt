find_package(GTest REQUIRED)

list(APPEND INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ../include)
include_directories(
    ${INCLUDE_DIRS}
)
list(APPEND LINK_LIBRARIES ${GTEST_BOTH_LIBRARIES})

list(REMOVE_ITEM SRC_FILES ${BASE_SOURCES})

add_executable(LaserControllerTest 
    main.cpp 
    TestLaserDriver.cpp 
    ${SRC_FILES}
)

target_link_libraries(LaserControllerTest ${LINK_LIBRARIES})

gtest_discover_tests(LaserControllerTest)
