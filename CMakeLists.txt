cmake_minimum_required(VERSION 3.0)
project(ArgonA)

# use Radon CMake framework
include("Integrate.cmake")

# integrate the Radon framework project
rcf_addlocation(RADONFRAMEWORK "git" "https://github.com/tak2004/RadonFramework.git")
Integrate(RADONFRAMEWORK "Radon framework" "RADONFRAMEWORK_HOME")

# put the libraries to the dist directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../dist)

# allow to compile as shared or static library
OPTION(ARGONA_BUILD_SHARED_LIBS "Build ArgonA as shared library" OFF)

set(srclist "code/library/VirtualArchive.cpp")
set(hdrlist "code/library/include/ArgonA/VirtualArchive.hpp")
source_group("Include" FILES ${hdrlist})
source_group("Source" FILES ${srclist})
set(ARGONA_FILES ${srclist} ${hdrlist})
Generate(MODULE ARGONA ArgonA "Libraries")
AddDependency(ARGONA RadonFramework)
AddPublicInclude(ARGONA ${ArgonA_SOURCE_DIR}/code/library/include)
Finalize(ARGONA)

if(${ARGONA_BUILD_SHARED_LIBS})
set(srclist "code/library/ArgonA.cpp")
set(hdrlist "code/library/include/ArgonA/ArgonA.h")
source_group("Include" FILES ${hdrlist})
source_group("Source" FILES ${srclist})
set(LIBARGONA_FILES ${srclist} ${hdrlist})
Generate(SHARED LIBARGONA libArgonA "Libraries")
AddDependency(LIBARGONA ArgonA)
Finalize(LIBARGONA)
endif()