message(STATUS "Compressing...")

file(READ ${CMAKE_CURRENT_LIST_DIR}/../src/version.h raw_contents)

string(REGEX MATCH "LC_VERSION_MAJOR ([0-9]*)" _ ${raw_contents})
set(ver_major ${CMAKE_MATCH_1})
string(REGEX MATCH "LC_VERSION_MINOR ([0-9]*)" _ ${raw_contents})
set(ver_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "LC_VERSION_BUILD ([0-9]*)" _ ${raw_contents})
set(ver_build ${CMAKE_MATCH_1})
string(REGEX MATCH "LC_VERSION_REVISION ([0-9]*)" _ ${raw_contents})
set(ver_revision ${CMAKE_MATCH_1})
string(REGEX MATCH "LC_VERSION_TAG ([a-zA-Z]*)" _ ${raw_contents})
set(ver_tag ${CMAKE_MATCH_1})

set(arch_file "${CMAKE_INSTALL_PREFIX}_release.${ver_major}.${ver_minor}.${ver_build}.${ver_revision}-${ver_tag}.zip")
message(STATUS "ver_string=${arch_file}")
message(STATUS "cmake_install_prefix=${CMAKE_INSTALL_PREFIX}")
file(ARCHIVE_CREATE 
    OUTPUT ${arch_file} 
    PATHS ${CMAKE_INSTALL_PREFIX}_release 
    FORMAT zip 
    VERBOSE)
