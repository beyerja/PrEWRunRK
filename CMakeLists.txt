cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)

project(PrEWRunRK) # Project name

# Find necessary external libraries
find_library( 
  PrEW PrEW 
  HINTS 
    ../PrEW/lib # On my local computer
    /afs/desy.de/group/flc/pool/beyerjac/TGCAnalysis/PrEW/lib # On the NAF
  REQUIRED 
)
find_library( 
  PrEWUtils PrEWUtils 
  HINTS 
    ../PrEWUtils/lib 
    /afs/desy.de/group/flc/pool/beyerjac/TGCAnalysis/PrEWUtils/lib
  REQUIRED 
)
find_library( 
  spdlog spdlog 
  HINTS 
    ../PrEW/external/spdlog/build 
    /afs/desy.de/group/flc/pool/beyerjac/TGCAnalysis/PrEW/external/spdlog/build 
  REQUIRED
)

# Find installation directories and set include paths accordingly
get_filename_component(PrEW_lib_dir ${PrEW} DIRECTORY)
get_filename_component(PrEWUtils_lib_dir ${PrEWUtils} DIRECTORY)
get_filename_component(spdlog_lib_dir ${spdlog} DIRECTORY)
include_directories(
  ${PrEW_lib_dir}/../source/prew/include
  ${PrEWUtils_lib_dir}/../source/include
  ${spdlog_lib_dir}/../include
)

# Find external packages
find_package( ROOT 6.16 REQUIRED ) 
include_directories( SYSTEM ${ROOT_INCLUDE_DIRS} )
link_libraries( ${ROOT_LIBRARIES} )
add_definitions( ${ROOT_DEFINITIONS} )

# Install in local folder instead of system
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

# Package name and version
set(CPACK_PACKAGE_NAME "PrEWRunRK")
set(CPACK_PACKAGE_VERSION "1.0.0")

# Name of binary
set(BINARY ${CMAKE_PROJECT_NAME})

# Create executable from source file
add_executable(${BINARY} source/main.cpp)

# Instructions for gcc on how to compile code
target_compile_options(
  ${BINARY} PUBLIC -std=c++1y 
  # Compiler warning/error flags
  -Wall -Wfloat-conversion -Wextra -Wunreachable-code -Wuninitialized 
  -pedantic-errors -Wold-style-cast -Wno-error=unused-variable
  -Wfloat-equal
  # Optimisation
  -O2
)

# Link libraries that are needed for executable to work
target_link_libraries(${BINARY} PUBLIC 
  ${PrEWUtils}
  ${PrEW}
  ${spdlog} # Logging
  ROOT::Minuit2 # Minimization
)

# Install the executable in the bin folder
install(TARGETS ${BINARY} RUNTIME DESTINATION bin)

# we don't want to split our program up into several things
set(CPACK_MONOLITHIC_INSTALL 1)

# This must be last
include(CPack)