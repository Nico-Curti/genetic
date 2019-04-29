cmake_minimum_required (VERSION 3.8.2)
project (Genetic LANGUAGES CXX VERSION 1.0.0 DESCRIPTION "Genetic algorithm in parallel and distributed computing")
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Genetic Version
set (GENETIC_MAJOR    1)
set (GENETIC_MINOR    0)
set (GENETIC_REVISION 0)
add_definitions (-DMAJOR=${GENETIC_MAJOR} -DMINOR=${GENETIC_MINOR} -DREVISION=${GENETIC_REVISION})

#################################################################
#                         COMPILE OPTIONS                       #
#################################################################

option (OMP     "Enable OpenMP                support" ON  )
option (MPI     "Enable MPI                   support" ON  )
option (VERBOSE "Enable VERBOSE stdout        support" ON  )

set(CMAKE_VERBOSE_MAKEFILE "FALSE" CACHE BOOL "Create verbose makefile")
set(BUILD_SHARED_LIBS "TRUE" CACHE BOOL "Create byronlib as a shared library")

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  message(STATUS "VCPKG found: $ENV{VCPKG_ROOT}")
  message(STATUS "Using VCPKG integration")
endif()

if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(USE_INTEGRATED_LIBS "TRUE" CACHE BOOL "Use libs distributed with this repo")
else()
  set(USE_INTEGRATED_LIBS "FALSE" CACHE BOOL "Use libs distributed with this repo")
endif()

#################################################################
#                         SETTING VARIABLES                     #
#################################################################

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})

if ( NOT APPLE )
  set (CMAKE_SKIP_BUILD_RPATH             FALSE )
  set (CMAKE_BUILD_WITH_INSTALL_RPATH     FALSE )
  set (CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE  )
endif()

