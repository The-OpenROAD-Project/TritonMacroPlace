cmake_minimum_required(VERSION 3.1)

project(TritonFPlan)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(FPLAN_HOME ${PROJECT_SOURCE_DIR} )
set(DEFLIB_HOME
  ${PROJECT_SOURCE_DIR}/module/def
)
set(LEFLIB_HOME
  ${PROJECT_SOURCE_DIR}/module/lef
)
set(OPENSTA_HOME
  ${PROJECT_SOURCE_DIR}/module/OpenSTA
)
set(EIGEN_HOME
  ${PROJECT_SOURCE_DIR}/module/eigen-git-mirror 
)
set(ABKCOMMON_HOME
  ${PROJECT_SOURCE_DIR}/module/ABKCommon 
)
set( PARQUET_HOME
  ${PROJECT_SOURCE_DIR}/module/ParquetFP
)

add_subdirectory(${DEFLIB_HOME})
add_subdirectory(${LEFLIB_HOME})
add_subdirectory(${OPENSTA_HOME})
add_subdirectory(${ABKCOMMON_HOME})
add_subdirectory(${PARQUET_HOME})

set(ENV{TYPE_CALL} library)


## configure settings
find_package(TCL REQUIRED)

find_package(ZLIB REQUIRED)
find_package(SWIG REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# for cimg
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

set (FPLAN_SRC 
  src/circuit.cpp
  src/defParser.cpp
  src/lefParser.cpp
  src/main.cpp
  src/parquet.cpp
  src/timingSta.cpp
  src/partition.cpp
)


set (FPLAN_HEADER
  src/lefdefIO.h
  src/parse.h
  src/plot.h
  src/timingSta.h
)

############################################################
# include directories 
############################################################
include_directories(
  ${BOOST_INCLUDEDIR}

  ${EIGEN_HOME}
  ${ABKCOMMON_HOME}
  ${PARQUET_HOME}/src

  ${DEFLIB_HOME}/def
  ${DEFLIB_HOME}/defzlib
  ${DEFLIB_HOME}/cdef
  ${DEFLIB_HOME}/cdefzlib

  ${LEFLIB_HOME}/lef
  ${LEFLIB_HOME}/lefzlib
  ${LEFLIB_HOME}/clef
  ${LEFLIB_HOME}/clefzlib

  ${OPENSTA_HOME}/app
  ${OPENSTA_HOME}/dcalc
  ${OPENSTA_HOME}/graph
  ${OPENSTA_HOME}/liberty
  ${OPENSTA_HOME}/network
  ${OPENSTA_HOME}/parasitics
  ${OPENSTA_HOME}/sdc
  ${OPENSTA_HOME}/sdf
  ${OPENSTA_HOME}/search
  ${OPENSTA_HOME}/tcl
  ${OPENSTA_HOME}/util
  ${OPENSTA_HOME}/verilog

  )

############################################################
# Create a library 
############################################################
add_library( fplanlib STATIC
  ${FPLAN_SRC}
)

set_target_properties( fplanlib 
  PROPERTIES OUTPUT_NAME "TritonFPlan"
)

############################################################
# Create an executable
############################################################
add_executable( fplan
  ${FPLAN_SRC}
)

add_dependencies( fplan sta )
add_dependencies( fplan def )
add_dependencies( fplan lef )

add_dependencies( fplanlib sta )
add_dependencies( fplanlib def )
add_dependencies( fplanlib lef )

target_link_libraries( fplan
  PUBLIC
  OpenSTA
  def
  defzlib
  cdef
  cdefzlib
  lef
  lefzlib
  clef
  clefzlib

  ParquetFP
  ABKCommon

  ${ZLIB_LIBRARIES}
  ${TCL_LIB}
  Threads::Threads
  -static-libstdc++ 
  )

############################################################
# Install  
############################################################

install(FILES ${FPLAN_HEADER} DESTINATION include)
install(TARGETS fplan DESTINATION bin)
install(TARGETS fplanlib DESTINATION lib)


