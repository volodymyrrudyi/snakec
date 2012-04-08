
cmake_minimum_required (VERSION 2.6) 
project (snakec) 

add_subdirectory (src) 
include_directories ("${PROJECT_SOURCE_DIR}/../snaked/include")
add_executable(snakec ${SNAKEC_SOURCES} ${SNAKEC_HEADERS})
target_link_libraries(snakec snakecore event)
