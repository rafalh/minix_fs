cmake_minimum_required (VERSION 3.1)

project (MinixFS)

if (UNIX OR MINGW)
	add_compile_options(-Wall -Wno-sign-compare)
elseif (MSVC)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
		_CRT_SECURE_NO_WARNINGS)
	add_compile_options(/W3)
endif()

file(GLOB SOURCES *.c)
file(GLOB HEADERS *.h)

add_executable(MinixFS ${SOURCES} ${HEADERS})

target_link_libraries(MinixFS ${CMAKE_SOURCE_DIR}/dokan.lib)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinixFS)
