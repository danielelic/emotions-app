# --------------
# CMake file emotions-app
# --------------

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(subProject emotions-app)

PROJECT(${subProject})

file(GLOB SRCS *.c*)
file(GLOB HDRS *.h*)

if( ${CMAKE_VERSION} VERSION_GREATER 2.8.11 )
    get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)  # PATH was updated to DIRECTORY in 2.8.12
else()
    get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} PATH)
endif()
set(COMMON_HDRS "${PARENT_DIR}/common/")
file(GLOB COMMON_HDRS_FILES ${COMMON_HDRS}/*.h*)

add_executable(${subProject} ${SRCS} ${HDRS} ${COMMON_HDRS_FILES})

target_include_directories(${subProject} PRIVATE ${Boost_INCLUDE_DIRS} ${AFFDEX_INCLUDE_DIR} ${COMMON_HDRS})

target_link_libraries( ${subProject} ${AFFDEX_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${CURL_LIBRARIES})

#Add to the apps list
list( APPEND ${rootProject}_APPS ${subProject} )
set( ${rootProject}_APPS ${${rootProject}_APPS} PARENT_SCOPE )

# Installation steps
install( TARGETS ${subProject}
        RUNTIME DESTINATION ${RUNTIME_INSTALL_DIRECTORY} )
