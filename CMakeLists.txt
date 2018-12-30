cmake_minimum_required (VERSION 3.8.2)
project (Genetic LANGUAGES CXX VERSION 1.0.0 DESCRIPTION "Genetic algorithm in parallel and distributed computing")
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Genetic Version
set (MAJOR    1)
set (MINOR    0)
set (REVISION 0)
add_definitions (-DMAJOR=${MAJOR} -DMINOR=${MINOR} -DREVISION=${REVISION})

#################################################################
#                         COMPILE OPTIONS                       #
#################################################################

option (OMP     "Enable OpenMP                support" ON  )
option (MPI     "Enable MPI                   support" ON  )

#################################################################
#                         SETTING VARIABLES                     #
#################################################################

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})

if ( NOT APPLE )
  set (CMAKE_SKIP_BUILD_RPATH             FALSE )
  set (CMAKE_BUILD_WITH_INSTALL_RPATH     FALSE )
  set (CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE  )
endif()

if ( CMAKE_COMPILER_IS_GNUCC )
  add_compile_options (-Wall -Wextra -Wno-unused-result)
  string (REGEX REPLACE "-O3" "-Ofast" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
endif()
if ( MSVC )
  add_compile_options (/wd4028)
  add_compile_options (/wd4244)
  add_compile_options (/wd4267)
  add_compile_options (/wd4305)
  add_compile_options (/wd4477)
  add_compile_options (/wd4996)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRTD")
  #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
  set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
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


#################################################################
#                         SETTING DIRECTORIES                   #
#################################################################

set(SRC_DIR    ./src)
set(INC_DIR    ./include)

file(GLOB SRC    "${SRC_DIR}/*.cpp")
file(GLOB HEADER "${INC_DIR}/*.h"  )

include_directories(${INC_DIR})

#################################################################
#                          SUMMARY                              #
#################################################################

message(STATUS ""                                                                     )
message(STATUS "=================== Genetic configuration Summary ==================" )
message(STATUS "   Genetic version: ${MAJOR}.${MINOR}.${REVISION}"                    )
message(STATUS ""                                                                     )
message(STATUS "   C++ :"                                                             )
message(STATUS "      C++ Compiler : ${CMAKE_CXX_COMPILER}"                           )
message(STATUS "      C++ flags    :"                                                 )
foreach(FLAG ${CMAKE_CXX_FLAGS})
  message(STATUS "                    * ${FLAG}"                                      )
endforeach(FLAG)
foreach(FLAG ${CMAKE_CXX_FLAGS_RELEASE})
  message(STATUS "                    * ${FLAG}"                                      )
endforeach(FLAG)
message(STATUS "      Linker flags : "                                                )
foreach(FLAG ${linked_libs})
  message(STATUS "                    * ${FLAG}"                                      )
endforeach(FLAG)
message(STATUS ""                                                                     )
message(STATUS "   OpenMP support : ${OMP}"                                           )
message(STATUS "   MPI    support : ${MPI}"                                           )
message(STATUS ""                                                                     )

#################################################################
#                         MAIN RULES                            #
#################################################################

# omp
add_custom_target(gen_omp)
add_executable(omp_gen ${SRC_DIR}/omp_gen.cpp)
target_link_libraries(omp_gen ${linked_libs})
add_dependencies(gen_omp omp_gen)

if (MPI)
# mpi
add_custom_target(gen_mpi)
add_executable(mpi_gen ${SRC_DIR}/boost_mpi_gen.cpp)
target_link_libraries(mpi_gen ${linked_libs} ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_compile_definitions(mpi_gen PUBLIC -D_MPI)
add_dependencies(gen_mpi mpi_gen)
endif()

#################################################################
#                          INSTALLERS                           #
#################################################################

install(TARGETS ${gen_omp}         DESTINATION ${CMAKE_SOURCE_DIR}/${OUT_DIR})
if (MPI)
  install(TARGETS ${gen_mpi}       DESTINATION ${CMAKE_SOURCE_DIR}/${OUT_DIR})
endif()