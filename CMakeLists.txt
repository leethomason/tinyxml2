cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)

project(tinyxml2)
include(GNUInstallDirs)
#enable_testing()

#CMAKE_BUILD_TOOL

################################
# set lib version here

set(GENERIC_LIB_VERSION "1.0.11")
set(GENERIC_LIB_SOVERSION "1")


################################
# Add common source 

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/.")

################################
# Add custom target to copy all data

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
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/empty.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/utf8test.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/utf8testverify.xml ${CMAKE_CURRENT_BINARY_DIR}/resources/)
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})

################################
# Add definitions

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

################################
# Add targets
set(BUILD_STATIC_LIBS ON CACHE BOOL "Set to ON to build static libraries")
if(BUILD_STATIC_LIBS)
	add_library(tinyxml2static STATIC tinyxml2.cpp tinyxml2.h)
	set_target_properties(tinyxml2static PROPERTIES OUTPUT_NAME tinyxml2)
endif(BUILD_STATIC_LIBS)
add_library(tinyxml2 SHARED tinyxml2.cpp tinyxml2.h)
set_target_properties(tinyxml2 PROPERTIES
	VERSION "${GENERIC_LIB_VERSION}"
	SOVERSION "${GENERIC_LIB_SOVERSION}")

add_executable(test xmltest.cpp)
add_dependencies(test tinyxml2)
add_dependencies(test ${TARGET_DATA_COPY})
target_link_libraries(test tinyxml2)


if(BUILD_STATIC_LIBS)
	install(TARGETS tinyxml2 tinyxml2static
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
else(BUILD_STATIC_LIBS)
	install(TARGETS tinyxml2
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif(BUILD_STATIC_LIBS)
install(FILES tinyxml2.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

foreach(p LIB INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
	endif()
endforeach()

configure_file(tinyxml2.pc.in tinyxml2.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tinyxml2.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#add_test(test ${SAMPLE_NAME} COMMAND $<TARGET_FILE:${SAMPLE_NAME}>)
