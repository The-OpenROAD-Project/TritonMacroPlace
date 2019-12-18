cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1 -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -D_GLIBCXX_USE_CXX11_ABI=1")

set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(OPENSTA_HOME ${PROJECT_SOURCE_DIR}/src/OpenSTA )
set(EIGEN_HOME ${PROJECT_SOURCE_DIR}/src/eigen )
set(OPENDB_HOME ${PROJECT_SOURCE_DIR}/src/OpenDB )

set(ABKCOMMON_HOME ${MPLACE_HOME}/module/ABKCommon )
set(PARQUET_HOME ${MPLACE_HOME}/module/ParquetFP )

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

set (MPLACE_SRC 
  src/circuit.cpp
  src/main.cpp
  src/timingSta.cpp
  src/partition.cpp
  src/parse.cpp
  src/graph.cpp
  src/macro.cpp
  src/macroPlace_wrap.cpp
  src/tritonmp_external.cpp
  src/macroPlace.cpp
  src/MakeTritonMp.cpp
)


set (MPLACE_HEADER
)

############################################################
# include directories 
############################################################

############################################################
# Create a library 
############################################################

add_custom_command(OUTPUT ${MPLACE_HOME}/src/macroPlace_wrap.cpp
  COMMAND ${SWIG_EXECUTABLE} -tcl8 -c++ -o ${MPLACE_HOME}/src/macroPlace_wrap.cpp -I${MPLACE_HOME}/include ${MPLACE_HOME}/src/macroPlace.i   
  WORKING_DIRECTORY ${MPLACE_HOME}
  DEPENDS ${MPLACE_HOME}/src/macroPlace.i "src/tritonmp_external.cpp" "include/tritonmp/tritonmp_external.h"
)

############################################################
# Create an executable
############################################################
add_library( TritonMacroPlace 
  ${MPLACE_SRC}
)

# add_dependencies( TritonMacroPlace sta )

target_include_directories( TritonMacroPlace 
PUBLIC
  ${MPLACE_HOME}/src
  ${MPLACE_HOME}/include

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

  ${OPENDB_HOME}/include
  )

############################################################
# Install  
############################################################

install(FILES ${MPLACE_HEADER} DESTINATION include)
install(TARGETS TritonMacroPlace DESTINATION bin)


