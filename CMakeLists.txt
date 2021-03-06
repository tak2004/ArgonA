# Define a project if this file wasn't included.
if(NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" MATCHES "^ArgonA$")
    cmake_minimum_required(VERSION 3.0)
    project(ArgonA)
    set(RADONFRAMEWORK_USE_OPENGL OFF CACHE BOOL "" FORCE)
    set(RADONFRAMEWORK_USE_FORMS OFF CACHE BOOL "" FORCE)
    
    # put the libraries to the dist directory
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist)    
endif()

# use Radon CMake framework
include("Integrate.cmake")

# integrate the Radon framework project
rcf_addlocation(RADONFRAMEWORK "git" "https://github.com/tak2004/RadonFramework.git")
Integrate(RADONFRAMEWORK "Radon framework" "RADONFRAMEWORK_HOME")

# additional targets
OPTION(ARGONA_BUILD_SHARED_LIBS "Build ArgonA shared library" OFF)
OPTION(ARGONA_BUILD_CONSOLE "Build ArgonA console" ON)

set(srclist "code/library/VirtualArchive.cpp")
set(hdrlist "code/library/include/ArgonA/VirtualArchive.hpp")
source_group("Include" FILES ${hdrlist})
source_group("Source" FILES ${srclist})
set(ARGONA_FILES ${srclist} ${hdrlist})
Generate(MODULE ARGONA ArgonA "Libraries")
AddDependency(ARGONA RadonFramework)
AddPublicInclude(ARGONA ${CMAKE_CURRENT_SOURCE_DIR}/code/library/include)
Finalize(ARGONA)

if(${ARGONA_BUILD_SHARED_LIBS})
set(srclist "code/library/ArgonA.cpp")
set(hdrlist "code/library/include/ArgonA/ArgonA.h")
source_group("Include" FILES ${hdrlist})
source_group("Source" FILES ${srclist})
set(LIBARGONA_FILES ${srclist} ${hdrlist})
Generate(SHARED LIBARGONA libArgonA "Libraries")
AddDependency(LIBARGONA ArgonA)
AddPublicInclude(ARGONA ${CMAKE_CURRENT_SOURCE_DIR}/code/library/include)
Finalize(LIBARGONA)
endif()

if(${ARGONA_BUILD_CONSOLE})
set(srclist "code/client/main.cpp" "code/client/console.cpp")
set(hdrlist "code/client/console.hpp")
source_group("Include" FILES ${hdrlist})
source_group("Source" FILES ${srclist})
set(ARGONACONSOLE_FILES ${srclist} ${hdrlist})
Generate(EXECUTABLE ARGONACONSOLE Console "Tools")
AddDependency(ARGONACONSOLE ArgonA)
Finalize(ARGONACONSOLE)
endif()