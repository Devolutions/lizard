
set(LZMA_DIR "${CMAKE_SOURCE_DIR}/src/lzma")

include_directories(${LZMA_DIR})

set(${MODULE_PREFIX}_SOURCES
	lizard.c)

add_executable(lizard-app lizard.c)

target_link_libraries(lizard-app lizard)

set_target_properties(lizard-app PROPERTIES OUTPUT_NAME "lizard")
