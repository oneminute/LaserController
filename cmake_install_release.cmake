set(CMAKE_INSTALL_OPENMP_LIBRARIES true)
#set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS true)

INSTALL(TARGETS ${PROJECT_NAME}
    DESTINATION "${CMAKE_INSTALL_PREFIX}_Release" COMPONENT runtime
	CONFIGURATIONS Release)

#get_target_property(TBB_DLL_PATH_RELEASE TBB::tbb IMPORTED_LOCATION_RELEASE)
#get_target_property(TBBMALLOC_DLL_PATH_RELEASE TBB::tbbmalloc IMPORTED_LOCATION_RELEASE)

INSTALL(FILES 
            "${CMAKE_SOURCE_DIR}/third/bin/LaserLib32.dll"
            "${CMAKE_SOURCE_DIR}/third/others/AccBuf.txt"
            "${CMAKE_SOURCE_DIR}/ReleaseNotes.md"
            #"${TBB_DLL_PATH_RELEASE}"
            #"${TBBMALLOC_DLL_PATH_RELEASE}"
        DESTINATION ${CMAKE_INSTALL_PREFIX}_Release
		CONFIGURATIONS Release)

INSTALL(FILES 
	"${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_zh_CN.qm"
	DESTINATION ${CMAKE_INSTALL_PREFIX}_Release/translations
	CONFIGURATIONS Release)

foreach(_lib IN LISTS OpenCV_LIBS)
	#get_target_property(${_lib}_location_Debug ${_lib} IMPORTED_LOCATION_DEBUG)
	get_target_property(${_lib}_location_Release ${_lib} IMPORTED_LOCATION_RELEASE)
	#get_target_property(${_lib}_location_RelWithDebInfo ${_lib} IMPORTED_LOCATION_RELWITHDEUBINFO)
	INSTALL(FILES 
		${${_lib}_location_Release}
		DESTINATION ${CMAKE_INSTALL_PREFIX}_Release
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

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)
install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}_Release
	CONFIGURATIONS Release)

get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
INSTALL(CODE "
	execute_process(
		COMMAND \"${QT_BIN_DIR}/windeployqt.exe\"
			--dir ${CMAKE_INSTALL_PREFIX}_Release
			--verbose 1
			--release
			--no-translations
			${CMAKE_INSTALL_PREFIX}_Release/${PROJECT_NAME}.exe)"
	CONFIGURATIONS Release)

set(BASE_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}_Release")

INSTALL(SCRIPT "cmake/Compress.cmake"
    CONFIGURATIONS Release)