# ParquetFP CMAKE Compile Settings
project(ParquetFP
  LANGUAGES CXX 
)

cmake_minimum_required(VERSION 2.6)

set(PARQUET_HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(ABKCOMMON_HOME ${CMAKE_CURRENT_SOURCE_DIR}/../ABKCommon)

message(STATUS "ParquetFP_home: ${PARQUET_HOME}")

############################################################
# Create a library for PARQUET
############################################################
set( PARQUET_SRC
  src/AnalytSolve.cxx
  src/Annealer.cxx
  src/baseannealer.cxx
  src/basepacking.cxx
  src/btreeanneal.cxx
  src/btreecompact.cxx
#  src/btreecompactsstree.cxx
  src/btree.cxx
#  src/btreefromsstree.cxx
  src/btreeslackeval.cxx
  src/ClusterDB.cxx
  src/CommandLine.cxx
  src/DB.cxx
#  src/debug.cxx
  src/FPcommon.cxx
  #  src/FPEvalTest1.cxx
  #  src/FPEvalTest2.cxx
  #  src/FPEvalTest3.cxx
  #  src/FPEvalTest4.cxx
  src/mixedpacking.cxx
  src/mixedpackingfromdb.cxx
  src/Net.cxx
  src/netlist.cxx
  src/Nets.cxx
  src/Node.cxx
  src/Nodes.cxx
  src/Parquet.cxx
#   src/pinCache.cxx
  src/plcompact.cxx
  src/plsptobtree.cxx
  src/pltobtree.cxx
  src/PlToSP.cxx
  src/SeqPair.cxx
  src/skyline.cxx
  src/SolveMulti.cxx
  src/SPeval.cxx
)

set ( PARQUET_HEADER
  src/allparquet.h
  src/AnalytSolve.h
  src/Annealer.h
  src/baseannealer.h
  src/basepacking.h
  src/btreeanneal.h
  src/btreecompact.h
  src/btreecompactsstree.h
  src/btreefromsstree.h
  src/btree.h
  src/btreeslackeval.h
  src/ClusterDB.h
  src/CommandLine.h
  src/DB.h
  src/debugflags.h
  src/debug.h
  src/FPcommon.h
  src/mixedpackingfromdb.h
  src/mixedpacking.h
  src/Net.h
  src/netlist.h
  src/Nets.h
  src/Node.h
  src/Nodes.h
  src/Parquet.h
  src/parsers.h
  src/pinCache.h
  src/plcompact.h
  src/plsptobtree.h
  src/pltobtree.h
  src/PlToSP.h
  src/SeqPair.h
  src/skyline.h
  src/SolveMulti.h
  src/SPeval.h
)

add_library( ParquetFP STATIC ${PARQUET_SRC} )
add_executable( ParquetFPexe 
  ${PARQUET_SRC} 
  )


set_target_properties( ParquetFP
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PARQUET_HOME}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PARQUET_HOME}/lib
  RUNTIME_OUTPUT_DIRECTORY ${PARQUET_HOME}/lib
)

# include directory settings
target_include_directories( ParquetFP
  PUBLIC
  ${ABKCOMMON_HOME}
  ${PARQUET_HOME}
  src/
  )

target_include_directories( ParquetFPexe
  PUBLIC
  ${ABKCOMMON_HOME}
  ${PARQUET_HOME}
  src/
  )

target_link_libraries( ParquetFPexe 
  ${ABKCOMMON_HOME}/lib/libABKCommon.a 
  )

# add_dependencies( cParquetFPzlib cParquetFP )

######################################################
# install

install(FILES ${PARQUET_HEADERS} DESTINATION include)
install(TARGETS ParquetFP DESTINATION lib)

