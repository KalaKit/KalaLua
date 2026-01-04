cmake_minimum_required(VERSION 3.29.2)

set(PROGRAM_VERSION "KalaLua")
set(PROGRAM_VERSION_NUMBER 1.0.0.0)

project("KalaLua" VERSION ${PROGRAM_VERSION_NUMBER} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Runtime lib handling for MSVC
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Platform Detection
if (WIN32)
    message(STATUS "[KALALUA] Platform = Windows")
else()
    message(FATAL_ERROR "[KALALUA] Unsupported platform. Only Windows is supported.")
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
set(EXT_LUA_DIR "${CMAKE_SOURCE_DIR}/_external_shared/lua")

# Library Paths
if(IS_RELEASE)
    set(LUA_LIBRARY_PATH "${EXT_LUA_DIR}/release/lua5_5.lib")
else()
    set(LUA_LIBRARY_PATH "${EXT_LUA_DIR}/debug/lua5_5d.lib")
endif()

# Source Files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*/*.cpp"
)

# SHARED LIBRARY (.dll)
add_library(KalaLua SHARED ${SOURCE_FILES})

# Enable C++ exception handling semantics for MSVC
if (MSVC)
    target_compile_options(KalaLua PRIVATE /EHsc)
endif()
	
# Get suffix
if (IS_RELEASE)
	set(KALA_SUFFIX "")
else()
	set(KALA_SUFFIX "d")
endif()

# Set output name with renderer suffix and debug postfix
set_target_properties(KalaLua PROPERTIES
	OUTPUT_NAME "KalaLua1${KALA_SUFFIX}"
)

# Includes
file(GLOB_RECURSE HEADERS
	configure_depends
	"${CMAKE_SOURCE_DIR}/include/*.hpp"
)
target_sources(KalaLua PRIVATE ${HEADERS})
target_include_directories(KalaLua PRIVATE
	"${INCLUDE_DIR}"
	"${EXT_SHARED_DIR}"
	"${EXT_SHARED_DIR}/lua/src"
)

# Preprocessor Defines
target_compile_definitions(KalaLua PRIVATE 
	LIB_EXPORT          # dll import/export
	WIN32_LEAN_AND_MEAN # cleaner windows.h
	NOMINMAX            # skips min/max windows macros in favor of the std variants
	UNICODE             # selects wide api variants over ansi for UTF16 windows functions
	_UNICODE            # uses wide text types in the C runtime layer
)

# Link libraries
target_link_libraries(KalaLua PRIVATE
	${LUA_LIBRARY_PATH}
)

# Expose version string
add_compile_definitions(
	PROGRAM_VERSION="${PROGRAM_VERSION}"
)

# Copy external binaries
if(IS_RELEASE)
	set(BIN_LUA "${EXT_LUA_DIR}/release")
else()
	set(BIN_LUA "${EXT_LUA_DIR}/debug")
endif()

if (IS_RELEASE)
	file(GLOB BIN_FILES
		"${BIN_LUA}/*.dll"
	)
else()
	file(GLOB BIN_FILES
		"${BIN_LUA}/*.dll"
		"${BIN_LUA}/*.pdb"
	)
endif()

set(BIN_TARGET_DIR "$<TARGET_FILE_DIR:KalaLua>")

foreach(BIN_FILE ${BIN_FILES})
    get_filename_component(BIN_NAME ${BIN_FILE} NAME)
    add_custom_command(TARGET KalaLua POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
			"${BIN_FILE}"
			"${BIN_TARGET_DIR}/${BIN_NAME}"
    )
endforeach()

# Copy important files
set(COPY_TARGETS README.md LICENSE.md)
foreach(FILE ${COPY_TARGETS})
add_custom_command(TARGET KalaLua POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f 
		"$<TARGET_FILE_DIR:KalaLua>/${FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_SOURCE_DIR}/${FILE}"
		"$<TARGET_FILE_DIR:KalaLua>/${FILE}"
)
endforeach()

# Copy entire docs folder
add_custom_command(TARGET KalaLua POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rm -rf
		"$<TARGET_FILE_DIR:KalaLua>/docs"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/docs"
		"$<TARGET_FILE_DIR:KalaLua>/docs"
)