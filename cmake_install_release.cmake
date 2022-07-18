INSTALL(TARGETS ${PROJECT_NAME}
    DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT runtime
	CONFIGURATIONS Release)

INSTALL(FILES 
            "${CMAKE_SOURCE_DIR}/third/bin/LaserLib${CNELaser_ARCH}.dll"
            "${CMAKE_SOURCE_DIR}/third/others/AccBuf.txt"
            "${CMAKE_SOURCE_DIR}/ReleaseNotes.md"
        DESTINATION ${CMAKE_INSTALL_PREFIX}
		CONFIGURATIONS Release)

INSTALL(FILES 
	"${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_zh_CN.qm"
	DESTINATION ${CMAKE_INSTALL_PREFIX}/translations
	CONFIGURATIONS Release)

foreach(_lib IN LISTS OpenCV_LIBS)
	get_target_property(${_lib}_location_Release ${_lib} IMPORTED_LOCATION_RELEASE)
	INSTALL(FILES 
		${${_lib}_location_Release}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		CONFIGURATIONS Release)
endforeach()

if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)
install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
	CONFIGURATIONS Release)

get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
INSTALL(CODE "
	execute_process(
		COMMAND \"${QT_BIN_DIR}/windeployqt.exe\"
			--dir ${CMAKE_INSTALL_PREFIX}
			--verbose 1
			--release
			--no-translations
			${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe)"
	CONFIGURATIONS Release)

set(BASE_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

INSTALL(SCRIPT "cmake/compress.cmake"
    CONFIGURATIONS Release)
