cmake_minimum_required(VERSION 3.1)

project(TritonFPlan)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1 -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -D_GLIBCXX_USE_CXX11_ABI=1")

set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(MPLACE_HOME ${PROJECT_SOURCE_DIR} )
set(OPENSTA_HOME
  ${PROJECT_SOURCE_DIR}/module/OpenSTA
)
set(EIGEN_HOME
  ${PROJECT_SOURCE_DIR}/module/eigen 
)
set(ABKCOMMON_HOME
  ${PROJECT_SOURCE_DIR}/module/ABKCommon 
)
set(PARQUET_HOME
  ${PROJECT_SOURCE_DIR}/module/ParquetFP
)
set(OPENDB_HOME 
  ${PROJECT_SOURCE_DIR}/module/OpenDB
)

add_subdirectory(${OPENSTA_HOME})
add_subdirectory(${ABKCOMMON_HOME})
add_subdirectory(${PARQUET_HOME})
add_subdirectory(${OPENDB_HOME})

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

set (MPLACE_SRC 
  src/circuit.cpp
  src/main.cpp
  src/timingSta.cpp
  src/partition.cpp
  src/parse.cpp
  src/graph.cpp
  src/macro.cpp
  src/mPlace_wrap.cpp
  src/mPlaceExternal.cpp
  src/mPlace.cpp
)


set (MPLACE_HEADER
)

############################################################
# include directories 
############################################################

############################################################
# Create a library 
############################################################

add_custom_command(OUTPUT ${MPLACE_HOME}/src/mPlace_wrap.cpp
  COMMAND ${SWIG_EXECUTABLE} -tcl8 -c++ -o ${MPLACE_HOME}/src/mPlace_wrap.cpp ${MPLACE_HOME}/src/mPlace.i   
  WORKING_DIRECTORY ${MPLACE_HOME}
  DEPENDS ${MPLACE_HOME}/src/mPlace.i "src/mPlaceExternal.cpp" "src/mPlaceExternal.h"
)

############################################################
# Create an executable
############################################################
add_executable( mplace
  ${MPLACE_SRC}
)

add_dependencies( mplace sta )

target_include_directories( mplace
PRIVATE
  ${BOOST_INCLUDEDIR}

  ${EIGEN_HOME}
  ${ABKCOMMON_HOME}
  ${PARQUET_HOME}/src

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

  ${OPENDB_HOME}/include/opendb
  ${OPENDB_HOME}/include
  )

target_link_libraries( mplace
  PUBLIC
  OpenSTA

  ParquetFP
  ABKCommon

  ${ZLIB_LIBRARIES}
  ${TCL_LIB}
  Threads::Threads
  )

############################################################
# Install  
############################################################

install(FILES ${MPLACE_HEADER} DESTINATION include)
install(TARGETS mplace DESTINATION bin)


