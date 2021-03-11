###############################################################################
# Find PolyClipping
#
# This sets the following variables:
# FLANN_FOUND - True if FLANN was found.
# FLANN_INCLUDE_DIRS - Directories containing the FLANN include files.
# FLANN_LIBRARIES - Libraries needed to use FLANN.
# FLANN_DEFINITIONS - Compiler flags for FLANN.

set(PolyClipping_DIR "" CACHE PATH "")

message(PolyClipping_DIR=${PolyClipping_DIR})
find_path(PolyClipping_INCLUDE_DIR polyclipping/clipper.hpp HINTS ${PolyClipping_DIR}/include)
message(PolyClipping_INCLUDE_DIR=${PolyClipping_INCLUDE_DIR})

find_library(PolyClipping_LIBRARY_RELEASE polyclipping HINTS ${PolyClipping_DIR}/lib)
find_library(PolyClipping_LIBRARY_DEBUG polyclippingd HINTS ${PolyClipping_DIR}/lib)
message(PolyClipping_LIBRARY_RELEASE=${PolyClipping_LIBRARY_RELEASE})
message(PolyClipping_LIBRARY_DEBUG=${PolyClipping_LIBRARY_DEBUG})

set(PolyClipping_INCLUDE_DIRS ${PolyClipping_INCLUDE_DIR})
#set(PolyClipping_LIBRARIES Optimized ${PolyClipping_LIBRARY_RELEASE}
#						   Debug ${PolyClipping_LIBRARY_DEBUG})
						   
add_library(PolyClipping STATIC IMPORTED)
set_target_properties(PolyClipping PROPERTIES IMPORTED_LOCATION_DEBUG ${PolyClipping_LIBRARY_DEBUG})
set_target_properties(PolyClipping PROPERTIES IMPORTED_LOCATION_RELEASE ${PolyClipping_LIBRARY_RELEASE})
						   
message(PolyClipping=${PolyClipping})

mark_as_advanced(PolyClipping_LIBRARY_DEBUG PolyClipping_LIBRARY_RELEASE PolyClipping_INCLUDE_DIR)

