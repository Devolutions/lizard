cmake_minimum_required(VERSION 3.0)

project(lizard C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT DEFINED LIZARD_INSTALL)
	set(LIZARD_INSTALL ON)
endif()

include(CMakePackageConfigHelpers)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_STRINGS)
list(GET VERSION_STRINGS 0 VERSION_STRING)

string(REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+" "\\1" LIZARD_VERSION_MAJOR ${VERSION_STRING})
string(REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+" "\\1" LIZARD_VERSION_MINOR ${VERSION_STRING})
string(REGEX REPLACE "[0-9]+.[0-9]+.([0-9]+)" "\\1" LIZARD_VERSION_PATCH ${VERSION_STRING})
set(LIZARD_VERSION "${LIZARD_VERSION_MAJOR}.${LIZARD_VERSION_MINOR}.${LIZARD_VERSION_PATCH}")

if(MSVC)
	include(MSVCRuntime)

	if(NOT DEFINED MSVC_RUNTIME)
		set(MSVC_RUNTIME "static")
	endif()

	configure_msvc_runtime()
endif()

if(WIN32)
	set(C_FLAGS "")
	set(C_FLAGS "${C_FLAGS} -D_UNICODE")
	set(C_FLAGS "${C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
	set(C_FLAGS "${C_FLAGS} -DWIN32_LEAN_AND_MEAN")
	set(C_FLAGS "${C_FLAGS} -D_WINSOCK_DEPRECATED_NO_WARNINGS")
	set(C_FLAGS "${C_FLAGS} -DWINVER=0x0601 -D_WIN32_WINNT=0x0601")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS}")
endif()

set(CMAKE_EXPORT_DIR "lib/cmake/lizard")

include_directories(include)

SET(BUILD_APP ON)

if(LIZARD_FUZZING)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=fuzzer")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer")
	set(LIZARD_ASAN ON)
	set(BUILD_APP OFF)
	add_definitions(-DLIZARD_FUZZING)
endif()

if(LIZARD_ASAN)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

set(CMAKE_EXPORT_DIR "lib/cmake/lizard")

add_subdirectory(src)

if(BUILD_APP)
	add_subdirectory(app)
endif()

if(LIZARD_FUZZING)
	add_subdirectory(fuzzing)
endif()

# installation

if(LIZARD_INSTALL)
	export(PACKAGE lizard)

	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/lizard-config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/lizard-config.cmake"
		INSTALL_DESTINATION "${CMAKE_EXPORT_DIR}")

	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/lizard-config-version.cmake"
		VERSION ${LIZARD_VERSION}
		COMPATIBILITY SameMajorVersion)

	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/lizard"
		DESTINATION include
		FILES_MATCHING PATTERN "*.h")

	install(FILES
		${PROJECT_BINARY_DIR}/lizard-config.cmake
		${PROJECT_BINARY_DIR}/lizard-config-version.cmake
		DESTINATION "${CMAKE_EXPORT_DIR}")
endif()