# make sure that the default is a RELEASE
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if ( CMAKE_COMPILER_IS_GNUCXX )
  if ( OMP )
    add_compile_options (-Wall -Wextra -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -Wno-narrowing -Wpedantic)
  endif()
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string (REGEX REPLACE "-O[0123]" "-Og" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG" )
    set (CMAKE_CXX_FLAGS_RELEASE "")
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    string (REGEX REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )
    set (CMAKE_CXX_FLAGS_DEBUG "")
  endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_compile_options(-Wno-deprecated -Wno-writable-strings -stdlib=libstdc++ -std=c++1z)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string (REGEX REPLACE "-O0" "-Og" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG" )
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    string (REGEX REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )
  endif()
endif()

if (MSVC)
  set (CMAKE_CXX_FLAGS "/wd4013 /wd4018 /wd4028 /wd4047 /wd4068 /wd4090 /wd4101 /wd4113 /wd4133 /wd4190 /wd4244 /wd4267 /wd4305 /wd4477 /wd4996 /wd4819 /fp:fast ${CMAKE_CXX_FLAGS}")
  string (REGEX REPLACE "/O2" "/Ox" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  add_definitions (-D_CRT_RAND_S)
  add_definitions (-DNOMINMAX)
  add_definitions (-D_USE_MATH_DEFINES)
  add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
  set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if (COMPILER_SUPPORTS_MARCH_NATIVE AND NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

#################################################################
#                         PARSE OPTIONS                         #
#################################################################

if (OMP)
  find_package(OpenMP REQUIRED)
  if (OPENMP_FOUND)
    message(STATUS "OpenMP found")
    if (OpenMP_CXX_VERSION_MAJOR LESS 4)
      message(FATAL_ERROR " Your OpenMP is too old. Required OpenMP 4.0. Please upgrade.")
    endif()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    if (APPLE)
      list(APPEND linked_libs OpenMP::OpenMP_CXX)
    endif()
  endif()
else()
  message(STATUS "OpenMP disabled")
endif()

if (MPI)
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})

  find_package(Boost 1.4 COMPONENTS mpi serialization REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(STATUS "MPI disabled")
endif()

if (VERBOSE)
  message (STATUS "Verbose state: ON")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERBOSE") # printable
  #add_definitions (-DVERBOSE)
else()
  message (STATUS "Verbose disabled")
endif()

#################################################################
#                         SETTING DIRECTORIES                   #
#################################################################

set(SRC_DIR    ./src)
set(INC_DIR    ./include)
set(EXAMPLE    ./example)
set(OUT_DIR    ./bin)

set(geneticlib genetic)

file(GLOB_RECURSE SRC    "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADER "${INC_DIR}/*.h"  )

include_directories(${INC_DIR})

#################################################################
#                          SUMMARY                              #
#################################################################

message(STATUS ""                                                                          )
message(STATUS "=================== Genetic configuration Summary =================="      )
message(STATUS "   Genetic version: ${GENETIC_MAJOR}.${GENETIC_MINOR}.${GENETIC_REVISION}" )
message(STATUS ""                                                                          )
message(STATUS "   Build type : ${CMAKE_BUILD_TYPE}"                                       )
message(STATUS "   C++ :"                                                                  )
message(STATUS "      C++ Compiler : ${CMAKE_CXX_COMPILER}"                                )
message(STATUS "      C++ flags    :"                                                      )
foreach(FLAG ${CMAKE_CXX_FLAGS_LIST})
  message(STATUS "                    * ${FLAG}"                                           )
endforeach(FLAG)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "      C++ Debug flags:"                                                  )
  foreach(FLAG ${CMAKE_CXX_FLAGS_DEBUG})
    message(STATUS "                    * ${FLAG}"                                         )
  endforeach(FLAG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "      C++ Release flags  :"                                              )
  foreach(FLAG ${CMAKE_CXX_FLAGS_RELEASE})
    message(STATUS "                    * ${FLAG}"                                         )
  endforeach(FLAG)
endif()
message(STATUS "      Linker flags : "                                                     )
foreach(FLAG ${linked_libs})
  message(STATUS "                    * ${FLAG}"                                           )
endforeach(FLAG)
message(STATUS ""                                                                          )
message(STATUS "   OpenMP  support : ${OMP}"                                               )
message(STATUS "   MPI     support : ${MPI}"                                               )
message(STATUS "   VERBOSE support : ${VERBOSE}"                                           )
message(STATUS ""                                                                          )

#################################################################
#                         MAIN RULES                            #
#################################################################

add_library(${geneticlib} SHARED ${SRC} ${HEADER})
set_property(TARGET ${geneticlib} PROPERTY POSITION_INDEPENDENT_CODE ON)
if (MPI)
  target_link_libraries(${geneticlib} PRIVATE ${linked_libs} ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_compile_definitions(${geneticlib} PUBLIC -D_MPI)
endif()
target_link_libraries(${geneticlib} ${linked_libs})

# omp
add_custom_target(gen_omp)
add_executable(omp_gen ${EXAMPLE}/omp_gen.cpp)
target_link_libraries(omp_gen ${linked_libs} ${geneticlib})
if (MPI)
  target_link_libraries(omp_gen ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES}) # necessary for only one lib!
endif()
add_dependencies(gen_omp omp_gen)

if (MPI)
# mpi
add_custom_target(gen_mpi)
add_executable(mpi_gen ${EXAMPLE}/boost_mpi_gen.cpp)
target_link_libraries(mpi_gen ${linked_libs} ${geneticlib} ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_compile_definitions(mpi_gen PUBLIC -D_MPI)
add_dependencies(gen_mpi mpi_gen)
endif()

#################################################################
#                          INSTALLERS                           #
#################################################################

install(TARGETS omp_gen    DESTINATION ${CMAKE_SOURCE_DIR}/${OUT_DIR})
if (MPI)
  install(TARGETS mpi_gen  DESTINATION ${CMAKE_SOURCE_DIR}/${OUT_DIR})
endif()
