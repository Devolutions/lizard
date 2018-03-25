
if(NOT LIZARD_VERSION)
	cmake_minimum_required(VERSION 3.0)

	project(lizard-app C)

	find_package(lizard CONFIG REQUIRED)
endif()

set(${MODULE_PREFIX}_SOURCES
	lizard.c)

add_executable(lizard-app lizard.c)

target_link_libraries(lizard-app lizard)

set_target_properties(lizard-app PROPERTIES OUTPUT_NAME "lizard")
