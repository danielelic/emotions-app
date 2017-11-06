cmake_minimum_required(VERSION 2.6)

set(rootProject emoj)
project(${rootProject})

# CMake includes
include(cmake_modules/Macros.cmake)         # Some custom macros we have writtens

# -------------------
# CMAKE - ENVIRONMENT
# --------------------
set(CXX_COMPILER_WARNINGS "-Wreturn-type" CACHE STRING "Compiler warnings to use")
set(CMAKE_VERBOSE ON CACHE BOOL "Verbose mode")
# Setup "Profile" build type
set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg")
set(CMAKE_C_FLAGS_PROFILE "-O3 -pg")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg")
set(CMAKE_MODULE_LINKER_FLAGS_PROFILE "-pg")

# Setup additional compiler warnings
status("Setting up compiler warnings")
if( MSVC )
    # Force to always compile with W4
    if( CMAKE_CXX_FLAGS MATCHES "/W[0-4]" )
        string( REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    else()
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
    endif()
elseif( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
    # Update if necessary
    set( CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} ${CXX_COMPILER_WARNINGS}" )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    status("Updating compiler to make use of C++14")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(bitness 64)   # We have a 64-bit machine
else()
    set(bitness 32)   # We have a 32-bit machine
endif()
status("Bitness detected: ${bitness}")


# Setup install locations
if( NOT RUNTIME_INSTALL_DIRECTORY )
    set( RUNTIME_INSTALL_DIRECTORY "bin" CACHE STRING "Install sub-directory of CMAKE_INSTALL_PREFIX for RUNTIME targets (binaries, and *.dll on windows)." )
endif( NOT RUNTIME_INSTALL_DIRECTORY )

# --------------------
# LOCATE DEPENDENCIES
# --------------------

# Curl
# ----------------------------------------------------------------------------

find_package(CURL)
if(CURL_FOUND)
    message(STATUS "Curl libraries found at: ${CURL_LIBRARIES}")
    message(STATUS "Curl includes found at: ${CURL_INCLUDE_DIRS}")
else()
    message(SEND_ERROR "Could not find cURL on your system")
endif(CURL_FOUND)

# OpenCV
# ----------------------------------------------------------------------------
# find_package OpenCV to get OpenCV_FOUND, OpenCV_INCLUDE_DIRS, OpenCV_LIBS, OpenCV_LINK_LIBRARIES
# ----------------------------------------------------------------------------
set( OPENCV_COMPONENTS ml highgui core imgproc objdetect )
if( DEFINED OpenCV_DIR )    # Force the user to tell us which OpenCV they want (otherwise find_package can find the wrong one, cache it and changes to OpenCV_DIR are ignored)
    find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
    if( NOT OpenCV_FOUND)
        message(SEND_ERROR "Failed to find OpenCV. Double check that \"OpenCV_DIR\" to the root build directory of OpenCV.")
    endif(NOT OpenCV_FOUND)
else( DEFINED OpenCV_DIR )
    set( OpenCV_DIR "" CACHE PATH "Root directory for opencv BUILD directory." )
    message(FATAL_ERROR "\"OpenCV_DIR\" not set. Please explicitly provide the path to the root build directory of OpenCV.")
endif( DEFINED OpenCV_DIR )

# Boost package
# ----------------------------------------------------------------------------
# BOOST_ROOT  is needed by  BoostConfig.cmake configuration file to
# look for the Boost includes / libraries:
#         Boost_FOUND, Boost_INCLUDE_DIRS, Boost_LIBRARY_DIRS, Boost_LIBRARIES,Boost_VERSION
set(Boost_USE_MULTITHREADED ON)
set( BOOST_COMPONENTS system filesystem date_time regex thread timer chrono serialization log log_setup program_options)

set( BOOST_MIN_VERSION "1.54.0" CACHE STRING "Minimum version of boost you would like to link against (e.g. C:/BOOST_1_55_0 is 1.55.0" )
status("")
if( ANDROID )
    find_host_package( Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS} )
else( ANDROID )
    find_package( Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS} )
endif()
if( NOT Boost_FOUND )
    if( NOT DEFINED BOOST_ROOT )
        set( BOOST_ROOT "" CACHE PATH "Root directory for Boost." )
    endif( NOT DEFINED BOOST_ROOT )
    message( FATAL_ERROR "Failed to find Boost (or missing components). Double check that \"BOOST_ROOT\" is properly set")
endif( NOT Boost_FOUND )



# Affdex package
# ----------------------------------------------------------------------------

set (AFFDEX_FOUND FALSE)

if( DEFINED AFFDEX_DIR ) 
   find_path(AFFDEX_INCLUDE_DIR FrameDetector.h
             HINTS "${AFFDEX_DIR}/include" )

   find_library(AFFDEX_LIBRARY NAMES affdex-native
               HINTS "${AFFDEX_DIR}/lib" )

   set(AFFDEX_INCLUDE_DIRS "${AFFDEX_INCLUDE_DIR}")
   set(AFFDEX_LIBRARIES "${AFFDEX_LIBRARY}")

   if (AFFDEX_INCLUDE_DIR AND AFFDEX_LIBRARY)
      set(AFFDEX_FOUND TRUE)
   endif (AFFDEX_INCLUDE_DIR AND AFFDEX_LIBRARY)

   set(AFFDEX_DATA_DIR "${AFFDEX_DIR}/data")


   if (NOT AFFDEX_FOUND)
       message(FATAL_ERROR "Unable to find the Affdex found")
   endif (NOT AFFDEX_FOUND)

else (DEFINED AFFDEX_DIR)
    message(FATAL_ERROR "Please define AFFDEX_DIR")
endif (DEFINED AFFDEX_DIR)


add_subdirectory(emotions-app)

# --------------------
# SUMMARY
# --------------------
status("${CMAKE_INCLUDE_DIRECTORIES}")
status( "------- SUMMARY ------- " )
status( "Boost version found   = ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} (${Boost_VERSION})" )
foreach( comp ${BOOST_COMPONENTS} )
    string( TOUPPER ${comp} COMP )
    status( "  - ${comp}" 1 THEN "${Boost_${COMP}_LIBRARY}" )
endforeach( comp )

status("")
status("Affdex")
foreach( lib ${AFFDEX_LIBRARIES} )
    status( "${lib}")
endforeach( lib )

status("")
status( "OpenCV version found   = ${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH} (${OpenCV_VERSION})" )
status( "OpenCV_LIB_DIR         = ${OpenCV_DIR}/lib" )
foreach( lib ${OpenCV_LIBRARIES} )
    foreach( comp ${OPENCV_COMPONENTS} )
        if( ${lib} MATCHES ${comp} )
            status( "  - ${comp}" 1 THEN "${lib}" )
        endif( ${lib} MATCHES ${comp} )
    endforeach( comp )
endforeach( lib )

status("")
status( "Apps identified for building:" )
foreach( app ${${rootProject}_APPS} )
    status( "   - ${app}" )
endforeach( app ${${rootProject}_APPS} )
