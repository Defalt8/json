# examples

link_libraries( json )
add_compile_definitions( RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/" )

add_executable( vector2f_serial vector2f_serial.cpp )
add_executable( player_serial player_serial.cpp )
