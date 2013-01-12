cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Set build type (Release | Debug)" FORCE)

project(tinyxml2)
include(GNUInstallDirs)
#enable_testing()

################################
# Options

option(BUILD_SHARED_LIBS "Build as shared library (Yes | No)" ON)
option(TINYXML2_BUILD_TESTS "Build test executable (Yes | No)" OFF)

################################
# Set library versions

# Read from external file
include("${CMAKE_CURRENT_SOURCE_DIR}/config/version.cmake")

################################
# Add common source 

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/.")

################################
# Add definitions

if(NOT BUILD_SHARED_LIBS)
	add_definitions(-DTINYXML2_STATIC)
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

################################
# Add and set up targets

# Set up library
set(LIB_NAME ${CMAKE_PROJECT_NAME})
add_library(${LIB_NAME} tinyxml2.cpp tinyxml2.h)
set_target_properties(${LIB_NAME} PROPERTIES DEFINE_SYMBOL TINYXML2_EXPORTS)

if(BUILD_SHARED_LIBS)
	if(MSVC)
		set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX -d)
		set_target_properties(${LIB_NAME} PROPERTIES SUFFIX "-${VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	else()
		set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX -d)
	endif()
else()
	set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(${LIB_NAME} PROPERTIES RELEASE_POSTFIX -s)
endif()

set_target_properties(${LIB_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

install(TARGETS ${LIB_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES tinyxml2.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Set up tests executable
if(TINYXML2_BUILD_TESTS)
add_executable(tests xmltest.cpp)
add_dependencies(tests tinyxml2)
target_link_libraries(tests tinyxml2)
set_target_properties(tests PROPERTIES OUTPUT_NAME run_tests)

install(TARGETS tests
		RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/tests")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/tests")
endif()

# Write pkgconfig file
configure_file(tinyxml2.pc.in tinyxml2.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tinyxml2.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#add_test(tests ${SAMPLE_NAME} COMMAND $<TARGET_FILE:${SAMPLE_NAME}>)
