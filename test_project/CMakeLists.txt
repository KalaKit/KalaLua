cmake_minimum_required(VERSION 3.29.2)

set(PROGRAM_VERSION "test-project")
set(PROGRAM_VERSION_NUMBER 1.0.0.0)

project("test-project" VERSION ${PROGRAM_VERSION_NUMBER} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Runtime lib handling for MSVC
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Platform Detection
if (WIN32)
    message(STATUS "[TEST-PROJECT] Platform = Windows")
else()
    message(FATAL_ERROR "[TEST-PROJECT] Unsupported platform. Only Windows is supported.")
endif()

# Build Type Detection
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IS_DEBUG TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(IS_RELEASE TRUE)
else()
    message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'! Must be Debug, Release, or RelWithDebInfo.")
endif()

# Paths
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(EXT_SHARED_DIR "${CMAKE_SOURCE_DIR}/_external_shared")
set(EXT_KALALUA_DIR "${CMAKE_SOURCE_DIR}/_external_shared/KalaLua")

# Library Paths
if(IS_RELEASE)
    set(KALALUA_LIBRARY_PATH "${EXT_KALALUA_DIR}/release/KalaLua1.lib")
else()
    set(KALALUA_LIBRARY_PATH "${EXT_KALALUA_DIR}/debug/KalaLua1d.lib")
endif()

# Source Files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*/*.cpp"
)

# Executable
add_executable(test-project ${SOURCE_FILES})

# Enable C++ exception handling semantics for MSVC
if (MSVC)
    target_compile_options(test-project PRIVATE /EHsc)
endif()

# Set output name with renderer suffix and debug postfix
set_target_properties(test-project PROPERTIES
	OUTPUT_NAME "test-project"
)

# Includes
file(GLOB_RECURSE HEADERS
	configure_depends
	"${CMAKE_SOURCE_DIR}/include/*.hpp"
)
target_sources(test-project PRIVATE ${HEADERS})
target_include_directories(test-project PRIVATE
	"${INCLUDE_DIR}"
	"${EXT_SHARED_DIR}"
	"${EXT_SHARED_DIR}/KalaLua/include"
	"${EXT_SHARED_DIR}/lua/src"
)

# Preprocessor Defines
target_compile_definitions(test-project PRIVATE 
	LIB_EXPORT
    WIN32_LEAN_AND_MEAN
    NOMINMAX
	UNICODE
	_UNICODE
)

# Link libraries
target_link_libraries(test-project PRIVATE
	${KALALUA_LIBRARY_PATH}
)

# Expose version string
add_compile_definitions(
	PROGRAM_VERSION="${PROGRAM_VERSION}"
)

# Copy external binaries
if(IS_RELEASE)
	set(BIN_KALALUA "${EXT_KALALUA_DIR}/release")
else()
	set(BIN_KALALUA "${EXT_KALALUA_DIR}/debug")
endif()

if (IS_RELEASE)
	file(GLOB BIN_FILES
		"${BIN_KALALUA}/*.dll"
	)
else()
	file(GLOB BIN_FILES
		"${BIN_KALALUA}/*.dll"
		"${BIN_KALALUA}/*.pdb"
	)
endif()

set(BIN_TARGET_DIR "$<TARGET_FILE_DIR:test-project>")

foreach(BIN_FILE ${BIN_FILES})
    get_filename_component(BIN_NAME ${BIN_FILE} NAME)
    add_custom_command(TARGET test-project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
			"${BIN_FILE}"
			"${BIN_TARGET_DIR}/${BIN_NAME}"
    )
endforeach()

# Copy important files
set(COPY_TARGETS README.md LICENSE.md)
foreach(FILE ${COPY_TARGETS})
add_custom_command(TARGET test-project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f 
		"$<TARGET_FILE_DIR:test-project>/${FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_SOURCE_DIR}/${FILE}"
		"$<TARGET_FILE_DIR:test-project>/${FILE}"
)
endforeach()

# Copy entire docs folder
add_custom_command(TARGET test-project POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rm -rf
		"$<TARGET_FILE_DIR:test-project>/docs"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/docs"
		"$<TARGET_FILE_DIR:test-project>/docs"
)

# Package
include(CPack)