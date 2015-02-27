# Look for JSONCPP, use our own if not found

mark_as_advanced(JSON_LIBRARY JSON_INCLUDE_DIR)

find_path(JSON_INCLUDE_DIR json/features.h)
find_library(JSON_LIBRARY NAMES jsoncpp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSONCPP DEFAULT_MSG JSON_LIBRARY JSON_INCLUDE_DIR)

if(JSON_INCLUDE_DIR AND JSON_LIBRARY)
	message(STATUS "Found system JSONCPP header file in ${JSON_INCLUDE_DIR}")
	message(STATUS "Found system JSONCPP library ${JSON_LIBRARY}")
else()
	message(STATUS "Using bundled JSONCPP library.")
	set(JSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/json)
	set(JSON_LIBRARY jsoncpp)
	add_subdirectory(json)
endif()

