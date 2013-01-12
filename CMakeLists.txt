cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Set build type (Release | Debug)" FORCE)

project(tinyxml2)
include(GNUInstallDirs)
#enable_testing()

################################
# Options

option(BUILD_SHARED_LIBS "Build as shared library (Yes | No)" ON)
option(TINYXML2_BUILD_TEST_EXEC "Build test executable (Yes | No)" OFF)

################################
# Library versions

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 9)

################################
# Add common source 

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/.")

################################
# Add custom target to copy all data

# this should be changed to install target
set(TARGET_DATA_COPY DATA_COPY)
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	add_custom_target(
		${TARGET_DATA_COPY}
	 	COMMAND ${CMAKE_COMMAND} -E echo "In source build")
else(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	make_directory(${CMAKE_CURRENT_BINARY_DIR}/resources/)
	add_custom_target(
		${TARGET_DATA_COPY}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/dream.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/utf8test.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/utf8testverify.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/)
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})

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
add_library(${CMAKE_PROJECT_NAME} tinyxml2.cpp tinyxml2.h)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES DEFINE_SYMBOL TINYXML2_EXPORTS)
if(BUILD_SHARED_LIBS)
	if(MSVS)
		set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)
		set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SUFFIX "-${VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	else()
		set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)
	endif()
else()
	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -s)
endif()

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

install(TARGETS ${CMAKE_PROJECT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES tinyxml2.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#Set up test executable
if(TINYXML2_BUILD_TEST_EXEC)
add_executable(test xmltest.cpp)
add_dependencies(test tinyxml2)
add_dependencies(test ${TARGET_DATA_COPY})
target_link_libraries(test tinyxml2)
endif()


foreach(p LIB INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
	endif()
endforeach()

configure_file(tinyxml2.pc.in tinyxml2.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tinyxml2.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#add_test(test ${SAMPLE_NAME} COMMAND $<TARGET_FILE:${SAMPLE_NAME}>)
