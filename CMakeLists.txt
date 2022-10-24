cmake_minimum_required( VERSION 3.4 )
project( JSON VERSION 0.27.54 )

set( JSON_BUILD_EXAMPLES TRUE CACHE BOOL "" )

set( JSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/ ) 
set( JSON_SOURCE_DIR ${PROJECT_SOURCE_DIR}/source/ ) 
set( JSON_HEADER_FILES 
	"${JSON_INCLUDE_DIR}json.hpp"
	"${JSON_INCLUDE_DIR}json_serial.hpp"
)
set( JSON_SOURCE_FILES 
	"${JSON_SOURCE_DIR}json.cpp"
)

add_library( json ${JSON_HEADER_FILES} ${JSON_SOURCE_FILES} )
target_include_directories( json PUBLIC ${JSON_INCLUDE_DIR} )

if( JSON_BUILD_EXAMPLES )
	add_subdirectory( examples "json_examples" )
endif()

